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

static void dump_value_record(const gds_record_t *record) {
    switch (record->type) {
    case GDS_RECORD_VALUE_S32:
        printf("%" PRIi32, record->payload.value.s32);
        break;

    case GDS_RECORD_VALUE_F32:
        printf("%f", record->payload.value.f32);
        break;

    case GDS_RECORD_BLOCK_START:
        printf("{");
        break;
    case GDS_RECORD_BLOCK_END:
        printf("}");
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
        printf("%s", gds_record_type_to_string(record->type));
        break;
    }
}

static bool dump_command_call(gds_reader_t *reader, const gds_record_t *command,
                              bool consume_single_condition) {
    bool first_arg = true;

    printf("%s(", gds_opcode_to_string(command->payload.opcode));

    if (consume_single_condition && gds_reader_remaining(reader) > 0U) {
        size_t saved_offset = reader->offset;
        gds_record_t argument;

        if (!gds_read_record(reader, &argument)) {
            fprintf(stderr, "Invalid argument at offset %zu\n", saved_offset);
            return false;
        }

        if (argument.type == GDS_RECORD_COMMAND) {
            if (!dump_command_call(reader, &argument, false)) {
                return false;
            }
        } else if (argument.type == GDS_RECORD_BLOCK_START ||
                   argument.type == GDS_RECORD_BLOCK_END) {
            reader->offset = saved_offset;
            return true;
        } else {
            dump_value_record(&argument);
        }
    }

    if (!consume_single_condition) {
        while (gds_reader_remaining(reader) > 0U) {
            size_t saved_offset = reader->offset;
            gds_record_t argument;

            if (!gds_read_record(reader, &argument)) {
                fprintf(stderr, "Invalid argument at offset %zu\n",
                        saved_offset);
                return false;
            }

            if (argument.type == GDS_RECORD_COMMAND ||
                argument.type == GDS_RECORD_BLOCK_START ||
                argument.type == GDS_RECORD_BLOCK_END) {
                reader->offset = saved_offset;
                break;
            }

            if (!first_arg) {
                printf(", ");
            }

            dump_value_record(&argument);
            first_arg = false;
        }
    }

    printf(")");
    return true;
}

bool dump_gds_raw(const uint8_t *data, size_t size) {
    gds_reader_t reader;
    gds_record_t record;

    gds_reader_init(&reader, data, size);

    while (gds_reader_remaining(&reader) > 0U) {
        size_t old_offset = reader.offset;

        if (!gds_read_record(&reader, &record)) {
            fprintf(stderr, "Invalid record at offset %zu\n", old_offset);
            return false;
        }

        printf("%04zx  %-13s", old_offset,
               gds_record_type_to_string(record.type));

        switch (record.type) {
        case GDS_RECORD_COMMAND:
            printf("%s", gds_opcode_to_string(record.payload.opcode));
            break;

        case GDS_RECORD_VALUE_S32:
            printf("%" PRIi32 " (0x%08" PRIx32 ")", record.payload.value.s32,
                   record.payload.value.u32);
            break;

        case GDS_RECORD_VALUE_F32:
            printf("%f (0x%08" PRIx32 ")", record.payload.value.f32,
                   record.payload.value.u32);
            break;

        case GDS_RECORD_BLOCK_START:
        case GDS_RECORD_BLOCK_END:
            printf("BLOCK_EDGE 0x%08" PRIx32, record.payload.value.u32);
            break;

        case GDS_RECORD_STRING:
            printf("\"%.*s\"", (int)record.payload.bytes.size,
                   record.payload.bytes.data);
            break;

        case GDS_RECORD_BYTES:
            printf("size=%zu data=", record.payload.bytes.size);
            dump_bytes(record.payload.bytes.data, record.payload.bytes.size);
            continue;

        default:
            break;
        }

        putchar('\n');
    }

    return true;
}

bool dump_gds(const uint8_t *data, size_t size) {
    gds_reader_t reader;
    gds_record_t record;
    size_t indent = 0U;

    gds_reader_init(&reader, data, size);

    while (gds_reader_remaining(&reader) > 0U) {
        size_t old_offset = reader.offset;

        if (!gds_read_record(&reader, &record)) {
            fprintf(stderr, "Invalid record at offset %zu\n", old_offset);
            return false;
        }

        if (record.type == GDS_RECORD_BLOCK_END && indent > 0U) {
            indent--;
        }

        if (record.type != GDS_RECORD_BLOCK_START &&
            record.type != GDS_RECORD_BLOCK_END) {
            dump_indent(indent);
            printf("%04zx  ", old_offset);
        }

        if (record.type == GDS_RECORD_COMMAND) {
            bool consume_single_condition =
                gds_is_block_start(record.payload.opcode) ||
                record.payload.opcode == SCRIPT_CMD_ELSEIF;

            if (!dump_command_call(&reader, &record,
                                   consume_single_condition)) {
                return false;
            }

            putchar('\n');
        } else if (record.type != GDS_RECORD_BLOCK_START &&
                   record.type != GDS_RECORD_BLOCK_END) {
            dump_value_record(&record);
            putchar('\n');
        }

        if (record.type == GDS_RECORD_BLOCK_START) {
            indent++;
        }
    }

    return true;
}
