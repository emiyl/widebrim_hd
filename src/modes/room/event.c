#include "event.h"

#include <stdio.h>

bool room_add_event(struct mode_room_impl_t *impl, int32_t x, int32_t y,
                    int32_t width, int32_t height, int32_t sprite_id,
                    int32_t event_id) {
    if (!impl) {
        fprintf(stderr, "widebrim: room_add_event called with NULL impl\n");
        return false;
    }

    printf("widebrim: [room] AddEvent(x=%d, y=%d, width=%d, height=%d, "
           "sprite_id=%d, event_id=%d)\n",
           x, y, width, height, sprite_id, event_id);

    return true;
}
