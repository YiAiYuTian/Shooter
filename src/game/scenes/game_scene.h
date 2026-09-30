#ifndef GAME_SCENE_H
#define GAME_SCENE_H

#include "scene.h"

namespace shooter
{

class GameScene : public Scene
{
public:
    explicit GameScene(int mode) : m_mode(mode) {}
    ~GameScene() override = default;

    struct Impl;

    void on_enter() override;
    void on_exit() override;
    void on_update(float dt) override;
    void on_event() override;
    void on_render() override;
private:
    Impl *m_impl = nullptr;
    int m_mode = 0;
};

}

#endif