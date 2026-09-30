#include "character.h"
#include "../../render/render_manager.h"

#include <SDL3/SDL.h>

namespace shooter
{

void draw_actor(float px, float py, float w, float h, float tex_w, float tex_h, SDL_Texture *tex)
{
    RenderManager::draw_texture(tex, nullptr,
        { px - (tex_w - w) * 0.5f, py - (tex_h - h), tex_w, tex_h });
}

void draw_hp(float x, float y, float w, float hp, float max_hp)
{
    RenderManager::draw_fill_rect({ x, y, w, 6.0f }, RED);
    glm::vec4 hc = hp > max_hp * 0.6f ? GREEN : (hp > max_hp * 0.2f ? YELLOW : RED);
    RenderManager::draw_fill_rect({ x, y, w * hp / max_hp, 6.0f }, hc);
    RenderManager::draw_rect({ x, y, w, 6.0f }, WHITE);
}

void draw_gun(float x, float y, int facing, const glm::vec4 &color)
{
    float gx = x + (facing > 0 ? 18.0f : -18.0f);
    RenderManager::draw_fill_rect({ gx, y, 18.0f, 6.0f }, color);
    RenderManager::draw_rect({ gx, y, 18.0f, 6.0f }, WHITE);
}

}