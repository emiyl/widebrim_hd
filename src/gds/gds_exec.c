#include "gds_exec.h"

#include "gds_func.h"

#include <stdio.h>

static bool gds_execute_default_command(gds_reader_t *reader,
                                        const gds_record_t *record,
                                        void *user_data) {
    (void)user_data;
    bool should_break = false;

    while (gds_reader_remaining(reader) > 0U) {
        size_t saved_offset = reader->offset;
        gds_record_t next_record;

        if (!gds_read_record(reader, &next_record)) {
            fprintf(stderr, "gds: failed to read argument for %s\n",
                    gds_opcode_to_string(record->payload.opcode));
            return false;
        }

        switch (next_record.type) {
        case GDS_RECORD_COMMAND:
        case GDS_RECORD_BREAKPOINT:
            reader->offset = saved_offset;
            should_break = true;
            break;
        default:
            break;
        }

        if (should_break) {
            break;
        }
    }

    return true;
}

bool gds_execute_command(gds_reader_t *reader, const gds_record_t *record,
                         void *user_data) {
    gds_command_handler_fn handler = NULL;

    if (reader == NULL || record == NULL) {
        fprintf(stderr,
                "gds: command execution requires a reader and record\n");
        return false;
    }

    if (record->type != GDS_RECORD_COMMAND) {
        fprintf(stderr, "gds: expected command record, got %s\n",
                gds_record_type_to_string(record->type));
        return false;
    }

    if (gds_func_lookup(record->payload.opcode, &handler) && handler != NULL) {
        return handler(reader, record, user_data);
    }

    return gds_execute_default_command(reader, record, user_data);
}

bool gds_execute_script(const uint8_t *data, size_t size, void *user_data) {
    gds_reader_t reader;
    gds_record_t record;

    if (data == NULL && size != 0U) {
        fprintf(stderr, "gds: script pointer is null but size is non-zero\n");
        return false;
    }

    gds_reader_init(&reader, data, size);

    while (gds_reader_remaining(&reader) > 0U) {
        size_t old_offset = reader.offset;

        if (!gds_read_record(&reader, &record)) {
            fprintf(stderr, "gds: invalid record near offset %zu\n",
                    old_offset);
            return false;
        }

        if (record.type == GDS_RECORD_BREAKPOINT ||
            record.type == GDS_RECORD_EMPTY_5 ||
            record.type == GDS_RECORD_EMPTY_8 ||
            record.type == GDS_RECORD_EMPTY_9 ||
            record.type == GDS_RECORD_EMPTY_10 ||
            record.type == GDS_RECORD_EMPTY_11 ||
            record.type == GDS_RECORD_VALUE_6 ||
            record.type == GDS_RECORD_VALUE_7) {
            continue;
        }

        if (record.type != GDS_RECORD_COMMAND) {
            fprintf(stderr, "gds: expected command at offset %zu, got %s\n",
                    old_offset, gds_record_type_to_string(record.type));
            return false;
        }

        if (!gds_execute_command(&reader, &record, user_data)) {
            fprintf(stderr, "gds: failed to execute %s at offset %zu\n",
                    gds_opcode_to_string(record.payload.opcode), old_offset);
            return false;
        }
    }

    return true;
}
