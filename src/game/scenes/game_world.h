#ifndef GAME_WORLD_H
#define GAME_WORLD_H

#include "scene.h"

#include <utility>

namespace shooter
{

class App;

class GameWorld
{
public:
    GameWorld() = default;
    ~GameWorld() = default;

    bool init(App *app);
    void quit();
    void quit_game();

    template <typename T, typename... Args>
    void change_scene(Args &&...args)
    {
        Scene *scene = new T(std::forward<Args>(args)...);
        scene->set_world(this);
        m_next_scene = scene;
    }

    void on_update(float dt);
    void on_event();
    void on_render();
private:
    void switch_scene();
private:
    Scene *m_scene = nullptr;
    Scene *m_next_scene = nullptr;
    App *m_app = nullptr;
};

}

#endif
