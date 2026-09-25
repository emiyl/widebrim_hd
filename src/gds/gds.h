#ifndef GDS_H
#define GDS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "gds_opcode.h"
#include "gds_reader.h"

typedef enum {
    GDS_RECORD_COMMAND = 0,
    GDS_RECORD_VALUE_S32 = 1,
    GDS_RECORD_VALUE_F32 = 2,
    GDS_RECORD_STRING = 3,
    GDS_RECORD_BYTES = 4,
    GDS_RECORD_EMPTY_5 = 5,
    GDS_RECORD_VALUE_6 = 6,
    GDS_RECORD_VALUE_7 = 7,
    GDS_RECORD_EMPTY_8 = 8,
    GDS_RECORD_EMPTY_9 = 9,
    GDS_RECORD_EMPTY_10 = 10,
    GDS_RECORD_EMPTY_11 = 11,
    GDS_RECORD_BREAKPOINT = 12
} gds_record_type_t;

static inline char *gds_record_type_to_string(gds_record_type_t type) {
    switch (type) {
    case GDS_RECORD_COMMAND:
        return "COMMAND";
    case GDS_RECORD_VALUE_S32:
        return "S32";
    case GDS_RECORD_VALUE_F32:
        return "F32";
    case GDS_RECORD_STRING:
        return "STRING";
    case GDS_RECORD_BYTES:
        return "BYTES";
    case GDS_RECORD_EMPTY_5:
        return "EMPTY_5";
    case GDS_RECORD_VALUE_6:
        return "VALUE_6";
    case GDS_RECORD_VALUE_7:
        return "VALUE_7";
    case GDS_RECORD_EMPTY_8:
        return "EMPTY_8";
    case GDS_RECORD_EMPTY_9:
        return "EMPTY_9";
    case GDS_RECORD_EMPTY_10:
        return "EMPTY_10";
    case GDS_RECORD_EMPTY_11:
        return "EMPTY_11";
    case GDS_RECORD_BREAKPOINT:
        return "BREAKPOINT";
    default:
        return "UNKNOWN_GDS_RECORD_TYPE";
    }
}

typedef struct {
    uint16_t type;

    union {
        gds_opcode_t opcode;
        union {
            int32_t s32;
            uint32_t u32;
            float f32;
        } value;
        struct {
            const uint8_t *data;
            size_t size;
        } bytes;
    } payload;
} gds_record_t;

bool gds_read_record(gds_reader_t *reader, gds_record_t *record);
bool gds_read_s32_args(gds_reader_t *reader, int32_t *argv, size_t count,
                       const char *function_name);
bool gds_extract_payload(const uint8_t *file, size_t file_size,
                         const uint8_t **payload, size_t *payload_size);
bool gds_load_from_file_path(const char *file_path, const uint8_t **payload,
                             size_t *payload_size);
void gds_free_payload(const uint8_t *payload);

#endif // GDS_H
