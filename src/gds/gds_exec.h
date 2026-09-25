#ifndef GDS_EXEC_H
#define GDS_EXEC_H

#include <stdbool.h>
#include <stddef.h>

#include "gds/gds.h"

bool gds_execute_command(gds_reader_t *reader, const gds_record_t *record,
                         void *user_data);

bool gds_execute_script(const uint8_t *data, size_t size, void *user_data);

#endif // GDS_EXEC_H
