#include "move.h"

#include <stdbool.h>
#include <stdio.h>

void set_move_mode(mode_room_impl_t *impl, bool enable) {
    if (!impl) {
        return;
    }

    impl->in_move_mode = enable;

    bool allow_exit_interaction = impl->in_move_mode;
    for (int i = 0; i < impl->exit_count; i++) {
        object_t *exit = impl->exits[i];
        if (!exit) {
            continue;
        }
        object_set_interactive(exit, allow_exit_interaction);
    }

    bool allow_tobj_interaction = !impl->in_move_mode;
    for (int i = 0; i < impl->tobj_count; i++) {
        object_t *text_obj = impl->text_obj[i];
        if (!text_obj) {
            continue;
        }
        object_set_interactive(text_obj, allow_tobj_interaction);
    }
}

void toggle_move_mode(mode_room_impl_t *impl) {
    if (!impl) {
        return;
    }

    set_move_mode(impl, !impl->in_move_mode);

    if (impl->in_move_mode) {
        object_fade_out(impl->move_mode_btn, impl->base.controller->renderer,
                        MOVE_MODE_TRANSITION);
    } else {
        object_fade_in(impl->move_mode_btn, impl->base.controller->renderer,
                       MOVE_MODE_TRANSITION);
    }

    bool allow_exit_interaction = impl->in_move_mode;
    for (int i = 0; i < impl->exit_count; i++) {
        object_t *exit = impl->exits[i];
        if (!exit) {
            continue;
        }
        if (allow_exit_interaction) {
            object_fade_in(exit, impl->base.controller->renderer,
                           MOVE_MODE_TRANSITION);
        } else {
            object_fade_out(exit, impl->base.controller->renderer,
                            MOVE_MODE_TRANSITION);
        }
    }
}

bool mode_room_on_move_mode_icon_click(void *user, const input_event_t *event,
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

void mode_room_load_move_mode_btn(mode_room_impl_t *impl) {
    if (!impl) {
        return;
    }

    renderer_t *renderer = impl->base.controller->renderer;
    game_state_t *state = impl->base.state;

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
    object_set_interactive(btn, true);
    object_set_interaction_callback(btn, mode_room_on_move_mode_icon_click,
                                    impl);
}
