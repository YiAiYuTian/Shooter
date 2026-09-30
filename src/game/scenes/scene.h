#ifndef SCENE_H
#define SCENE_H

namespace shooter
{

class GameWorld;

class Scene
{
public:
    Scene() = default;
    virtual ~Scene() = default;

    void set_world(GameWorld *world) { m_world = world; }

    virtual void on_enter() = 0;
    virtual void on_exit() = 0;

    virtual void on_update(float dt) = 0;
    virtual void on_event() = 0;
    virtual void on_render() = 0;
protected:
    GameWorld *m_world = nullptr;
};

}

#endif