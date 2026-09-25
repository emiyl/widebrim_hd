#ifndef MODE_RESET_H
#define MODE_RESET_H

#include "game_state.h"
#include "mode.h"
#include "screen_controller.h"

typedef struct {
    // Game state and controller must be at the beginning of the struct
    game_state_t *state;
    screen_controller_t *controller;

    bool done;
} mode_reset_impl_t;

mode_handler_t mode_reset_create(game_state_t *state,
                                 screen_controller_t *controller);

#endif // MODE_RESET_H
