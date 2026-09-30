#ifndef SHOOTER_EXPLOSION_H
#define SHOOTER_EXPLOSION_H

#include "../core/object.h"

namespace shooter
{

class Explosion : public Object
{
public:
    void init(glm::vec2 p)
    {
        set_pos(p);
        m_radius = 0.0f;
        m_age = 0.0f;
        set_alive(true);
    }
    void update(float dt);
    void draw() const;

    void on_update(float dt) override { update(dt); }
    void on_event() override {}
    void on_render() override { draw(); }
private:
    float m_radius = 0.0f;
    float m_age = 0.0f;
};

}

#endif