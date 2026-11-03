#ifndef GDS_FUNC_H
#define GDS_FUNC_H

#include <stdbool.h>

#include "gds.h"
#include "gds_opcode.h"

#define GDS_ARGS                                                               \
    gds_reader_t *reader, const gds_record_t *command, void *user_data
#define DEF(name) static bool gds_func_##name(GDS_ARGS)

#define GDS_FUNC_LOOKUP_TABLE(X) GDS_OPCODE_HANDLER_LIST(X)

#define GDS_FUNC_LOOKUP_CASE(name)                                             \
    case SCRIPT_CMD_##name:                                                                 \
        *handler = gds_func_##name;                                            \
        return true;

typedef bool (*gds_command_handler_fn)(gds_reader_t *reader,
                                       const gds_record_t *command,
                                       void *user_data);

bool gds_func_lookup(gds_opcode_t opcode, gds_command_handler_fn *handler);

#endif // GDS_FUNC_H
