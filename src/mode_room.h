#ifndef MODE_ROOM_H
#define MODE_ROOM_H

#include "game_state.h"
#include "mode.h"
#include "screen_controller.h"

typedef struct {
    // Game state and controller must be at the beginning of the struct
    game_state_t *state;
    screen_controller_t *controller;

    sprite_instance_t *move_mode_sprite;
    bool in_move_mode;
    bool done;
} mode_room_impl_t;

mode_handler_t mode_room_create(game_state_t *state,
                                screen_controller_t *controller);

#endif // MODE_ROOM_H
