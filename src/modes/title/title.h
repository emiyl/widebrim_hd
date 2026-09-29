#ifndef MODE_TITLE_H
#define MODE_TITLE_H

#include "mode_impl.h"

typedef struct {
    // Base must be at the beginning of the struct
    mode_impl_t base;

    object_t *start_car;
    object_t *title_logo;

    // Button sprites for user interaction
    object_t *start_btn;
    object_t *continue_btn;
    object_t *bonus_btn;

    // Currently active button that has been clicked by the user
    object_t *active_click_btn;
} mode_title_impl_t;

mode_handler_t mode_title_create(game_state_t *state,
                                 screen_controller_t *controller);

#endif // MODE_TITLE_H
