#ifndef MODE_ROOM_H
#define MODE_ROOM_H

#include "game_state.h"
#include "mode.h"
#include "screen_controller.h"

mode_handler_t mode_room_create(game_state_t *state,
                                screen_controller_t *controller);

#endif // MODE_ROOM_H
