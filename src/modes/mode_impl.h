#ifndef MODE_IMPL_H
#define MODE_IMPL_H

#include "../game_state.h"
#include "../screen_controller.h"

typedef struct {
    game_state_t *state;
    screen_controller_t *controller;
    bool done;
} mode_impl_t;

#endif // MODE_IMPL_H
