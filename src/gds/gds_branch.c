#include "gds_branch.h"

#include <stdio.h>

#include "gds/gds.h"
#include "gds/gds_opcode.h"
#include "gds_state.h"
#include "modes/mode_impl.h"

#define TRACE

static bool gds_branch_is_ignored_record_type(gds_record_type_t type) {
    switch (type) {
    case GDS_RECORD_EMPTY_5:
    case GDS_RECORD_EMPTY_11:
    case GDS_RECORD_BREAKPOINT:
        return true;
    default:
        return false;
    }
}

static game_state_t *gds_branch_get_game_state(void *user_data) {
    mode_impl_t *impl = (mode_impl_t *)user_data;

    if (impl != NULL && impl->state != NULL) {
        return impl->state;
    }

    return (game_state_t *)user_data;
}

static gds_state_t *gds_branch_get_gds_state(void *user_data) {
    game_state_t *game_state = gds_branch_get_game_state(user_data);
    if (game_state != NULL) {
        return &game_state->gds;
    }
    return NULL;
}

static bool gds_branch_jump_to_block_target(gds_reader_t *reader,
                                            gds_record_type_t type,
                                            uint32_t target) {
    if (reader == NULL ||
        (type != GDS_RECORD_BLOCK_START && type != GDS_RECORD_BLOCK_END)) {
        return false;
    }

    if (target == 0U) {
        return false;
    }

    reader->offset = (size_t)target;
    return true;
}

static bool gds_branch_read_condition_term(gds_reader_t *reader, bool *result,
                                           void *user_data) {
    size_t start = 0U;
    gds_record_t record;
    gds_command_handler_fn handler = NULL;
    game_state_t *game_state = NULL;
    gds_state_t *gds = NULL;

    if (reader == NULL || result == NULL) {
        return false;
    }

    start = reader->offset;
    game_state = gds_branch_get_game_state(user_data);
    if (game_state == NULL) {
        fprintf(stderr,
                "gds: condition evaluation requires game state context\n");
        return false;
    }
    gds = &game_state->gds;

    if (gds_reader_remaining(reader) == 0U) {
        fprintf(stderr, "gds: condition at end of stream is not valid\n");
        return false;
    }

    if (!gds_read_record(reader, &record)) {
        reader->offset = start;
        fprintf(stderr, "gds: failed to read condition record\n");
        return false;
    }

    while (gds_branch_is_ignored_record_type(record.type)) {
        if (gds_reader_remaining(reader) == 0U) {
            fprintf(stderr, "gds: condition at end of stream is not valid\n");
            reader->offset = start;
            return false;
        }

        if (!gds_read_record(reader, &record)) {
            reader->offset = start;
            fprintf(stderr, "gds: failed to read condition record\n");
            return false;
        }
    }

    if (record.type == GDS_RECORD_NOT) {
        bool value = false;

        printf("gds: NOT\n");

        if (!gds_branch_read_condition_term(reader, &value, user_data)) {
            reader->offset = start;
            return false;
        }

        *result = !value;
        return true;
    }

    switch (record.type) {
    case GDS_RECORD_AND:
    case GDS_RECORD_OR:
        reader->offset = start;
        fprintf(
            stderr,
            "gds: logical operator %s is not a valid standalone condition\n",
            gds_record_type_to_string(record.type));
        return false;
    case GDS_RECORD_COMMAND:
        if (gds_func_lookup(record.payload.opcode, &handler) &&
            handler != NULL) {
            if (!handler(reader, &record, user_data)) {
                reader->offset = start;
                fprintf(stderr, "gds: condition command %s failed\n",
                        gds_opcode_to_string(record.payload.opcode));
                return false;
            }
            *result = gds->condition_result;
            return true;
        }
        reader->offset = start;
        fprintf(stderr, "gds: unsupported condition opcode %s\n",
                gds_opcode_to_string(record.payload.opcode));
        return false;
    case GDS_RECORD_VALUE_S32:
        *result = record.payload.value.s32 != 0;
        return true;
    case GDS_RECORD_BLOCK_START:
    case GDS_RECORD_BLOCK_END:
        reader->offset = start;
        fprintf(stderr,
                "gds: block boundary record %s is not a valid condition\n",
                gds_record_type_to_string(record.type));
        return false;
    case GDS_RECORD_STRING:
        *result = record.payload.bytes.size != 0U;
        return true;
    case GDS_RECORD_BYTES:
        *result = record.payload.bytes.size != 0U;
        return true;
    default:
        reader->offset = start;
        fprintf(stderr, "gds: unsupported condition record type %s\n",
                gds_record_type_to_string(record.type));
        return false;
    }
}

