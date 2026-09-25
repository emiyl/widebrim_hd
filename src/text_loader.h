#ifndef TEXT_LOADER_H
#define TEXT_LOADER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "game_state.h"

bool text_loader_load_path(game_state_t *state, const char *rel_path,
                           char *out_text, size_t out_text_size);
bool text_loader_load_room_text(game_state_t *state, int32_t text_id,
                                char *out_text, size_t out_text_size);

#endif // TEXT_LOADER_H
