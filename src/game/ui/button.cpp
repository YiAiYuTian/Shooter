#include "button.h"
#include "../core/types.h"
#include "../utils.h"
#include "../../core/resource_manager.h"
#include "../../render/render_manager.h"
#include "../../event/input_manager.h"
#include "../../core/logger.h"

namespace shooter
{

Button::Button()
{
    m_handle = EventManager::subscribe<MouseButtonEvent>(
        [this](const MouseButtonEvent &e)
        {
            if (e.button != SDL_BUTTON_LEFT) return;
            glm::vec2 p = { e.x, e.y };
            if (e.down)
            {
                if (is_in_rect(p, m_world_rect)) m_is_pressed = true;
            }
            else
            {
                bool was = m_is_pressed;
                m_is_pressed = false;
                if (was && is_in_rect(p, m_world_rect) && m_callback) m_callback();
            }
        }
    );
}

Button::~Button()
{
    if (m_handle != INVALID_EVENT_HANDLE)
        EventManager::unsubscribe(m_handle);
}

void Button::set_callback(Callback cb)
{
    m_callback = std::move(cb);
}

void Button::set_hovered_callback(Callback cb)
{
    m_hovered_callback = std::move(cb);
}

void Button::on_update(float dt)
{
    (void)dt;
    if (m_is_hovered = is_in_rect(InputManager::get_mouse_pos(), m_world_rect); m_is_hovered)
    {
        if (m_hovered_callback) m_hovered_callback();
    }

    UIBase::on_update(dt);
}

void Button::on_event()
{
    UIBase::on_event();
}

void Button::on_render()
{
    glm::vec4 r = m_world_rect;
    if (m_textures[0])
    {
        SDL_Texture *tex = m_is_pressed ? m_textures[2] : (m_is_hovered ? m_textures[1] : m_textures[0]);
        RenderManager::draw_texture(tex, nullptr, r);
    }
    else
    {
        glm::vec4 bg = m_is_pressed ? PRESSED_COLOR : (m_is_hovered ? HOVERED_COLOR : NROMAL_COLOR);
        RenderManager::draw_fill_rect(r, bg);
        RenderManager::draw_rect(r, WHITE);
    }

    UIBase::on_render();
}

void Button::set_texture(SDL_Texture *normal, SDL_Texture *hover, SDL_Texture *click)
{
    if (!normal || !hover || !click) return;

    m_textures[0] = normal;
    m_textures[1] = hover;
    m_textures[2] = click;
}

}    
