#include "ai.h"
#include "bullet.h"
#include "../physics/physics.h"
#include "../../core/resource_manager.h"
#include "../../render/render_manager.h"

#include <SDL3/SDL.h>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>


namespace shooter
{

AIPlayer::AIPlayer()
{
    m_anim_idle_l = std::make_unique<Animation>(Atlas({
        "res/img/warrior_idle_left_0.png",
        "res/img/warrior_idle_left_1.png",
        "res/img/warrior_idle_left_2.png",
        "res/img/warrior_idle_left_3.png" }));
    m_anim_idle_r = std::make_unique<Animation>(Atlas({
        "res/img/warrior_idle_right_0.png",
        "res/img/warrior_idle_right_1.png",
        "res/img/warrior_idle_right_2.png",
        "res/img/warrior_idle_right_3.png" }));
    m_anim_walk_l = std::make_unique<Animation>(Atlas({
        "res/img/warrior_walk_left_0.png",
        "res/img/warrior_walk_left_1.png",
        "res/img/warrior_walk_left_2.png",
        "res/img/warrior_walk_left_3.png",
        "res/img/warrior_walk_left_4.png",
        "res/img/warrior_walk_left_5.png",
        "res/img/warrior_walk_left_6.png",
        "res/img/warrior_walk_left_7.png" }));
    m_anim_walk_r = std::make_unique<Animation>(Atlas({
        "res/img/warrior_walk_right_0.png",
        "res/img/warrior_walk_right_1.png",
        "res/img/warrior_walk_right_2.png",
        "res/img/warrior_walk_right_3.png",
        "res/img/warrior_walk_right_4.png",
        "res/img/warrior_walk_right_5.png",
        "res/img/warrior_walk_right_6.png",
        "res/img/warrior_walk_right_7.png" }));
    for (auto *a : { m_anim_idle_l.get(), m_anim_idle_r.get(), m_anim_walk_l.get(), m_anim_walk_r.get() })
        if (a) a->set_interval(0.1f);
    m_anim = m_anim_idle_r.get();
}
void AIPlayer::init(const char *name_, glm::vec2 sp, glm::vec4 col, float diff_)
{
    m_name = name_;
    m_spawn = sp;
    set_pos(sp);
    m_color = col;
    m_difficulty = diff_;
    m_body.dynamic = true;
    m_body.gravity = true;
    m_body.collide = true;
    m_body.gravity_scale = 1.0f;
    m_body.max_fall = MAX_FALL;
    set_collider({ 6.0f, 6.0f, 52.0f, 60.0f });
}

void AIPlayer::reset()
{
    m_hp = 5;
    set_pos(m_spawn);
    m_body.vel = { 0.0f, 0.0f };
    m_body.on_ground = false;
    set_alive(true);
}

void AIPlayer::update(float dt, const Player (&players)[2], std::vector<Bullet> &bullets)
{
    if (!is_alive()) return;
    m_shoot_cd -= dt;

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
        const Weapon &wp = WEAPONS[m_weapon];
        float ideal = wp.auto_fire() ? 180.0f : 120.0f;
        m_facing = dx > 0.0f ? 1 : -1;
        if (dist > ideal + 36.0f)
            m_body.vel.x = 250.0f * (dx > 0.0f ? 1.0f : -1.0f);
        else if (dist < ideal - 36.0f)
            m_body.vel.x = -250.0f * (dx > 0.0f ? 1.0f : -1.0f);
        if (dy < -60.0f && m_body.on_ground && std::fabs(dx) < 240.0f)
        {
            m_body.vel.y = JUMP_AI;
            m_body.on_ground = false;
        }
        if (m_shoot_cd <= 0.0f)
        {
            m_shoot_cd = 0.55f;
            if ((float)(std::rand() % 100) / 100.0f < 0.2f * m_difficulty)
            {
                float gx = get_pos().x + (m_facing > 0 ? get_w() : -4.0f);
                float gy = get_pos().y + 18.0f;
                Bullet b;
                b.init({ gx, gy }, m_facing, wp, nullptr, this);
                bullets.push_back(b);
                if (m_weapon == 4) m_body.vel.x -= m_facing * RECOIL;
            }
        }
    }
    Animation *a = m_body.on_ground
        ? (m_facing > 0 ? m_anim_idle_r.get() : m_anim_idle_l.get())
        : (m_facing > 0 ? m_anim_walk_r.get() : m_anim_walk_l.get());
    if (a != m_anim) m_anim = a;
    if (m_anim) m_anim->on_update(dt);
}

void AIPlayer::draw(TTF_Font *font) const
{
    if (!is_alive()) return;
    draw_actor(get_pos().x, get_pos().y, get_w(), get_h(), 64.0f, 72.0f, m_anim ? m_anim->current() : nullptr);
    draw_gun(get_pos().x, get_pos().y + 16.0f, m_facing, WEAPONS[m_weapon].color());
    draw_hp(get_pos().x - 2.0f, get_pos().y - 22.0f, 34.0f, (float)m_hp, 5.0f);
    RenderManager::draw_text(m_name, font, { (int)get_pos().x - 4, (int)get_pos().y - 34 }, 16, m_color);
}

}