#include "runtime.h"

#include <SDL3/SDL.h>
#include <stdio.h>

#define TARGET_FRAMERATE 60.0
#define WINDOW_SCALE 2

int runtime_init(runtime_t *rt, const char *assets_root, const char *language) {
    (void)assets_root;
    (void)language;

    if (!rt)
        return 0;

    rt->window = NULL;
    rt->input = NULL;
    rt->running = false;

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        fprintf(stderr, "widebrim: SDL_Init failed: %s\n", SDL_GetError());
        return 0;
    }

    fprintf(stderr, "widebrim: Runtime initialisation not implemented yet\n");
    return 0;
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