static bool gds_branch_read_condition(gds_reader_t *reader, bool *result,
                                      void *user_data) {
    bool left = false;
    bool right = false;
    size_t start = 0U;
    gds_record_t record;

    if (reader == NULL || result == NULL) {
        return false;
    }

    start = reader->offset;
    if (!gds_branch_read_condition_term(reader, &left, user_data)) {
        reader->offset = start;
        return false;
    }

    while (gds_reader_remaining(reader) > 0U) {
        size_t saved_offset = reader->offset;

        if (!gds_read_record(reader, &record)) {
            reader->offset = saved_offset;
            break;
        }

        if (record.type != GDS_RECORD_AND && record.type != GDS_RECORD_OR) {
            reader->offset = saved_offset;
            break;
        }

        if (!gds_branch_read_condition_term(reader, &right, user_data)) {
            reader->offset = start;
            return false;
        }

        if (record.type == GDS_RECORD_AND) {
            printf("gds: AND\n");
            left = left && right;
        } else {
            printf("gds: OR\n");
            left = left || right;
        }
    }

    *result = left;
    return true;
}

static bool gds_branch_TRUE(gds_reader_t *reader, const gds_record_t *command,
                            void *user_data) {
    (void)reader;
    (void)command;

    game_state_t *game_state = gds_branch_get_game_state(user_data);
    gds_state_t *gds = NULL;

    if (game_state == NULL) {
        fprintf(stderr, "gds: TRUE called without game state context\n");
        return false;
    }

#ifdef TRACE
    const char *function_name = gds_record_to_string(command);
    printf("gds: %s()\n", function_name);
#endif

    gds = &game_state->gds;
    gds->condition_result = true;

    return true;
}

static bool gds_branch_FALSE(gds_reader_t *reader, const gds_record_t *command,
                             void *user_data) {
    (void)reader;
    (void)command;

    game_state_t *game_state = gds_branch_get_game_state(user_data);
    gds_state_t *gds = NULL;

    if (game_state == NULL) {
        fprintf(stderr, "gds: FALSE called without game state context\n");
        return false;
    }

#ifdef TRACE
    const char *function_name = gds_record_to_string(command);
    printf("gds: %s()\n", function_name);
#endif

    gds = &game_state->gds;
    gds->condition_result = false;

    return true;
}

static bool gds_branch_IF(gds_reader_t *reader, const gds_record_t *command,
                          void *user_data) {
    bool condition = true;
    size_t checkpoint = 0U;
    gds_record_t block_start;

    (void)command;

#ifdef TRACE
    const char *function_name = gds_record_to_string(command);
    printf("gds: %s()\n", function_name);
#endif

    if (!gds_branch_read_condition(reader, &condition, user_data)) {
        return false;
    }

    checkpoint = reader->offset;
    if (gds_reader_remaining(reader) == 0U) {
        return true;
    }

    if (!gds_read_record(reader, &block_start) ||
        block_start.type != GDS_RECORD_BLOCK_START) {
        reader->offset = checkpoint;
        return true;
    }

    if (!condition) {
        return gds_branch_jump_to_block_target(reader, block_start.type,
                                               block_start.payload.value.u32);
    }

    {
        gds_state_t *gds_state = gds_branch_get_gds_state(user_data);
        if (gds_state != NULL) {
            gds_state->skip_next_else_depth += 1U;
        }
    }

    return true;
}

