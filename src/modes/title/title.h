#ifndef MODE_TITLE_H
#define MODE_TITLE_H

#include "game_state.h"
#include "mode.h"
#include "screen_controller.h"

typedef struct {
    // Game state and controller must be at the beginning of the struct
    game_state_t *state;
    screen_controller_t *controller;

    sprite_instance_t *start_car_sprite;
    sprite_instance_t *title_sprite;

    // Button sprites for user interaction
    sprite_instance_t *start_button;
    sprite_instance_t *continue_button;
    sprite_instance_t *bonus_button;
    sprite_instance_t *active_click_sprite;
    bool done;
} mode_title_impl_t;

mode_handler_t mode_title_create(game_state_t *state,
                                 screen_controller_t *controller);

#endif // MODE_TITLE_H
