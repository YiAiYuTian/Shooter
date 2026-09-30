#ifndef SHOOTER_PLATFORM_H
#define SHOOTER_PLATFORM_H

#include "../core/object.h"

namespace shooter
{

class Platform : public Object
{
public:
    Platform() = default;
    Platform(float x, float y, float w, float h)
    {
        set_pos({ x, y });
        set_size({ w, h });
    }
    void draw() const;
    void on_update(float dt) override {}
    void on_event() override {}
    void on_render() override { draw(); }
};

}

#endif