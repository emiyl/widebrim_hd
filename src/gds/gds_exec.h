#ifndef GDS_EXEC_H
#define GDS_EXEC_H

#include <stdbool.h>
#include <stddef.h>

#include "gds.h"

typedef bool (*gds_command_handler_fn)(gds_reader_t *reader,
                                       const gds_record_t *command,
                                       void *user_data);

typedef struct {
    gds_opcode_t opcode;
    gds_command_handler_fn handler;
} gds_command_handler_t;

bool gds_execute_command(gds_reader_t *reader, const gds_record_t *record,
                         const gds_command_handler_t *handlers,
                         size_t handler_count, void *user_data);

bool gds_execute_script(const uint8_t *data, size_t size,
                        const gds_command_handler_t *handlers,
                        size_t handler_count, void *user_data);

#endif // GDS_EXEC_H
