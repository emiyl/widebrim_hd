#include "mode_title.h"

#include <stdlib.h>

#include "bg_loader.h"

typedef struct {
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

static bool mode_title_advance(mode_title_impl_t *impl);

static bool mode_title_on_sprite_click(void *user, const input_event_t *event,
                                       sprite_instance_t *sprite) {
    mode_title_impl_t *impl = (mode_title_impl_t *)user;

    (void)event;
    (void)sprite;

    if (!impl) {
        return false;
    }

    switch (event->type) {
    case INPUT_EVENT_MOUSE_BUTTON_DOWN:
        if (sprite && sprite->frame_count > 1U) {
            sprite_layer_set_playing(sprite, false);
            sprite_layer_set_frame(sprite, 1U);
        }
        impl->active_click_sprite = sprite;
        return true;
    case INPUT_EVENT_MOUSE_BUTTON_UP:
        if (sprite == impl->active_click_sprite) {
            impl->active_click_sprite = NULL;
            if (sprite && sprite->frame_count > 1U) {
                sprite_layer_set_playing(sprite, false);
                sprite_layer_set_frame(sprite, 0U);
            }
            return mode_title_advance(impl);
        }
        return false;
    default:
        return false;
    }
}

static void mode_title_load_start_car_sprite(mode_title_impl_t *impl,
                                             game_state_t *state,
                                             screen_controller_t *controller) {
    if (!impl || !state || !controller) {
        return;
    }

    impl->start_car_sprite = screen_controller_add_sprite_asset(
        controller, state, "ani/start_car.spr", 0, 0, 0, 255U, 100.0f, true);
    if (!impl->start_car_sprite) {
        fprintf(stderr, "widebrim: failed to add start_car sprite asset\n");
        return;
    }

    sprite_layer_center_sprite(impl->start_car_sprite, WB_SCREEN_WIDTH,
                               WB_SCREEN_HEIGHT);
    sprite_layer_set_sprite_position(
        impl->start_car_sprite, impl->start_car_sprite->x,
        impl->start_car_sprite->y + WB_SCREEN_HEIGHT + 150);
}

static void mode_title_load_title_sprite(mode_title_impl_t *impl,
                                         game_state_t *state,
                                         screen_controller_t *controller) {
    if (!impl || !state || !controller) {
        return;
    }

    impl->title_sprite = screen_controller_add_sprite_asset(
        controller, state, "ani/title_logo.spr", 0, 0, 0, 255U, 0.0f, false);
    if (!impl->title_sprite) {
        fprintf(stderr, "widebrim: failed to add title sprite asset\n");
        return;
    }

    sprite_layer_center_sprite(impl->title_sprite, WB_SCREEN_WIDTH,
                               WB_SCREEN_HEIGHT);
    sprite_layer_set_sprite_position(impl->title_sprite, impl->title_sprite->x,
                                     impl->title_sprite->y - 40);
    sprite_layer_set_interactive(impl->title_sprite, true,
                                 mode_title_on_sprite_click, impl);
}

static void mode_title_load_button_sprites(mode_title_impl_t *impl,
                                           game_state_t *state,
                                           screen_controller_t *controller) {
    if (!impl || !state || !controller) {
        return;
    }

    int y_pos = WB_SCREEN_HEIGHT + 160;
    const int offset = 75;
    int index = 0;

    impl->start_button = screen_controller_add_sprite_asset(
        controller, state, "ani/startbutton.spr", 0, 0, 0, 255U, 0.0f, false);
    if (!impl->start_button) {
        fprintf(stderr, "widebrim: failed to add start_button sprite asset\n");
        return;
    } else {
        sprite_layer_center_sprite(impl->start_button, WB_SCREEN_WIDTH,
                                   WB_SCREEN_HEIGHT);
        sprite_layer_set_sprite_position(impl->start_button,
                                         impl->start_button->x,
                                         y_pos + index++ * offset);
        sprite_layer_set_interactive(impl->start_button, true,
                                     mode_title_on_sprite_click, impl);
    }

    impl->continue_button = screen_controller_add_sprite_asset(
        controller, state, "ani/continuebutton.spr", 0, 0, 0, 255U, 0.0f,
        false);
    if (!impl->continue_button) {
        fprintf(stderr,
                "widebrim: failed to add continue_button sprite asset\n");
    } else {
        sprite_layer_center_sprite(impl->continue_button, WB_SCREEN_WIDTH,
                                   WB_SCREEN_HEIGHT);
        sprite_layer_set_sprite_position(impl->continue_button,
                                         impl->continue_button->x,
                                         y_pos + index++ * offset);
        sprite_layer_set_interactive(impl->continue_button, true,
                                     mode_title_on_sprite_click, impl);
    }

    impl->bonus_button = screen_controller_add_sprite_asset(
        controller, state, "ani/secretbutton.spr", 0, 0, 0, 255U, 0.0f, false);
    if (!impl->bonus_button) {
        fprintf(stderr, "widebrim: failed to add bonus_button sprite asset\n");
    } else {
        sprite_layer_center_sprite(impl->bonus_button, WB_SCREEN_WIDTH,
                                   WB_SCREEN_HEIGHT);
        sprite_layer_set_sprite_position(impl->bonus_button,
                                         impl->bonus_button->x,
                                         y_pos + index++ * offset);
        sprite_layer_set_interactive(impl->bonus_button, true,
                                     mode_title_on_sprite_click, impl);
    }
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

    mode_title_impl_t *impl = (mode_title_impl_t *)user;
    if (impl->controller) {
        screen_controller_clear_sprite_layer(impl->controller);
        screen_controller_clear_bg_layer(impl->controller);
    }

    free(impl);
}

static bool mode_title_advance(mode_title_impl_t *impl) {
    if (!impl) {
        fprintf(stderr,
                "widebrim: mode_title_advance called with NULL impl pointer\n");
        return false;
    }

    if (impl->done) {
        return false;
    }

    fprintf(stderr,
            "widebrim: mode_title_advance called, advancing to MODE_ROOM\n");
    game_state_set_mode(impl->state, MODE_ROOM);

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
    if (!event) {
        return false;
    }

    if (impl->controller && impl->controller->sprite &&
        sprite_layer_handle_event(impl->controller->sprite, event)) {
        return true;
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
    impl->controller = controller;
    impl->done = false;
    impl->start_car_sprite = NULL;
    impl->title_sprite = NULL;
    impl->active_click_sprite = NULL;

    const char *bg_path = "bg/select_title.png";
    const char *sub_bg_path = "bg/start_select2.png";
    const char *sub_bg_overlay_path = "bg/start_select.png";

    bg_loader_load(state, controller, bg_path, screen_controller_set_bg_main);
    bg_loader_load(state, controller, sub_bg_path,
                   screen_controller_set_bg_sub);
    bg_loader_load(state, controller, sub_bg_overlay_path,
                   screen_controller_set_bg_sub2);

    mode_title_load_start_car_sprite(impl, state, controller);
    mode_title_load_title_sprite(impl, state, controller);
    mode_title_load_button_sprites(impl, state, controller);

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
