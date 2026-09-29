#ifndef SCRIPT_H
#define SCRIPT_H

#include <stdbool.h>

#include "game_state.h"
#include "gds/gds.h"
#include "gds/gds_state.h"

bool script_load_and_execute(game_state_t *state, const char *relative_path,
                             void *user_data, gds_state_t *gds_state);

#endif // SCRIPT_H
