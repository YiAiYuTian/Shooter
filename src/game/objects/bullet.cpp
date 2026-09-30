#include "bullet.h"
#include "../physics/physics.h"
#include "../../render/render_manager.h"

#include <cstdlib>

namespace shooter
{

void Bullet::init(glm::vec2 origin, int facing, const Weapon &wp, Player *op_, AIPlayer *oa_)
{
    set_pos(origin);
    set_collider({ -1.0f, -1.0f, 8.0f, 8.0f });
    m_body.vel = { facing * wp.speed(), 0.0f };
    if (wp.auto_fire())
        m_body.vel.y = ((float)(std::rand() % 200) - 100.0f) / 100.0f * 48.0f;
    m_dmg = wp.dmg();
    m_color = wp.color();
    m_explosion = wp.explosion();
    m_op = op_;
    m_oa = oa_;
}

void Bullet::update(float dt)
{
    if (!is_alive()) return;
    m_age += dt;
    glm::vec2 p = get_pos();
    m_trail[m_trail_n % 6] = p;
    m_trail_n += 1;
    p += m_body.vel * dt;
    set_pos(p);
    if (m_explosion) m_body.vel.y += 4.8f * dt;
    for (auto *pl : PhysicsManager::get_bodies())
    {
        if (pl->get_body().dynamic) continue;
        if (PhysicsManager::is_overlap(p.x - 3.0f, p.y - 3.0f, 6.0f, 6.0f,
                                       pl->get_pos().x, pl->get_pos().y, pl->get_w(), pl->get_h()))
        {
            set_alive(false);
            break;
        }
    }
    if (p.x < -30.0f || p.x > 1310.0f || p.y < -30.0f || p.y > 750.0f)
        set_alive(false);
    if (m_age > 5.0f) set_alive(false);
}

void Bullet::draw() const
{
    if (!is_alive()) return;
    glm::vec2 p = get_pos();
    for (int i = 0; i < m_trail_n && i < 6; ++i)
    {
        glm::vec2 t = m_trail[(m_trail_n - 1 - i + 6) % 6];
        RenderManager::draw_fill_rect({ t.x - 2.0f, t.y - 2.0f, 4.0f, 4.0f }, m_color);
    }
    if (m_explosion)
    {
        RenderManager::draw_fill_rect({ p.x - 6.0f, p.y - 6.0f, 12.0f, 12.0f }, m_color);
        RenderManager::draw_fill_rect({ p.x - 3.0f, p.y - 3.0f, 6.0f, 6.0f }, YELLOW);
    }
    else
    {
        RenderManager::draw_fill_rect({ p.x - 3.0f, p.y - 3.0f, 6.0f, 6.0f }, m_color);
        RenderManager::draw_fill_rect({ p.x - 1.0f, p.y - 1.0f, 2.0f, 2.0f }, WHITE);
    }
}

}