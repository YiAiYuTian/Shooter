#ifndef SHOOTER_ZOMBIE_H
#define SHOOTER_ZOMBIE_H

#include "character.h"
#include "player.h"

namespace shooter
{

class Zombie : public Character
{
public:
    void init(const char *prefix, glm::vec2 pos, float speed, float hp);
    void update(float dt, const Player (&players)[2]);
    void draw() const;

    float speed() const { return m_speed; }
    float atk_cd() const { return m_atk_cd; }
    void set_atk_cd(float v) { m_atk_cd = v; }
private:
    float m_speed = 90.0f;
    float m_atk_cd = 0.0f;
    int m_frame_count = 8;
    const char *m_prefix = "snail";
};

}

#endif