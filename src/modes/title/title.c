#include "title.h"

#include <stdlib.h>

#include "safe.h"
#include "sprite.h"

static bool mode_title_advance(mode_title_impl_t *impl);

static bool mode_title_on_sprite_click(void *user, const input_event_t *event,
                                       object_t *obj) {
    mode_title_impl_t *impl = (mode_title_impl_t *)user;

    if (!impl || !event) {
        return false;
    }

    switch (event->type) {
    case INPUT_EVENT_MOUSE_BUTTON_DOWN:
        if (obj && obj->sprite->frame_count > 1U) {
            sprite_set_playing(obj->sprite, false);
            sprite_set_frame(obj->sprite, 1U);
        }
        impl->active_click_btn = obj;
        return true;

    case INPUT_EVENT_MOUSE_BUTTON_UP: {
        if (obj != impl->active_click_btn) {
            return false;
        }

        impl->active_click_btn = NULL;

        if (obj && obj->sprite->frame_count > 1U) {
            sprite_set_playing(obj->sprite, false);
            sprite_set_frame(obj->sprite, 0U);
        }

        if (obj == impl->start_btn) {
            printf("widebrim: start button clicked\n");
        }
        if (obj == impl->continue_btn) {
            return mode_title_advance(impl);
        }
        if (obj == impl->bonus_btn) {
            printf("widebrim: bonus button clicked\n");
        }

        return false;
    }

    default:
        return false;
    }
}

static void mode_title_load_start_car(mode_title_impl_t *impl) {
    if (!impl) {
        return;
    }

    renderer_t *renderer = impl->base.controller->renderer;
    game_state_t *state = impl->base.state;

    object_t *car = impl->start_car;
    object_init(car);

    if (!car) {
        fprintf(stderr, "widebrim: failed to initialize start_car object\n");
        return;
    }

    sprite_t *spr = car->sprite;
    sprite_new(spr, renderer, state, "start_car.spr", 100.0f, true);

    if (!spr) {
        fprintf(stderr, "widebrim: failed to create start_car sprite\n");
        return;
    }

    int spr_w, spr_h;
    sprite_get_size(spr, renderer, &spr_w, &spr_h);
    object_set_size(car, spr_w, spr_h);

    object_center_position(car, WB_SCREEN_WIDTH, WB_SCREEN_HEIGHT);
    object_set_position(car, car->x, car->y + WB_SCREEN_HEIGHT + 150);
    sprite_take_object_position(spr, car);
}

static void mode_title_load_title_sprite(mode_title_impl_t *impl) {
    if (!impl) {
        return;
    }

    renderer_t *renderer = impl->base.controller->renderer;
    game_state_t *state = impl->base.state;

    object_t *logo = impl->title_logo;
    object_init(logo);

    if (!logo) {
        fprintf(stderr, "widebrim: failed to initialize title_logo object\n");
        return;
    }

    sprite_t *spr = logo->sprite;
    sprite_new(spr, renderer, state, "title_logo.spr", 0.0f, false);

    if (!spr) {
        fprintf(stderr, "widebrim: failed to add title sprite asset\n");
        return;
    }

    int spr_w, spr_h;
    sprite_get_size(spr, renderer, &spr_w, &spr_h);
    object_set_size(logo, spr_w, spr_h);

    object_center_position(logo, WB_SCREEN_WIDTH, WB_SCREEN_HEIGHT);
    object_set_position(logo, logo->x, logo->y - 40);
    object_set_interactive(logo, true);
    object_set_interaction_callback(logo, mode_title_on_sprite_click, impl);
    sprite_take_object_position(spr, logo);
}

