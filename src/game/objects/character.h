#ifndef SHOOTER_CHARACTER_H
#define SHOOTER_CHARACTER_H

#include "../core/object.h"
#include "../core/types.h"

struct TTF_Font;

namespace shooter
{

class Character : public Object
{
public:
    Character() = default;
    ~Character() override = default;

    int hp() const { return m_hp; }
    void set_hp(int v) { m_hp = v; }
    int max_hp() const { return m_max_hp; }
    void set_max_hp(int v) { m_max_hp = v; }
    int facing() const { return m_facing; }
    void set_facing(int v) { m_facing = v; }
    State state() const { return m_state; }
    void set_state(State s) { m_state = s; }
    const char *name() const { return m_name; }
    void set_name(const char *n) { m_name = n; }
    glm::vec4 color() const { return m_color; }
    void set_color(glm::vec4 c) { m_color = c; }

    int frame(int n) const { return m_frame % n; }
    void tick_anim(float dt, int frames)
    {
        m_timer += dt;
        if (m_timer >= 0.1f) { m_timer = 0.0f; m_frame = (m_frame + 1) % frames; }
    }

    void on_update(float dt) override {}
    void on_event() override {}
    void on_render() override {}
protected:
    int m_hp = 5;
    int m_max_hp = 5;
    int m_facing = 1;
    State m_state = State::Idle;
    const char *m_name = "";
    glm::vec4 m_color = BLUE;
    int m_frame = 0;
    float m_timer = 0.0f;
};

void draw_actor(float px, float py, float w, float h, float tex_w, float tex_h, SDL_Texture *tex);
void draw_hp(float x, float y, float w, float hp, float max_hp);
void draw_gun(float x, float y, int facing, const glm::vec4 &color);

}

#endif