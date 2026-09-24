#include "mode_title.h"

#include <stdlib.h>

#include "bg_loader.h"

typedef struct {
    game_state_t *state;
    bool done;
} mode_title_impl_t;

static void mode_title_load_start_car_sprite(game_state_t *state,
                                             screen_controller_t *controller) {
    sprite_instance_t *start_car = NULL;

    if (!state || !controller) {
        return;
    }

    const int start_car_width = 245;
    const int start_car_height = 155;

    start_car = screen_controller_add_sprite_asset(
        controller, state, "ani/start_car.spr", 0, 0, 0, 255U, 100.0f, true);
    if (!start_car) {
        fprintf(stderr, "widebrim: failed to add start_car sprite asset\n");
        return;
    }

    sprite_layer_center_sprite(start_car, WB_SCREEN_WIDTH, WB_SCREEN_HEIGHT);
    sprite_layer_set_sprite_position(
        start_car, start_car->x,
        WB_SCREEN_HEIGHT + (WB_SCREEN_HEIGHT - start_car_height) / 2 + 150);
    (void)start_car_width;
    (void)start_car_height;
}

static void mode_title_load_title_sprite(game_state_t *state,
                                         screen_controller_t *controller) {
    sprite_instance_t *title = NULL;

    if (!state || !controller) {
        return;
    }

    title = screen_controller_add_sprite_asset(
        controller, state, "ani/title_logo.spr", 0, 0, 0, 255U, 0.0f, false);
    if (!title) {
        fprintf(stderr, "widebrim: failed to add title sprite asset\n");
        return;
    }

    sprite_layer_center_sprite(title, WB_SCREEN_WIDTH, WB_SCREEN_HEIGHT);
    sprite_layer_set_sprite_position(title, title->x, title->y - 40);
}

static bool mode_title_is_done(void *user) {
    if (!user) {
        fprintf(stderr,
                "widebrim: mode_title_is_done called with NULL user pointer\n");
        return false;
    }

    mode_title_impl_t *impl = (mode_title_impl_t *)user;
    return impl->done;
}

static void mode_title_destroy(void *user) {
    if (!user) {
        fprintf(stderr,
                "widebrim: mode_title_destroy called with NULL user pointer\n");
        return;
    }

    free(user);
}

static bool mode_title_advance(mode_title_impl_t *impl) {
    if (!impl) {
        fprintf(stderr,
                "widebrim: mode_title_advance called with NULL impl pointer\n");
        return false;
    }

    fprintf(stderr,
            "widebrim: mode_title_advance called, advancing to next state\n");

    impl->done = true;
    return true;
}

static bool mode_title_handle_event(void *user, const input_event_t *event) {
    if (!user) {
        fprintf(stderr, "widebrim: mode_title_handle_event called with NULL "
                        "user pointer\n");
        return false;
    }

    mode_title_impl_t *impl = (mode_title_impl_t *)user;
    if (event) {
        switch (event->type) {
        case INPUT_EVENT_KEY_DOWN:
        case INPUT_EVENT_MOUSE_BUTTON_DOWN:
            return mode_title_advance(impl);
        default:
            return false;
        }
    }

    return false;
}

mode_handler_t mode_title_create(game_state_t *state,
                                 screen_controller_t *controller) {
    mode_handler_t handler;
    mode_title_impl_t *impl =
        (mode_title_impl_t *)malloc(sizeof(mode_title_impl_t));

    if (!impl) {
        fprintf(stderr,
                "widebrim: failed to allocate memory for mode_title_impl_t\n");
        exit(EXIT_FAILURE);
    }

    if (!state) {
        fprintf(stderr,
                "widebrim: mode_title_create called with NULL state pointer\n");
        exit(EXIT_FAILURE);
    }

    if (!controller) {
        fprintf(stderr, "widebrim: mode_title_create called with NULL "
                        "controller pointer\n");
        exit(EXIT_FAILURE);
    }

    impl->state = state;
    impl->done = false;

    const char *bg_path = "bg/select_title.png";
    const char *sub_bg_path = "bg/start_select2.png";
    const char *sub_bg_overlay_path = "bg/start_select.png";

    bg_loader_load(state, controller, bg_path, screen_controller_set_bg_main);
    bg_loader_load(state, controller, sub_bg_path,
                   screen_controller_set_bg_sub);
    bg_loader_load(state, controller, sub_bg_overlay_path,
                   screen_controller_set_bg_sub2);

    mode_title_load_start_car_sprite(state, controller);
    mode_title_load_title_sprite(state, controller);

    screen_controller_set_bg_sub_scroll(controller, -45.0f, true);
    screen_controller_set_bg_sub2_scroll(controller, -90.0f, true);

    screen_controller_fade_in(controller, FADER_DEFAULT_DURATION_MS, NULL,
                              NULL);

    handler.layer.impl = impl;
    handler.layer.update = NULL;
    handler.layer.draw = NULL;
    handler.layer.handle_event = mode_title_handle_event;
    handler.layer.on_quit = NULL;
    handler.layer.destroy = mode_title_destroy;
    handler.is_done = mode_title_is_done;
    handler.valid = true;

    return handler;
}
