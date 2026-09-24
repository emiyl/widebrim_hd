#ifndef BG_LOADER_H
#define BG_LOADER_H

#include <stdbool.h>
#include <stdint.h>

#include "game_state.h"
#include "screen_controller.h"

bool bg_loader_load(game_state_t *state, screen_controller_t *controller,
                    const char *rel_path,
                    void (*setter)(screen_controller_t *, const uint8_t *, int,
                                   int));

#endif // BG_LOADER_H
