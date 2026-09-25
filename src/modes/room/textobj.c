#include "textobj.h"

#include <stdio.h>
#include <stdlib.h>

#include "text_loader.h"

static bool mode_room_on_textobj_click(void *user, const input_event_t *event,
                                       sprite_instance_t *sprite) {
    typedef struct {
        mode_room_impl_t *impl;
        int32_t text_id;
    } room_text_click_t;

    room_text_click_t *click = (room_text_click_t *)user;
    (void)sprite;

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

static sprite_instance_t *
mode_room_add_textobj_area(mode_room_impl_t *impl, int32_t x, int32_t y,
                           int32_t width, int32_t height, int32_t text_id) {
    typedef struct {
        mode_room_impl_t *impl;
        int32_t text_id;
    } room_text_click_t;

    uint8_t *transparent = NULL;
    sprite_instance_t *sprite = NULL;
    room_text_click_t *click = NULL;

    if (!impl || !impl->controller || width <= 0 || height <= 0) {
        return NULL;
    }

    transparent = (uint8_t *)calloc((size_t)width * (size_t)height * 4U,
                                    sizeof(*transparent));
    if (!transparent) {
        fprintf(
            stderr,
            "widebrim: failed to allocate transparent hitbox for textobj\n");
        return NULL;
    }

    click = (room_text_click_t *)malloc(sizeof(*click));
    if (!click) {
        free(transparent);
        fprintf(stderr,
                "widebrim: failed to allocate textobj callback state\n");
        return NULL;
    }

    click->impl = impl;
    click->text_id = text_id;

    sprite = screen_controller_add_sprite_z(impl->controller, transparent,
                                            width, height, x, y, 0, 0U);
    free(transparent);
    if (!sprite) {
        free(click);
        fprintf(stderr, "widebrim: failed to create textobj hitbox\n");
        return NULL;
    }

    object_layer_set_visible(sprite, false);
    object_layer_set_interactive(sprite, true, mode_room_on_textobj_click,
                                 click);
    return sprite;
}

void mode_room_add_text_obj(mode_room_impl_t *impl, int32_t x, int32_t y,
                            int32_t width, int32_t height, int32_t text_id,
                            int32_t param7) {
    (void)param7;

    if (!impl || !impl->controller) {
        return;
    }

    if (width <= 0 || height <= 0) {
        return;
    }

    mode_room_add_textobj_area(impl, x, y, width, height, text_id);
}
