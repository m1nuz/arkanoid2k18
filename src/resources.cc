#include <json.hpp>
#include <GL/glcore.h>
#include <AL/al.h>

#include "config.hh"
#include "journal.hh"
#include "resources.hh"
#include "game.hh"

#include "texture_format.inl"

using json = nlohmann::json;

auto load_targa(SDL_RWops *rw) -> std::optional<resources::image_t>;
auto load_wave(SDL_RWops *rw) -> std::optional<resources::wave_t>;

namespace resources {

    static auto compile_shader(const GLenum type, const std::string_view source) -> uint32_t {
        auto id = glCreateShader(type);

        const char *fullsource[] =  {source.data(), 0};

        glShaderSource(id, 1, fullsource, 0);
        glCompileShader(id);

        GLint status = 0;
        glGetShaderiv(id, GL_COMPILE_STATUS, &status);

        if (!status) {
            GLint lenght = 0;
            glGetShaderiv(id, GL_INFO_LOG_LENGTH, &lenght);

            std::string log_text(lenght, 0);

            GLsizei written = 0;
            glGetShaderInfoLog(id, lenght, &written, &log_text[0]);
            log_text.resize(written);

            journal::error("%1", log_text);

            glDeleteShader(id);
            id = 0;
        }

        return id;
    }

    static auto link_program(const uint32_t pid) -> bool {
        glLinkProgram(pid);

        GLint status = 0;
        glGetProgramiv(pid, GL_LINK_STATUS, &status);

        if (!status) {
            GLint lenght = 0;
            glGetProgramiv(pid, GL_INFO_LOG_LENGTH, &lenght);

            std::string log_text(lenght, 0);

            GLsizei written = 0;
            glGetProgramInfoLog(pid, lenght, &written, &log_text[0]);
            log_text.resize(written);

            journal::error("%1", log_text);
            return false;
        }

        return true;
    }

    static auto get_program_uniforms(shader_t &sh) -> void {
        int total = -1;
        glGetProgramiv(sh.id, GL_ACTIVE_UNIFORMS, &total);

        if (total < 0)
            return;

        sh.uniforms.reserve(total);

        char name[1024] = {};
        int name_len = -1, num = -1;
        GLenum type = GL_ZERO;
        for (auto i = 0; i < total; i++) {
            glGetActiveUniform(sh.id, i, sizeof(name) - 1, &name_len, &num, &type, name);

            const auto location = glGetUniformLocation(sh.id, name);

            sh.uniforms.emplace(name, uniform_t{location, num, type});

            journal::debug("-u- %1 %2 %3", name, location, type);
        }
    }

    static auto get_program_attributes(shader_t &sh) -> void {
        int total = -1;
        glGetProgramiv(sh.id, GL_ACTIVE_ATTRIBUTES, &total);

        if (total < 0)
            return;

        sh.attributes.reserve(total);

        // TODO: GL_ACTIVE_ATTRIBUTE_MAX_LENGTH
        char name[1024] = {};
        int name_len = -1, num = -1;
        GLenum type = GL_ZERO;
        for (auto i = 0; i < total; i++) {
            glGetActiveAttrib(sh.id, i, sizeof(name) - 1, &name_len, &num, &type, name);

            const auto location = glGetAttribLocation(sh.id, name);

            sh.attributes.emplace(name, attribute_t{location, num, type});

            journal::debug("-a- %1 %2", name, location);
        }
    }

    static auto compile(const std::string_view vert_source, const std::string_view frag_source, const std::string_view geom_source = {}) -> std::optional<shader_t> {
        (void)geom_source;

        shader_t sh;
        sh.id = glCreateProgram();

        const auto vs = compile_shader(GL_VERTEX_SHADER, vert_source);
        const auto fs = compile_shader(GL_FRAGMENT_SHADER, frag_source);

        glAttachShader(sh.id, vs);
        glAttachShader(sh.id, fs);

        if (!link_program(sh.id)) {
            glDeleteProgram(sh.id);
            sh.id = 0;

            return {};
        }

        get_program_uniforms(sh);
        get_program_attributes(sh);

        return sh;
    }

    static auto create_texture(const image_t &image, const bool mipmaps = true) -> std::optional<texture_t> {
        auto id = 0u;
        glGenTextures(1, &id);
        glBindTexture(GL_TEXTURE_2D, id);

        const auto w = static_cast<GLsizei>(image.width);
        const auto h = static_cast<GLsizei>(image.height);

        auto internal_format = static_cast<GLint >(0);
        auto format = static_cast<GLenum>(0);
        auto type = static_cast<GLenum>(0);
        get_texture_format_from_pixelformat(image.format, internal_format, format, type);

        const auto pixels = image.pixels.empty() ? nullptr : reinterpret_cast<const void*>(&image.pixels[0]);

        glTexImage2D(GL_TEXTURE_2D, 0, internal_format, static_cast<int32_t>(w), static_cast<int32_t>(h), 0, format, type, pixels);

        if (mipmaps)
            glGenerateMipmap(GL_TEXTURE_2D);

        glBindTexture(GL_TEXTURE_2D, 0);

        return texture_t{id, GL_TEXTURE_2D, image.width, image.height, 0};
    }

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

