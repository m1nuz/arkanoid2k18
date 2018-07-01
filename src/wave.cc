#include <optional>
#include <SDL2/SDL_rwops.h>

#include "resources.hh"
#include "audio.hh"

#ifdef OPENAL_BACKEND

#pragma pack(push, wave_align)
#pragma pack(1)
struct riff_header {
    uint8_t  chunkID[4];
    uint32_t chunkSize;   // size not including chunkSize or chunkID
    uint8_t  format[4];
};

struct wave_format {
    uint8_t  subChunkID[4];
    uint32_t subChunkSize;
    uint16_t audioFormat;
    uint16_t numChannels;
    uint32_t sampleRate;
    uint32_t byteRate;
    uint16_t blockAlign;
    uint16_t bitsPerSample;
};

struct wave_data {
    char     subChunkID[4]; // should contain the word data
    uint32_t subChunk2Size; // Stores the size of the data block
};
#pragma pack(pop, wave_align)

auto load_wave(SDL_RWops *rw) -> std::optional<resources::wave_t> {
    if (!rw)
        return {};

    riff_header riff;
    SDL_RWread(rw, &riff, sizeof riff, 1);

    if (memcmp(riff.chunkID, "RIFF", sizeof riff.chunkID) != 0 && memcmp(riff.format, "WAVE", sizeof riff.format) != 0)
        return {};

    wave_format format;
    SDL_RWread(rw, &format, sizeof format, 1);

    if (memcmp(format.subChunkID, "fmt ", sizeof riff.chunkID) != 0)
        return {};

    resources::wave_t wave;
    while (true) {
        wave_data data;
        if (SDL_RWread(rw, &data, sizeof data, 1) == 0)
            break;

        if (memcmp(data.subChunkID, "data", sizeof format.subChunkID) == 0) {
            std::vector<uint8_t> bytes;
            bytes.resize(data.subChunk2Size);
            SDL_RWread(rw, &bytes[0], data.subChunk2Size, 1);

            wave.size = static_cast<uint32_t>(data.subChunk2Size);
            wave.frequency = format.sampleRate;

            if (format.numChannels == 1) {
                if (format.bitsPerSample == 8)
                    wave.format = resources::audio_format::mono8;
                else if (format.bitsPerSample == 16)
                    wave.format = resources::audio_format::mono16;
            } else {
                if (format.bitsPerSample == 8)
                    wave.format = resources::audio_format::stereo8;
                else if(format.bitsPerSample == 16)
                    wave.format = resources::audio_format::stereo16;
            }

            break;
        }

        SDL_RWseek(rw, data.subChunk2Size, RW_SEEK_CUR);
    }

    return wave;
}

#elif SDL_MIXER_BACKEND
auto load_wave(SDL_RWops *rw) -> std::optional<resources::wave_t> {
    resources::wave_t wav;

    wav = Mix_LoadWAV_RW(rw, 1);
    if (!wav)
        return {};

    return wav;
}

#endif // SDL_MIXER_BACKEND
