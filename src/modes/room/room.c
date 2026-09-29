#include "room.h"
#include "event.h"
#include "exit.h"
#include "map.h"
#include "move.h"
#include "textobj.h"

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

#include "safe.h"
#include "script.h"

static bool mode_room_load_and_execute_script(mode_room_impl_t *impl,
                                              int room_num) {
    char script_path[32];

    snprintf(script_path, sizeof(script_path), "script/rooms/room%d_in.gds",
             room_num);
    if (!script_load_and_execute(impl->base.state, script_path, impl,
                                 &impl->base.state->gds)) {
        return false;
    }

    if (impl->base.done) {
        return true;
    }

    snprintf(script_path, sizeof(script_path), "script/rooms/room%d_param.gds",
             room_num);
    if (!script_load_and_execute(impl->base.state, script_path, impl,
                                 &impl->base.state->gds)) {
        return false;
    }

    return true;
}

static void mode_room_reset_room(mode_room_impl_t *impl) {
    if (!impl || !impl->base.state || !impl->base.controller) {
        return;
    }

    game_state_t *state = impl->base.state;
    bg_layer_t *bg = impl->base.controller->bg;

    int room_num = game_state_get_place_num(state);
    char bg_sub_path[256];
    snprintf(bg_sub_path, sizeof(bg_sub_path), "room_%d_bg.bgx", room_num);

    if (!bg->load_sub(bg, state, bg_sub_path)) {
        fprintf(stderr, "widebrim: failed to load room %d background\n",
                room_num);
    }

    object_layer_remove_all_objects(impl->base.controller->object);

    object_layer_add_object(impl->base.controller->object, impl->map_place);
    object_layer_add_object(impl->base.controller->object, impl->map_purpose);
    object_layer_add_object(impl->base.controller->object, impl->move_mode_btn);

    set_move_mode(impl, false);
    object_reset_fade(impl->move_mode_btn, impl->base.controller->renderer);

    impl->exit_count = 0;
    impl->tobj_count = 0;
    impl->event_count = 0;
    impl->base.done = false;

    if (!mode_room_load_and_execute_script(impl, room_num)) {
        fprintf(stderr, "widebrim: failed to reset room %d script\n", room_num);
    }

    if (!impl->base.done) {
        screen_controller_fade_in(impl->base.controller,
                                  FADER_DEFAULT_DURATION_MS, NULL, NULL);
    }
}

static void mode_room_reload_room_after_fade_out(void *user) {
    mode_room_impl_t *impl = (mode_room_impl_t *)user;
    text_layer_clear(impl->base.controller->text);
    mode_room_reset_room(impl);
}

void mode_room_reload_room(mode_room_impl_t *impl) {
    if (!impl || !impl->base.state || !impl->base.controller) {
        return;
    }

    screen_controller_fade_out(impl->base.controller, FADER_DEFAULT_DURATION_MS,
                               mode_room_reload_room_after_fade_out, impl);
}

static bool mode_room_is_done(void *user) {
    if (!user) {
        fprintf(stderr,
                "widebrim: mode_room_is_done called with NULL user pointer\n");
        return false;
    }

    mode_room_impl_t *impl = (mode_room_impl_t *)user;
    return impl->base.done;
}

static void mode_room_destroy(void *user) {
    if (!user) {
        fprintf(stderr,
                "widebrim: mode_room_destroy called with NULL user pointer\n");
        return;
    }

    mode_room_impl_t *impl = (mode_room_impl_t *)user;

    object_layer_clear(impl->base.controller->object);

    if (impl->move_mode_btn) {
        object_destroy(impl->move_mode_btn, impl->base.controller->renderer);
        free(impl->move_mode_btn);
        impl->move_mode_btn = NULL;
    }
    if (impl->map_place) {
        object_destroy(impl->map_place, impl->base.controller->renderer);
        free(impl->map_place);
        impl->map_place = NULL;
    }
    if (impl->map_purpose) {
        object_destroy(impl->map_purpose, impl->base.controller->renderer);
        free(impl->map_purpose);
        impl->map_purpose = NULL;
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

    if (impl->base.controller && impl->base.controller->object &&
        object_layer_handle_event(impl->base.controller->object, event)) {
        return true;
    }

    if (event->type == INPUT_EVENT_MOUSE_BUTTON_UP && impl->in_move_mode) {
        bool clicked_empty_bg = true;
        for (int i = 0; i < impl->exit_count; ++i) {
            object_t *exit = impl->exits[i];
            if (exit && exit->visible &&
                object_contains_point(exit, event->data.mouse_button.x,
                                      event->data.mouse_button.y)) {
                clicked_empty_bg = false;
                break;
            }
        }

        if (clicked_empty_bg && impl->move_mode_btn &&
            impl->move_mode_btn->visible &&
            object_contains_point(impl->move_mode_btn,
                                  event->data.mouse_button.x,
                                  event->data.mouse_button.y)) {
            clicked_empty_bg = false;
        }

        if (clicked_empty_bg) {
            toggle_move_mode(impl);
            return true;
        }
    }

    int place_num = game_state_get_place_num(impl->base.state);

    switch (event->type) {
    case INPUT_EVENT_KEY_DOWN:
        switch (event->data.key.key) {
        case 'm':
        case 'M':
            toggle_move_mode(impl);
            return true;
        case 'q':
            if (place_num > 1) {
                game_state_set_place_num(impl->base.state, place_num - 1);
                mode_room_reload_room(impl);
                printf("widebrim: moved to room %d\n", place_num - 1);
                return true;
            }
            break;
        case 'w':
            game_state_set_place_num(impl->base.state, place_num + 1);
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

    impl->base.state = state;
    impl->base.controller = controller;
    impl->tobj_count = 0;
    impl->exit_count = 0;

    impl->set_map = mode_room_set_map;
    impl->add_text_obj = mode_room_add_text_obj;
    impl->add_exit = room_add_exit;
    impl->add_event = room_add_event;

    impl->move_mode_btn = smalloc(sizeof(object_t));
    impl->map_place = smalloc(sizeof(object_t));
    impl->map_purpose = smalloc(sizeof(object_t));
    for (int i = 0; i < MAX_TOBJ; i++) {
        impl->text_obj[i] = smalloc(sizeof(object_t));
    }
    for (int i = 0; i < MAX_EXITS; i++) {
        impl->exits[i] = smalloc(sizeof(object_t));
    }
    for (int i = 0; i < MAX_EVENTS; i++) {
        impl->event[i] = smalloc(sizeof(object_t));
    }

    mode_room_load_move_mode_btn(impl);
    mode_room_load_map_place(impl);
    mode_room_load_map_purpose(impl);

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
