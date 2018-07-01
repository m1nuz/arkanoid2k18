namespace audio {

    inline namespace al_backend {

        auto init(game::context_t &ctx) -> std::optional<context_t> {
            (void)ctx;
            context_t a;

            //const auto default_device_name = alcGetString(nullptr, ALC_DEFAULT_DEVICE_SPECIFIER);

            const auto device = alcOpenDevice(nullptr);
            if (!device) {
                journal::error("%1", "Unable to open default audio device");
                return {};
            }

            journal::debug("Audio device: %1", alcGetString(device, ALC_DEVICE_SPECIFIER));

            a.audio_device = device;

            alGetError();

            const auto context = alcCreateContext(device, nullptr);
            if (!alcMakeContextCurrent(context)) {
                journal::error("%1", "Failed to make default context\n");
                return {};
            }

            a.audio_context = context;

            alGenSources(MAX_AUDIO_SOURCES, &a.sources[0]);

            const float orientation[] = {0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f };

            alListener3f(AL_POSITION, 0, 0, 1.0f);
            alListener3f(AL_VELOCITY, 0, 0, 0);
            alListenerfv(AL_ORIENTATION, orientation);

            return a;
        }

        auto cleanup(context_t &ctx) -> void {
            alDeleteSources(MAX_AUDIO_SOURCES, &ctx.sources[0]);

            alcMakeContextCurrent(nullptr);
            alcDestroyContext(ctx.audio_context);

            alcCloseDevice(ctx.audio_device);
        }

        auto play_sound(context_t &ctx, const resources::sound_t &sound, const bool looped) -> void {

            for (auto source : ctx.sources) {
                ALint status = 0;
                alGetSourcei(source, AL_SOURCE_STATE, &status);

                if ((status == AL_INITIAL) || (status == AL_STOPPED) || (status == 0)) {
                    alSource3f(source, AL_POSITION, 0.0f, 0.0f, 0.0f);
                    alSource3f(source, AL_VELOCITY, 0.0f, 0.0f, 0.0f);
                    alSourcef(source, AL_PITCH, 1.0f);
                    alSourcef(source, AL_GAIN, static_cast<float>(ctx.volume) / MAX_AUDIO_VOLUME);

                    if (looped != 0)
                        alSourcei(source, AL_LOOPING, AL_TRUE);

                    alSourcei(source, AL_BUFFER, sound.buffer);
                    alSourcePlay(source);

                    break;
                }
            }
        }

        auto enable_sound() -> void {

        }

        auto disabel_sound() -> void {

        }

        auto change_volume(context_t &ctx, int volume) -> void {
            ctx.volume = abs(volume) % MAX_AUDIO_VOLUME;
        }

    } // namespace al_backend

} // namespace audio

namespace resources {

    inline auto convert_audio_format(const audio_format fmt) -> ALenum {
        switch (fmt) {
        case audio_format::unknown:
            return 0;
        case audio_format::mono8:
            return AL_FORMAT_MONO8;
        case audio_format::mono16:
            return AL_FORMAT_MONO16;
        case audio_format::stereo8:
            return AL_FORMAT_STEREO8;
        case audio_format::stereo16:
            return AL_FORMAT_STEREO16;
        }

        return AL_NONE;
    }

    auto create_sound(const wave_t &wave) -> std::optional<sound_t> {
        ALuint buf = 0;
        ALenum format = convert_audio_format(wave.format);

        alGenBuffers(1, &buf);
        alBufferData(buf, format, &wave.bytes[0], wave.size, wave.frequency);

        sound_t snd;
        snd.buffer = buf;
        snd.format = format;
        snd.frequency = wave.frequency;
        snd.size = wave.size;

        return snd;
    }

    auto destroy_sound(sound_t &snd) -> void {
        alDeleteBuffers(1, &snd.buffer);
    }

} // namespace resources
