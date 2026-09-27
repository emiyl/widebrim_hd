#ifndef MODE_ROOM_H
#define MODE_ROOM_H

#include "exit.h"
#include "game_state.h"
#include "mode.h"
#include "object.h"
#include "screen_controller.h"

typedef struct mode_room_impl_t {
    // Game state and controller must be at the beginning of the struct
    game_state_t *state;
    screen_controller_t *controller;

    void (*setmap)(struct mode_room_impl_t *impl, int32_t map_text_id,
                   int32_t map_background_id, int32_t param3, int32_t param4,
                   int32_t param5);
    void (*add_text_obj)(struct mode_room_impl_t *impl, int32_t type_or_flag,
                         int32_t x, int32_t y, int32_t width, int32_t height,
                         int32_t text_id, int32_t param7);
    void (*add_exit)(struct mode_room_impl_t *impl, int32_t exit_sprite_id,
                     int32_t target_map_id, int32_t x, int32_t y, int32_t width,
                     int32_t height, int32_t param7, int32_t param8);

    object_t *move_mode_btn;
    object_t *map_place;

    room_exit_t exits[8];
    int32_t exit_count;

    bool in_move_mode;
    bool done;
} mode_room_impl_t;

mode_handler_t mode_room_create(game_state_t *state,
                                screen_controller_t *controller);

#endif // MODE_ROOM_H
