#include "game_scene.h"
#include "menu_scene.h"
#include "game_world.h"
#include "../core/types.h"
#include "../utils.h"
#include "../physics/physics.h"
#include "../objects/player.h"
#include "../objects/zombie.h"
#include "../objects/ai.h"
#include "../objects/platform.h"
#include "../objects/bullet.h"
#include "../objects/explosion.h"
#include "../../render/render_manager.h"
#include "../../core/resource_manager.h"
#include "../../event/event_type.h"
#include "../../event/event_manager.h"
#include "../../event/input_manager.h"

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <vector>

namespace shooter
{

namespace
{

constexpr float RESPAWN_TIME = 1.5f;
constexpr float GAME_TIME_LIMIT = 120.0f;
constexpr float EXPLOSION_RANGE = 100.0f;

struct Note
{
    char text[64]{};
    glm::vec4 color = WHITE;
    float timer = 0.0f;
};

struct PlatDef
{
    float x, y, w, h;
};

constexpr PlatDef BASE_PLATS[25] = {
    { 0.0f, 690.0f, 1280.0f, 30.0f },
    { 0.0f, 576.0f, 284.0f, 17.0f },
    { 370.0f, 576.0f, 256.0f, 17.0f },
    { 711.0f, 576.0f, 284.0f, 17.0f },
    { 1081.0f, 576.0f, 199.0f, 17.0f },
    { 71.0f, 468.0f, 256.0f, 17.0f },
    { 427.0f, 468.0f, 227.0f, 17.0f },
    { 739.0f, 468.0f, 284.0f, 17.0f },
    { 1109.0f, 468.0f, 170.0f, 17.0f },
    { 0.0f, 360.0f, 213.0f, 17.0f },
    { 299.0f, 360.0f, 256.0f, 17.0f },
    { 640.0f, 360.0f, 227.0f, 17.0f },
    { 953.0f, 360.0f, 327.0f, 17.0f },
    { 114.0f, 252.0f, 242.0f, 17.0f },
    { 441.0f, 252.0f, 199.0f, 17.0f },
    { 725.0f, 252.0f, 256.0f, 17.0f },
    { 1066.0f, 252.0f, 213.0f, 17.0f },
    { 0.0f, 144.0f, 185.0f, 17.0f },
    { 270.0f, 144.0f, 227.0f, 17.0f },
    { 583.0f, 144.0f, 213.0f, 17.0f },
    { 882.0f, 144.0f, 256.0f, 17.0f },
    { 1223.0f, 144.0f, 43.0f, 17.0f },
    { 0.0f, 0.0f, 1280.0f, 14.0f },
    { 0.0f, 0.0f, 14.0f, 720.0f },
    { 1266.0f, 0.0f, 14.0f, 720.0f },
};

}

struct GameScene::Impl
{
    Mode mode = Mode::Pvp;
    TTF_Font *font = nullptr;

    Player players[2];
    AIPlayer ais[2];
    std::vector<Zombie*> zombies;
    std::vector<Bullet> bullets;
    std::vector<Explosion> explosions;
    std::vector<Platform*> plats;
    std::vector<Note> notes;

    float game_timer = 0.0f;
    float zombie_spawn_timer = 0.0f;
    float round_timer = 0.0f;
    bool over = false;
    char result_text[128]{};

    int stage_level = 1;
    int stage_zombies_total = 0;
    int stage_zombies_spawned = 0;
    int stage_zombies_killed = 0;
    float stage_spawn_timer = 0.0f;
    bool stage_clear = false;
    float stage_clear_timer = 0.0f;

