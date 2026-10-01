#ifndef SHOOTER_AI_H
#define SHOOTER_AI_H

#include "character.h"
#include "bullet.h"
#include "player.h"
#include "weapon.h"
#include <vector>

namespace shooter
{

class AIPlayer : public Character
{
public:
    AIPlayer();
    void init(const char *name, glm::vec2 spawn, glm::vec4 color, float difficulty);
    void reset();
    void update(float dt, const Player (&players)[2], std::vector<Bullet> &bullets);
    void draw(TTF_Font *font) const;

    void set_weapon(int w) { m_weapon = w; }
    int weapon() const { return m_weapon; }
    void add_kill() { m_kills += 1; }
    int kills() const { return m_kills; }
    void set_permanently_dead(bool v) { m_permanently_dead = v; }
    bool permanently_dead() const { return m_permanently_dead; }
    void set_difficulty(float d) { m_difficulty = d; }
private:
    int m_weapon = 0;
    int m_kills = 0;
    bool m_permanently_dead = false;
    float m_shoot_cd = 0.0f;
    float m_difficulty = 0.5f;
    glm::vec2 m_spawn{};
};

}

#endif