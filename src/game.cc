
#include <cstdint>
#include <variant>
#include <algorithm>
#include <optional>
#include <fstream>

#include <SDL2/SDL.h>
#include <GL/glcore.h>
#include <json.hpp>

#include "config.hh"
#include "journal.hh"
#include "game.hh"
#include "resources.hh"
#include "video.hh"
#include "audio.hh"
#include "collisions.hh"
#include "particle_emitter.hh"

using json = nlohmann::json;

namespace game {

    auto move_ball(ball_object& ball, const float dt, const float window_width) -> vec2 {
        if (!ball.is_stuck) {
            ball.position += ball.velocity * dt;

            if (ball.position.x <= 0.0f) {
                ball.velocity.x = -ball.velocity.x;
                ball.position.x = 0.0f;
            } else if (ball.position.x + ball.size.x >= window_width) {
                ball.velocity.x = -ball.velocity.x;
                ball.position.x = window_width - ball.size.x;
            } if (ball.position.y <= 0.0f) {
                ball.velocity.y = -ball.velocity.y;
                ball.position.y = 0.0f;
            }
        }

        return ball.position;
    }

    auto reset_ball(context_t &ctx, ball_object& ball) -> void {
        ball.position = ctx.player.position + vec2(PLAYER_SIZE.x / 2 - BALL_RADIUS, -BALL_RADIUS * 2.f);
        ball.size = vec2{BALL_RADIUS * 2.f};
        ball.radius = BALL_RADIUS;
        ball.velocity = INITIAL_BALL_VELOCITY;
        ball.color = vec3{1.f};
        ball.is_stuck = true;
        ball.is_sticky = false;
        ball.is_pass_through = false;

        ctx.particles.particles.clear();
        ctx.particles.last_used_particle = 0;
    }

    auto reset_player(context_t &ctx, object& player) {
        player.position = vec2{ctx.width / 2.f - PLAYER_SIZE.x / 2.f, ctx.height - PLAYER_SIZE.y};
        player.size = PLAYER_SIZE;
        player.color = vec3{1.f};
    }

    auto spawn_powerups(context_t &ctx, object& target) {
        if (random(1, 100) < 15) {
            powerup_object p;
            p.type = powerup_t::speed;
            p.color = vec3{0.5f, 0.5f, 1.0f};
            p.position = target.position;
            p.duration = 0.f;
            p.size = POWERUP_SIZE;
            p.velocity = POWERUP_VELOCITY;
            if (auto tex = resources::get_texture(ctx, "speed"); tex)
                p.texture = tex.value();

            ctx.powerups.push_back(p);
            return;
        }

        if (random(1, 100) < 15) {
            powerup_object p;
            p.type = powerup_t::sticky;
            p.color = vec3{1.0f, 0.5f, 1.0f};
            p.position = target.position;
            p.duration = 20.f;
            p.size = POWERUP_SIZE;
            p.velocity = POWERUP_VELOCITY;
            if (auto tex = resources::get_texture(ctx, "sticky"); tex)
                p.texture = tex.value();

            ctx.powerups.push_back(p);
            return;
        }

        if (random(1, 100) < 15) {
            powerup_object p;
            p.type = powerup_t::pass_through;
            p.color = vec3{0.5f, 1.0f, 0.5f};
            p.position = target.position;
            p.duration = 10.f;
            p.size = POWERUP_SIZE;
            p.velocity = POWERUP_VELOCITY;
            if (auto tex = resources::get_texture(ctx, "passthrough"); tex)
                p.texture = tex.value();

            ctx.powerups.push_back(p);
            return;
        }

        if (random(1, 100) < 15) {
            powerup_object p;
            p.type = powerup_t::pad_size_increase;
            p.color = vec3{1.0f, 0.6f, 0.4};
            p.position = target.position;
            p.duration = 0.f;
            p.size = POWERUP_SIZE;
            p.velocity = POWERUP_VELOCITY;
            if (auto tex = resources::get_texture(ctx, "size-increase"); tex)
                p.texture = tex.value();

            ctx.powerups.push_back(p);
            return;
        }
    }

