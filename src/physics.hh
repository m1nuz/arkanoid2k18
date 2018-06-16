#pragma once

#include "utils.hh"

namespace physics {
    using glm::vec2;
    using glm::vec3;
    using glm::vec4;

    namespace detail {

        struct state {
            state() = default;

            vec2 position = {0.f, 0.f};
            vec2 velocity = {0.f, 0.f};
            //float orientation = {0.f};
            float rotate = 0.f;
            vec2 size = {1.f, 1.f};

            float acceleration = {0.f};
        };

        struct derivative {
            derivative() = default;

            vec2 position = {0.f, 0.f};
            vec2 velocity = {0.f, 0.f};
        };

    } // namespace detail

    struct body {
        detail::state current;
        detail::state previous;
    };

    inline detail::state interpolate(const detail::state &current, const detail::state &previous, const float alpha) {
        using namespace glm;

        detail::state state;

        state.position = mix(previous.position, current.position, alpha);
        //state.orientation = mix(previous.orientation, current.orientation, alpha);
        state.rotate = mix(previous.rotate, current.rotate, alpha);
        state.size = mix(previous.size, current.size, alpha);

        return state;
    }

    inline vec2 accelerate(const detail::state &state) {
        return state.velocity * state.acceleration;
    }

    inline detail::derivative evaluate0(const detail::state &initial) {
        detail::derivative d;
        d.position = initial.velocity;

        auto a = accelerate(initial);

        d.velocity = a;

        return d;
    }

    inline detail::derivative evaluate1(const detail::state &initial, const float dt, const detail::derivative &d) {
        detail::state state;
        state.position = initial.position + d.position * vec2{dt};
        state.velocity = initial.velocity + d.velocity * vec2{dt};

        detail::derivative result;
        result.position = state.velocity;

        auto a = accelerate(initial);
        result.velocity = a;

        result.velocity *= 0.1f;

        return result;
    }

    inline void integrate(detail::state &state, const float dt) {
        detail::derivative a = evaluate0(state);
        detail::derivative b = evaluate1(state, dt + 0.5f, a);
        detail::derivative c = evaluate1(state, dt + 0.5f, b);
        detail::derivative d = evaluate1(state, dt, c);

        const float dxdt = 1.f / 6.f * (a.position.x + 2.f * (b.position.x + c.position.x) + d.position.x);
        const float dydt = 1.f / 6.f * (a.position.y + 2.f * (b.position.y + c.position.y) + d.position.y);

        const float dvdt = 1.f / 6.f * (a.velocity.x + 2.f * (b.velocity.x + c.velocity.x) + d.velocity.x);
        const float dwdt = 1.f / 6.f * (a.velocity.y + 2.f * (b.velocity.y + c.velocity.y) + d.velocity.y);

        state.position += vec2{dxdt * dt, dydt * dt};
        state.velocity += vec2{dvdt * dt, dwdt * dt};
    }

    inline void add_impulse(body &b, const vec2 v) {
        b.current.velocity += v;
    }

} // namespace physics
