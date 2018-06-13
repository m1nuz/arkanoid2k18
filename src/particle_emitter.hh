#pragma once

#include <optional>
#include <glm/glm.hpp>

#include "resources.hh"

namespace game {
    using glm::vec2;
    using glm::vec3;
    using glm::vec4;

    struct object;

    typedef struct particle_type {
        particle_type() = default;

        vec2 position = {0.f, 0.f};
        vec2 velocity = {0.f, 0.f};
        vec4 color = {1.f, 1.f, 1.f, 1.f};
        float life = 0.f;
    } particle_t;

    struct particle_emitter {
        particle_emitter() = default;

        std::vector<particle_t> particles;
        resources::texture_t texture;
        size_t amount = 0;
        size_t last_used_particle = 0;
    };

    auto create_emitter(resources::texture_t texture, const size_t amount) -> std::optional<particle_emitter>;
    auto update_emitter(particle_emitter &emitter, const float dt, const object &obj, const size_t new_particles, const vec2 &offset) -> void;
    auto reset_emitter(particle_emitter &emitter) -> void;
} // namespace game
