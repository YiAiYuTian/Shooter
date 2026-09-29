#include "render_manager.h"
#include "../core/resource_manager.h"
#include "../core/logger.h"

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>

namespace shooter
{

struct RenderManager::Impl
{
    SDL_Renderer *renderer = nullptr;
    TTF_TextEngine *text_engine = nullptr;    
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
    if (!texture) return;

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
        LOG_WARN("Failed to set font size to: {}", size);
        return;
    }

    TTF_Text *ttf_text = TTF_CreateText(m_impl->text_engine, font, text, 0);
    TTF_SetTextColorFloat(ttf_text, color.x, color.y, color.z, color.w);
    TTF_DrawRendererText(ttf_text, pos.x, pos.y);

    TTF_DestroyText(ttf_text);
}

}    
