#ifndef MODE_RESET_H
#define MODE_RESET_H

#include "../mode_impl.h"

typedef struct {
    // Base must be at beginning of struct
    mode_impl_t base;
} mode_reset_impl_t;

mode_handler_t mode_reset_create(game_state_t *state,
                                 screen_controller_t *controller);

#endif // MODE_RESET_H
