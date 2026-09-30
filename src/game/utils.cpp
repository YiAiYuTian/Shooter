#include "utils.h"

namespace shooter
{

bool is_in_rect(glm::vec2 pos, glm::vec4 rect)
{
    float left   = rect.x;
    float top    = rect.y;
    float right  = rect.x + rect.z;
    float bottom = rect.y + rect.w;

    return pos.x >= left  && pos.x <= right &&
           pos.y >= top   && pos.y <= bottom;
}

bool aabb_overlap(float ax, float ay, float aw, float ah, float bx, float by, float bw, float bh)
{
    return ax < bx + bw && ax + aw > bx && ay < by + bh && ay + ah > by;
}

}
