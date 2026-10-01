#ifndef SHOOTER_PLAYER_H
#define SHOOTER_PLAYER_H

#include "bullet.h"
#include "character.h"
#include "weapon.h"
#include <vector>

namespace shooter
{

class Player : public Character
{
public:
    Player();
    void reset();
    void update(float dt, std::vector<Bullet> &bullets);
    void draw(TTF_Font *font) const;

    void set_weapon(int w) { m_weapon = w; }
    int weapon() const { return m_weapon; }
    void add_kill() { m_kills += 1; }
    int kills() const { return m_kills; }
    void set_lives(int v) { m_lives = v; }
    int lives() const { return m_lives; }
    void set_permanently_dead(bool v) { m_permanently_dead = v; }
    bool permanently_dead() const { return m_permanently_dead; }
    void set_respawn_cd(float v) { m_respawn_cd = v; }
    float respawn_cd() const { return m_respawn_cd; }
    void set_hurt(float v) { m_hurt_timer = v; }
    void set_attack(float v) { m_attack_timer = v; }
    void set_prefix(const char *p) { m_prefix = p; }
    void set_spawn(glm::vec2 p) { m_spawn = p; }
    void set_keys(SDL_Scancode l, SDL_Scancode r, SDL_Scancode u, SDL_Scancode s)
    {
        m_k_left = l; m_k_right = r; m_k_up = u; m_k_shoot = s;
    }
private:
    void shoot(float dt, std::vector<Bullet> &bullets);
    int m_weapon = 0;
    int m_kills = 0;
    int m_lives = 4;
    bool m_permanently_dead = false;
    float m_respawn_cd = 0.0f;
    float m_attack_timer = 0.0f;
    float m_hurt_timer = 0.0f;
    const char *m_prefix = "warrior";
    SDL_Scancode m_k_left = SDL_SCANCODE_A;
    SDL_Scancode m_k_right = SDL_SCANCODE_D;
    SDL_Scancode m_k_up = SDL_SCANCODE_W;
    SDL_Scancode m_k_shoot = SDL_SCANCODE_F;
    glm::vec2 m_spawn{};
    float m_clock = 0.0f;
    float m_last_shot = 0.0f;
    bool m_shoot_pressed = false;
};

}

#endif