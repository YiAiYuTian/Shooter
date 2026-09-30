#ifndef SHOOTER_TYPES_H
#define SHOOTER_TYPES_H

#include <SDL3/SDL.h>
#include <glm/glm.hpp>

namespace shooter
{

enum class Mode { Pvp = 0, Zombie = 1, Coop = 2, Stage = 3 };
enum class State { Idle, Run, Air, Attack, Hurt, Dead };

constexpr glm::vec4 WHITE    = { 1.0f, 1.0f, 1.0f, 1.0f };
constexpr glm::vec4 YELLOW   = { 1.0f, 0.863f, 0.196f, 1.0f };
constexpr glm::vec4 GREEN    = { 0.196f, 0.784f, 0.196f, 1.0f };
constexpr glm::vec4 CYAN     = { 0.196f, 0.784f, 0.863f, 1.0f };
constexpr glm::vec4 ORANGE   = { 0.941f, 0.627f, 0.157f, 1.0f };
constexpr glm::vec4 PURPLE   = { 0.706f, 0.235f, 0.863f, 1.0f };
constexpr glm::vec4 RED      = { 0.863f, 0.196f, 0.196f, 1.0f };
constexpr glm::vec4 BLUE     = { 0.196f, 0.392f, 0.863f, 1.0f };
constexpr glm::vec4 GRAY     = { 0.627f, 0.627f, 0.627f, 1.0f };
constexpr glm::vec4 BG       = { 0.078f, 0.078f, 0.137f, 1.0f };
constexpr glm::vec4 PANEL    = { 0.078f, 0.078f, 0.157f, 1.0f };
constexpr glm::vec4 PLAT_BODY= { 0.275f, 0.275f, 0.353f, 1.0f };
constexpr glm::vec4 PLAT_TOP = { 0.392f, 0.392f, 0.510f, 1.0f };
constexpr glm::vec4 PLAT_EDGE= { 0.549f, 0.549f, 0.627f, 1.0f };
constexpr glm::vec4 WALL_BODY= { 0.353f, 0.275f, 0.235f, 1.0f };
constexpr glm::vec4 WALL_LIT = { 0.471f, 0.392f, 0.314f, 1.0f };
constexpr glm::vec4 WALL_DARK= { 0.275f, 0.196f, 0.157f, 1.0f };

}

#endif
