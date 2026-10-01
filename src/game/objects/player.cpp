#include "player.h"
#include "bullet.h"
#include "../physics/physics.h"
#include "../../event/input_manager.h"
#include "../../core/resource_manager.h"
#include "../../render/render_manager.h"

#include <SDL3/SDL.h>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>


namespace shooter
{

Player::Player()
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
void Player::reset()
{
    m_body.dynamic = true;
    m_body.gravity = true;
    m_body.collide = true;
    m_body.gravity_scale = 1.0f;
    m_body.max_fall = MAX_FALL;
    set_collider({ 6.0f, 6.0f, 52.0f, 60.0f });
    set_pos(m_spawn);
    m_body.vel = { 0.0f, 0.0f };
    m_hp = m_max_hp;
    set_alive(true);
    m_body.on_ground = false;
    m_respawn_cd = 0.0f;
    m_state = State::Idle;
}

void Player::update(float dt, std::vector<Bullet> &bullets)
{
    if (!is_alive())
    {
        if (m_permanently_dead) return;
        m_respawn_cd -= dt;
        if (m_respawn_cd <= 0.0f) reset();
        return;
    }
    m_clock += dt;
    if (m_hurt_timer > 0.0f) m_hurt_timer -= dt;
    if (m_attack_timer > 0.0f) m_attack_timer -= dt;

    m_body.vel.x = 0.0f;
    if (InputManager::is_key_held(m_k_left)) { m_body.vel.x = -RUN_SPEED; m_facing = -1; }
    if (InputManager::is_key_held(m_k_right)) { m_body.vel.x = RUN_SPEED; m_facing = 1; }
    if (InputManager::is_key_held(m_k_up) && m_body.on_ground)
    {
        m_body.vel.y = JUMP_PLAYER;
        m_body.on_ground = false;
    }

    if (!m_body.on_ground) m_state = State::Air;
    else if (m_body.vel.x != 0.0f) m_state = State::Run;
    else m_state = State::Idle;
    if (m_hurt_timer > 0.0f) m_state = State::Hurt;

    shoot(dt, bullets);
    Animation *a = (m_state == State::Run || m_state == State::Air)
        ? (m_facing > 0 ? m_anim_walk_r.get() : m_anim_walk_l.get())
        : (m_facing > 0 ? m_anim_idle_r.get() : m_anim_idle_l.get());
    if (a != m_anim) m_anim = a;
    if (m_anim) m_anim->on_update(dt);
}

void Player::shoot(float dt, std::vector<Bullet> &bullets)
{
    (void)dt;
    const Weapon &wp = WEAPONS[m_weapon];
    bool pressed = InputManager::is_key_held(m_k_shoot);
    if (wp.auto_fire())
    {
        if (!pressed) return;
        if (m_clock - m_last_shot < wp.rate_ms() / 1000.0f) return;
    }
    else
    {
        if (!(pressed && !m_shoot_pressed)) { m_shoot_pressed = pressed; return; }
        if (m_clock - m_last_shot < wp.rate_ms() / 1000.0f) return;
    }
    m_shoot_pressed = pressed;
    m_last_shot = m_clock;
    float gx = get_pos().x + (m_facing > 0 ? get_w() : -4.0f);
    float gy = get_pos().y + 18.0f;
    Bullet b;
    b.init({ gx, gy }, m_facing, wp, this, nullptr);
    bullets.push_back(b);
    m_attack_timer = 0.12f;
    m_state = State::Attack;
    if (m_weapon == 4) m_body.vel.x -= m_facing * RECOIL;
}

void Player::draw(TTF_Font *font) const
{
    if (!is_alive())
    {
        if (!m_permanently_dead)
        {
            float mx = get_pos().x + get_w() * 0.5f;
            float my = get_pos().y + get_h() * 0.5f;
            RenderManager::draw_fill_rect({ mx - 12.0f, my - 2.0f, 24.0f, 4.0f }, m_color);
            RenderManager::draw_fill_rect({ mx - 2.0f, my - 12.0f, 4.0f, 24.0f }, m_color);
        }
        return;
    }
    draw_actor(get_pos().x, get_pos().y, get_w(), get_h(), 64.0f, 72.0f, m_anim ? m_anim->current() : nullptr);
    draw_gun(get_pos().x, get_pos().y + 16.0f, m_facing, WEAPONS[m_weapon].color());
    if (m_attack_timer > 0.0f)
    {
        const char *tn = m_facing > 0 ? "res/img/attack_right.png" : "res/img/attack_left.png";
        RenderManager::draw_texture(ResourceManager::get_texture(tn), nullptr,
            { get_pos().x + (m_facing > 0 ? get_w() + 10.0f : -50.0f), get_pos().y + 8.0f, 40.0f, 40.0f });
    }
    draw_hp(get_pos().x - 2.0f, get_pos().y - 22.0f, 34.0f, (float)m_hp, (float)m_max_hp);
    RenderManager::draw_text(WEAPONS[m_weapon].name(), font, { (int)get_pos().x - 2, (int)get_pos().y - 38 }, 14, WEAPONS[m_weapon].color());
}

}