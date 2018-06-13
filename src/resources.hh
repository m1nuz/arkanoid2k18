#pragma once

#include <cstdint>
#include <string>
#include <optional>
#include <vector>
#include <unordered_map>

namespace game {

    struct context_type;
    typedef context_type context_t;

} // namespace game

namespace resources {

    typedef struct uniform_type {
        uniform_type() = default;

        int32_t         location = -1;
        int32_t         num = 0;
        uint32_t        type = 0;
    } uniform_t;

    typedef struct attribute_type {
        attribute_type() = default;

        int32_t         location = -1;
        int32_t         num = 0;
        uint32_t        type = 0;
    } attribute_t;

    typedef struct shader_type {
        shader_type() = default;

        uint32_t id = 0;

        std::unordered_map<std::string, uniform_t> uniforms;
        std::unordered_map<std::string, attribute_t> attributes;        
    } shader_t;

    typedef struct texture_type {
        texture_type() = default;

        uint32_t id = 0;
        uint32_t target = 0;
        uint32_t width = 0;
        uint32_t height = 0;
        uint32_t depth = 0;
    } texture_t;

    enum class pixel_format : uint32_t {
        unknown,
        r8,
        rg8,
        rgb8,
        rgba8,
        bgr8,
        bgra8,
        r16f,
        r32f,
        rgb16f,
        rgba16f,
        rgb32f,
        rgba32f,
        depth
    };

    typedef struct image_type {
        image_type() = default;

        uint32_t width = 0;
        uint32_t height = 0;
        uint32_t depth = 0;
        pixel_format format = pixel_format::unknown;
        std::vector<uint8_t> pixels;
    } image_t;

    typedef struct sound_type {
        sound_type() = default;

        uint32_t buffer = 0;
        int32_t size = 0;
        int32_t format = 0;
        int32_t frequency = 0;
    } sound_t;

    enum class audio_format : uint32_t {
        unknown,
        mono8,
        mono16,
        stereo8,
        stereo16
    };

    typedef struct wave_type {
        wave_type() = default;

        int32_t frequency = 0;
        uint32_t size = 0;
        audio_format format = audio_format::unknown;
        std::vector<uint8_t> bytes;
    } wave_t;

    auto init(game::context_t &ctx, const std::string_view assets_path) -> bool;
    auto cleanup(game::context_t &ctx) -> void;

    auto get_shader(const game::context_t &ctx, const std::string_view name) -> std::optional<shader_t>;
    auto get_texture(const game::context_t &ctx, const std::string_view name) -> std::optional<texture_t>;
    auto get_sound(const game::context_t &ctx, const std::string_view name) -> std::optional<sound_t>;

} // namespace resources
