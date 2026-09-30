#ifndef MENU_SCENE_H
#define MENU_SCENE_H

#include "scene.h"

namespace shooter
{

class MenuScene : public Scene
{
public:
    MenuScene() = default;
    ~MenuScene() override = default;

    void on_enter() override;
    void on_exit() override;

    void on_update(float dt) override;
    void on_event() override;
    void on_render() override;
private:
    struct Impl;
    Impl *m_impl = nullptr;
};

}    

#endif // !MENU_SCENE_H
