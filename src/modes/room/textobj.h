#ifndef MODE_ROOM_TEXTOBJ_H
#define MODE_ROOM_TEXTOBJ_H

#include <stdbool.h>
#include <stdint.h>

#include "room.h"

typedef struct tobj_impl_t {
    int32_t text_id;
    char text[256];
} tobj_impl_t;

void mode_room_add_text_obj(mode_room_impl_t *impl, int32_t type_or_flag,
                            int32_t x, int32_t y, int32_t width, int32_t height,
                            int32_t text_id, int32_t param7);

#endif // MODE_ROOM_TEXTOBJ_H