static bool gds_branch_ELSEIF(gds_reader_t *reader, const gds_record_t *command,
                              void *user_data) {
    bool condition = true;
    size_t checkpoint = 0U;
    gds_record_t block_start;

    (void)command;

#ifdef TRACE
    const char *function_name = gds_record_to_string(command);
    printf("gds: %s()\n", function_name);
#endif

    if (!gds_branch_read_condition(reader, &condition, user_data)) {
        return false;
    }

    checkpoint = reader->offset;
    if (gds_reader_remaining(reader) == 0U) {
        return true;
    }

    if (!gds_read_record(reader, &block_start) ||
        block_start.type != GDS_RECORD_BLOCK_START) {
        reader->offset = checkpoint;
        return true;
    }

    if (!condition) {
        return gds_branch_jump_to_block_target(reader, block_start.type,
                                               block_start.payload.value.u32);
    }

    {
        gds_state_t *gds_state = gds_branch_get_gds_state(user_data);
        if (gds_state != NULL) {
            gds_state->skip_next_else_depth += 1U;
        }
    }

    return true;
}

static bool gds_branch_ELSE(gds_reader_t *reader, const gds_record_t *command,
                            void *user_data) {
    (void)reader;
    (void)command;
    (void)user_data;

#ifdef TRACE
    const char *function_name = gds_record_to_string(command);
    printf("gds: %s()\n", function_name);
#endif

    return true;
}

static bool gds_branch_WHILE(gds_reader_t *reader, const gds_record_t *command,
                             void *user_data) {
    bool condition = true;

    (void)command;

#ifdef TRACE
    const char *function_name = gds_record_to_string(command);
    printf("gds: %s()\n", function_name);
#endif

    if (!gds_branch_read_condition(reader, &condition, user_data)) {
        return false;
    }

    if (!condition) {
        size_t checkpoint = reader->offset;
        gds_record_t next_record;

        if (gds_reader_remaining(reader) > 0U &&
            gds_read_record(reader, &next_record) &&
            (next_record.type == GDS_RECORD_BLOCK_START ||
             next_record.type == GDS_RECORD_BLOCK_END)) {
            return gds_branch_jump_to_block_target(
                reader, next_record.type, next_record.payload.value.u32);
        }

        reader->offset = checkpoint;
        return true;
    }

    return true;
}

static bool gds_branch_Loop(gds_reader_t *reader, const gds_record_t *command,
                            void *user_data) {
    bool condition = true;

    (void)command;

#ifdef TRACE
    const char *function_name = gds_record_to_string(command);
    printf("gds: %s()\n", function_name);
#endif

    if (!gds_branch_read_condition(reader, &condition, user_data)) {
        return false;
    }

    if (!condition) {
        size_t checkpoint = reader->offset;
        gds_record_t next_record;

        if (gds_reader_remaining(reader) > 0U &&
            gds_read_record(reader, &next_record) &&
            (next_record.type == GDS_RECORD_BLOCK_START ||
             next_record.type == GDS_RECORD_BLOCK_END)) {
            return gds_branch_jump_to_block_target(
                reader, next_record.type, next_record.payload.value.u32);
        }

        reader->offset = checkpoint;
        return true;
    }

    return true;
}

bool gds_branch_lookup(gds_opcode_t opcode, gds_command_handler_fn *handler) {
    if (handler == NULL) {
        return false;
    }

    switch (opcode) {
    case SCRIPT_CMD_TRUE:
        *handler = gds_branch_TRUE;
        return true;
    case SCRIPT_CMD_FALSE:
        *handler = gds_branch_FALSE;
        return true;
    case SCRIPT_CMD_IF:
        *handler = gds_branch_IF;
        return true;
    case SCRIPT_CMD_ELSEIF:
        *handler = gds_branch_ELSEIF;
        return true;
    case SCRIPT_CMD_ELSE:
        *handler = gds_branch_ELSE;
        return true;
    case SCRIPT_CMD_WHILE:
        *handler = gds_branch_WHILE;
        return true;
    case SCRIPT_CMD_Loop:
        *handler = gds_branch_Loop;
        return true;
    default:
        *handler = NULL;
        return false;
    }
}
