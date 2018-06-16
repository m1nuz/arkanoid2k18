#pragma once

#include <optional>
#include <vector>

#include <glm/glm.hpp>

#include "resources.hh"

namespace game {

    struct context_type;
    typedef context_type context_t;

    struct sprite_type;
    typedef sprite_type sprite_t;

    struct particle_type;
    typedef particle_type particle_t;
    struct particle_emitter;

} // namespace game

namespace video {
    using glm::vec2;
    using glm::vec3;
    using glm::vec4;
    using glm::mat4;

    enum OPTIONS : uint32_t {
        OP_SHAKE = 1 << 0
    };

    typedef struct sprite_type {
        sprite_type() = default;

        vec2 position = {0.f, 0.f};
        vec2 size = {1.f, 1.f};
        float rotate = 0;
        resources::texture_t texture;
        vec3 color = {1.f, 1.f, 1.f};
    } sprite_t;

    typedef struct context_type {
        context_type() = default;

        std::vector<sprite_t> sprites;
        std::vector<game::particle_emitter> particles;
        resources::shader_t sprite_shader;
        resources::shader_t particle_shader;
        resources::shader_t postprocess_shader;
        uint32_t particle_va = 0;
        uint32_t sprite_va = 0;
        uint32_t screenquad_va = 0;
        uint32_t texture_sampler = 0;
        uint32_t sampled_fb = 0;
        uint32_t sampled_rb = 0;
        uint32_t color_fb = 0;
        uint32_t color_tex = 0;
        uint32_t options = 0;
    } context_t;

    auto init(game::context_t &ctx) -> std::optional<context_t>;
    auto present(const int w, const int h, const float ticks, context_t &ctx, const mat4 &proj, const mat4 &view) -> void;
    auto cleanup(context_t &ctx) -> void;

    auto draw_sprite(context_t &ctx, const resources::texture_t &texture, const vec2 &position, const vec2 &size = vec2{10, 10}, const float rotate = 0.0f, const glm::vec3 &color = vec3{1.0f}) -> void;
    auto draw_particles(context_t &ctx, const game::particle_emitter &emitter) -> void;
} // namespace video
