#ifndef GDS_H
#define GDS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "gds_reader.h"
#include "language.h"

typedef enum {
    GDS_RECORD_COMMAND = 0,
    GDS_RECORD_VALUE_1 = 1,
    GDS_RECORD_VALUE_2 = 2,
    GDS_RECORD_BYTES_1 = 3,
    GDS_RECORD_BYTES_2 = 4,
    GDS_RECORD_EMPTY_5 = 5,
    GDS_RECORD_VALUE_6 = 6,
    GDS_RECORD_VALUE_7 = 7,
    GDS_RECORD_EMPTY_8 = 8,
    GDS_RECORD_EMPTY_9 = 9,
    GDS_RECORD_EMPTY_10 = 10,
    GDS_RECORD_EMPTY_11 = 11,
    GDS_RECORD_EMPTY_12 = 12
} gds_record_type_t;

typedef struct {
    uint16_t type;

    union {
        uint16_t opcode;
        uint32_t value;
        struct {
            const uint8_t *data;
            size_t size;
        } bytes;
    } payload;
} gds_record_t;

bool gds_read_record(gds_reader_t *reader, gds_record_t *record);
bool gds_extract_payload(const uint8_t *file, size_t file_size,
                         const uint8_t **payload, size_t *payload_size);
bool gds_load_from_file_path(const char *file_path, const uint8_t **payload,
                             size_t *payload_size);
void gds_free_payload(const uint8_t *payload);

#endif // GDS_H
