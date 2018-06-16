#include <json.hpp>

#include "game.hh"
#include "journal.hh"
#include "level.hh"

using json = nlohmann::json;

namespace game {
    using glm::vec2;
    using glm::vec3;
    using glm::vec4;

    enum TILES_TYPES {
        SOLID_TILE = 1,
        TILE_1 = 2,
        TILE_2 = 3,
        TILE_3 = 4,
        TILE_4 = 5
    };

    auto create_level(context_t &ctx, const std::string_view name, const std::vector<std::vector<uint8_t>> &tiles, const uint32_t level_w, const uint32_t level_h) -> std::optional<level_t> {
        if (tiles.empty())
            return {};

        const auto height = tiles.size();
        const auto width = tiles[0].size();

        if (height == 0)
            return {};

        if (width == 0)
            return {};

        level_t level;
        level.name = name;

        const auto unit_width = static_cast<float>(level_w) / static_cast<float>(width);
        const auto unit_height = static_cast<float>(level_h) / height;

        for (size_t y = 0; y < height; ++y) {
            for (size_t x = 0; x < width; ++x) {
                if (tiles[y][x] == SOLID_TILE) {
                    const auto pos = vec2{unit_width * x, unit_height * y};
                    const auto size = vec2{unit_width, unit_height} * 0.995f;

                    object obj;
                    obj.texture = resources::get_texture(ctx, "block_solid").value_or(resources::texture_t{});
                    obj.body.current.position = pos;
                    obj.body.current.size = size;
                    obj.color = vec3{0.8f, 0.8f, 0.7f};
                    obj.is_solid = true;

                    level.bricks.push_back(obj);
                } else if (tiles[y][x] > 1) {
                    const auto get_color = [] (const auto value) {
                        if (value == TILE_1)
                            return vec3{0.2f, 0.6f, 1.0f};
                        if (value == TILE_2)
                            return vec3{0.0f, 0.7f, 0.0f};
                        if (value == TILE_3)
                            return vec3{0.8f, 0.8f, 0.4f};
                        if (value == TILE_4)
                            return vec3{1.0f, 0.5f, 0.0f};

                        return vec3{1.f};
                    };

                    const auto pos = vec2{unit_width * x, unit_height * y};
                    const auto size = vec2{unit_width, unit_height} * 0.995f;

                    object obj;
                    obj.texture = resources::get_texture(ctx, "block").value_or(resources::texture_t{});
                    obj.body.current.position = pos;
                    obj.body.current.size = size;
                    obj.color = get_color(tiles[y][x]);

                    level.bricks.push_back(obj);
                }
            }
        }

        return level;
    }

    auto load_levels(context_t &ctx, const std::string_view levels_path, const uint32_t level_w, const uint32_t level_h) -> std::vector<level_t> {
        using namespace std;

        auto contents = get_config(levels_path);
        if (!contents) {
            journal::critical("%1", "Can't read levels config");
            return {};
        }

        auto j = json::parse(contents.value());

        if (j.find("levels") == j.end())
            return {};

        vector<level_t> all_levels;

        for (auto& level : j["levels"]) {
            const auto name = level.find("name") != level.end() ? level["name"].get<string>() : string{};
            const auto enable = level.find("enable") != level.end() ? level["enable"].get<bool>() : false;

            if (!enable)
                continue;

            if (level.find("data") != level.end()) {
                vector<vector<uint8_t>> tiles;

                for (auto& d : level["data"]) {
                    const auto line = d.get<vector<uint8_t>>();
                    tiles.push_back(line);
                }

                if (const auto l = create_level(ctx, name, tiles, level_w, level_h); l) {
                    journal::info("Level '%1' loaded", name);
                    all_levels.push_back(l.value());
                }

            } else {
                journal::warning("Data for level '%1' not found!", name);
            }
        }

        return all_levels;
    }
} // namespace game
