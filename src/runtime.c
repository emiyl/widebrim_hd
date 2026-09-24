#include "runtime.h"

#include <SDL3/SDL.h>
#include <stdio.h>

#include "bg_layer.h"
#include "renderer.h"

#define TARGET_FRAMERATE 60.0
#define WINDOW_SCALE 0.5f

int runtime_init(runtime_t *rt, const char *assets_root, const char *language) {
    (void)language;

    if (!rt) {
        fprintf(stderr, "widebrim: Invalid runtime pointer\n");
        return -1;
    }

    rt->window = NULL;
    rt->input = NULL;
    rt->running = false;

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        fprintf(stderr, "widebrim: SDL_Init failed: %s\n", SDL_GetError());
        return -1;
    }

    rt->input = input_create_sdl();
    if (!rt->input) {
        fprintf(stderr, "widebrim: Failed to create input\n");
        runtime_destroy(rt);
        return -1;
    }

    rt->window = window_create_sdl("widebrim", WB_SCREEN_WIDTH * WINDOW_SCALE,
                                   WB_SCREEN_HEIGHT * WINDOW_SCALE * 2, 0);
    if (!rt->window) {
        fprintf(stderr, "widebrim: Failed to create window\n");
        runtime_destroy(rt);
        return -1;
    }

    renderer_t *renderer =
        renderer_create_sdl(window_as_sdl3_renderer(rt->window));
    if (!renderer) {
        fprintf(stderr, "widebrim: Failed to create renderer\n");
        runtime_destroy(rt);
        return -1;
    }

    window_set_scale(rt->window, WINDOW_SCALE, WINDOW_SCALE);

    if (game_state_init(&rt->state, assets_root) != 0) {
        fprintf(stderr, "widebrim: Failed to initialize game state\n");
        runtime_destroy(rt);
        return -1;
    }

    mode_spawner_init(&rt->spawner, &rt->state, renderer);
    game_state_set_mode(&rt->state, MODE_RESET);

    rt->running = true;
    return 0;
}

void runtime_destroy(runtime_t *rt) {
    if (!rt)
        return;
    if (rt->input) {
        input_destroy(rt->input);
        rt->input = NULL;
    }
    if (rt->window) {
        window_destroy(rt->window);
        rt->window = NULL;
    }
    rt->running = false;
    SDL_Quit();
}

void runtime_run(runtime_t *rt) {
    if (!rt) {
        fprintf(stderr, "widebrim: Invalid runtime pointer\n");
        return;
    }

    const double interval_sec = 1.0 / TARGET_FRAMERATE;
    const double interval_ms = interval_sec * 1000.0;
    double dt_ms = 0.0;

    clock_init(&rt->clock);

    while (rt->running) {
        input_event_t event;

        mode_spawner_update(&rt->spawner, (float)dt_ms);

        renderer_clear(rt->spawner.controller.renderer, 0, 0, 0, 255);
        mode_spawner_draw(&rt->spawner, rt->spawner.controller.renderer);
        renderer_present(rt->spawner.controller.renderer);

        while (input_poll_event(rt->input, &event)) {
            window_convert_event_to_render_coordinates(rt->window, &event);

            switch (event.type) {
            case INPUT_EVENT_QUIT:
                rt->running = false;
                mode_spawner_on_quit(&rt->spawner);
                break;
            case INPUT_EVENT_MOUSE_BUTTON_DOWN:
            case INPUT_EVENT_MOUSE_BUTTON_UP:
            case INPUT_EVENT_MOUSE_MOTION:
            case INPUT_EVENT_KEY_DOWN:
            case INPUT_EVENT_KEY_UP:
                mode_spawner_handle_event(&rt->spawner, &event);
                break;
            default:
                break;
            }
        }

        dt_ms = wb_clock_tick(&rt->clock, interval_sec);
        if (dt_ms / interval_ms > 1.25) {
            dt_ms = interval_ms;
        }
    }
}
