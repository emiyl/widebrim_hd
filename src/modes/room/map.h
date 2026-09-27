#ifndef MODES_ROOM_MAP_H
#define MODES_ROOM_MAP_H

#include "room.h"

void mode_room_load_map_place(mode_room_impl_t *impl);
void mode_room_setmap(mode_room_impl_t *self, int32_t map_text_id,
                      int32_t map_background_id, int32_t param3, int32_t param4,
                      int32_t param5);

#endif // MODES_ROOM_MAP_H
