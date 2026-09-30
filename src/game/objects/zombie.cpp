#include "zombie.h"
#include "../physics/physics.h"
#include "../../core/resource_manager.h"
#include "../../render/render_manager.h"

#include <SDL3/SDL.h>
#include <cmath>
#include <cstdio>
#include <cstring>

namespace shooter
{

void Zombie::init(const char *prefix_, glm::vec2 p, float speed_, float hp_)
{
    m_prefix = prefix_;
    set_pos(p);
    m_speed = speed_;
    m_max_hp = hp_;
    m_hp = hp_;
    m_frame_count = 8;
    if (std::strcmp(prefix_, "bee") == 0) m_frame_count = 4;
    else if (std::strcmp(prefix_, "boar") == 0) m_frame_count = 6;
    m_body.dynamic = true;
    m_body.gravity = true;
    m_body.collide = true;
    m_body.gravity_scale = 0.95f;
    m_body.max_fall = 1300.0f;
    set_collider({ 6.0f, 6.0f, 40.0f, 48.0f });
    set_alive(true);
}

void Zombie::update(float dt, const Player (&players)[2])
{
    if (!is_alive()) return;
    if (m_atk_cd > 0.0f) m_atk_cd -= dt;

    const Player *nearest = nullptr;
    float min_d = 1e9f;
    for (int i = 0; i < 2; ++i)
    {
        const Player &p = players[i];
        if (!p.is_alive()) continue;
        float dx = get_pos().x - p.get_pos().x;
        float dy = get_pos().y - p.get_pos().y;
        float d = std::sqrt(dx * dx + dy * dy);
        if (d < min_d) { min_d = d; nearest = &p; }
    }
    m_body.vel.x = 0.0f;
    if (nearest)
    {
        float dx = nearest->get_pos().x - get_pos().x;
        float dy = nearest->get_pos().y - get_pos().y;
        float dist = std::sqrt(dx * dx + dy * dy);
        if (dist > 0.0f)
        {
            m_body.vel.x = (dx / dist) * m_speed;
            m_facing = dx > 0.0f ? 1 : -1;
        }
        if (dy < -36.0f && m_body.on_ground && std::fabs(dx) < 120.0f)
        {
            m_body.vel.y = JUMP_ZOMBIE;
            m_body.on_ground = false;
        }
    }
    tick_anim(dt, m_frame_count);
}

void Zombie::draw() const
{
    if (!is_alive()) return;
    char tex_name[128];
    std::snprintf(tex_name, sizeof(tex_name), "res/img/%s_%s_%d.png", m_prefix, m_facing > 0 ? "right" : "left", frame(m_frame_count));
    draw_actor(get_pos().x, get_pos().y, get_w(), get_h(), 52.0f, 60.0f, ResourceManager::get_texture(tex_name));
    if (m_hp < m_max_hp)
    {
        RenderManager::draw_fill_rect({ get_pos().x, get_pos().y - 10.0f, 24.0f, 4.0f }, RED);
        RenderManager::draw_fill_rect({ get_pos().x, get_pos().y - 10.0f, 24.0f * m_hp / m_max_hp, 4.0f }, GREEN);
    }
}

}