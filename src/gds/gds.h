#ifndef GDS_H
#define GDS_H

#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "gds_opcode.h"
#include "gds_reader.h"

typedef enum {
    GDS_RECORD_COMMAND = 0,
    GDS_RECORD_VALUE_S32 = 1,
    GDS_RECORD_VALUE_F32 = 2,
    GDS_RECORD_STRING = 3,
    GDS_RECORD_BYTES = 4,
    GDS_RECORD_EMPTY_5 = 5,
    GDS_RECORD_BLOCK_START = 6,
    GDS_RECORD_BLOCK_END = 7,
    GDS_RECORD_NOT = 8,
    GDS_RECORD_AND = 9,
    GDS_RECORD_OR = 10,
    GDS_RECORD_EMPTY_11 = 11,
    GDS_RECORD_BREAKPOINT = 12
} gds_record_type_t;

static inline const char *gds_record_type_to_string(gds_record_type_t type) {
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
    case GDS_RECORD_BLOCK_START:
        return "BLOCK_START";
    case GDS_RECORD_BLOCK_END:
        return "BLOCK_END";
    case GDS_RECORD_NOT:
        return "NOT";
    case GDS_RECORD_AND:
        return "AND";
    case GDS_RECORD_OR:
        return "OR";
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

static inline const char *gds_record_to_string(const gds_record_t *record) {
    static char buffer[256];
    if (record == NULL) {
        return "NULL_RECORD";
    }

    switch (record->type) {
    case GDS_RECORD_COMMAND:
        snprintf(buffer, sizeof(buffer), "%s",
                 gds_opcode_to_string(record->payload.opcode));
        break;

    case GDS_RECORD_VALUE_S32:
        snprintf(buffer, sizeof(buffer), "%" PRIi32, record->payload.value.s32);
        break;

    case GDS_RECORD_VALUE_F32:
        snprintf(buffer, sizeof(buffer), "%f", record->payload.value.f32);
        break;

    case GDS_RECORD_BLOCK_START:
    case GDS_RECORD_BLOCK_END:
        snprintf(buffer, sizeof(buffer), "%" PRIu32, record->payload.value.u32);
        break;

    case GDS_RECORD_STRING:
        snprintf(buffer, sizeof(buffer), "\"%.*s\"",
                 (int)record->payload.bytes.size, record->payload.bytes.data);
        break;

    case GDS_RECORD_BYTES:
        snprintf(buffer, sizeof(buffer),
                 "size=%zu data=", record->payload.bytes.size);
        break;

    default:
        break;
    }

    return buffer;
}

bool gds_read_record(gds_reader_t *reader, gds_record_t *record);
bool gds_read_args(gds_reader_t *reader, gds_record_t *argv, size_t count,
                   const gds_record_t *command);
bool gds_read_s32_args(gds_reader_t *reader, int32_t *argv, size_t count,
                       const gds_record_t *command);
bool gds_read_string_args(gds_reader_t *reader, const char **argv, size_t count,
                          const gds_record_t *command);
bool gds_extract_payload(const uint8_t *file, size_t file_size,
                         const uint8_t **payload, size_t *payload_size);
bool gds_load_from_file_path(const char *file_path, const uint8_t **payload,
                             size_t *payload_size);
void gds_free_payload(const uint8_t *payload);

#endif // GDS_H
