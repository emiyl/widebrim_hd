#include "gds.h"

#include <inttypes.h>
#include <stdio.h>

#include "gds_opcode.h"

static void dump_bytes(const uint8_t *data, size_t size) {
    size_t i;

    for (i = 0; i < size; i++) {
        printf("%02" PRIx8 " ", data[i]);
    }

    putchar('\n');
}

static void dump_indent(size_t indent) {
    for (size_t i = 0; i < indent * 4U; i++) {
        putchar(' ');
    }
}

static bool gds_is_block_start(gds_opcode_t opcode) {
    switch (opcode) {
    case SCRIPT_CMD_IF:
    case SCRIPT_CMD_Loop:
    case SCRIPT_CMD_WHILE:
        return true;
    default:
        return false;
    }
}

static bool gds_is_block_else(gds_opcode_t opcode) {
    switch (opcode) {
    case SCRIPT_CMD_ELSE:
    case SCRIPT_CMD_ELSEIF:
        return true;
    default:
        return false;
    }
}

static void dump_record(size_t offset, const gds_record_t *record,
                        size_t indent) {
    dump_indent(indent);
    printf("%04zx  %-13s", offset, gds_record_type_to_string(record->type));

    switch (record->type) {
    case GDS_RECORD_COMMAND:
        printf("%s", gds_opcode_to_string(record->payload.opcode));
        break;

    case GDS_RECORD_VALUE_S32:
        printf("%" PRIi32 " (0x%08" PRIx32 ")", record->payload.value.s32,
               record->payload.value.u32);
        break;

    case GDS_RECORD_VALUE_F32:
        printf("%f (0x%08" PRIx32 ")", record->payload.value.f32,
               record->payload.value.u32);
        break;

    case GDS_RECORD_VALUE_6:
    case GDS_RECORD_VALUE_7:
        printf("%" PRIu32 " (0x%08" PRIx32 ")", record->payload.value.u32,
               record->payload.value.u32);
        break;

    case GDS_RECORD_STRING:
        printf("\"%.*s\"", (int)record->payload.bytes.size,
               record->payload.bytes.data);
        break;

    case GDS_RECORD_BYTES:
        printf("size=%zu data=", record->payload.bytes.size);

        dump_bytes(record->payload.bytes.data, record->payload.bytes.size);
        return;

    default:
        break;
    }

    putchar('\n');
}

bool dump_gds(const uint8_t *data, size_t size) {
    gds_reader_t reader;
    gds_record_t record;
    bool previous_was_command = false;
    size_t indent = 0U;

    gds_reader_init(&reader, data, size);

    while (gds_reader_remaining(&reader) > 0) {
        size_t old_offset = reader.offset;

        if (!gds_read_record(&reader, &record)) {
            fprintf(stderr, "Invalid record at offset %zu\n", old_offset);
            return false;
        }

        if (record.type == GDS_RECORD_COMMAND && previous_was_command) {
            putchar('\n');
        }

        if (record.type == GDS_RECORD_COMMAND &&
            gds_is_block_else(record.payload.opcode) && indent > 0U) {
            indent--;
        }

        dump_record(old_offset, &record, indent);

        if (record.type == GDS_RECORD_COMMAND &&
            (gds_is_block_start(record.payload.opcode) ||
             gds_is_block_else(record.payload.opcode))) {
            indent++;
        }

        previous_was_command = record.type == GDS_RECORD_COMMAND;
    }

    return true;
}
