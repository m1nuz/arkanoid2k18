#pragma once

#include <vector>
#include <string>
#include <optional>

namespace game {

    struct context_type;
    struct object;

    typedef context_type context_t;

    typedef struct level_type {
        level_type() = default;

        std::string name;
        bool is_complited = false;

        std::vector<object> bricks;
    } level_t;

    auto create_level(context_t &ctx, const std::string_view name, const std::vector<std::vector<uint8_t>> &tiles, const uint32_t level_w, const uint32_t level_h) -> std::optional<level_t>;
    auto load_levels(context_t &ctx, const std::string_view levels_path, const uint32_t level_w, const uint32_t level_h) -> std::vector<level_t>;

} // namespace game
