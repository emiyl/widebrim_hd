#include "textobj.h"

#include <stdio.h>
#include <stdlib.h>

#include "text_loader.h"

static bool mode_room_on_textobj_click(void *user, const input_event_t *event,
                                       object_t *obj) {
    (void)obj;
    typedef struct {
        mode_room_impl_t *impl;
        int32_t text_id;
    } room_text_click_t;

    room_text_click_t *click = (room_text_click_t *)user;

    if (!click || !click->impl || !event) {
        return false;
    }

    if (event->type != INPUT_EVENT_MOUSE_BUTTON_DOWN &&
        event->type != INPUT_EVENT_MOUSE_BUTTON_UP) {
        return false;
    }

    if (click->impl->popup_text && click->impl->popup_text->visible) {
        screen_controller_set_text_visible(click->impl->controller,
                                           click->impl->popup_text, false);
        return true;
    }

    {
        char text_buffer[4096];
        text_instance_t *popup = NULL;
        rect_t rect = {0.0f, 0.0f, (float)WB_SCREEN_WIDTH,
                       (float)WB_SCREEN_HEIGHT};

        if (!text_loader_load_room_text(click->impl->state, click->text_id,
                                        text_buffer, sizeof(text_buffer))) {
            fprintf(
                stderr,
                "widebrim: failed to load room text object asset for id %d\n",
                click->text_id);
            return false;
        }

        if (!click->impl->popup_text) {
            popup = screen_controller_add_text(click->impl->controller, 0, 0,
                                               text_buffer);
            if (!popup) {
                fprintf(stderr,
                        "widebrim: failed to create popup text for id %d\n",
                        click->text_id);
                return false;
            }
            click->impl->popup_text = popup;
        } else {
            popup = click->impl->popup_text;
            screen_controller_set_text_contents(click->impl->controller, popup,
                                                text_buffer);
        }

        text_layer_center_text_in_rect(popup, &rect);
        text_layer_set_visible(popup, true);
        return true;
    }
}

static void mode_room_add_textobj_area(mode_room_impl_t *impl, int32_t x,
                                       int32_t y, int32_t width, int32_t height,
                                       int32_t text_id) {
    (void)text_id;

    object_t object = {0};
    object_init(&object);
    object_set_position(&object, x, y);
    object_set_size(&object, width, height);

    object_set_visible(&object, true);
    object_set_interactive(&object, true, mode_room_on_textobj_click, impl);
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
