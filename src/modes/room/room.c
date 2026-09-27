#include "room.h"
#include "exit.h"
#include "map.h"
#include "textobj.h"

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

#include "bg_loader.h"
#include "safe.h"

#include "gds/gds.h"
#include "gds/gds_exec.h"

#define MOVE_MODE_TRANSITION 250.0f

static void toggle_move_mode(mode_room_impl_t *impl) {
    if (!impl) {
        return;
    }

    impl->in_move_mode = !impl->in_move_mode;
    if (impl->in_move_mode) {
        object_fade_out(impl->move_mode_btn, impl->controller->renderer,
                        MOVE_MODE_TRANSITION);
        for (int i = 0; i < impl->exit_count; i++) {
            object_fade_in(impl->exits[i].object, impl->controller->renderer,
                           MOVE_MODE_TRANSITION);
        }
    } else {
        object_fade_in(impl->move_mode_btn, impl->controller->renderer,
                       MOVE_MODE_TRANSITION);
        for (int i = 0; i < impl->exit_count; i++) {
            object_fade_out(impl->exits[i].object, impl->controller->renderer,
                            MOVE_MODE_TRANSITION);
        }
    }
}

static bool mode_room_on_move_mode_icon_click(void *user,
                                              const input_event_t *event,
                                              object_t *obj) {
    (void)obj;
    if (!user) {
        fprintf(stderr, "widebrim: mode_room_on_move_mode_icon_click called "
                        "with NULL user pointer\n");
        return false;
    }

    mode_room_impl_t *impl = (mode_room_impl_t *)user;
    static bool clicked = false;

    switch (event->type) {
    case INPUT_EVENT_MOUSE_BUTTON_DOWN:
        clicked = true;
        sprite_set_position(impl->move_mode_btn->sprite,
                            impl->move_mode_btn->x + 5,
                            impl->move_mode_btn->y + 5);
        break;
    case INPUT_EVENT_MOUSE_BUTTON_UP:
        sprite_set_position(impl->move_mode_btn->sprite, impl->move_mode_btn->x,
                            impl->move_mode_btn->y);
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

static void mode_room_load_move_mode_btn(mode_room_impl_t *impl) {
    if (!impl) {
        return;
    }

    renderer_t *renderer = impl->controller->renderer;
    game_state_t *state = impl->state;

    object_t *btn = impl->move_mode_btn;
    if (!btn) {
        fprintf(stderr, "widebrim: failed to create move mode button\n");
        return;
    }

    object_clear(btn, renderer);

    sprite_t *spr = btn->sprite;
    sprite_new(spr, renderer, state, "movemode.spr", 0.0f, false);

    if (!spr) {
        fprintf(stderr,
                "widebrim: failed to create sprite for move mode button\n");
        return;
    }

    int spr_w, spr_h;
    sprite_get_size(spr, renderer, &spr_w, &spr_h);
    int x = WB_SCREEN_WIDTH - spr_w - 20;
    int y = WB_SCREEN_HEIGHT * 2 - spr_h - 20;

    object_set_size(btn, spr_w, spr_h);
    object_set_position(btn, x, y);
    sprite_take_object_position(spr, btn);
    object_set_interactive(btn, true, mode_room_on_move_mode_icon_click, impl);
}

static bool mode_room_load_and_execute_script(mode_room_impl_t *impl,
                                              int room_num) {
    char script_path[256];
    char resolved_path[1024];
    const uint8_t *script_payload = NULL;
    size_t script_payload_size = 0U;

    snprintf(script_path, sizeof(script_path), "script/rooms/room%d_param.gds",
             room_num);

    if (!asset_path_resolve(impl->state->assets_root, impl->state->language,
                            script_path, resolved_path,
                            sizeof(resolved_path))) {
        fprintf(stderr, "widebrim: Failed to resolve asset path for %s\n",
                script_path);
        return false;
    }

    fprintf(stderr, "widebrim: Loading script for room %d: %s\n", room_num,
            script_path);
    if (!gds_load_from_file_path(resolved_path, &script_payload,
                                 &script_payload_size)) {
        fprintf(stderr, "widebrim: Failed to load script for room %d: %s\n",
                room_num, script_path);
        return false;
    }

    if (!gds_execute_script(script_payload, script_payload_size, impl)) {
        fprintf(stderr, "widebrim: failed to execute script for room %d: %s\n",
                room_num, script_path);
        gds_free_payload(script_payload);
        return false;
    }

    gds_free_payload(script_payload);
    return true;
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

static void mode_room_reset_room(mode_room_impl_t *impl) {
    if (!impl || !impl->state || !impl->controller) {
        return;
    }

    int room_num = game_state_get_place_num(impl->state);
    char bg_sub_path[256];
    snprintf(bg_sub_path, sizeof(bg_sub_path), "bg/room_%d_bg.png", room_num);

    if (!bg_loader_load(impl->state, impl->controller, bg_sub_path,
                        screen_controller_set_bg_sub)) {
        fprintf(stderr, "widebrim: failed to reload room %d background\n",
                room_num);
    }

    object_layer_remove_all_objects(impl->controller->object);

    object_layer_add_object(impl->controller->object, impl->map_place);
    object_layer_add_object(impl->controller->object, impl->move_mode_btn);

    if (!mode_room_load_and_execute_script(impl, room_num)) {
        fprintf(stderr, "widebrim: failed to reset room %d script\n", room_num);
    }

    screen_controller_fade_in(impl->controller, FADER_DEFAULT_DURATION_MS, NULL,
                              NULL);
    impl->done = false;
    impl->in_move_mode = false;
    bg_layer_set_touch_callback(impl->controller->bg,
                                mode_room_on_background_touch, impl);
}

static void mode_room_reload_room_after_fade_out(void *user) {
    mode_room_impl_t *impl = (mode_room_impl_t *)user;
    text_layer_clear(impl->controller->text);
    mode_room_reset_room(impl);
}

static void mode_room_reload_room(mode_room_impl_t *impl) {
    if (!impl || !impl->state || !impl->controller) {
        return;
    }

    screen_controller_fade_out(impl->controller, FADER_DEFAULT_DURATION_MS,
                               mode_room_reload_room_after_fade_out, impl);
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

static void mode_room_destroy(void *user) {
    if (!user) {
        fprintf(stderr,
                "widebrim: mode_room_destroy called with NULL user pointer\n");
        return;
    }

    mode_room_impl_t *impl = (mode_room_impl_t *)user;

    object_layer_clear(impl->controller->object);

    if (impl->move_mode_btn) {
        object_destroy(impl->move_mode_btn, impl->controller->renderer);
        free(impl->move_mode_btn);
        impl->move_mode_btn = NULL;
    }
    if (impl->map_place) {
        object_destroy(impl->map_place, impl->controller->renderer);
        free(impl->map_place);
        impl->map_place = NULL;
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

    if (impl->controller && impl->controller->object &&
        object_layer_handle_event(impl->controller->object, event)) {
        return true;
    }

    int place_num = game_state_get_place_num(impl->state);

    switch (event->type) {
    case INPUT_EVENT_KEY_DOWN:
        switch (event->data.key.key) {
        case 'm':
        case 'M':
            toggle_move_mode(impl);
            return true;
        case 'q':
            if (place_num > 1) {
                game_state_set_place_num(impl->state, place_num - 1);
                mode_room_reload_room(impl);
                printf("widebrim: moved to room %d\n", place_num - 1);
                return true;
            }
            break;
        case 'w':
            game_state_set_place_num(impl->state, place_num + 1);
            mode_room_reload_room(impl);
            printf("widebrim: moved to room %d\n", place_num + 1);
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
    mode_handler_t handler = {0};
    mode_room_impl_t *impl = smalloc(sizeof(mode_room_impl_t));

    impl->state = state;
    impl->controller = controller;
    impl->exit_count = 0;

    impl->setmap = mode_room_setmap;
    impl->add_text_obj = mode_room_add_text_obj;
    impl->add_exit = room_add_exit;

    impl->move_mode_btn = smalloc(sizeof(object_t));
    impl->map_place = smalloc(sizeof(object_t));

    mode_room_load_move_mode_btn(impl);
    mode_room_load_map_place(impl);

    mode_room_reset_room(impl);

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
