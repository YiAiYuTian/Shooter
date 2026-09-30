#include "menu_scene.h"
#include "game_scene.h"
#include "game_world.h"
#include "../core/types.h"
#include "../ui/button.h"
#include "../ui/text.h"
#include "../ui/panel.h"
#include "../../render/render_manager.h"
#include "../../core/resource_manager.h"
#include "../../core/logger.h"
#include "../../event/event_type.h"
#include "../../event/event_manager.h"
#include "../animation/animation.h"

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

namespace shooter
{

namespace
{

struct MenuOption
{
    const char *name;
    glm::vec4 color;
};

constexpr MenuOption OPTIONS[4] = {
    { "对战模式 (PvP)",      BLUE },
    { "僵尸模式 (合作)",     GREEN },
    { "2v2 模式 (vs AI)",   PURPLE },
    { "闯关模式 (合作)",     CYAN },
};

const char *DESCS[4][3] = {
    { "玩家1(蓝): WASD移动, F射击", "玩家2(红): 方向键移动, L射击", "每次击杀升级武器, 用火箭弹击杀获胜!" },
    { "合作抵抗僵尸潮!", "2分钟内消灭尽可能多的僵尸", "死亡后快速复活, 击杀数多者获胜!" },
    { "P1+P2 合作 vs 2个AI对手", "每人5血, 用火箭弹击杀AI获胜!", "AI会用火箭弹反击, 小心!" },
    { "P1+P2 合作闯关!", "每关消灭所有僵尸即可通关", "每人4次复活, 每关重置!" },
};

void draw_preview(TTF_Font *font, int sel)
{
    for (int i = 0; i < 3; ++i) RenderManager::draw_text(DESCS[sel][i], font, {410, 450 + i * 30}, 20, WHITE);

    if (sel == 0)
    {
        static const float play_pos_y = 590.0f;
        RenderManager::draw_texture(ResourceManager::get_texture("res/img/warrior_idle_right_0.png"), nullptr, { 500.0f, play_pos_y, 72.0f, 72.0f });
        RenderManager::draw_texture(ResourceManager::get_texture("res/img/warrior_idle_left_0.png"),  nullptr, { 620.0f, play_pos_y, 72.0f, 72.0f });
        RenderManager::draw_text("玩家1", font, { 508, play_pos_y + 80.0f }, 18, BLUE);
        RenderManager::draw_text("玩家2", font, { 628, play_pos_y + 80.0f }, 18, RED);
        static const glm::vec4 wc[6] = { YELLOW, GREEN, CYAN, ORANGE, PURPLE, RED };
        static const char *wn[6] = { "手枪", "步枪", "冲锋枪", "机枪", "加特林", "火箭弹" };
        for (int i = 0; i < 6; ++i)
        {
            float x = 410.0f + i * 72.0f;
            RenderManager::draw_fill_rect({ x, 555.0f, 62.0f, 32.0f }, wc[i]);
            RenderManager::draw_text(wn[i], font, { (int)x + 2, 561 }, 15, WHITE);
        }
    }
    else if (sel == 1)
    {
        static const char *names[3] = { "res/img/snail_left_0.png", "res/img/bee_left_0.png", "res/img/boar_left_0.png" };
        for (int i = 0; i < 3; ++i)
            RenderManager::draw_texture(ResourceManager::get_texture(names[i]), nullptr, { 470.0f + i * 105.0f, 555.0f, 64.0f, 64.0f });
        RenderManager::draw_text("僵尸潮来袭!", font, { 535, 635 }, 18, GREEN);
    }
    else if (sel == 2)
    {
        RenderManager::draw_texture(ResourceManager::get_texture("res/img/warrior_idle_right_0.png"), nullptr, { 420.0f, 555.0f, 72.0f, 72.0f });
        RenderManager::draw_texture(ResourceManager::get_texture("res/img/warrior_idle_left_0.png"),  nullptr, { 520.0f, 555.0f, 72.0f, 72.0f });
        RenderManager::draw_texture(ResourceManager::get_texture("res/img/boar_right_0.png"),        nullptr, { 660.0f, 555.0f, 72.0f, 72.0f });
        RenderManager::draw_texture(ResourceManager::get_texture("res/img/bee_right_0.png"),         nullptr, { 760.0f, 555.0f, 72.0f, 72.0f });
        RenderManager::draw_text("玩家队", font, { 430, 635 }, 18, BLUE);
        RenderManager::draw_text("AI-1", font, { 680, 635 }, 18, PURPLE);
        RenderManager::draw_text("AI-2", font, { 780, 635 }, 18, ORANGE);
    }
    else
    {
        for (int i = 0; i < 3; ++i)
        {
            float x = 370.0f + i * 180.0f;
            RenderManager::draw_fill_rect({ x, 590.0f, 130.0f, 12.0f }, CYAN);
        }
        RenderManager::draw_text("第1关", font, { 408, 612 }, 16, CYAN);
        RenderManager::draw_text("3只僵尸", font, { 400, 636 }, 15, GREEN);
        RenderManager::draw_text("第2关", font, { 588, 612 }, 16, CYAN);
        RenderManager::draw_text("5只僵尸", font, { 580, 636 }, 15, GREEN);
        RenderManager::draw_text("第3关", font, { 768, 612 }, 16, CYAN);
        RenderManager::draw_text("7只僵尸", font, { 760, 636 }, 15, GREEN);
    }
}

} // namespace

struct MenuScene::Impl
{
    TTF_Font *font = nullptr;

