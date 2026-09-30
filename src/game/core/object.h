#ifndef SHOOTER_OBJECT_H
#define SHOOTER_OBJECT_H

#include <glm/glm.hpp>

class b2Body;

namespace shooter
{

struct PhysicsBody
{
    glm::vec2 vel{};
    bool dynamic = false;
    bool gravity = true;
    bool collide = true;
    float gravity_scale = 1.0f;
    float max_fall = 1400.0f;
    bool on_ground = false;
    b2Body *body = nullptr;
};

class Object
{
public:
    Object() = default;
    virtual ~Object() = default;

    virtual void on_update(float dt) = 0;
    virtual void on_event() = 0;
    virtual void on_render() = 0;

    glm::vec2 get_pos() const { return { m_pos_size.x, m_pos_size.y }; }
    void set_pos(glm::vec2 p) { m_pos_size.x = p.x; m_pos_size.y = p.y; }
    float get_w() const { return m_pos_size.z; }
    float get_h() const { return m_pos_size.w; }
    void set_size(glm::vec2 s) { m_pos_size.z = s.x; m_pos_size.w = s.y; }
    bool is_alive() const { return m_alive; }
    void set_alive(bool a) { m_alive = a; }
    PhysicsBody &get_body() { return m_body; }

    glm::vec4 get_collider() const { return m_collider; }
    void set_collider(glm::vec4 c) { m_collider = c; }

protected:
    PhysicsBody m_body;
    glm::vec4 m_pos_size{};
    glm::vec4 m_collider{};
    bool m_alive = true;
};

}

#endif