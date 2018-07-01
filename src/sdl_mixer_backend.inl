namespace audio {
    inline namespace sdl_mixer_backend {

        auto init(game::context_t &ctx) -> std::optional<context_t>  {
            (void)ctx;

            context_t a;

            if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 4096) == -1) {
                journal::critical("%1", SDL_GetError());
                return {};
            }

            return a;
        }

        auto cleanup(context_t &ctx) -> void {
            Mix_CloseAudio();
        }

        auto play_sound(context_t &ctx, const resources::sound_t &sound, const bool looped) -> void {
            if (Mix_PlayChannel(-1, sound.chunk, 0) == -1) {
                journal::critical("%1", SDL_GetError());
            }
        }

    } // namespace sdl_mixer_backend

} // namespace audio

namespace resources {

    auto create_sound(const wave_t &wave) -> std::optional<sound_t> {
        sound_t snd;
        snd.chunk = wave;

        return snd;
    }

    auto destroy_sound(sound_t &snd) -> void {
        Mix_FreeChunk(snd.chunk);
    }

} // namespace resources