    auto activate_powerup(context_t &ctx, powerup_object &powerup) {
        switch (powerup.type) {
        case powerup_t::speed:
            ctx.ball.velocity *= 1.2f;
            break;
        case powerup_t::sticky:
            ctx.ball.is_sticky = true;
            ctx.player.color = vec3{1.0f, 0.5f, 1.0f};
            break;
        case powerup_t::pass_through:
            ctx.ball.is_pass_through = true;
            ctx.ball.color = vec3{1.0f, 0.5f, 0.5f};
            break;
        case powerup_t::pad_size_increase:
            ctx.player.size.x += 50.f;
            break;
        default:
            break;
        }
    }

    auto do_collisions(context_t &ctx, audio::context_t &atx) {
        using namespace glm;

        auto &level = ctx.level;
        auto &player = ctx.player;
        auto &ball = ctx.ball;

        for (auto &box : level.bricks) {
            if (!box.is_destroyed) {
                auto [overlapped, dir, diff_vector] = check_collison(ball, box);

                if (overlapped) {
                    if (!box.is_solid) {
                        box.is_destroyed = true;
                        spawn_powerups(ctx, box);
                        auto snd = resources::get_sound(ctx, "bleep");
                        if (snd)
                            audio::play_sound(atx, snd.value());

                    } else {
                        ctx.shake_time = 0.5f;
                        ctx.render_options |= video::OP_SHAKE;

                        auto snd = resources::get_sound(ctx, "solid");
                        if (snd)
                            audio::play_sound(atx, snd.value());
                    }

                    if (!(!box.is_solid && ball.is_pass_through)) {
                        if (dir == direction_t::LEFT || dir == direction_t::RIGHT) {
                            ball.velocity.x = -ball.velocity.x;
                            const auto penetration = ball.radius - std::abs(diff_vector.x);
                            if (dir == direction_t::LEFT)
                                ball.position.x += penetration;
                            else
                                ball.position.x -= penetration;
                        } else {
                            ball.velocity.y = -ball.velocity.y;
                            const auto penetration = ball.radius - std::abs(diff_vector.y);
                            if (dir == direction_t::UP)
                                ball.position.y -= penetration;
                            else
                                ball.position.y += penetration;
                        }
                    }
                }
            }
        }

        {
            auto [overlapped, dir, diff_vector] = check_collison(ball, player);
            (void)dir, (void)diff_vector;
            if (overlapped && !ball.is_stuck) {
                const float center_board = player.position.x + player.size.x / 2;
                const float distance = (ball.position.x + ball.radius) - center_board;
                const float percentage = distance / (player.size.x / 2);

                const float strength = 2.0f;
                const auto old_velocity = ball.velocity;
                ball.velocity.x = INITIAL_BALL_VELOCITY.x * percentage * strength;
                //ball.velocity.y = -ball.velocity.y;
                ball.velocity = normalize(ball.velocity) * length(old_velocity);
                ball.velocity.y = -1.0 * std::abs(ball.velocity.y);
                ball.is_stuck = ball.is_sticky;
            }
        }

        for (auto &powerup : ctx.powerups) {
            if (!powerup.is_destroyed) {
                if (powerup.position.y > ctx.height)
                    powerup.is_destroyed = true;

                if (check_collison(player, powerup)) {
                    activate_powerup(ctx, powerup);
                    powerup.is_activated = true;
                    powerup.is_destroyed = true;

                    auto snd = resources::get_sound(ctx, "powerup");
                    if (snd)
                        audio::play_sound(atx, snd.value());
                }
            }
        }
    }

    auto is_same_powerup_active(const std::vector<powerup_object> &powerups, const powerup_t type) {
        for (const auto &p : powerups) {
            if (p.type == type && p.is_activated)
                return true;
        }

        return false;
    }

