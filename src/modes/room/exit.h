#ifndef MODES_ROOM_EXIT_H
#define MODES_ROOM_EXIT_H

#include <stdint.h>

#include "object.h"

struct mode_room_impl_t;

typedef struct {
    object_t *object;
    int32_t exit_sprite_id;
    int32_t target_map_id;
} room_exit_t;

void room_add_exit(struct mode_room_impl_t *impl, int32_t exit_sprite_id,
                   int32_t target_map_id, int32_t x, int32_t y, int32_t width,
                   int32_t height);

#endif // MODES_ROOM_EXIT_H
