#ifndef GDS_EXEC_H
#define GDS_EXEC_H

#include <stdbool.h>
#include <stddef.h>

#include "game_state.h"
#include "gds.h"

bool gds_execute_command(gds_reader_t *reader, const gds_record_t *record,
                         game_state_t *state);

bool gds_execute_script(const uint8_t *data, size_t size, game_state_t *state);

#endif // GDS_EXEC_H
