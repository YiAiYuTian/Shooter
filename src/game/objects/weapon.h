#ifndef SHOOTER_WEAPON_H
#define SHOOTER_WEAPON_H

#include "../core/object.h"
#include "../core/types.h"

namespace shooter
{

class Weapon : public Object
{
public:
    Weapon() = default;
    Weapon(const char *n, int dmg_, float rate_ms_, float speed_,
           bool auto_, float recoil_, bool expl, glm::vec4 col)
        : m_name(n), m_dmg(dmg_), m_rate_ms(rate_ms_), m_speed(speed_),
          m_auto(auto_), m_recoil(recoil_), m_explosion(expl), m_color(col) {}

    const char *name() const { return m_name; }
    int dmg() const { return m_dmg; }
    float rate_ms() const { return m_rate_ms; }
    float speed() const { return m_speed; }
    bool auto_fire() const { return m_auto; }
    float recoil() const { return m_recoil; }
    bool explosion() const { return m_explosion; }
    glm::vec4 color() const { return m_color; }

    void on_update(float dt) override {}
    void on_event() override {}
    void on_render() override {}
private:
    const char *m_name = "";
    int m_dmg = 0;
    float m_rate_ms = 0.0f;
    float m_speed = 0.0f;
    bool m_auto = false;
    float m_recoil = 0.0f;
    bool m_explosion = false;
    glm::vec4 m_color = WHITE;
};

inline const Weapon WEAPONS[6] = {
    { "手枪",   1, 400.0f, 720.0f,  false, 0.0f,   false, YELLOW },
    { "步枪",   1, 250.0f, 1080.0f, false, 0.0f,   false, GREEN  },
    { "冲锋枪", 1, 90.0f,  840.0f,  true,  0.0f,   false, CYAN   },
    { "机枪",   1, 50.0f,  780.0f,  true,  0.0f,   false, ORANGE },
    { "加特林", 1, 28.0f,  780.0f,  true,  240.0f, false, PURPLE },
    { "火箭弹", 5, 800.0f, 420.0f,  false, 0.0f,   true,  RED    },
};

}

#endif