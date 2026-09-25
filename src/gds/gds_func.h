#ifndef GDS_FUNC_H
#define GDS_FUNC_H

#include <stdbool.h>

#include "gds.h"

typedef bool (*gds_command_handler_fn)(gds_reader_t *reader,
                                       const gds_record_t *command,
                                       void *user_data);

bool gds_func_lookup(gds_opcode_t opcode, gds_command_handler_fn *handler);

#endif // GDS_FUNC_H
