#include "gds_func.h"

#include <stdio.h>

#include "mode_room.h"

static bool gds_func_is_block_start(gds_opcode_t opcode) {
    switch (opcode) {
    case SCRIPT_CMD_IF:
    case SCRIPT_CMD_Loop:
    case SCRIPT_CMD_WHILE:
        return true;
    default:
        return false;
    }
}

static bool gds_func_is_block_else(gds_opcode_t opcode) {
    switch (opcode) {
    case SCRIPT_CMD_ELSE:
    case SCRIPT_CMD_ELSEIF:
        return true;
    default:
        return false;
    }
}

static bool gds_func_read_condition(gds_reader_t *reader, bool *result) {
    size_t start = reader->offset;
    gds_record_t record;

    if (result == NULL) {
        return false;
    }

    if (gds_reader_remaining(reader) == 0U) {
        *result = true;
        return true;
    }

    if (!gds_read_record(reader, &record)) {
        reader->offset = start;
        return false;
    }

    switch (record.type) {
    case GDS_RECORD_COMMAND:
        if (record.payload.opcode == SCRIPT_CMD_TRUE) {
            *result = true;
            return true;
        }
        if (record.payload.opcode == SCRIPT_CMD_FALSE) {
            *result = false;
            return true;
        }
        break;
    case GDS_RECORD_VALUE_S32:
        *result = record.payload.value.s32 != 0;
        return true;
    case GDS_RECORD_VALUE_6:
    case GDS_RECORD_VALUE_7:
        *result = record.payload.value.u32 != 0U;
        return true;
    default:
        break;
    }

    reader->offset = start;
    *result = true;
    return true;
}

static bool gds_func_skip_to_next_clause(gds_reader_t *reader) {
    size_t depth = 0U;

    while (gds_reader_remaining(reader) > 0U) {
        size_t checkpoint = reader->offset;
        gds_record_t record;

        if (!gds_read_record(reader, &record)) {
            reader->offset = checkpoint;
            return false;
        }

        if (record.type != GDS_RECORD_COMMAND) {
            continue;
        }

        if (gds_func_is_block_start(record.payload.opcode)) {
            depth++;
            continue;
        }

        if (gds_func_is_block_else(record.payload.opcode)) {
            if (depth == 0U) {
                reader->offset = checkpoint;
                return true;
            }
            depth--;
            continue;
        }

        if (depth == 0U) {
            continue;
        }
    }

    return true;
}

static bool gds_func_IF(gds_reader_t *reader, const gds_record_t *command,
                        void *user_data) {
    bool condition = true;

    (void)command;
    (void)user_data;

    if (!gds_func_read_condition(reader, &condition)) {
        return false;
    }

    if (!condition) {
        return gds_func_skip_to_next_clause(reader);
    }

    return true;
}

static bool gds_func_ELSEIF(gds_reader_t *reader, const gds_record_t *command,
                            void *user_data) {
    bool condition = true;

    (void)command;
    (void)user_data;

    if (!gds_func_read_condition(reader, &condition)) {
        return false;
    }

    if (!condition) {
        return gds_func_skip_to_next_clause(reader);
    }

    return true;
}

static bool gds_func_ELSE(gds_reader_t *reader, const gds_record_t *command,
                          void *user_data) {
    (void)reader;
    (void)command;
    (void)user_data;
    return true;
}

static bool gds_func_WHILE(gds_reader_t *reader, const gds_record_t *command,
                           void *user_data) {
    bool condition = true;

    (void)command;
    (void)user_data;

    if (!gds_func_read_condition(reader, &condition)) {
        return false;
    }

    if (!condition) {
        return gds_func_skip_to_next_clause(reader);
    }

    return true;
}

static bool gds_func_Loop(gds_reader_t *reader, const gds_record_t *command,
                          void *user_data) {
    bool condition = true;

    (void)command;
    (void)user_data;

    if (!gds_func_read_condition(reader, &condition)) {
        return false;
    }

    if (!condition) {
        return gds_func_skip_to_next_clause(reader);
    }

    return true;
}

static bool gds_func_SetMap(gds_reader_t *reader, const gds_record_t *command,
                            void *user_data) {
    (void)command;
    int32_t argv[5];

    if (!gds_read_s32_args(reader, argv, 5, "SetMap")) {
        return false;
    }

    int32_t map_text_id = argv[0];
    int32_t map_background_id = argv[1];
    int32_t param3 = argv[2];
    int32_t param4 = argv[3];
    int32_t param5 = argv[4];

    mode_room_impl_t *impl = (mode_room_impl_t *)user_data;
    if (impl->setmap)
        impl->setmap(impl, map_text_id, map_background_id, param3, param4,
                     param5);
    else {
        fprintf(stderr, "gds: setmap function pointer is NULL - is this "
                        "called from a room?\n");
    }

    return true;
}

static bool gds_func_AddTextObj(gds_reader_t *reader,
                                const gds_record_t *command, void *user_data) {
    (void)reader;
    (void)command;
    (void)user_data;
    int32_t argv[7];

    if (!gds_read_s32_args(reader, argv, 7, "AddTextObj")) {
        return false;
    }

    printf(
        "AddTextObj command received with args: %d, %d, %d, %d, %d, %d, %d\n",
        argv[0], argv[1], argv[2], argv[3], argv[4], argv[5], argv[6]);

    return true;
}

bool gds_func_lookup(gds_opcode_t opcode, gds_command_handler_fn *handler) {
    if (handler == NULL) {
        return false;
    }

    switch (opcode) {
    case SCRIPT_CMD_IF:
        *handler = gds_func_IF;
        return true;
    case SCRIPT_CMD_ELSEIF:
        *handler = gds_func_ELSEIF;
        return true;
    case SCRIPT_CMD_ELSE:
        *handler = gds_func_ELSE;
        return true;
    case SCRIPT_CMD_WHILE:
        *handler = gds_func_WHILE;
        return true;
    case SCRIPT_CMD_Loop:
        *handler = gds_func_Loop;
        return true;
    case SCRIPT_CMD_SetMap:
        *handler = gds_func_SetMap;
        return true;
    case SCRIPT_CMD_AddTextObj:
        *handler = gds_func_AddTextObj;
        return true;
    default:
        *handler = NULL;
        return false;
    }
}
