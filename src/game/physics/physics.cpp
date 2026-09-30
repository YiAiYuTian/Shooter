#include "physics.h"
#include "../utils.h"

#include <box2d/box2d.h>
#include <cmath>
#include <unordered_map>

namespace shooter
{

namespace
{

float collider_w(Object *o)
{
    float w = o->get_collider().z;
    return w > 0.0f ? w : o->get_w();
}

float collider_h(Object *o)
{
    float h = o->get_collider().w;
    return h > 0.0f ? h : o->get_h();
}

}

void PhysicsManager::ensure()
{
    if (m_world) return;
    m_world = new b2World({ 0.0f, GRAVITY * PHYSICS_SCALE });
}

void PhysicsManager::create_body(Object *o)
{
    b2BodyDef bd;
    bd.type = o->get_body().dynamic ? b2_dynamicBody : b2_staticBody;
    bd.fixedRotation = true;
    float ox = o->get_collider().x;
    float oy = o->get_collider().y;
    float cw = collider_w(o);
    float ch = collider_h(o);
    bd.position.Set(((o->get_pos().x + ox) + cw * 0.5f) * PHYSICS_SCALE,
                    ((o->get_pos().y + oy) + ch * 0.5f) * PHYSICS_SCALE);
    b2Body *b = m_world->CreateBody(&bd);

    b2PolygonShape main;
    main.SetAsBox(cw * PHYSICS_SCALE * 0.5f, ch * PHYSICS_SCALE * 0.5f);

    if (o->get_body().dynamic)
    {
        b2FixtureDef fd;
        fd.shape = &main;
        fd.density = 1.0f;
        fd.friction = 0.0f;
        b->CreateFixture(&fd);

        b2PolygonShape gs;
        gs.SetAsBox(cw * PHYSICS_SCALE * 0.45f, 0.02f,
                    { 0.0f, ch * PHYSICS_SCALE * 0.5f + 0.01f }, 0.0f);
        b2FixtureDef gfd;
        gfd.shape = &gs;
        gfd.isSensor = true;
        b->CreateFixture(&gfd);

        b->SetGravityScale(o->get_body().gravity_scale);
    }
    else b->CreateFixture(&main, 0.0f);

    o->get_body().body = b;
}

void PhysicsManager::register_body_impl(Object *obj)
{
    if (!obj) return;
    ensure();
    for (auto *o : m_bodies)
        if (o == obj) return;
    m_bodies.push_back(obj);
    if (!obj->get_body().body)
        create_body(obj);
}

void PhysicsManager::unregister_body_impl(Object *obj)
{
    if (!obj) return;
    for (size_t i = 0; i < m_bodies.size(); ++i)
        if (m_bodies[i] == obj)
        {
            m_bodies.erase(m_bodies.begin() + i);
            break;
        }
    if (obj->get_body().body)
    {
        if (m_world) m_world->DestroyBody(static_cast<b2Body *>(obj->get_body().body));
        obj->get_body().body = nullptr;
    }
}

void PhysicsManager::clear_impl()
{
    for (Object *o : m_bodies)
        if (o->get_body().body)
        {
            if (m_world) m_world->DestroyBody(static_cast<b2Body *>(o->get_body().body));
            o->get_body().body = nullptr;
        }
    m_bodies.clear();
}

void PhysicsManager::on_update_impl(float dt)
{
    ensure();
    if (!m_world) return;

    for (Object *o : m_bodies)
    {
        if (!o->is_alive() || !o->get_body().dynamic || !o->get_body().body) continue;
        PhysicsBody &b = o->get_body();
        b2Body *bd = static_cast<b2Body *>(b.body);

        bd->SetGravityScale(b.gravity ? b.gravity_scale : 0.0f);

        float ox = o->get_collider().x;
        float oy = o->get_collider().y;
        float cw = collider_w(o);
        float ch = collider_h(o);
        b2Vec2 bp = bd->GetPosition();
        float expx = ((o->get_pos().x + ox) + cw * 0.5f) * PHYSICS_SCALE;
        float expy = ((o->get_pos().y + oy) + ch * 0.5f) * PHYSICS_SCALE;
        if (std::fabs(bp.x - expx) > 0.05f || std::fabs(bp.y - expy) > 0.05f)
            bd->SetTransform({ expx, expy }, 0.0f);

        b2Vec2 v = bd->GetLinearVelocity();
        v.x = b.vel.x * PHYSICS_SCALE;
        if (b.vel.y != 0.0f)
        {
            v.y = b.vel.y * PHYSICS_SCALE;
            b.vel.y = 0.0f;
        }
        if (v.y > b.max_fall * PHYSICS_SCALE) v.y = b.max_fall * PHYSICS_SCALE;
        bd->SetLinearVelocity(v);
    }

    m_world->Step(dt, 8, 3);

    for (Object *o : m_bodies)
    {
        if (!o->is_alive() || !o->get_body().dynamic || !o->get_body().body) continue;
        b2Body *bd = static_cast<b2Body *>(o->get_body().body);
        b2Vec2 p = bd->GetPosition();
        float ox = o->get_collider().x;
        float oy = o->get_collider().y;
        float cw = collider_w(o);
        float ch = collider_h(o);
        o->set_pos({ p.x / PHYSICS_SCALE - ox - cw * 0.5f, p.y / PHYSICS_SCALE - oy - ch * 0.5f });

        bool grounded = false;
        for (b2ContactEdge *ce = bd->GetContactList(); ce; ce = ce->next)
        {
            b2Contact *c = ce->contact;
            if (!c->IsTouching()) continue;
            b2Fixture *fa = c->GetFixtureA();
            b2Fixture *fb = c->GetFixtureB();
            if (fa->IsSensor() != fb->IsSensor())
            {
                if ((fa->GetBody() == bd && fa->IsSensor()) || (fb->GetBody() == bd && fb->IsSensor()))
                {
                    grounded = true;
                    break;
                }
            }
        }
        o->get_body().on_ground = grounded;
    }
}

bool PhysicsManager::is_overlap_impl(float ax, float ay, float aw, float ah, float bx, float by, float bw, float bh) const
{
    return aabb_overlap(ax, ay, aw, ah, bx, by, bw, bh);
}

}
