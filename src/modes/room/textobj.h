#ifndef MODE_ROOM_TEXTOBJ_H
#define MODE_ROOM_TEXTOBJ_H

#include <stdint.h>

#include "room.h"

void mode_room_add_text_obj(mode_room_impl_t *impl, int32_t x, int32_t y,
                            int32_t width, int32_t height, int32_t text_id,
                            int32_t param7);

#endif // MODE_ROOM_TEXTOBJ_H
