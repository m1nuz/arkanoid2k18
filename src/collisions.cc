#include "game.hh"
#include "collisions.hh"

namespace game {

    /*direction_t get_vector_direction(const vec2 &target) {
        using namespace glm;

        const vec2 compass[] = {
            vec2{0.0f, 1.0f},	// up
            vec2{1.0f, 0.0f},	// right
            vec2{0.0f, -1.0f},	// down
            vec2{-1.0f, 0.0f}	// left
        };

        float max = 0.0f;
        int best_match = -1;

        for (int i = 0; i < 4; i++) {
            const auto dot_product = dot(normalize(target), compass[i]);

            if (dot_product > max) {
                max = dot_product;
                best_match = i;
            }
        }

        return static_cast<direction_t>(best_match);
    }

    auto check_collison(const object &one, const object &two) -> bool {
        const bool is_axis_x = one.position.x + one.size.x >= two.position.x && two.position.x + two.size.x >= one.position.x;
        const bool is_axis_y = one.position.y + one.size.y >= two.position.y && two.position.y + two.size.y >= one.position.y;
        return is_axis_x && is_axis_y;
    }

    auto check_collison(const ball_object &one, const object &two) -> collision_t {
        using namespace glm;

        const auto center = vec2{one.position + one.radius};
        const auto aabb_half_extents = vec2{two.size.x / 2, two.size.y / 2};
        const auto aabb_center = vec2{two.position.x + aabb_half_extents.x, two.position.y + aabb_half_extents.y};

        const auto difference = center - aabb_center;
        const auto clamped = clamp(difference, -aabb_half_extents, aabb_half_extents);
        const auto closest = aabb_center + clamped;

        const auto diff = closest - center;

        if (length(diff) <= one.radius)
            return std::make_tuple(true, get_vector_direction(diff), diff);
        else
            return std::make_tuple(false, direction_t::UP, diff);
    }*/

} // namespace game
