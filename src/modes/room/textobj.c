#include "textobj.h"

#include <stdio.h>
#include <stdlib.h>

#include "safe.h"
#include "text_loader.h"

static bool mode_room_textobj_on_click(void *user, const input_event_t *event,
                                       object_t *inst) {
    mode_room_impl_t *impl = (mode_room_impl_t *)user;

    if (!impl) {
        return false;
    }

    tobj_impl_t *tobj_impl = (tobj_impl_t *)inst->self_vars;

    switch (event->type) {
    case INPUT_EVENT_MOUSE_BUTTON_DOWN:
        inst->clicked = true;
        break;
    case INPUT_EVENT_MOUSE_BUTTON_UP:
        if (inst->clicked) {
            inst->clicked = false;

            game_state_t *game_state = impl->state;
            int32_t text_id = tobj_impl->text_id;
            text_loader_load_room_text(game_state, text_id, tobj_impl->text,
                                       256);

            fprintf(stderr, "Room text: %s\n", tobj_impl->text);
            return true;
        }
        break;
    default:
        break;
    }

    return true;
}

static void mode_room_add_textobj_area(mode_room_impl_t *impl, int32_t x,
                                       int32_t y, int32_t width, int32_t height,
                                       int32_t text_id) {
    object_t *object;

    if (!impl || !impl->controller || !impl->controller->object) {
        return;
    }

    if (impl->tobj_count >= 16) {
        fprintf(stderr, "widebrim: [room_add_text_obj] maximum number of text "
                        "objects reached\n");
        return;
    }

    object = impl->text_obj[impl->tobj_count];
    object_init_kind(object, OBJECT_KIND_TOBJ);

    tobj_impl_t *tobj_impl = smalloc(sizeof(*tobj_impl));

    if (!tobj_impl) {
        return;
    }

    *tobj_impl = (tobj_impl_t){.text_id = text_id, .text = ""};

    object->self_vars = tobj_impl;

    object_set_position(object, x, y + WB_SCREEN_HEIGHT);
    sprite_take_object_position(object->sprite, object);
    object_set_size(object, width, height);
    object_set_visible(object, false);
    object_set_interactive(object, true);
    object_set_interaction_callback(object, mode_room_textobj_on_click, impl);

    object_layer_add_object(impl->controller->object, object);
    impl->tobj_count++;
}

void mode_room_add_text_obj(mode_room_impl_t *impl, int32_t type_or_flag,
                            int32_t x, int32_t y, int32_t width, int32_t height,
                            int32_t text_id, int32_t param7) {
    (void)type_or_flag;
    (void)param7;

    if (!impl || !impl->controller) {
        return;
    }

    if (width <= 0 || height <= 0) {
        return;
    }

    mode_room_add_textobj_area(impl, x, y, width, height, text_id);
}
