#include <algorithm>

#include "config.hh"
#include "audio.hh"
#include "journal.hh"
#include "video.hh"
#include "game.hh"

extern auto main(int argc, char *argv[]) -> int {
    (void)argc, (void)argv;

    if (auto app = game::init(GAME_CONF_PATH, true); app) {
        auto audio_engine = audio::init(app.value());
        if (!audio_engine) {
            journal::critical("%1", "Couldn't init audio");
            return EXIT_FAILURE;
        }

        if (!game::start(app.value()))
            return EXIT_FAILURE;

        auto render = video::init(app.value());

        if (!render) {
            journal::critical("%1", "Couldn't init video");
            return EXIT_FAILURE;
        }

        auto current = 0ull;
        auto last = 0ull;
        auto timesteps = 0ull;
        auto accumulator = 0.0f;

        while (app.value().running) {
            last = current;
            current = SDL_GetPerformanceCounter();

            const auto freq = SDL_GetPerformanceFrequency();
            const auto dt = static_cast<float>(static_cast<double>(current - last) / static_cast<double>(freq));

            accumulator += std::clamp(dt, 0.f, 0.2f);

            game::process_events(app.value(), dt);

            while (accumulator >= game::timestep) {
                accumulator -= game::timestep;

                game::update(app.value(), audio_engine.value(), game::timestep);

                timesteps++;
            }

            game::draw(app.value(), render.value());

            auto projection = glm::ortho(0.0f, static_cast<float>(app.value().width), static_cast<float>(app.value().height), 0.0f, -1.0f, 1.0f);
            auto view = glm::mat4{1.f};

            const auto ticks = static_cast<float>(SDL_GetTicks()) / 1000.f;

            render.value().options = app.value().render_options;
            video::present(app.value().width, app.value().height, ticks, render.value(), projection, view);

            SDL_GL_SwapWindow(app.value().window);
        }

        audio::cleanup(audio_engine.value());
        video::cleanup(render.value());
        game::cleanup(app.value());

        return EXIT_SUCCESS;
    }

    return EXIT_FAILURE;
}
