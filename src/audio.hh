#pragma once

#include <optional>
#include <array>

#include <AL/alc.h>

#include "resources.hh"

namespace game {

    struct context_type;
    typedef context_type context_t;

} // namespace game

namespace audio {

    constexpr int MAX_AUDIO_VOLUME = 128;
    constexpr size_t MAX_AUDIO_SOURCES = 8;

    typedef struct context_type {
        context_type() = default;

        ALCdevice    *audio_device;
        ALCcontext   *audio_context;
        int volume = MAX_AUDIO_VOLUME;

        std::array<uint32_t, MAX_AUDIO_SOURCES> sources;
    } context_t;

    auto init(game::context_t &ctx) -> std::optional<context_t>;
    auto cleanup(context_t &ctx) -> void;

    auto play_sound(context_t &ctx, const resources::sound_t &sound, const bool looped = false) -> void;

    auto enable_sound() -> void;
    auto disabel_sound() -> void;
    auto change_volume(context_t &ctx, int volume) -> void;
} // namespace audio
