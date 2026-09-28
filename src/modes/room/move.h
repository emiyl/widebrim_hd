#ifndef MODES_ROOM_MOVE_H
#define MODES_ROOM_MOVE_H

#include "room.h"

#define MOVE_MODE_TRANSITION 250.0f

void set_move_mode(mode_room_impl_t *impl, bool enable);
void toggle_move_mode(mode_room_impl_t *impl);
bool mode_room_on_move_mode_icon_click(void *user, const input_event_t *event,
                                       object_t *obj);
void mode_room_load_move_mode_btn(mode_room_impl_t *impl);

#endif // MODES_ROOM_MOVE_H
