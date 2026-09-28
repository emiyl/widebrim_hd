#include "event.h"

#include <stdio.h>

#include "room.h"
#include "safe.h"

bool room_add_event(struct mode_room_impl_t *room, int32_t x, int32_t y,
                    int32_t width, int32_t height, int32_t sprite_id,
                    int32_t event_id) {
    if (!room) {
        fprintf(stderr, "widebrim: room_add_event called with NULL room\n");
        return false;
    }

    if (room->event_count >= 16) {
        fprintf(stderr, "widebrim: room_add_event called but room is full\n");
    }

    room_event_impl_t *event_data = smalloc(sizeof(*event_data));
    *event_data =
        (room_event_impl_t){.sprite_id = sprite_id, .event_id = event_id};

    object_t *event = room->event[room->event_count];
    object_init_kind(event, OBJECT_KIND_EVENT);
    event->self_vars = event_data;

    if (!event) {
        fprintf(stderr,
                "widebrim: [room_add_event] failed to allocate object\n");
        return false;
    }

    event->x = x;
    event->y = y + WB_SCREEN_HEIGHT;
    event->width = width;
    event->height = height;

    sprite_t *sprite = event->sprite;
    if (!sprite) {
        fprintf(stderr,
                "widebrim: [room_add_event] failed to allocate sprite\n");
        return false;
    }

    renderer_t *renderer = room->controller->renderer;
    game_state_t *state = room->state;

    char sprite_filename[12] = "";
    snprintf(sprite_filename, sizeof(sprite_filename), "obj_%d.spr", sprite_id);
    sprite_new(sprite, renderer, state, sprite_filename, 750.0f, true);

    sprite_take_object_position(sprite, event);

    object_set_visible(event, true);
    object_layer_add_object(room->controller->object, event);

    room->event_count++;

    return true;
}
