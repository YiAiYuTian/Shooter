#include "explosion.h"
#include "../core/types.h"
#include "../../render/render_manager.h"

namespace shooter
{

void Explosion::update(float dt)
{
    if (!is_alive()) return;
    m_age += dt;
    m_radius += 60.0f * dt;
    if (m_radius >= 100.0f) set_alive(false);
}

void Explosion::draw() const
{
    if (!is_alive()) return;
    glm::vec2 p = get_pos();
    RenderManager::draw_rect({ p.x - m_radius, p.y - m_radius, m_radius * 2.0f, m_radius * 2.0f }, { 1.0f, 0.45f, 0.1f, 1.0f });
    RenderManager::draw_rect({ p.x - m_radius * 0.5f, p.y - m_radius * 0.5f, m_radius, m_radius }, YELLOW);
}

}