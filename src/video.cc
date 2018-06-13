#include <algorithm>

#include <GL/glcore.h>
#include <GL/ext_texture_filter_anisotropic.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "journal.hh"
#include "game.hh"
#include "video.hh"
#include "shader_uniform.inl"

namespace video {

    auto init(game::context_t &ctx) -> std::optional<context_t> {
        context_t r;
        r.sprites.reserve(1000);

        if (auto sh = resources::get_shader(ctx, "sprite"); sh) {
            r.sprite_shader = sh.value();
        } else {
            journal::error("%1", "'sprite' shader not found");
            return {};
        }

        if (auto sh = resources::get_shader(ctx, "particle"); sh) {
            r.particle_shader = sh.value();
        } else {
            journal::error("%1", "'particle' shader not found");
            return {};
        }

        if (auto sh = resources::get_shader(ctx, "postprocess"); sh) {
            r.postprocess_shader = sh.value();
        } else {
            journal::error("%1", "'postprocess' shader not found");
            return {};
        }

        {
            GLuint vb = 0;
            GLfloat vertices[] = {
                // Pos      // Tex
                0.0f, 1.0f, 0.0f, 1.0f,
                1.0f, 0.0f, 1.0f, 0.0f,
                0.0f, 0.0f, 0.0f, 0.0f,

                0.0f, 1.0f, 0.0f, 1.0f,
                1.0f, 1.0f, 1.0f, 1.0f,
                1.0f, 0.0f, 1.0f, 0.0f
            };

            glGenVertexArrays(1, &r.sprite_va);
            glGenBuffers(1, &vb);

            glBindBuffer(GL_ARRAY_BUFFER, vb);
            glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

            glBindVertexArray(r.sprite_va);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindVertexArray(0);
        }

        {
            GLuint vb = 0;
            const GLfloat particle_quad[] = {
                0.0f, 1.0f, 0.0f, 1.0f,
                1.0f, 0.0f, 1.0f, 0.0f,
                0.0f, 0.0f, 0.0f, 0.0f,

                0.0f, 1.0f, 0.0f, 1.0f,
                1.0f, 1.0f, 1.0f, 1.0f,
                1.0f, 0.0f, 1.0f, 0.0f
            };

            glGenVertexArrays(1, &r.particle_va);
            glGenBuffers(1, &vb);
            glBindVertexArray(r.particle_va);
            glBindBuffer(GL_ARRAY_BUFFER, vb);
            glBufferData(GL_ARRAY_BUFFER, sizeof particle_quad, particle_quad, GL_STATIC_DRAW);

            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
            glBindVertexArray(0);

        }

        {
            GLuint vb = 0;
            GLfloat vertices[] = {
                // Pos        // Tex
                -1.0f, -1.0f, 0.0f, 0.0f,
                1.0f,  1.0f, 1.0f, 1.0f,
                -1.0f,  1.0f, 0.0f, 1.0f,

                -1.0f, -1.0f, 0.0f, 0.0f,
                1.0f, -1.0f, 1.0f, 0.0f,
                1.0f,  1.0f, 1.0f, 1.0f
            };
            glGenVertexArrays(1, &r.screenquad_va);
            glGenBuffers(1, &vb);

            glBindBuffer(GL_ARRAY_BUFFER, vb);
            glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

            glBindVertexArray(r.screenquad_va);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GL_FLOAT), (GLvoid*)0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindVertexArray(0);
        }

        glGenTextures(1, &r.color_tex);
        glBindTexture(GL_TEXTURE_2D, r.color_tex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, ctx.width, ctx.height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
        glBindTexture(GL_TEXTURE_2D, 0);

        glGenSamplers(1, &r.texture_sampler);
        glSamplerParameteri(r.texture_sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glSamplerParameteri(r.texture_sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        //glSamplerParameteri(r.texture_sampler, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16);
        glSamplerParameteri(r.texture_sampler, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glSamplerParameteri(r.texture_sampler, GL_TEXTURE_WRAP_T, GL_REPEAT);

        glGenFramebuffers(1, &r.color_fb);
        glGenFramebuffers(1, &r.sampled_fb);
        glGenRenderbuffers(1, &r.sampled_rb);

        glBindFramebuffer(GL_FRAMEBUFFER, r.sampled_fb);
        glBindRenderbuffer(GL_RENDERBUFFER, r.sampled_rb);
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, 16, GL_RGB, ctx.width, ctx.height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, r.sampled_rb);
        if (auto status = glCheckFramebufferStatus(GL_FRAMEBUFFER); status != GL_FRAMEBUFFER_COMPLETE) {
            journal::error("Incomplite framebuffer %1", status);
            return {};
        }

        glBindFramebuffer(GL_FRAMEBUFFER, r.color_fb);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, r.color_tex, 0);
        if (auto status = glCheckFramebufferStatus(GL_FRAMEBUFFER); status != GL_FRAMEBUFFER_COMPLETE) {
            journal::error("Incomplite framebuffer %1", status);
            return {};
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        return r;
    }    

    auto present_particles(context_t &ctx, const game::particle_emitter &emitter) {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);

        for (const auto& p : emitter.particles) {
            if (p.life > 0.0f) {
                set_value(ctx.particle_shader, "offset", p.position);
                set_value(ctx.particle_shader, "color", p.color);
                set_value(ctx.particle_shader, "image", 0);

                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, emitter.texture.id);
                glBindSampler(0, ctx.texture_sampler);

                glBindVertexArray(ctx.particle_va);
                glDrawArrays(GL_TRIANGLES, 0, 6);
                glBindVertexArray(0);
            }
        }

        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    static auto present_sprite(context_t &ctx, const game::sprite_t &sp) {
        using namespace glm;

        auto model = translate(mat4{1}, vec3(sp.position, 0.0f));
        model = glm::translate(model, vec3(0.5f * sp.size.x, 0.5f * sp.size.y, 0.0f));
        model = glm::rotate(model, sp.rotate, vec3(0.0f, 0.0f, 1.0f));
        model = glm::translate(model, vec3(-0.5f * sp.size.x, -0.5f * sp.size.y, 0.0f));

        model = glm::scale(model, vec3(sp.size, 1.0f));

        set_value(ctx.sprite_shader, "model", model);
        set_value(ctx.sprite_shader, "color", sp.color);
        set_value(ctx.sprite_shader, "image", 0);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, sp.texture.id);
        glBindSampler(0, ctx.texture_sampler);

        glBindVertexArray(ctx.sprite_va);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
    }

