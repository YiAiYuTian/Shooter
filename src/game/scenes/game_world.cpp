#include "game_world.h"
#include "menu_scene.h"
#include "game_scene.h"
#include "../../core/app.h"
#include "../../core/logger.h"

#include <utility>

namespace shooter
{

bool GameWorld::init(App *app)
{
    m_app = app;

    change_scene<MenuScene>();
    switch_scene();
    return true;
}

void GameWorld::quit()
{
    delete m_scene;
    m_scene = nullptr;
}

void GameWorld::quit_game()
{
    m_app->quit_game();
}

void GameWorld::on_update(float dt)
{
    if (m_next_scene) switch_scene();
    if (m_scene) m_scene->on_update(dt);
}

void GameWorld::on_event()
{
    if (m_scene) m_scene->on_event();
}

void GameWorld::on_render()
{
    if (m_scene) m_scene->on_render();
}

void GameWorld::switch_scene()
{
    if (m_scene)
    {
        m_scene->on_exit();
        delete m_scene;
    }
    m_next_scene->on_enter();

    std::swap(m_scene, m_next_scene);
    m_next_scene = nullptr;
}

}
