#ifndef SHOOTER_BULLET_H
#define SHOOTER_BULLET_H

#include "../core/object.h"
#include "weapon.h"

namespace shooter
{

class Player;
class AIPlayer;

class Bullet : public Object
{
public:
    void init(glm::vec2 origin, int facing, const Weapon &wp, Player *op, AIPlayer *oa);
    void update(float dt);
    void draw() const;

    void on_update(float dt) override { update(dt); }
    void on_event() override {}
    void on_render() override { draw(); }

    int dmg() const { return m_dmg; }
    glm::vec4 color() const { return m_color; }
    bool explosion() const { return m_explosion; }
    Player *op() const { return m_op; }
    AIPlayer *oa() const { return m_oa; }
private:
    int m_dmg = 1;
    glm::vec4 m_color = YELLOW;
    bool m_explosion = false;
    Player *m_op = nullptr;
    AIPlayer *m_oa = nullptr;
    float m_age = 0.0f;
    glm::vec2 m_trail[6];
    int m_trail_n = 0;
};

}

#endif