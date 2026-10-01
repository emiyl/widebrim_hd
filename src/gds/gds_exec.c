#include "gds_exec.h"

#include "gds_func.h"

#include <stdio.h>

static bool gds_execute_default_command(gds_reader_t *reader,
                                        const gds_record_t *record,
                                        void *user_data) {
    (void)user_data;
    bool should_break = false;
    bool first_arg = true;

    fprintf(stderr, "gds: Unknown command: %s(",
            gds_opcode_to_string(record->payload.opcode));

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
        case GDS_RECORD_BLOCK_START:
        case GDS_RECORD_BLOCK_END:
            reader->offset = saved_offset;
            should_break = true;
            break;
        default:
            if (!first_arg) {
                fprintf(stderr, ", ");
            }
            fprintf(stderr, "%s %s",
                    gds_record_type_to_string(next_record.type),
                    gds_record_to_string(&next_record));
            first_arg = false;
            break;
        }

        if (should_break) {
            break;
        }
    }

    fprintf(stderr, ")\n");
    fflush(stderr);

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

bool gds_execute_script(const uint8_t *data, size_t size, void *user_data,
                        gds_state_t *state) {
    gds_reader_t reader;
    gds_record_t record;

    if (data == NULL && size != 0U) {
        fprintf(stderr, "gds: script pointer is null but size is non-zero\n");
        return false;
    }

    gds_state_reset(state);
    gds_reader_init(&reader, data, size);

    while (gds_reader_remaining(&reader) > 0U) {
        size_t old_offset = reader.offset;

        if (!gds_read_record(&reader, &record)) {
            fprintf(stderr, "gds: invalid record near offset %zu\n",
                    old_offset);
            return false;
        }

        if (record.type == GDS_RECORD_EMPTY_5 ||
            record.type == GDS_RECORD_NOT || record.type == GDS_RECORD_AND ||
            record.type == GDS_RECORD_OR ||
            record.type == GDS_RECORD_EMPTY_11 ||
            record.type == GDS_RECORD_BLOCK_START) {
            continue;
        }

        if (record.type == GDS_RECORD_BLOCK_END) {
            if (state != NULL) {
                if (state->skipping_else_block) {
                    state->skipping_else_block = false;
                    continue;
                }

                if (state->skip_next_else_depth > 0U) {
                    size_t after_block_end = reader.offset;
                    gds_record_t next_record;

                    state->skip_next_else_depth -= 1U;

                    if (gds_reader_remaining(&reader) > 0U &&
                        gds_read_record(&reader, &next_record)) {
                        if (next_record.type == GDS_RECORD_COMMAND &&
                            (next_record.payload.opcode == SCRIPT_CMD_ELSE ||
                             next_record.payload.opcode == SCRIPT_CMD_ELSEIF)) {
                            size_t after_else_command = reader.offset;
                            gds_record_t else_block_start;

                            if (gds_reader_remaining(&reader) > 0U &&
                                gds_read_record(&reader, &else_block_start) &&
                                else_block_start.type ==
                                    GDS_RECORD_BLOCK_START) {
                                reader.offset =
                                    (size_t)else_block_start.payload.value.u32;
                                state->skipping_else_block = true;
                                continue;
                            }

                            reader.offset = after_else_command;
                        } else {
                            reader.offset = after_block_end;
                        }
                    }
                }
            }
            continue;
        }

        if (record.type == GDS_RECORD_BREAKPOINT) {
            printf("gds: hit breakpoint at offset %zu\n", old_offset);
            break;
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

        if (record.payload.opcode == SCRIPT_CMD_ExitScript) {
            printf("gds: exiting script at offset %zu\n", old_offset);
            break;
        }
    }

    return true;
}
