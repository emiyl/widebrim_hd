#ifndef RUNTIME_H
#define RUNTIME_H

#include <stdbool.h>

#include "clock.h"
#include "game_state.h"
#include "input.h"
#include "mode_spawner.h"
#include "window.h"

typedef struct {
    window_t *window;
    input_t *input;
    game_state_t state;
    mode_spawner_t spawner;
    wb_clock_t clock;
    bool running;
} runtime_t;

int runtime_init(runtime_t *runtime, const char *assets_root,
                 language_t language);
void runtime_destroy(runtime_t *runtime);
void runtime_run(runtime_t *runtime);

#endif // RUNTIME_H
