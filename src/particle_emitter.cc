#include <GL/glcore.h>

#include "particle_emitter.hh"
#include "game.hh"

namespace game {

    auto create_emitter(resources::texture_t texture, const size_t amount) -> std::optional<particle_emitter> {
        particle_emitter pe;
        pe.texture = texture;
        pe.amount = amount;

        for (size_t i = 0; i < amount; ++i)
            pe.particles.push_back(particle_t{});

        return pe;
    }

    static auto find_unused_particle(particle_emitter &emitter) -> size_t {
        for (size_t i = emitter.last_used_particle; i < emitter.amount; ++i){
            if (emitter.particles[i].life <= 0.0f) {
                return emitter.last_used_particle = i;
            }
        }

        for (size_t i = 0; i < emitter.last_used_particle; ++i){
            if (emitter.particles[i].life <= 0.0f){
                return emitter.last_used_particle = i;
            }
        }

        return emitter.last_used_particle = 0;
    }

    static auto respawn_particle(particle_t &particle, const object &obj, const vec2 &offset) -> void {
        const auto rnd_pos = (random(0, 100) - 50) / 10.0f;
        const auto rnd_color = 0.5f + (random(0, 100) / 100.0f);
        particle.position = obj.position + rnd_pos + offset;
        particle.color = vec4{rnd_color, rnd_color, rnd_color, 1.0f};
        particle.life = 1.0f;
        particle.velocity = obj.velocity * 0.1f;
    }

    auto update_emitter(particle_emitter &emitter, const float dt, const object &obj, const size_t new_particles, const vec2 &offset) -> void {
        for (size_t i = 0; i < new_particles; ++i) {
            const auto unused_particle = find_unused_particle(emitter);
            respawn_particle(emitter.particles[unused_particle], obj, offset);
        }

        for (size_t i = 0; i < emitter.amount; i++) {
            auto &p = emitter.particles[i];
            p.life -= dt;
            if (p.life > 0.0f) {
                p.position -= p.velocity * dt;
                p.color.a -= dt * 2.5;
            }
        }
    }

    auto reset_emitter(particle_emitter &emitter) -> void {
        emitter.particles.clear();
        emitter.particles.resize(emitter.amount);
        emitter.last_used_particle = 0;
    }

} // namespace game