    auto present(const int w, const int h, const float ticks, context_t &ctx, const mat4 &proj, const mat4 &view) -> void {
        (void)view;

        glBindFramebuffer(GL_FRAMEBUFFER, ctx.sampled_fb);

        glEnable(GL_CULL_FACE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glViewport(0, 0, w, h);
        //glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClearColor(63.f/255.0f * 0.6f, 124.f/255.f * 0.6f, 182.f/255.f * 0.6f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(ctx.sprite_shader.id);

        set_value(ctx.sprite_shader, "projection", proj);

        for (const auto& sp : ctx.sprites)
            present_sprite(ctx, sp);

        glUseProgram(ctx.particle_shader.id);

        set_value(ctx.sprite_shader, "projection", proj);

        for (const auto& p : ctx.particles)
            present_particles(ctx, p);

        glBindFramebuffer(GL_READ_FRAMEBUFFER, ctx.sampled_fb);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, ctx.color_fb);
        glBlitFramebuffer(0, 0, w, h, 0, 0, w, h, GL_COLOR_BUFFER_BIT, GL_NEAREST);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        const float blur_kernel[9] = {
            1.0 / 16, 2.0 / 16, 1.0 / 16,
            2.0 / 16, 4.0 / 16, 2.0 / 16,
            1.0 / 16, 2.0 / 16, 1.0 / 16
        };

        const float offset = 1.0f / 300.0f;
        const std::vector<vec2> offsets = {vec2{ -offset,  offset  },  // top-left
                                           vec2{  0.0f,    offset  },  // top-center
                                           vec2{  offset,  offset  },  // top-right
                                           vec2{ -offset,  0.0f    },  // center-left
                                           vec2{  0.0f,    0.0f    },  // center-center
                                           vec2{  offset,  0.0f    },  // center - right
                                           vec2{ -offset, -offset  },  // bottom-left
                                           vec2{  0.0f,   -offset  },  // bottom-center
                                           vec2{  offset, -offset  }}; // bottom-right

        glUseProgram(ctx.postprocess_shader.id);

        set_value(ctx.postprocess_shader, "scene", 0);
        set_value(ctx.postprocess_shader, "time", ticks);
        set_value(ctx.postprocess_shader, "confuse", 0);
        set_value(ctx.postprocess_shader, "chaos", 0);
        set_value(ctx.postprocess_shader, "shake", static_cast<int32_t>(ctx.options & OP_SHAKE));
        set_value(ctx.postprocess_shader, "offsets", offsets);
        set_value(ctx.postprocess_shader, "blur_kernel", blur_kernel);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, ctx.color_tex);
        glBindSampler(0, ctx.texture_sampler);

        glBindVertexArray(ctx.screenquad_va);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        glUseProgram(0);

        ctx.sprites.clear();
        ctx.particles.clear();
    }

    auto cleanup(context_t &ctx) -> void {
        glDeleteVertexArrays(1, &ctx.sprite_va);
        glDeleteVertexArrays(1, &ctx.particle_va);
        glDeleteVertexArrays(1, &ctx.screenquad_va);

        glDeleteSamplers(1, &ctx.texture_sampler);

        glDeleteRenderbuffers(1, &ctx.sampled_rb);
        glDeleteFramebuffers(1, &ctx.sampled_fb);
        glDeleteFramebuffers(1, &ctx.color_fb);
    }

    auto draw_sprite(context_t &ctx, const resources::texture_t &texture, const vec2 &position, const vec2 &size, const float rotate, const vec3 &color) -> void {
        game::sprite_t sp;
        sp.texture = texture;
        sp.position = position;
        sp.size = size;
        sp.rotate = rotate;
        sp.color = color;

        ctx.sprites.push_back(sp);
    }

    auto draw_particles(context_t &ctx, const game::particle_emitter &emitter) -> void {
        ctx.particles.push_back(emitter);
    }

} // namespace video
