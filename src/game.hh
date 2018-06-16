#pragma once

#include <variant>
#include <unordered_map>
#include <fstream>

#include <SDL2/SDL.h>
#include <glm/glm.hpp>

#include "resources.hh"
#include "particle_emitter.hh"
#include "level.hh"
#include "utils.hh"
#include "physics.hh"

namespace video {

    struct context_type;
    typedef context_type context_t;

} // namespace video

namespace audio {

    struct context_type;
    typedef context_type context_t;

} // namespace audio

namespace game {

    using glm::vec2;
    using glm::vec3;
    using glm::vec4;

    typedef struct sprite_type {
        sprite_type() = default;

//        vec2 position = {0.f, 0.f};
//        vec2 size = {1.f, 1.f};
//        float rotate = 0;

        physics::body body;

        resources::texture_t texture;
        vec3 color = {1.f, 1.f, 1.f};
    } sprite_t;

    struct object : public sprite_type {
        object() = default;

        //vec2 velocity = {0.f, 0.f};

        bool is_solid = false;
        bool is_destroyed = false;
    };

    struct ball_object : public object {
        float radius = 0.0f;
        bool is_stuck = true;
        bool is_sticky = false;
        bool is_pass_through = false;
    };

    enum class powerup_t {
        speed,
        sticky,
        pass_through,
        pad_size_increase
    };

    struct powerup_object : public object {
        powerup_t type = powerup_t::speed;
        float duration = 0.f;
        bool is_activated = false;
    };    

    constexpr auto timestep = 0.01f;

    enum class state_t {
        active,
        main_menu,
        win
    };

    typedef struct context_type {
        SDL_Window *window = nullptr;
        SDL_GLContext graphic = nullptr;

        state_t state = state_t::active;

        std::unordered_map<std::string, resources::shader_t> shaders;
        std::unordered_map<std::string, resources::texture_t> textures;
        std::unordered_map<std::string, resources::sound_t> sounds;

        std::vector<level_t> levels;
        size_t current_level = 0;
        level_t level;
        object player;
        ball_object ball;
        particle_emitter particles;
        std::vector<powerup_object> powerups;
        float shake_time = 0.0f;

        uint32_t render_options = 0;

        int width = 0;
        int height = 0;
        bool running = true;
    } context_t;


    auto init(const std::string_view conf_path, const bool debug) -> std::optional<context_t>;
    auto start(context_t &ctx) -> bool;
    auto process_events(context_t &ctx, const float dt) -> void;
    auto update(context_t &ctx, audio::context_t &atx, const float dt) -> void;
    auto draw(context_t &ctx, video::context_t &gtx, const float interpolation) -> void;
    auto cleanup(context_t &ctx) -> void;
} // namespace game
