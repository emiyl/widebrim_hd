#include "runtime.h"

#include <SDL3/SDL.h>
#include <stdio.h>

#include "bg_layer.h"
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

    rt->window =
        window_create_sdl("widebrim", WB_SCREEN_WIDTH, WB_SCREEN_HEIGHT, 0);
    if (!rt->window) {
        fprintf(stderr, "widebrim: Failed to create window\n");
        SDL_Quit();
        return -1;
    }

    renderer_t *renderer =
        renderer_create_sdl(window_as_sdl3_renderer(rt->window));
    if (!renderer) {
        fprintf(stderr, "widebrim: Failed to create renderer\n");
        window_destroy(rt->window);
        SDL_Quit();
        return -1;
    }

    fprintf(stderr, "widebrim: Runtime initialisation not implemented yet\n");
    return -1;
}

void runtime_destroy(runtime_t *runtime) {
    if (!runtime)
        return;
    runtime->running = false;
}

void runtime_run(runtime_t *runtime) {
    if (!runtime)
        return;
    while (runtime->running) {
        // Main loop logic here
    }
}
