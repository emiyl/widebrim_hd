#include "mode_room.h"

#include <stdio.h>
#include <stdlib.h>

#include "bg_loader.h"

#define MOVE_MODE_ICON_SIZE 84
#define MOVE_MODE_TRANSITION 250.0f

typedef struct {
    game_state_t *state;
    screen_controller_t *controller;
    sprite_instance_t *move_mode_sprite;
    bool done;
    bool in_move_mode;
    bool room_touch_pending;
    int room_touch_start_x;
    int room_touch_start_y;
    int room_touch_last_x;
    int room_touch_last_y;
    bool room_touch_dragged;
} mode_room_impl_t;

static void toggle_move_mode(mode_room_impl_t *impl) {
    if (!impl) {
        return;
    }

    impl->in_move_mode = !impl->in_move_mode;
    if (impl->in_move_mode) {
        sprite_layer_fade_out(impl->move_mode_sprite, MOVE_MODE_TRANSITION);
    } else {
        sprite_layer_fade_in(impl->move_mode_sprite, MOVE_MODE_TRANSITION);
    }
}

static bool mode_room_on_move_mode_icon_click(void *user,
                                              const input_event_t *event,
                                              sprite_instance_t *sprite) {
    (void)sprite;
    if (!user) {
        fprintf(stderr, "widebrim: mode_room_on_move_mode_icon_click called "
                        "with NULL user pointer\n");
        return false;
    }

    mode_room_impl_t *impl = (mode_room_impl_t *)user;
    static bool clicked = false;
    static bool sprite_is_offset = false;
    const int clicked_offset = 5;

    switch (event->type) {
    case INPUT_EVENT_MOUSE_BUTTON_DOWN:
        clicked = true;
        if (!sprite_is_offset) {
            sprite_layer_set_sprite_position(
                impl->move_mode_sprite,
                impl->move_mode_sprite->x + clicked_offset,
                impl->move_mode_sprite->y + clicked_offset);
            sprite_is_offset = true;
        }
        break;
    case INPUT_EVENT_MOUSE_BUTTON_UP:
        if (sprite_is_offset) {
            sprite_layer_set_sprite_position(
                impl->move_mode_sprite,
                impl->move_mode_sprite->x - clicked_offset,
                impl->move_mode_sprite->y - clicked_offset);
            sprite_is_offset = false;
        }
        if (clicked) {
            clicked = false;
            toggle_move_mode(impl);
            return true;
        }
        break;
    default:
        break;
    }

    return false;
}

static void mode_room_load_move_mode_sprite(mode_room_impl_t *impl,
                                            game_state_t *state,
                                            screen_controller_t *controller) {
    if (!impl) {
        return;
    }

    const int x = WB_SCREEN_WIDTH - MOVE_MODE_ICON_SIZE - 20;
    const int y = WB_SCREEN_HEIGHT * 2 - MOVE_MODE_ICON_SIZE - 20;

    impl->move_mode_sprite = screen_controller_add_sprite_asset(
        controller, state, "ani/movemode.spr", x, y, 0, 255, 0.0f, false);
    if (!impl->move_mode_sprite) {
        fprintf(stderr, "widebrim: failed to load move mode sprite\n");
        return;
    }

    sprite_layer_set_interactive(impl->move_mode_sprite, true,
                                 mode_room_on_move_mode_icon_click, impl);
}

static bool mode_room_is_done(void *user) {
    if (!user) {
        fprintf(stderr,
                "widebrim: mode_room_is_done called with NULL user pointer\n");
        return false;
    }

    mode_room_impl_t *impl = (mode_room_impl_t *)user;
    return impl->done;
}

static void mode_room_on_background_touch(void *user, bg_touch_kind_t kind,
                                          int x, int y) {
    (void)x;
    (void)y;
    mode_room_impl_t *impl = (mode_room_impl_t *)user;
    if (!impl) {
        return;
    }

    if (kind == BG_TOUCH_KIND_TAP && impl->in_move_mode) {
        toggle_move_mode(impl);
        return;
    }

    switch (kind) {
    case BG_TOUCH_KIND_NONE:
    case BG_TOUCH_KIND_TAP:
    case BG_TOUCH_KIND_DRAG:
        break;
    }
}

static void mode_room_destroy(void *user) {
    if (!user) {
        fprintf(stderr,
                "widebrim: mode_room_destroy called with NULL user pointer\n");
        return;
    }

    free(user);
}

static bool mode_room_handle_event(void *user, const input_event_t *event) {
    if (!user) {
        return false;
    }

    mode_room_impl_t *impl = (mode_room_impl_t *)user;
    if (!impl) {
        return false;
    }

    switch (event->type) {
    case INPUT_EVENT_KEY_DOWN:
        switch (event->data.key.key) {
        case 'm':
        case 'M':
            toggle_move_mode(impl);
            return true;
        default:
            break;
        }
        break;
    default:
        break;
    }

    return false;
}

mode_handler_t mode_room_create(game_state_t *state,
                                screen_controller_t *controller) {
    mode_handler_t handler;
    mode_room_impl_t *impl = malloc(sizeof(mode_room_impl_t));
    if (!impl) {
        fprintf(stderr, "Failed to allocate memory for mode_room_impl_t\n");
        exit(EXIT_FAILURE);
    }

    int room_num = game_state_get_place_num(state);
    char bg_sub_path[256];
    snprintf(bg_sub_path, sizeof(bg_sub_path), "bg/room_%d_bg.png", room_num);

    bg_loader_load(state, controller, bg_sub_path,
                   screen_controller_set_bg_sub);
    mode_room_load_move_mode_sprite(impl, state, controller);

    screen_controller_fade_in(controller, FADER_DEFAULT_DURATION_MS, NULL,
                              NULL);

    impl->state = state;
    impl->controller = controller;
    impl->done = false;
    impl->in_move_mode = false;

    bg_layer_set_touch_callback(controller->bg, mode_room_on_background_touch,
                                impl);

    handler.layer.impl = impl;
    handler.layer.update = NULL;
    handler.layer.draw = NULL;
    handler.layer.handle_event = mode_room_handle_event;
    handler.layer.on_quit = NULL;
    handler.layer.destroy = mode_room_destroy;
    handler.is_done = mode_room_is_done;
    handler.valid = true;
    return handler;
}
