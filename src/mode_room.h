#ifndef MODE_ROOM_H
#define MODE_ROOM_H

#include "game_state.h"
#include "mode.h"
#include "screen_controller.h"

typedef struct mode_room_impl_t {
    // Game state and controller must be at the beginning of the struct
    game_state_t *state;
    screen_controller_t *controller;

    void (*setmap)(struct mode_room_impl_t *impl, int32_t map_text_id,
                   int32_t map_background_id, int32_t param3, int32_t param4,
                   int32_t param5);

    sprite_instance_t *move_mode_sprite;
    bool in_move_mode;
    bool done;
} mode_room_impl_t;

mode_handler_t mode_room_create(game_state_t *state,
                                screen_controller_t *controller);

#endif // MODE_ROOM_H