    auto update_powerups(context_t &ctx, const float dt) -> void {
        for (auto &powerup : ctx.powerups) {
            powerup.position += powerup.velocity * dt;

            if (powerup.is_activated) {
                powerup.duration -= dt;

                if (powerup.duration <= 0.f) {
                    powerup.is_activated = false;

                    switch (powerup.type) {
                    case powerup_t::speed:
                        if (!is_same_powerup_active(ctx.powerups, powerup.type)) {
                            ctx.ball.is_sticky = false;
                            ctx.player.color = vec3{1.f};
                        }
                        break;
                    case powerup_t::sticky:
                        if (!is_same_powerup_active(ctx.powerups, powerup.type)) {
                            ctx.ball.is_pass_through = false;
                            ctx.player.color = vec3{1.f};
                        }
                        break;
                    case powerup_t::pass_through:
                        break;
                    case powerup_t::pad_size_increase:
                        break;
                    }
                }
            }
        }

        ctx.powerups.erase(std::remove_if(ctx.powerups.begin(), ctx.powerups.end(), [] (const auto& powerup) {
            return powerup.is_destroyed && !powerup.is_activated;
        }), ctx.powerups.end());
    }

    auto init(const std::string_view conf_path, const bool debug) -> std::optional<context_t> {
        using namespace std;

        if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
            journal::critical("Unable to initialize SDL: %1", SDL_GetError());
            return {};
        }

        atexit(SDL_Quit);

        auto contents = get_config(conf_path);
        if (!contents) {
            journal::critical("%1", "Can't read game config");
            return {};
        }

        auto j = json::parse(contents.value());

        if (j.find("video") == j.end()) {
            journal::critical("%1", "Can't read video settings from config");
            return {};
        }

        auto video_conf = j["video"];

