#include "runtime.h"

#include <SDL3/SDL.h>
#include <stdio.h>

#include "bg_layer.h"
#include "clock.h"
#include "renderer.h"
#include "window.h"

#define TARGET_FRAMERATE 60.0

int runtime_init(runtime_t *rt, const char *assets_root, const char *language) {
    (void)assets_root;
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

    rt->window =
        window_create_sdl("widebrim", WB_SCREEN_WIDTH, WB_SCREEN_HEIGHT, 0);
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
        dt_ms = wb_clock_tick(&rt->clock, interval_sec);
        if (dt_ms / interval_ms > 1.25) {
            dt_ms = interval_ms;
        }
    }
}
