#ifndef MODES_ROOM_EXIT_H
#define MODES_ROOM_EXIT_H

#include <stdint.h>

#include "../../object.h"

struct mode_room_impl_t;

typedef struct room_exit_impl_t {
    int32_t target_map_id;
} room_exit_impl_t;

bool room_exit_on_event(void *user, const input_event_t *event,
                        struct object_t *object);

void room_add_exit(struct mode_room_impl_t *impl, int32_t exit_sprite_id,
                   int32_t target_map_id, int32_t x, int32_t y, int32_t width,
                   int32_t height, int32_t param7, int32_t param8);

#endif // MODES_ROOM_EXIT_H
