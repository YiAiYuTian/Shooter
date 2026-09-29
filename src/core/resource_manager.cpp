#include "resource_manager.h"
#include "logger.h"
#include "pak.h"

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <string>
#include <cstring>

namespace shooter
{

namespace
{

pak_t *g_pak = nullptr;

bool has_ext(const char *name, const char *ext)
{
    const char *dot = std::strrchr(name, '.');
    if (!dot) return false;
    return std::strcmp(dot + 1, ext) == 0;
}

bool is_image_ext(const char *name)
{
    return has_ext(name, "png") || has_ext(name, "jpg") || has_ext(name, "jpeg")
        || has_ext(name, "bmp") || has_ext(name, "gif") || has_ext(name, "webp")
        || has_ext(name, "tga") || has_ext(name, "avif");
}

bool is_font_ext(const char *name)
{
    return has_ext(name, "ttf") || has_ext(name, "otf") || has_ext(name, "ttc");
}

} // namespace

bool ResourceManager::init_impl()
{
    if (!TTF_Init())
    {
        LOG_FATAL("Failed to initialize SDL_ttf: {}", SDL_GetError());
        return false;
    }
    return true;
}

void ResourceManager::quit_impl()
{
    for (auto &kv : m_textures) SDL_DestroyTexture(kv.second);
    m_textures.clear();
    for (auto &kv : m_fonts) TTF_CloseFont(kv.second);
    m_fonts.clear();

    if (g_pak) { pak_close(g_pak); g_pak = nullptr; }

    TTF_Quit();
}

bool ResourceManager::load_resource_impl(std::string_view path)
{
    pak_err_t err;
    pak_t *p = pak_open_mm(std::string(path).c_str(), &err);
    if (!p)
    {
        LOG_ERROR("Failed to open resource pak '{}': {}", path, pak_strerror(err));
        return false;
    }

    for (uint32_t i = 0; i < pak_count(p); ++i)
    {
        const pak_entry_t *e = pak_entry(p, i);
        uint32_t size = 0;
        const void *data = pak_data(p, e, &size);

        if (!data)
        {
            LOG_WARN("Skip entry '{}': data not readable", e->name);
            continue;
        }

        if (is_image_ext(e->name))
        {
            if (m_textures.contains(e->name))
            {
                LOG_WARN("Duplicate texture name '{}', skip", e->name);
                continue;
            }
            SDL_IOStream *io = SDL_IOFromConstMem(data, size);
            SDL_Texture *tex = IMG_LoadTexture_IO(m_renderer, io, true);
            if (tex)
            {
                m_textures.emplace(e->name, tex);
                LOG_INFO("texture '{}' loaded ({} bytes)", e->name, size);
            }
            else
            {
                LOG_WARN("Failed to create texture '{}': {}", e->name, SDL_GetError());
            }
        }
        else if (is_font_ext(e->name))
        {
            if (m_fonts.contains(e->name))
            {
                LOG_WARN("Duplicate font name '{}', skip", e->name);
                continue;
            }
            SDL_IOStream *io = SDL_IOFromConstMem(data, size);
            TTF_Font *font = TTF_OpenFontIO(io, true, 16);
            if (font)
            {
                m_fonts.emplace(e->name, font);
                LOG_INFO("font '{}' loaded ({} bytes)", e->name, size);
            }
            else
            {
                LOG_WARN("Failed to create font '{}': {}", e->name, SDL_GetError());
            }
        }
        else
        {
            LOG_WARN("Skip unknown entry type '{}'", e->name);
        }
    }

    LOG_INFO("Loaded resource pak '{}': {} entries, {} textures, {} fonts",
             path, pak_count(p), m_textures.size(), m_fonts.size());

    g_pak = p;
    return true;
}

SDL_Texture *ResourceManager::get_texture_impl(std::string_view name)
{
    auto it = m_textures.find(std::string(name));
    if (it == m_textures.end())
    {
        LOG_WARN("Failed to find texture: {}", name);
        return nullptr;
    }

    return it->second;
}

TTF_Font *ResourceManager::get_font_impl(std::string_view name)
{
    auto it = m_fonts.find(std::string(name));
    if (it == m_fonts.end())
    {
        LOG_WARN("Failed to find font: {}", name);
        return nullptr;
    }

    return it->second;
}

}    