    int sel = 0;
    std::vector<UIBase*> uis;
    EventHandle key_handle = INVALID_EVENT_HANDLE;
};

void MenuScene::on_enter()
{
    m_impl = new MenuScene::Impl();
    m_impl->font = ResourceManager::get_font("res/font/wqy-microhei.ttc");

    for (int i = 0; i < 4; ++i)
    {
        // button panel
        Panel *p = new Panel();
        p->set_pos({ 440.0f, 198.0f });

        Button *b = p->add_child<Button>();
        b->set_pos({ 0.0f, i * 58.0f });
        b->set_size({ 400.0f, 44.0f });
        b->set_callback(
            [this, i]()
            {
                m_impl->sel = i;
                m_world->change_scene<GameScene>(i);
            }
        );
        b->set_hovered_callback(
            [this, i]()
            {
                m_impl->sel = i;
            }
        );

        Text *t = b->add_child<Text>();
        t->set_text(OPTIONS[i].name);
        t->set_font(m_impl->font);
        t->set_font_size(32);
        t->set_color(OPTIONS[i].color);
        t->set_pos({ 90.0f, 5.0f });

        m_impl->uis.push_back(p);
    }

    m_impl->key_handle = EventManager::subscribe<KeyEvent>(
        [this](const KeyEvent &e)
        {
            e.consumed = true;
            if (e.down && e.key == SDLK_ESCAPE && m_impl) m_world->quit_game();
        }
    );
}

void MenuScene::on_exit()
{
    if (m_impl && m_impl->key_handle != INVALID_EVENT_HANDLE)
        EventManager::unsubscribe(m_impl->key_handle);

    for (auto *u : m_impl->uis)
        delete u;
    m_impl->uis.clear();

    delete m_impl;
}

void MenuScene::on_update(float dt)
{
    for (auto &ui : m_impl->uis)
    {
        ui->on_update(dt);
        ui->set_layout({ 0.0f, 0.0f, 0.0f, 0.0f });
    }
}

void MenuScene::on_event()
{
}

void MenuScene::on_render()
{
    if (!m_impl || !m_impl->font) return;

    RenderManager::draw_fill_rect({ 0.0f, 0.0f, 1280.0f, 720.0f }, BG);
    RenderManager::draw_text_gradient(
        "隔 板 射 击 战", m_impl->font, { 380, 55 }, 80,
        { 1.0f, 0.314f, 0.314f, 1.0f },
        { 0.784f, 0.392f, 1.0f, 1.0f }
    );
    RenderManager::draw_text("选择游戏模式", m_impl->font, { 545, 160 }, 24, GRAY);

    for (auto *ui : m_impl->uis)
        ui->on_render();
    draw_preview(m_impl->font, m_impl->sel);
    RenderManager::draw_text("ESC返回", m_impl->font, { 550, 685 }, 22, GRAY);
}

}
