#pragma once

#include <tuple>
#include <glm/glm.hpp>

namespace game {
    using glm::vec2;
    using glm::vec3;

    struct context_type;
    typedef context_type context_t;

    struct object;
    struct ball_object;

    typedef enum direction_type {
        UP,
        RIGHT,
        DOWN,
        LEFT
    } direction_t;

    typedef std::tuple<bool, direction_t, vec2> collision_t;

    auto check_collison(const object &one, const object &two) -> bool;
    auto check_collison(const ball_object &one, const object &two) -> collision_t;
} // namespace game
