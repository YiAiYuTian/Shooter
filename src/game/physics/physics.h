#ifndef SHOOTER_PHYSICS_H
#define SHOOTER_PHYSICS_H

#include "../core/object.h"

#include <vector>

class b2World;

namespace shooter
{

inline constexpr float GRAVITY = 3200.0f;
inline constexpr float MAX_FALL = 3000.0f;
inline constexpr float RUN_SPEED = 500.0f;
inline constexpr float RECOIL = 240.0f;
inline constexpr float JUMP_PLAYER = -1000.0f;
inline constexpr float JUMP_ZOMBIE = -9000.0f;
inline constexpr float JUMP_AI = -1000.0f;
inline constexpr float PHYSICS_SCALE = 0.01f;

class PhysicsManager
{
public:
    static PhysicsManager &instance()
    {
        static PhysicsManager pm;
        return pm;
    }

    static void register_body(Object *obj)
    {
        instance().register_body_impl(obj);
    }

    static void unregister_body(Object *obj)
    {
        instance().unregister_body_impl(obj);
    }

    static void clear()
    {
        instance().clear_impl();
    }
    
    static void on_update(float dt)
    {
        instance().on_update_impl(dt);
    }
    
    static bool is_overlap(float ax, float ay, float aw, float ah, float bx, float by, float bw, float bh)
    {
        return instance().is_overlap_impl(ax, ay, aw, ah, bx, by, bw, bh);
    }

    static const std::vector<Object *> &get_bodies()
    {
        return instance().get_bodies_impl();
    }

private:
    void register_body_impl(Object *obj);
    void unregister_body_impl(Object *obj);
    void clear_impl();
    void on_update_impl(float dt);
    bool is_overlap_impl(float ax, float ay, float aw, float ah, float bx, float by, float bw, float bh) const;
    const std::vector<Object *> &get_bodies_impl() const { return m_bodies; }
    void ensure();
    void create_body(Object *obj);
private:
    b2World *m_world = nullptr;
    std::vector<Object *> m_bodies;
};

}

#endif
