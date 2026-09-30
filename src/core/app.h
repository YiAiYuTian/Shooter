#ifndef APP_H
#define APP_H

#include "../game/scenes/game_world.h"

namespace shooter
{

class App
{
public:
    App() = default;
    ~App() = default;

    int run(int argc, char **argv);

    void quit_game();
private:
    bool init();
    void quit();

    void on_update(float dt);
    void on_event();
    void on_render();
private:
    struct Impl;
    Impl *m_impl = nullptr;
    GameWorld m_game;
};

}

#endif // !APP_H
