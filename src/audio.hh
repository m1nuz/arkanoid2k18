#pragma once

#include <optional>
#include <array>

#include "resources.hh"

namespace game {

    struct context_type;
    typedef context_type context_t;

} // namespace game

namespace audio {

    constexpr int MAX_AUDIO_VOLUME = 128;
    constexpr size_t MAX_AUDIO_SOURCES = 8;

} // namespace audio

#ifdef OPENAL_BACKEND

#include <AL/alc.h>
#include <AL/al.h>

namespace audio {

    inline namespace al_backend {

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

    } // namespace al_backend

} // namespace audio

namespace resources {

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

    typedef struct sound_type {
        sound_type() = default;

        uint32_t buffer = 0;
        int32_t size = 0;
        int32_t format = 0;
        int32_t frequency = 0;
    } sound_t;

} // namespace resources

#elif SDL_MIXER_BACKEND

#include <SDL2/SDL_audio.h>
#include <SDL2/SDL_mixer.h>

namespace resources {

    typedef Mix_Chunk* wave_t;

    typedef struct sound_type {
        sound_type() = default;

        wave_t chunk = nullptr;
    } sound_t;

} // namespace resources

namespace audio {

    inline namespace sdl_mixer_backend {

        typedef struct playback_type {
            resources::sound_t *sound = nullptr;
            uint8_t     *position = nullptr;
            uint32_t    lenght = 0;
            bool        enable = true;
            bool        playing = false;
            int volume = MAX_AUDIO_VOLUME;
        } playback_t;

        typedef struct context_type {
            context_type() = default;

            SDL_AudioDeviceID device;
        } context_t;

        auto init(game::context_t &ctx) -> std::optional<context_t>;
        auto cleanup(context_t &ctx) -> void;

        auto play_sound(context_t &ctx, const resources::sound_t &sound, const bool looped = false) -> void;

        auto enable_sound() -> void;
        auto disabel_sound() -> void;
        auto change_volume(context_t &ctx, int volume) -> void;

    } // namespace sdl_mixer_backend

} // namespace audio

#endif // SDL_MIXER_BACKEND

namespace resources {

    auto create_sound(const wave_t &wave) -> std::optional<sound_t>;

    auto destroy_sound(sound_t &snd) -> void;

} // namespace resources
