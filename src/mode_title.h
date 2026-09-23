#ifndef MODE_TITLE_H
#define MODE_TITLE_H

#include "game_state.h"
#include "mode.h"
#include "screen_controller.h"

mode_handler_t mode_title_create(game_state_t *state,
                                 screen_controller_t *controller);

#endif // MODE_TITLE_H