    EventHandle key_handle = INVALID_EVENT_HANDLE;
};

namespace
{

std::vector<glm::vec4> generate_stage_platforms(int level)
{
    std::vector<glm::vec4> ps = {
        { 0.0f, 690.0f, 1280.0f, 30.0f },
        { 0.0f, 0.0f, 1280.0f, 14.0f },
        { 0.0f, 0.0f, 14.0f, 720.0f },
        { 1266.0f, 0.0f, 14.0f, 720.0f },
    };
    const float layers[5] = { 576.0f, 468.0f, 360.0f, 252.0f, 144.0f };
    int n = 2 + level / 3;
    if (n > 4) n = 4;
    for (float y : layers)
    {
        float used[4][2]{};
        int used_n = 0;
        for (int i = 0; i < n; ++i)
        {
            float pw = 100.0f + (float)(std::rand() % 121);
            float px = 30.0f + (float)(std::rand() % (int)(1150.0f - pw));
            bool overlap = false;
            for (int j = 0; j < used_n; ++j)
                if (!(px + pw + 24.0f < used[j][0] || px - 24.0f > used[j][1])) { overlap = true; break; }
            if (overlap) continue;
            ps.push_back({ px, y, pw, 16.0f });
            used[used_n][0] = px;
            used[used_n][1] = px + pw;
            used_n += 1;
        }
    }
    return ps;
}

void push_note(std::vector<Note> &notes, const char *text, glm::vec4 color, float timer)
{
    Note n;
    std::snprintf(n.text, sizeof(n.text), "%s", text);
    n.color = color;
    n.timer = timer;
    notes.push_back(n);
}

template <typename T>
void try_upgrade(T &who, std::vector<Note> &notes)
{
    if (who.weapon() >= 5) return;
    who.set_weapon(who.weapon() + 1);
    char buf[64];
    std::snprintf(buf, sizeof(buf), "升级: %s", WEAPONS[who.weapon()].name());
    push_note(notes, buf, WEAPONS[who.weapon()].color(), 2.0f);
}

void hit_player(GameScene::Impl &I, Player *victim, int dmg, Player *killer, AIPlayer *ai_killer, bool explosion_hit)
{
    if (!victim || !victim->is_alive()) return;
    victim->set_hp(victim->hp() - dmg);
    victim->set_hurt(0.25f);
    victim->set_state(State::Hurt);
    if (victim->hp() > 0) return;
    victim->set_hp(0);

    if (I.mode == Mode::Pvp)
    {
        if (explosion_hit && killer)
        {
            char buf[64];
            std::snprintf(buf, sizeof(buf), "%s 获胜!", killer->name());
            std::snprintf(I.result_text, sizeof(I.result_text), "%s", buf);
            I.over = true;
            return;
        }
        victim->set_alive(false);
        victim->set_respawn_cd(RESPAWN_TIME);
        if (killer)
        {
            killer->add_kill();
            try_upgrade(*killer, I.notes);
        }
        return;
    }

    if (I.mode == Mode::Zombie)
    {
        victim->set_hp(victim->max_hp());
        victim->reset();
        return;
    }

    if (I.mode == Mode::Coop)
    {
        if (explosion_hit)
        {
            victim->set_permanently_dead(true);
            victim->set_alive(false);
            if (ai_killer) ai_killer->add_kill();
            bool all_dead = !I.players[0].is_alive() && !I.players[1].is_alive();
            if (all_dead)
            {
                std::snprintf(I.result_text, sizeof(I.result_text), "AI队 获胜!");
                I.over = true;
            }
            return;
        }
        victim->set_alive(false);
        victim->set_respawn_cd(RESPAWN_TIME);
        if (ai_killer)
        {
            ai_killer->add_kill();
            try_upgrade(*ai_killer, I.notes);
        }
        return;
    }

    victim->set_alive(false);
    victim->set_lives(victim->lives() - 1);
    if (victim->lives() > 0)
    {
        victim->set_respawn_cd(RESPAWN_TIME);
        return;
    }
    victim->set_permanently_dead(true);
    bool all_dead = !I.players[0].is_alive() && !I.players[1].is_alive();
    if (all_dead)
    {
        std::snprintf(I.result_text, sizeof(I.result_text), "游戏结束!");
        I.over = true;
    }
}

void hit_zombie(GameScene::Impl &I, Zombie &z, int dmg, Player *killer)
{
    if (!z.is_alive()) return;
    z.set_hp(z.hp() - dmg);
    if (z.hp() > 0) return;
    z.set_alive(false);
    if (killer)
    {
        killer->add_kill();
        if (I.mode == Mode::Zombie) try_upgrade(*killer, I.notes);
    }
    I.stage_zombies_killed += 1;
}

void hit_ai(GameScene::Impl &I, AIPlayer &ai, int dmg, Player *killer, bool explosion_hit)
{
    if (!ai.is_alive()) return;
    ai.set_hp(ai.hp() - dmg);
    if (ai.hp() > 0) return;
    ai.set_hp(0);
    if (explosion_hit)
    {
        ai.set_permanently_dead(true);
        ai.set_alive(false);
        std::snprintf(I.result_text, sizeof(I.result_text), "玩家队 获胜!");
        I.over = true;
        return;
    }
    ai.set_alive(false);
    ai.reset();
    if (killer)
    {
        killer->add_kill();
        try_upgrade(*killer, I.notes);
    }
}

void check_hits(GameScene::Impl &I)
{
    for (auto &b : I.bullets)
    {
        if (!b.is_alive()) continue;
        float bx = b.get_pos().x - 4.0f;
        float by = b.get_pos().y - 4.0f;

        if (I.mode == Mode::Pvp)
        {
            for (auto &t : I.players)
            {
                if (!t.is_alive() || b.op() == &t) continue;
                if (!PhysicsManager::is_overlap(bx, by, 8.0f, 8.0f, t.get_pos().x, t.get_pos().y, t.get_w(), t.get_h())) continue;
                b.set_alive(false);
                if (b.explosion())
                {
                    Explosion e;
                    e.init(b.get_pos());
                    I.explosions.push_back(e);
                    for (auto &t2 : I.players)
                        if (t2.is_alive() && std::hypot(b.get_pos().x - (t2.get_pos().x + t2.get_w() * 0.5f), b.get_pos().y - (t2.get_pos().y + t2.get_h() * 0.5f)) < EXPLOSION_RANGE)
                            hit_player(I, &t2, b.dmg(), b.op(), b.oa(), true);
                }
                else hit_player(I, &t, b.dmg(), b.op(), b.oa(), false);
                break;
            }
        }
        else if (I.mode == Mode::Zombie || I.mode == Mode::Stage)
        {
            for (auto *z : I.zombies)
            {
                if (!z->is_alive()) continue;
                if (!PhysicsManager::is_overlap(bx, by, 8.0f, 8.0f, z->get_pos().x, z->get_pos().y, z->get_w(), z->get_h())) continue;
                b.set_alive(false);
                if (b.explosion())
                {
                    Explosion e;
                    e.init(b.get_pos());
                    I.explosions.push_back(e);
                    for (auto *z2 : I.zombies)
                        if (z2->is_alive() && std::hypot(b.get_pos().x - (z2->get_pos().x + z2->get_w() * 0.5f), b.get_pos().y - (z2->get_pos().y + z2->get_h() * 0.5f)) < EXPLOSION_RANGE)
                            hit_zombie(I, *z2, b.dmg(), b.op());
                }
                else hit_zombie(I, *z, b.dmg(), b.op());
                break;
            }
        }
        else
        {
            if (b.oa())
            {
                for (auto &t : I.players)
                {
                    if (!t.is_alive()) continue;
                    if (!PhysicsManager::is_overlap(bx, by, 8.0f, 8.0f, t.get_pos().x, t.get_pos().y, t.get_w(), t.get_h())) continue;
                    b.set_alive(false);
                    if (b.explosion())
                    {
                        Explosion e;
                        e.init(b.get_pos());
                        I.explosions.push_back(e);
                        for (auto &t2 : I.players)
                            if (t2.is_alive() && std::hypot(b.get_pos().x - (t2.get_pos().x + t2.get_w() * 0.5f), b.get_pos().y - (t2.get_pos().y + t2.get_h() * 0.5f)) < EXPLOSION_RANGE)
                                hit_player(I, &t2, b.dmg(), nullptr, b.oa(), true);
                    }
                    else hit_player(I, &t, b.dmg(), nullptr, b.oa(), false);
                    break;
                }
            }
            else
            {
                for (auto &ai : I.ais)
                {
                    if (!ai.is_alive()) continue;
                    if (!PhysicsManager::is_overlap(bx, by, 8.0f, 8.0f, ai.get_pos().x, ai.get_pos().y, ai.get_w(), ai.get_h())) continue;
                    b.set_alive(false);
                    if (b.explosion())
                    {
                        Explosion e;
                        e.init(b.get_pos());
                        I.explosions.push_back(e);
                        for (auto &ai2 : I.ais)
                            if (ai2.is_alive() && std::hypot(b.get_pos().x - (ai2.get_pos().x + ai2.get_w() * 0.5f), b.get_pos().y - (ai2.get_pos().y + ai2.get_h() * 0.5f)) < EXPLOSION_RANGE)
                                hit_ai(I, ai2, b.dmg(), b.op(), true);
                    }
                    else hit_ai(I, ai, b.dmg(), b.op(), false);
                    break;
                }
            }
        }
    }
}

void start_stage(GameScene::Impl &I)
{
    for (auto *z : I.zombies) { PhysicsManager::unregister_body(z); delete z; }
    I.zombies.clear();
    I.bullets.clear();
    I.explosions.clear();
    I.stage_zombies_total = 5 + (I.stage_level - 1) * 3;
    I.stage_zombies_spawned = 0;
    I.stage_zombies_killed = 0;
    I.stage_spawn_timer = 0.0f;
    I.stage_clear = false;
    I.stage_clear_timer = 0.0f;

    for (auto *p : I.plats) { PhysicsManager::unregister_body(p); delete p; }
    I.plats.clear();
    for (const auto &d : generate_stage_platforms(I.stage_level))
    {
        I.plats.push_back(new Platform(d.x, d.y, d.z, d.w));
        PhysicsManager::register_body(I.plats.back());
    }

    Player &p1 = I.players[0];
    p1.set_weapon(0);
    p1.set_lives(4);
    p1.set_permanently_dead(false);
    p1.reset();
    I.players[1].set_alive(false);
    I.players[1].set_permanently_dead(true);

    char buf[64];
    std::snprintf(buf, sizeof(buf), "第 %d 关 | 僵尸: %d", I.stage_level, I.stage_zombies_total);
    push_note(I.notes, buf, CYAN, 3.0f);
}

void spawn_zombie(GameScene::Impl &I, float hp)
{
    static const char *names[3] = { "snail", "bee", "boar" };
    Zombie *z = new Zombie();
    z->init(names[std::rand() % 3], { 0.0f, 0.0f }, 130.0f + (std::rand() % 3) * 40.0f, hp);
    z->set_size({ 40.0f, 48.0f });
    z->set_pos({ 40.0f + (float)(std::rand() % 1200), 60.0f });
    PhysicsManager::register_body(z);
    I.zombies.push_back(z);
}

} // namespace

void GameScene::on_enter()
{
    m_impl = new Impl();
    m_impl->mode = static_cast<Mode>(m_mode);
    m_impl->font = ResourceManager::get_font("res/font/wqy-microhei.ttc");
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    PhysicsManager::clear();

    for (const auto &d : BASE_PLATS)
    {
        m_impl->plats.push_back(new Platform(d.x, d.y, d.w, d.h));
        PhysicsManager::register_body(m_impl->plats.back());
    }

    Player &p1 = m_impl->players[0];
    Player &p2 = m_impl->players[1];
    p1.set_name("玩家1");
    p1.set_color(BLUE);
    p1.set_prefix("warrior");
    p1.set_spawn({ 80.0f, 78.0f });
    p1.set_size({ 52.0f, 60.0f });
    p2.set_name("玩家2");
    p2.set_color(RED);
    p2.set_prefix("warrior");
    p2.set_spawn({ 1148.0f, 624.0f });
    p2.set_size({ 52.0f, 60.0f });

    if (m_impl->mode == Mode::Stage)
    {
        p1.set_keys(SDL_SCANCODE_LEFT, SDL_SCANCODE_RIGHT, SDL_SCANCODE_UP, SDL_SCANCODE_PERIOD);
        start_stage(*m_impl);
    }
    else
    {
        p1.set_keys(SDL_SCANCODE_A, SDL_SCANCODE_D, SDL_SCANCODE_W, SDL_SCANCODE_F);
        p2.set_keys(SDL_SCANCODE_LEFT, SDL_SCANCODE_RIGHT, SDL_SCANCODE_UP, SDL_SCANCODE_L);
        p1.reset();
        p2.reset();

        if (m_impl->mode == Mode::Coop)
        {
            m_impl->ais[0].init("AI-1", { 1148.0f, 90.0f }, PURPLE, 0.4f);
            m_impl->ais[1].init("AI-2", { 80.0f, 624.0f }, ORANGE, 0.5f);
            m_impl->ais[0].set_size({ 52.0f, 60.0f });
            m_impl->ais[1].set_size({ 52.0f, 60.0f });
            PhysicsManager::register_body(&m_impl->ais[0]);
            PhysicsManager::register_body(&m_impl->ais[1]);
        }
        else if (m_impl->mode == Mode::Zombie)
        {
            m_impl->zombie_spawn_timer = 2.0f;
        }
    }

    PhysicsManager::register_body(&p1);
    PhysicsManager::register_body(&p2);

    m_impl->key_handle = EventManager::subscribe<KeyEvent>(
        [this](const KeyEvent &e)
        {
            e.consumed = true;
            if (e.down && e.key == SDLK_ESCAPE && m_impl)
                m_world->change_scene<MenuScene>();
        }
    );
}

void GameScene::on_exit()
{
    if (!m_impl) return;
    if (m_impl->key_handle != INVALID_EVENT_HANDLE)
        EventManager::unsubscribe(m_impl->key_handle);
    PhysicsManager::clear();
    for (auto *z : m_impl->zombies) delete z;
    for (auto *p : m_impl->plats) delete p;
    delete m_impl;
    m_impl = nullptr;
}

void GameScene::on_update(float dt)
{
    if (!m_impl) return;
    Impl &I = *m_impl;

    PhysicsManager::on_update(dt);

    for (auto &p : I.players) p.update(dt, I.bullets);
    for (auto &ai : I.ais) ai.update(dt, I.players, I.bullets);
    for (auto *z : I.zombies) z->update(dt, I.players);
    for (auto &b : I.bullets) b.update(dt);
    for (auto &e : I.explosions) e.update(dt);

    check_hits(I);

    if (I.mode == Mode::Zombie)
    {
        I.game_timer += dt;
        I.zombie_spawn_timer -= dt;
        if (I.zombie_spawn_timer <= 0.0f)
        {
            I.zombie_spawn_timer = 2.0f;
            spawn_zombie(I, 2.0f);
        }
        if (I.game_timer >= GAME_TIME_LIMIT && !I.over)
        {
            std::snprintf(I.result_text, sizeof(I.result_text),
                          "玩家1 击杀 %d | 玩家2 击杀 %d",
                          I.players[0].kills(), I.players[1].kills());
            I.over = true;
        }
    }
    else if (I.mode == Mode::Stage)
    {
        if (!I.stage_clear)
        {
            I.stage_spawn_timer -= dt;
            if (I.stage_spawn_timer <= 0.0f && I.stage_zombies_spawned < I.stage_zombies_total)
            {
                I.stage_spawn_timer = 1.0f;
                spawn_zombie(I, 1.0f + (float)(I.stage_level - 1) * 0.5f);
                I.stage_zombies_spawned += 1;
            }
            if (I.stage_zombies_killed >= I.stage_zombies_total)
            {
                I.stage_clear = true;
                I.stage_clear_timer = 0.0f;
            }
        }
        else
        {
            I.stage_clear_timer += dt;
            if (I.stage_clear_timer >= 2.5f)
            {
                I.stage_level += 1;
                start_stage(I);
            }
        }
    }

    if (I.mode == Mode::Zombie || I.mode == Mode::Stage)
    {
        for (auto *z : I.zombies)
        {
            if (!z->is_alive()) continue;
            for (auto &p : I.players)
            {
                if (!p.is_alive()) continue;
                if (PhysicsManager::is_overlap(z->get_pos().x - 6.0f, z->get_pos().y - 6.0f,
                                               z->get_w() + 12.0f, z->get_h() + 12.0f,
                                               p.get_pos().x, p.get_pos().y, p.get_w(), p.get_h()))
                {
                    if (z->atk_cd() <= 0.0f)
                    {
                        z->set_atk_cd(0.8f);
                        hit_player(I, &p, 1, nullptr, nullptr, false);
                    }
                    break;
                }
            }
        }
    }

    for (auto it = I.bullets.begin(); it != I.bullets.end();)
        it->is_alive() ? (void)++it : (void)(it = I.bullets.erase(it));
    for (auto it = I.explosions.begin(); it != I.explosions.end();)
        it->is_alive() ? (void)++it : (void)(it = I.explosions.erase(it));
    for (auto it = I.zombies.begin(); it != I.zombies.end();)
    {
        if (!(*it)->is_alive())
        {
            PhysicsManager::unregister_body(*it);
            delete *it;
            it = I.zombies.erase(it);
        }
        else ++it;
    }
    for (auto it = I.notes.begin(); it != I.notes.end();)
    {
        it->timer -= dt;
        it->timer > 0.0f ? (void)++it : (void)(it = I.notes.erase(it));
    }

    if (I.over)
    {
        I.round_timer += dt;
        if (I.round_timer >= 3.0f)
            m_world->change_scene<MenuScene>();
    }
}

void GameScene::on_event()
{
}

void GameScene::on_render()
{
    if (!m_impl || !m_impl->font) return;
    Impl &I = *m_impl;

    RenderManager::draw_fill_rect({ 0.0f, 0.0f, 1280.0f, 720.0f }, BG);

    for (const auto *p : I.plats) p->draw();
    for (const auto *z : I.zombies) z->draw();
    for (const auto &b : I.bullets) b.draw();
    for (const auto &e : I.explosions) e.draw();
    for (const auto &ai : I.ais) ai.draw(I.font);
    for (const auto &p : I.players) p.draw(I.font);

    static const char *mode_names[4] = { "对战模式", "僵尸模式", "2v2 对战", "闯关模式" };
    RenderManager::draw_text(mode_names[(int)I.mode], I.font, { 20, 12 }, 26, CYAN);

    char hud[128];
    if (I.mode == Mode::Pvp || I.mode == Mode::Coop)
    {
        std::snprintf(hud, sizeof(hud), "P1 击杀 %d | P2 击杀 %d", I.players[0].kills(), I.players[1].kills());
        RenderManager::draw_text(hud, I.font, { 20, 46 }, 20, WHITE);
        if (I.mode == Mode::Coop)
        {
            std::snprintf(hud, sizeof(hud), "AI1 击杀 %d | AI2 击杀 %d", I.ais[0].kills(), I.ais[1].kills());
            RenderManager::draw_text(hud, I.font, { 20, 70 }, 20, PURPLE);
        }
    }
    else if (I.mode == Mode::Zombie)
    {
        int remain = (int)(GAME_TIME_LIMIT - I.game_timer);
        if (remain < 0) remain = 0;
        std::snprintf(hud, sizeof(hud), "剩余 %d 秒 | 击杀 %d", remain, I.players[0].kills() + I.players[1].kills());
        RenderManager::draw_text(hud, I.font, { 20, 46 }, 20, GREEN);
    }
    else
    {
        std::snprintf(hud, sizeof(hud), "第 %d 关 | 僵尸 %d/%d | 生命 %d",
                      I.stage_level, I.stage_zombies_killed, I.stage_zombies_total,
                      I.players[0].lives());
        RenderManager::draw_text(hud, I.font, { 20, 46 }, 20, CYAN);
    }

    int ny = 60;
    for (const auto &n : I.notes)
    {
        RenderManager::draw_text(n.text, I.font, { 1080, ny }, 20, n.color);
        ny += 26;
    }

    RenderManager::draw_text("ESC 返回", I.font, { 20, 685 }, 18, GRAY);

    if (I.over)
    {
        RenderManager::draw_fill_rect({ 340.0f, 260.0f, 600.0f, 200.0f }, PANEL);
        RenderManager::draw_rect({ 340.0f, 260.0f, 600.0f, 200.0f }, WHITE);
        RenderManager::draw_text(I.result_text, I.font, { 640 - (int)std::strlen(I.result_text) * 12, 330 }, 36, YELLOW);
    }
}

}
