#include "render_manager.h"
#include "../core/resource_manager.h"
#include "../core/logger.h"

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <string>
#include <cstring>
#include <unordered_map>

namespace shooter
{

struct TextCacheEntry
{
    SDL_Texture *tex = nullptr;
    int w = 0, h = 0;
};

static constexpr size_t TEXT_CACHE_MAX = 128;

struct RenderManager::Impl
{
    SDL_Renderer *renderer = nullptr;
    TTF_TextEngine *text_engine = nullptr;
    std::unordered_map<std::string, TextCacheEntry> text_cache;
};

bool RenderManager::init_impl(SDL_Window *window)
{
    m_impl = new RenderManager::Impl();
    m_impl->renderer = SDL_CreateRenderer(window, nullptr);

    if (!m_impl->renderer)
    {
        LOG_FATAL("Failed to initialize renderer!");
        return false;
    }

    m_impl->text_engine = TTF_CreateRendererTextEngine(m_impl->renderer);
    if (!m_impl->text_engine)
    {
        LOG_FATAL("Failed to initialize text_engine: {}", SDL_GetError());
        return false;
    }

    SDL_SetRenderDrawBlendMode(m_impl->renderer, SDL_BLENDMODE_BLEND);
    return true;
}

SDL_Renderer *RenderManager::renderer_impl()
{
    return m_impl ? m_impl->renderer : nullptr;
}

void RenderManager::quit_impl()
{
    if (m_impl)
    {
        for (auto &kv : m_impl->text_cache) SDL_DestroyTexture(kv.second.tex);
        m_impl->text_cache.clear();
        TTF_DestroyRendererTextEngine(m_impl->text_engine);
        SDL_DestroyRenderer(m_impl->renderer);
    }
    delete m_impl;
}

void RenderManager::begin_frame_impl(glm::vec4 color)
{
    SDL_Renderer *rd = m_impl->renderer;
    SDL_SetRenderDrawColorFloat(rd, color.x, color.y, color.z, color.w);
    SDL_RenderClear(rd);
}

void RenderManager::end_frame_impl()
{
    SDL_RenderPresent(m_impl->renderer);
}

void RenderManager::draw_rect_impl(glm::vec4 rect, glm::vec4 color)
{
    SDL_FRect frect = {
        rect.x,
        rect.y,
        rect.z,
        rect.w
    };

    SDL_Renderer *rd = m_impl->renderer;
    SDL_SetRenderDrawColorFloat(rd, color.x, color.y, color.z, color.w);
    SDL_RenderRect(rd, &frect);
}

void RenderManager::draw_rect_fill_impl(glm::vec4 rect, glm::vec4 color)
{
    SDL_FRect frect = {
        rect.x,
        rect.y,
        rect.z,
        rect.w
    };

    SDL_Renderer *rd = m_impl->renderer;
    SDL_SetRenderDrawColorFloat(rd, color.x, color.y, color.z, color.w);
    SDL_RenderFillRect(rd, &frect);
}

void RenderManager::draw_texture_impl(SDL_Texture *texture, glm::vec4 *src_rect, glm::vec4 dst_rect)
{
    if (!texture)
    {
        LOG_WARN("texture is null");
        return;
    }

    SDL_FRect dst_frect = {
        dst_rect.x,
        dst_rect.y,
        dst_rect.z,
        dst_rect.w
    };

    if (!src_rect) SDL_RenderTexture(m_impl->renderer, texture, nullptr, &dst_frect);
    else
    {
        SDL_FRect src_frect = {
            src_rect->x,
            src_rect->y,
            src_rect->z,
            src_rect->w
        };
        SDL_RenderTexture(m_impl->renderer, texture, &src_frect, &dst_frect);        
    }
}

void RenderManager::draw_text_impl(const char *text, TTF_Font *font, glm::ivec2 pos, float size, glm::vec4 color)
{
    if (!TTF_SetFontSize(font, size))
    {
        LOG_WARN("Failed to set font size to: {}, {}", size, SDL_GetError());
        return;
    }

    TTF_Text *ttf_text = TTF_CreateText(m_impl->text_engine, font, text, 0);
    if (!ttf_text)
    {
        LOG_WARN("Failed to create text: {}", SDL_GetError());
        return;
    }
    TTF_SetTextColorFloat(ttf_text, color.x, color.y, color.z, color.w);
    if (!TTF_DrawRendererText(ttf_text, pos.x, pos.y))
    {
        LOG_WARN("Failed to render text: {}", SDL_GetError());
    }

    TTF_DestroyText(ttf_text);
}

void RenderManager::draw_text_gradient_impl(const char *text, TTF_Font *font, glm::ivec2 pos, float size, glm::vec4 top_color, glm::vec4 bottom_color)
{
    if (!font || !text || !m_impl || !m_impl->renderer) return;

    std::string key(text);
    key.push_back('\x01');
    key.append(reinterpret_cast<const char *>(&font), sizeof(font));
    key.push_back('\x01');
    uint32_t size_bits;
    std::memcpy(&size_bits, &size, sizeof(size_bits));
    key.append(reinterpret_cast<const char *>(&size_bits), sizeof(size_bits));

    TextCacheEntry *ent = nullptr;
    auto it = m_impl->text_cache.find(key);
    if (it != m_impl->text_cache.end())
    {
        ent = &it->second;
    }
    else
    {
        if (!TTF_SetFontSize(font, size))
        {
            LOG_WARN("Failed to set font size to: {}", size);
            return;
        }
        SDL_Surface *raw = TTF_RenderText_Solid(font, text, 0, SDL_Color{ 255, 255, 255, 255 });
        if (!raw)
        {
            LOG_WARN("Failed to render text to surface: {}", SDL_GetError());
            return;
        }
        SDL_Texture *tex = SDL_CreateTextureFromSurface(m_impl->renderer, raw);
        int w = raw->w, h = raw->h;
        SDL_DestroySurface(raw);
        if (!tex)
        {
            LOG_WARN("Failed to create texture from text: {}", SDL_GetError());
            return;
        }
        SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);

        if (m_impl->text_cache.size() >= TEXT_CACHE_MAX)
        {
            for (auto &kv : m_impl->text_cache) SDL_DestroyTexture(kv.second.tex);
            m_impl->text_cache.clear();
        }
        auto r = m_impl->text_cache.emplace(std::move(key), TextCacheEntry{ tex, w, h });
        ent = &r.first->second;
    }

    const float x = (float)pos.x, y = (float)pos.y;
    const float w = (float)ent->w, h = (float)ent->h;
    SDL_Vertex verts[4] = {
        { { x,       y,     }, { top_color.x,    top_color.y,    top_color.z,    top_color.w },    { 0.0f, 0.0f } },
        { { x + w,   y,     }, { top_color.x,    top_color.y,    top_color.z,    top_color.w },    { 1.0f, 0.0f } },
        { { x,       y + h, }, { bottom_color.x, bottom_color.y, bottom_color.z, bottom_color.w }, { 0.0f, 1.0f } },
        { { x + w,   y + h, }, { bottom_color.x, bottom_color.y, bottom_color.z, bottom_color.w }, { 1.0f, 1.0f } },
    };
    const int indices[6] = { 0, 1, 2, 2, 1, 3 };
    if (!SDL_RenderGeometry(m_impl->renderer, ent->tex, verts, 4, indices, 6))
    {
        LOG_WARN("Failed to draw gradient text: {}", SDL_GetError());
    }
}

}    