    static auto create_sound(const wave_t &wave) -> std::optional<sound_t> {
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

    auto init(game::context_t &ctx, const std::string_view assets_path) -> bool {
        using namespace std;

        auto contents = get_config(assets_path);
        if (!contents) {
            journal::critical("%1", "Can't read asset config");
            return {};
        }

        auto j = json::parse(contents.value());

        unordered_map<string, string> shader_sources;

        if (j.find("shaders") != j.end()) {
            for (auto& sh : j["shaders"]) {
                const auto name = sh.find("name") != sh.end() ? sh["name"].get<string>() : string{};
                const auto source = sh.find("source") != sh.end() ? sh["source"].get<string>() : string{};

                shader_sources.emplace(name, source);
            }
        }

        if (j.find("programs") != j.end()) {
            for (auto& p : j["programs"]) {
                const auto program_name = p.find("name") != p.end() ? p["name"].get<string>() : string{};
                const auto vs_source_name = p.find("vertex") != p.end() ? p["vertex"].get<string>() : string{};
                const auto fs_source_name = p.find("fragment") != p.end() ? p["fragment"].get<string>() : string{};

                if (!vs_source_name.empty() && !fs_source_name.empty()) {
                    const auto vs_source = shader_sources.find(vs_source_name) != shader_sources.end() ? shader_sources[vs_source_name] : string{};
                    const auto fs_source = shader_sources.find(fs_source_name) != shader_sources.end() ? shader_sources[fs_source_name] : string{};

                    if (const auto sh = compile(vs_source, fs_source); sh) {
                        journal::debug("'%1' shader added", program_name);
                        ctx.shaders.emplace(program_name, sh.value());
                    }
                }
            }
        }

        if (j.find("textures") != j.end()) {
            for (auto& t : j["textures"]) {
                const auto texture_name = t.find("name") != t.end() ? t["name"].get<string>() : string{};
                const auto levels = t.find("levels") != t.end() ? t["levels"].get<vector<string>>() : vector<string>{};

                if (!levels.empty()) {
                    const auto path = GAME_ASSETS_DIR + string{"/"} + levels.front();

                    auto rw = SDL_RWFromFile(path.c_str(), "r");
                    if (!rw) {
                        journal::error("%1", SDL_GetError());
                        continue;
                    }

                    const auto image = load_targa(rw);

                    if (image) {
                        const auto tex = create_texture(image.value(), false);
                        if (tex) {
                            journal::debug("'%1' texture added", texture_name);
                            ctx.textures.emplace(texture_name, tex.value());
                        }

                    } else {
                        journal::warning("Can't load '%1' image", texture_name);
                    }
                }
            }
        }

        if (j.find("sounds") != j.end()) {
            for (auto& s : j["sounds"] ) {
                const auto sound_name = s.find("name") != s.end() ? s["name"].get<string>() : string{};
                const auto sound_source = s.find("source") != s.end() ? s["source"].get<string>() : string{};

                if (!sound_source.empty()) {
                    const auto path = GAME_ASSETS_DIR + string{"/"} + sound_source;

                    auto rw = SDL_RWFromFile(path.c_str(), "r");
                    if (!rw) {
                        journal::error("%1", SDL_GetError());
                        continue;
                    }

                    const auto wave = load_wave(rw);

                    if (wave) {
                        const auto snd = create_sound(wave.value());
                        if (snd) {
                            journal::debug("'%1' sound added", sound_name);
                            ctx.sounds.emplace(sound_name, snd.value());
                        }
                    } else {
                        journal::warning("Can't load '%1' sound", sound_name);
                    }
                }
            }
        }

        return true;
    }

    auto get_shader(const game::context_t &ctx, const std::string_view name) -> std::optional<shader_t> {
        const auto it = ctx.shaders.find(name.data());
        if (it == ctx.shaders.end())
            return {};

        return it->second;
    }

    auto get_texture(const game::context_t &ctx, const std::string_view name) -> std::optional<texture_t> {
        const auto it = ctx.textures.find(name.data());
        if (it == ctx.textures.end())
            return {};

        return it->second;
    }

    auto get_sound(const game::context_t &ctx, const std::string_view name) -> std::optional<sound_t> {
        const auto it = ctx.sounds.find(name.data());
        if (it == ctx.sounds.end())
            return {};

        return it->second;
    }

    auto cleanup(game::context_t &ctx) -> void {
        for (auto sh : ctx.shaders)
            glDeleteProgram(sh.second.id);

        for (auto tex : ctx.textures)
            glDeleteTextures(1, &tex.second.id);

        for (auto snd : ctx.sounds)
            alDeleteBuffers(1, &snd.second.buffer);
    }

} // namespace resources
