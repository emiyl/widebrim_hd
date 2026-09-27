#include "gds_func.h"

#include <stdio.h>

#include "gds/gds.h"
#include "gds_state.h"
#include "modes/spawner.h"
#include "room/room.h"

static bool gds_func_TRUE(gds_reader_t *reader, const gds_record_t *command,
                          void *user_data) {
    (void)reader;
    (void)command;

    game_state_t *game_state = (game_state_t *)user_data;
    gds_state_t *gds = &game_state->gds;

    gds->condition_result = true;

    return true;
}

static bool gds_func_FALSE(gds_reader_t *reader, const gds_record_t *command,
                           void *user_data) {
    (void)reader;
    (void)command;

    game_state_t *game_state = (game_state_t *)user_data;
    gds_state_t *gds = &game_state->gds;

    gds->condition_result = false;

    return true;
}

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

static bool gds_func_is_ignored_record_type(gds_record_type_t type) {
    switch (type) {
    case GDS_RECORD_EMPTY_5:
    case GDS_RECORD_EMPTY_8:
    case GDS_RECORD_EMPTY_9:
    case GDS_RECORD_EMPTY_10:
    case GDS_RECORD_EMPTY_11:
    case GDS_RECORD_BREAKPOINT:
        return true;
    default:
        return false;
    }
}

static bool gds_func_read_condition(gds_reader_t *reader, bool *result,
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
    game_state = (game_state_t *)user_data;
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

    while (gds_func_is_ignored_record_type(record.type)) {
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

    switch (record.type) {
    case GDS_RECORD_COMMAND:
        if (gds_func_lookup(record.payload.opcode, &handler) &&
            handler != NULL) {
            if (!handler(reader, &record, &gds->condition_result)) {
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
    case GDS_RECORD_VALUE_6:
    case GDS_RECORD_VALUE_7:
        *result = record.payload.value.u32 != 0U;
        return true;
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

    if (!gds_func_read_condition(reader, &condition, user_data)) {
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

    if (!gds_func_read_condition(reader, &condition, user_data)) {
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

    if (!gds_func_read_condition(reader, &condition, user_data)) {
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

    if (!gds_func_read_condition(reader, &condition, user_data)) {
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
    if (!impl || !impl->setmap) {
        fprintf(stderr, "gds: setmap function pointer is NULL - is this "
                        "called from a room?\n");
        return false;
    }

    impl->setmap(impl, map_text_id, map_background_id, param3, param4, param5);
    return true;
}

static bool gds_func_AddTextObj(gds_reader_t *reader,
                                const gds_record_t *command, void *user_data) {
    (void)command;
    int32_t argv[7];
    mode_room_impl_t *impl = (mode_room_impl_t *)user_data;

    if (!gds_read_s32_args(reader, argv, 7, "AddTextObj")) {
        return false;
    }

    if (!impl || !impl->add_text_obj) {
        fprintf(stderr, "gds: AddTextObj called without room context\n");
        return false;
    }

    int32_t type_or_flag = argv[0];
    int32_t x = argv[1];
    int32_t y = argv[2];
    int32_t width = argv[3];
    int32_t height = argv[4];
    int32_t text_id = argv[5];
    int32_t param7 = argv[6];

    if (!impl || !impl->add_text_obj) {
        fprintf(stderr, "gds: AddTextObj called without room context\n");
        return false;
    }

    impl->add_text_obj(impl, type_or_flag, x, y, width, height, text_id,
                       param7);
    return true;
}

static bool gds_func_AddBGObject(gds_reader_t *reader,
                                 const gds_record_t *command, void *user_data) {
    (void)command;
    (void)user_data;

    mode_impl_t *impl = (mode_impl_t *)user_data;
    game_state_t *state = impl->game_state;
    screen_controller_t *sc = impl->screen_controller;
    object_layer_t *ol = sc->object;

    gds_record_t args[3];
    if (!gds_read_args(reader, args, 3, "AddBGObject")) {
        return false;
    }

    int32_t x = args[0].payload.value.s32;
    int32_t y = args[1].payload.value.s32;
    char *filename = (char *)args[2].payload.bytes.data;

    if (ol && ol->add_bg_object) {
        ol->add_bg_object(ol, state, x, y, filename);
    }

    return true;
}

bool gds_func_StoryFlag(gds_reader_t *reader, const gds_record_t *command,
                        void *user_data) {
    (void)command;
    mode_impl_t *impl = (mode_impl_t *)user_data;
    game_state_t *state = impl->game_state;
    gds_state_t *gds = &state->gds;

    if (!impl || !impl->game_state) {
        fprintf(stderr, "gds: StoryFlag called without game state context\n");
        return false;
    }

    int32_t args[1];
    if (!gds_read_s32_args(reader, args, 1, "StoryFlag")) {
        return false;
    }
    int32_t story_flag = args[0];

    gds->condition_result = impl->game_state->story_flag == (int16_t)story_flag;

    return true;
}

bool gds_func_SetStoryFlag(gds_reader_t *reader, const gds_record_t *command,
                           void *user_data) {
    (void)command;
    mode_impl_t *impl = (mode_impl_t *)user_data;

    if (!impl || !impl->game_state) {
        fprintf(stderr,
                "gds: SetStoryFlag called without game state context\n");
        return false;
    }

    int32_t args[1];
    if (!gds_read_s32_args(reader, args, 1, "SetStoryFlag")) {
        return false;
    }
    int32_t story_flag = args[0];

    impl->game_state->story_flag = (int16_t)story_flag;
    return true;
}

bool gds_func_lookup(gds_opcode_t opcode, gds_command_handler_fn *handler) {
    if (handler == NULL) {
        return false;
    }

    switch (opcode) {
    case SCRIPT_CMD_TRUE:
        *handler = gds_func_TRUE;
        return true;
    case SCRIPT_CMD_FALSE:
        *handler = gds_func_FALSE;
        return true;
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
    case SCRIPT_CMD_AddBGObject:
        *handler = gds_func_AddBGObject;
        return true;
    case SCRIPT_CMD_StoryFlag:
        *handler = gds_func_StoryFlag;
        return true;
    case SCRIPT_CMD_SetStoryFlag:
        *handler = gds_func_SetStoryFlag;
        return true;
    default:
        *handler = NULL;
        return false;
    }
}