static void mode_title_load_button_sprites(mode_title_impl_t *impl) {
    if (!impl) {
        return;
    }

    game_state_t *state = impl->base.state;
    screen_controller_t *controller = impl->base.controller;

    object_t *buttons[3] = {impl->start_btn, impl->continue_btn,
                            impl->bonus_btn};
    const char *sprite_names[3] = {"startbutton.spr", "continuebutton.spr",
                                   "secretbutton.spr"};

    int y_pos = WB_SCREEN_HEIGHT + 160;
    const int offset = 75;

    for (int i = 0; i < 3; i++) {
        object_init(buttons[i]);
        if (!buttons[i]) {
            fprintf(stderr, "widebrim: failed to initialize button object\n");
            return;
        }

        sprite_new(buttons[i]->sprite, controller->renderer, state,
                   sprite_names[i], 0.0f, false);
        if (!buttons[i]->sprite) {
            fprintf(stderr,
                    "widebrim: failed to create sprite for button object\n");
            return;
        }

        int spr_w, spr_h;
        sprite_get_size(buttons[i]->sprite, controller->renderer, &spr_w,
                        &spr_h);
        object_set_size(buttons[i], spr_w, spr_h);

        object_center_position(buttons[i], WB_SCREEN_WIDTH, WB_SCREEN_HEIGHT);
        object_set_position(buttons[i], buttons[i]->x, y_pos + i * offset);
        object_set_interactive(buttons[i], true);
        object_set_interaction_callback(buttons[i], mode_title_on_sprite_click,
                                        impl);
        sprite_take_object_position(buttons[i]->sprite, buttons[i]);
    }
}

static void mode_title_load_sprites(mode_title_impl_t *impl) {
    mode_title_load_start_car(impl);
    mode_title_load_title_sprite(impl);
    mode_title_load_button_sprites(impl);
}

static void mode_title_load_bg(mode_title_impl_t *impl) {
    const char *bg_path = "select_title.bgx";
    const char *sub_bg_path = "start_select2.bgx";
    const char *sub_bg_overlay_path = "start_select.bgx";

    game_state_t *state = impl->base.state;
    bg_layer_t *bg = impl->base.controller->bg;

    bg->load(bg, state, bg_path);
    bg->load_sub(bg, state, sub_bg_path);
    bg->load_sub2(bg, state, sub_bg_overlay_path);

    bg->set_sub_scroll(bg, -45.0f, true);
    bg->set_sub2_scroll(bg, -90.0f, true);
}

static bool mode_title_is_done(void *user) {
    if (!user) {
        fprintf(stderr,
                "widebrim: mode_title_is_done called with NULL user pointer\n");
        return false;
    }

    mode_title_impl_t *impl = (mode_title_impl_t *)user;
    return impl->base.done;
}

static void mode_title_destroy(void *user) {
    if (!user) {
        fprintf(stderr,
                "widebrim: mode_title_destroy called with NULL user pointer\n");
        return;
    }

    mode_title_impl_t *impl = (mode_title_impl_t *)user;
    if (impl->base.controller) {
        screen_controller_clear_object_layer(impl->base.controller);
        screen_controller_clear_bg_layer(impl->base.controller);
    }

    free(impl);
}

static bool mode_title_advance(mode_title_impl_t *impl) {
    if (!impl) {
        fprintf(stderr,
                "widebrim: mode_title_advance called with NULL impl pointer\n");
        return false;
    }

    if (impl->base.done) {
        return false;
    }

    game_state_set_mode(impl->base.state, MODE_ROOM);
    game_state_set_place_num(impl->base.state, 1);

    impl->base.done = true;
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

    if (impl->base.controller && impl->base.controller->object &&
        object_layer_handle_event(impl->base.controller->object, event)) {
        return true;
    }

    return false;
}

mode_handler_t mode_title_create(game_state_t *state,
                                 screen_controller_t *controller) {
    mode_handler_t handler;
    mode_title_impl_t *impl =
        (mode_title_impl_t *)smalloc(sizeof(mode_title_impl_t));

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

    impl->base.state = state;
    impl->base.controller = controller;
    impl->base.done = false;

    object_t **objects[] = {&impl->start_car, &impl->title_logo,
                            &impl->start_btn, &impl->continue_btn,
                            &impl->bonus_btn};

    for (size_t i = 0; i < sizeof(objects) / sizeof(objects[0]); i++) {
        *objects[i] = smalloc(sizeof(object_t));

        if (*objects[i]) {
            object_layer_add_object(controller->object, *objects[i]);
        }
    }

    mode_title_load_bg(impl);
    mode_title_load_sprites(impl);

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
