#ifndef MODE_EVENT_H
#define MODE_EVENT_H

#include "mode_impl.h"

typedef struct {
    // Base must be at beginning of struct
    mode_impl_t base;
} mode_event_impl_t;

mode_handler_t mode_event_create(game_state_t *state,
                                 screen_controller_t *controller);

#endif // MODE_EVENT_H