        const auto window_width = (video_conf.find("width") != video_conf.end()) ? video_conf["width"].get<int>() : 1024;
        const auto window_height = (video_conf.find("height") != video_conf.end()) ? video_conf["height"].get<int>() : 768;

        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG | (debug ? SDL_GL_CONTEXT_DEBUG_FLAG : 0));

        const auto window = SDL_CreateWindow(GAME_TITLE, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, window_width, window_height, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
        if (!window) {
            journal::critical("%1 %2", "Create window error: ", SDL_GetError());
            return {};
        }

        const auto graphic = SDL_GL_CreateContext(window);
        if (!graphic) {
            journal::critical("%1", "Init graphics error");
            return {};
        }

        glLoadFunctions();
        glLoadExtensions();

        context_t ctx;
        ctx.window = window;
        ctx.graphic = graphic;

        SDL_GetWindowSize(window, &ctx.width, &ctx.height);

        return ctx;
    }

    auto start(context_t &ctx) -> bool {
        if (!resources::init(ctx, GAME_ASSETS_PATH)) {
            journal::critical("%1", "Init resources error");
            return false;
        }

        const auto levels = load_levels(ctx, GAME_LEVELS_PATH, ctx.width, ctx.height * 0.5f);
        if (levels.empty()) {
            journal::critical("%1", "Couldn't load levels");
            return false;
        }

        ctx.levels = levels;

        const auto player_tex = resources::get_texture(ctx, "paddle");
        if (!player_tex) {
            journal::critical("%1", "Couldn't init player");
            return false;
        }
        ctx.current_level = 0;
        ctx.level = levels[ctx.current_level];

        ctx.player.texture = player_tex.value();
        ctx.player.position = vec2{ctx.width / 2.f - PLAYER_SIZE.x / 2.f, ctx.height - PLAYER_SIZE.y};
        ctx.player.size = PLAYER_SIZE;

        const auto ball_tex = resources::get_texture(ctx, "ball");
        if (!ball_tex) {
            journal::critical("%1", "Couldn't init ball");
            return false;
        }

        ctx.ball.position = ctx.player.position + vec2(PLAYER_SIZE.x / 2 - BALL_RADIUS, -BALL_RADIUS * 2.f);
        ctx.ball.size = vec2{BALL_RADIUS * 2.f};
        ctx.ball.radius = BALL_RADIUS;
        ctx.ball.velocity = INITIAL_BALL_VELOCITY;
        ctx.ball.texture = ball_tex.value();

        const auto particle_tex = resources::get_texture(ctx, "particle");
        if (!particle_tex) {
            journal::critical("%1", "Couldn't init particles");
            return false;
        }

        const auto particles = create_emitter(particle_tex.value(), 500);
        if (!particles) {
            journal::critical("%1", "Couldn't init particles");
            return false;
        }

        ctx.particles = particles.value();

        return true;
    }

    auto process_events(context_t &ctx, const float dt) -> void {
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            if (ev.key.keysym.sym == SDLK_ESCAPE)
                ctx.running = false; // Exit when esc

            if (ctx.state == state_t::active) {
                const auto velocity = PLAYER_VELOCITY * dt;
                auto &player = ctx.player;
                auto &ball = ctx.ball;

                switch (ev.key.keysym.sym) {
                case SDLK_LEFT:
                case SDLK_a:
                    if (player.position.x >= 0) {
                        player.position.x -= velocity;

                        if (ball.is_stuck)
                            ball.position.x -= velocity;
                    }
                    break;
                case SDLK_RIGHT:
                case SDLK_d:
                    if (player.position.x <= ctx.width - player.size.x) {
                        player.position.x += velocity;

                        if (ball.is_stuck)
                            ball.position.x += velocity;
                    }
                    break;
                case SDLK_SPACE:
                    ball.is_stuck = false;
                    break;
                }
            }
        }
    }

    auto update(context_t &ctx, audio::context_t &atx, const float dt) -> void {
        move_ball(ctx.ball, dt, ctx.width);
        do_collisions(ctx, atx);

        update_emitter(ctx.particles, dt, ctx.ball, ctx.ball.is_stuck ? 0 : 1, vec2{ctx.ball.radius/2});

        update_powerups(ctx, dt);

        if (ctx.ball.position.y >= ctx.height) {
            reset_player(ctx, ctx.player);
            reset_ball(ctx, ctx.ball);
            reset_emitter(ctx.particles);
            ctx.powerups.clear();
            ctx.level = ctx.levels[ctx.current_level];
        }

        if (ctx.shake_time > 0.f) {
            ctx.shake_time -= dt;

            if (ctx.shake_time <= 0.f)
                ctx.render_options &= ~video::OP_SHAKE;
        }
    }

    auto draw(context_t &ctx, video::context_t &gtx) -> void {
        if (ctx.state == state_t::active) {
            /*const auto background_tex = resources::get_texture(ctx, "background");
            if (background_tex)
                video::draw_sprite(gtx, background_tex.value(), {0, 0}, {ctx.width, ctx.height}, 0.0f, {1.0f, 1.0f, 1.0f});*/

            const auto &level = ctx.level;
            for (const auto& sp : level.bricks) {
                if (!sp.is_destroyed)
                    video::draw_sprite(gtx, sp.texture, sp.position, sp.size, sp.rotate, sp.color);
            }

            const auto &player = ctx.player;
            video::draw_sprite(gtx, player.texture, player.position, player.size, player.rotate, player.color);

            for (const auto &powerup : ctx.powerups)
                if (!powerup.is_destroyed)
                    video::draw_sprite(gtx, powerup.texture, powerup.position, powerup.size, 0.f, powerup.color);

            const auto &particles = ctx.particles;
            video::draw_particles(gtx, particles);

            const auto &ball = ctx.ball;
            video::draw_sprite(gtx, ball.texture, ball.position, ball.size, ball.rotate, ball.color);
        }
    }

    auto cleanup(context_t &ctx) -> void {
        resources::cleanup(ctx);

        SDL_GL_DeleteContext(ctx.graphic);
        SDL_DestroyWindow(ctx.window);
    }

} // namespace game
