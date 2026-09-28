#ifndef MODES_ROOM_EVENT_H
#define MODES_ROOM_EVENT_H

#include <stdbool.h>
#include <stdint.h>

struct mode_room_impl_t;

typedef struct room_event_impl_t {
    int32_t sprite_id;
    int32_t event_id;
} room_event_impl_t;

bool room_add_event(struct mode_room_impl_t *impl, int32_t x, int32_t y,
                    int32_t width, int32_t height, int32_t sprite_id,
                    int32_t event_id);

#endif // MODES_ROOM_EVENT_H
