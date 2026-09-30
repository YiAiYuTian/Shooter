#ifndef UTILS_H
#define UTILS_H

#include <glm/glm.hpp>

namespace shooter
{

bool is_in_rect(glm::vec2 pos, glm::vec4 rect);
bool aabb_overlap(float ax, float ay, float aw, float ah, float bx, float by, float bw, float bh);

}    

#endif // !UTILS_H
