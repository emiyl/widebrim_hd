#include "gds_func.h"

#include <stdio.h>

#include "gds/gds.h"
#include "gds/gds_opcode.h"
#include "gds_state.h"
#include "modes/spawner.h"
#include "room/room.h"

#define TRACE

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

static game_state_t *gds_func_get_game_state(void *user_data) {
    mode_impl_t *impl = (mode_impl_t *)user_data;

    if (impl != NULL && impl->game_state != NULL) {
        return impl->game_state;
    }

    return (game_state_t *)user_data;
}

static bool gds_func_jump_to_block_target(gds_reader_t *reader,
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
    game_state = gds_func_get_game_state(user_data);
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

static bool gds_func_TRUE(gds_reader_t *reader, const gds_record_t *command,
                          void *user_data) {
    (void)reader;
    (void)command;

    game_state_t *game_state = gds_func_get_game_state(user_data);
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

static bool gds_func_FALSE(gds_reader_t *reader, const gds_record_t *command,
                           void *user_data) {
    (void)reader;
    (void)command;

    game_state_t *game_state = gds_func_get_game_state(user_data);
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

static bool gds_func_IF(gds_reader_t *reader, const gds_record_t *command,
                        void *user_data) {
    bool condition = true;

    (void)command;

#ifdef TRACE
    const char *function_name = gds_record_to_string(command);
    printf("gds: %s()\n", function_name);
#endif

    if (!gds_func_read_condition(reader, &condition, user_data)) {
        return false;
    }

    if (!condition) {
        size_t checkpoint = reader->offset;
        gds_record_t next_record;

        if (gds_reader_remaining(reader) > 0U &&
            gds_read_record(reader, &next_record) &&
            (next_record.type == GDS_RECORD_BLOCK_START ||
             next_record.type == GDS_RECORD_BLOCK_END)) {
            return gds_func_jump_to_block_target(reader, next_record.type,
                                                 next_record.payload.value.u32);
        }

        reader->offset = checkpoint;
        return true;
    }

    return true;
}

static bool gds_func_ELSEIF(gds_reader_t *reader, const gds_record_t *command,
                            void *user_data) {
    bool condition = true;

    (void)command;

#ifdef TRACE
    const char *function_name = gds_record_to_string(command);
    printf("gds: %s()\n", function_name);
#endif

    if (!gds_func_read_condition(reader, &condition, user_data)) {
        return false;
    }

    if (!condition) {
        size_t checkpoint = reader->offset;
        gds_record_t next_record;

        if (gds_reader_remaining(reader) > 0U &&
            gds_read_record(reader, &next_record) &&
            (next_record.type == GDS_RECORD_BLOCK_START ||
             next_record.type == GDS_RECORD_BLOCK_END)) {
            return gds_func_jump_to_block_target(reader, next_record.type,
                                                 next_record.payload.value.u32);
        }

        reader->offset = checkpoint;
        return true;
    }

    return true;
}

static bool gds_func_ELSE(gds_reader_t *reader, const gds_record_t *command,
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

static bool gds_func_WHILE(gds_reader_t *reader, const gds_record_t *command,
                           void *user_data) {
    bool condition = true;

    (void)command;

#ifdef TRACE
    const char *function_name = gds_record_to_string(command);
    printf("gds: %s()\n", function_name);
#endif

    if (!gds_func_read_condition(reader, &condition, user_data)) {
        return false;
    }

    if (!condition) {
        size_t checkpoint = reader->offset;
        gds_record_t next_record;

        if (gds_reader_remaining(reader) > 0U &&
            gds_read_record(reader, &next_record) &&
            (next_record.type == GDS_RECORD_BLOCK_START ||
             next_record.type == GDS_RECORD_BLOCK_END)) {
            return gds_func_jump_to_block_target(reader, next_record.type,
                                                 next_record.payload.value.u32);
        }

        reader->offset = checkpoint;
        return true;
    }

    return true;
}

static bool gds_func_Loop(gds_reader_t *reader, const gds_record_t *command,
                          void *user_data) {
    bool condition = true;

    (void)command;

#ifdef TRACE
    const char *function_name = gds_record_to_string(command);
    printf("gds: %s()\n", function_name);
#endif

    if (!gds_func_read_condition(reader, &condition, user_data)) {
        return false;
    }

    if (!condition) {
        size_t checkpoint = reader->offset;
        gds_record_t next_record;

        if (gds_reader_remaining(reader) > 0U &&
            gds_read_record(reader, &next_record) &&
            (next_record.type == GDS_RECORD_BLOCK_START ||
             next_record.type == GDS_RECORD_BLOCK_END)) {
            return gds_func_jump_to_block_target(reader, next_record.type,
                                                 next_record.payload.value.u32);
        }

        reader->offset = checkpoint;
        return true;
    }

    return true;
}

static bool gds_func_SetMap(gds_reader_t *reader, const gds_record_t *command,
                            void *user_data) {
    int32_t args[5];
    if (!gds_read_s32_args(reader, args, 5, command)) {
        return false;
    }

    mode_room_impl_t *room = (mode_room_impl_t *)user_data;
    if (!room) {
        fprintf(stderr, "gds: SetMap called without room context\n");
        return false;
    }

    int32_t map_text_id = args[0];
    int32_t map_background_id = args[1];
    int32_t param3 = args[2];
    int32_t param4 = args[3];
    int32_t param5 = args[4];

#ifdef TRACE
    const char *function_name = gds_record_to_string(command);
    printf("gds: %s(map_text_id=%d, map_background_id=%d, param3=%d, "
           "param4=%d, param5=%d)\n",
           function_name, map_text_id, map_background_id, param3, param4,
           param5);
#endif

    room->set_map(room, map_text_id, map_background_id, param3, param4, param5);
    return true;
}

static bool gds_func_AddTextObj(gds_reader_t *reader,
                                const gds_record_t *command, void *user_data) {
    int32_t args[7];
    if (!gds_read_s32_args(reader, args, 7, command)) {
        return false;
    }

    mode_room_impl_t *room = (mode_room_impl_t *)user_data;
    if (!room) {
        fprintf(stderr, "gds: AddTextObj called without room context\n");
        return false;
    }

    int32_t type_or_flag = args[0];
    int32_t x = args[1];
    int32_t y = args[2];
    int32_t width = args[3];
    int32_t height = args[4];
    int32_t text_id = args[5];
    int32_t param7 = args[6];

#ifdef TRACE
    const char *function_name = gds_record_to_string(command);
    printf("gds: %s(type_or_flag=%d, x=%d, y=%d, width=%d, height=%d, "
           "text_id=%d, param7=%d)\n",
           function_name, type_or_flag, x, y, width, height, text_id, param7);
#endif

    room->add_text_obj(room, type_or_flag, x, y, width, height, text_id,
                       param7);
    return true;
}

static bool gds_func_AddBGObject(gds_reader_t *reader,
                                 const gds_record_t *command, void *user_data) {
    mode_impl_t *impl = (mode_impl_t *)user_data;
    game_state_t *state = impl->game_state;
    screen_controller_t *sc = impl->screen_controller;
    object_layer_t *ol = sc->object;

    gds_record_t args[3];
    if (!gds_read_args(reader, args, 3, command)) {
        return false;
    }

    int32_t x = args[0].payload.value.s32;
    int32_t y = args[1].payload.value.s32;
    char *filename = (char *)args[2].payload.bytes.data;

#ifdef TRACE
    const char *function_name = gds_record_to_string(command);
    printf("gds: %s(x=%d, y=%d, filename=%s)\n", function_name, x, y, filename);
#endif

    if (ol && ol->add_bg_object) {
        ol->add_bg_object(ol, state, x, y, filename);
    }

    return true;
}

bool gds_func_StoryFlag(gds_reader_t *reader, const gds_record_t *command,
                        void *user_data) {
    int32_t args[1];
    if (!gds_read_s32_args(reader, args, 1, command)) {
        return false;
    }

    mode_impl_t *impl = (mode_impl_t *)user_data;
    if (!impl || !impl->game_state) {
        fprintf(stderr, "gds: StoryFlag called without game state context\n");
        return false;
    }

    int32_t story_flag = args[0];

#ifdef TRACE
    const char *function_name = gds_record_to_string(command);
    printf("gds: %s(story_flag=%d)\n", function_name, story_flag);
#endif

    gds_state_t *gds = &impl->game_state->gds;

    gds->condition_result = impl->game_state->story_flag == (int16_t)story_flag;

    return true;
}

bool gds_func_SetStoryFlag(gds_reader_t *reader, const gds_record_t *command,
                           void *user_data) {
    int32_t args[1];
    if (!gds_read_s32_args(reader, args, 1, command)) {
        return false;
    }

    mode_impl_t *impl = (mode_impl_t *)user_data;
    if (!impl || !impl->game_state) {
        fprintf(stderr,
                "gds: SetStoryFlag called without game state context\n");
        return false;
    }

    int32_t story_flag = args[0];

#ifdef TRACE
    const char *function_name = gds_record_to_string(command);
    printf("gds: %s(story_flag=%d)\n", function_name, story_flag);
#endif

    bool isQuestionCheck = false;
    if (!isQuestionCheck)
        impl->game_state->story_flag = (int16_t)story_flag;

    return true;
}

bool gds_func_ViewedEvent(gds_reader_t *reader, const gds_record_t *command,
                          void *user_data) {
    int32_t args[1];
    if (!gds_read_s32_args(reader, args, 1, command)) {
        return false;
    }

    mode_impl_t *impl = (mode_impl_t *)user_data;
    if (!impl || !impl->game_state) {
        fprintf(stderr, "gds: ViewedEvent called without game state context\n");
        return false;
    }

    int32_t event_id = args[0];

#ifdef TRACE
    const char *function_name = gds_record_to_string(command);
    printf("gds: %s(event_id=%d)\n", function_name, event_id);
#endif

    if (event_id < 0 || event_id >= MAX_EVENT_VIEWED) {
        return false;
    }

    game_state_t *state = impl->game_state;
    gds_state_t *gds = &state->gds;

    gds->condition_result = (impl->game_state->event_viewed[event_id] & 1) != 0;

    return true;
}

bool gds_func_SetEventViewed(gds_reader_t *reader, const gds_record_t *command,
                             void *user_data) {
    int32_t args[1];
    if (!gds_read_s32_args(reader, args, 1, command)) {
        return false;
    }

    mode_impl_t *impl = (mode_impl_t *)user_data;
    if (!impl || !impl->game_state) {
        fprintf(stderr,
                "gds: SetEventViewed called without game state context\n");
        return false;
    }

    int32_t event_id = args[0];

#ifdef TRACE
    const char *function_name = gds_record_to_string(command);
    printf("gds: %s(event_id=%d)\n", function_name, event_id);
#endif

    if (event_id < 0 || event_id >= MAX_EVENT_VIEWED) {
        return false;
    }

    bool isQuestionCheck = false;
    if (!isQuestionCheck) {
        impl->game_state->event_viewed[event_id] |= 1;
    }

    return true;
}

bool gds_func_AddExit(gds_reader_t *reader, const gds_record_t *command,
                      void *user_data) {
    int32_t args[8];
    if (!gds_read_s32_args(reader, args, 8, command)) {
        return false;
    }

    mode_room_impl_t *room = (mode_room_impl_t *)user_data;
    if (!room) {
        fprintf(stderr, "gds: AddExit called without room context\n");
        return false;
    }

    int32_t exit_sprite_id = args[0];
    int32_t x = args[1];
    int32_t y = args[2];
    int32_t width = args[3];
    int32_t height = args[4];
    int32_t target_map_id = args[5];
    int32_t param7 = args[6];
    int32_t param8 = args[7];

#ifdef TRACE
    const char *function_name = gds_record_to_string(command);
    printf("gds: %s(exit_sprite_id=%d, x=%d, y=%d, width=%d, height=%d, "
           "target_map_id=%d, param7=%d, param8=%d)\n",
           function_name, exit_sprite_id, x, y, width, height, target_map_id,
           param7, param8);
#endif

    room->add_exit(room, exit_sprite_id, target_map_id, x, y, width, height,
                   param7, param8);

    return true;
}

bool gds_func_BitFlag(gds_reader_t *reader, const gds_record_t *command,
                      void *user_data) {
    int32_t args[1];
    if (!gds_read_s32_args(reader, args, 1, command)) {
        return false;
    }

#ifdef TRACE
    const char *function_name = gds_record_to_string(command);
    printf("gds: %s(flag=%d)\n", function_name, args[0]);
#endif

    mode_impl_t *impl = (mode_impl_t *)user_data;
    if (!impl || !impl->game_state) {
        fprintf(stderr, "gds: BitFlag called without game state context\n");
        return false;
    }

    game_state_t *state = impl->game_state;
    gds_state_t *gds = &state->gds;

    gds->condition_result = state->bit_flag(state, args[0]);

    return true;
}

bool gds_func_SetBitFlag(gds_reader_t *reader, const gds_record_t *command,
                         void *user_data) {
    int32_t args[2];
    if (!gds_read_s32_args(reader, args, 2, command)) {
        return false;
    }

    mode_impl_t *impl = (mode_impl_t *)user_data;
    if (!impl || !impl->game_state) {
        fprintf(stderr, "gds: SetBitFlag called without game state context\n");
        return false;
    }

    int32_t flag = args[0];
    bool value = args[1] != 0;

#ifdef TRACE
    const char *function_name = gds_record_to_string(command);
    printf("gds: %s(flag=%d, value=%d)\n", function_name, args[0], args[1]);
#endif

    game_state_t *state = impl->game_state;

    state->set_bit_flag(state, flag, value);

    return true;
}

bool gds_func_AddEvent(gds_reader_t *reader, const gds_record_t *command,
                       void *user_data) {
    int32_t args[6];
    if (!gds_read_s32_args(reader, args, 6, command)) {
        return false;
    }

    mode_room_impl_t *room = (mode_room_impl_t *)user_data;
    if (!room) {
        fprintf(stderr, "gds: AddEvent called without room context\n");
        return false;
    }

    int32_t x = args[0];
    int32_t y = args[1];
    int32_t width = args[2];
    int32_t height = args[3];
    int32_t sprite_id = args[4];
    int32_t event_id = args[5];

#ifdef TRACE
    const char *function_name = gds_record_to_string(command);
    printf(
        "gds: %s(x=%d, y=%d, width=%d, height=%d, sprite_id=%d, event_id=%d)\n",
        function_name, x, y, width, height, sprite_id, event_id);
#endif

    return room->add_event(room, x, y, width, height, sprite_id, event_id);
}

bool gds_func_SetCurrentQuestion(gds_reader_t *reader,
                                 const gds_record_t *command, void *user_data) {
    int32_t args[1];
    if (!gds_read_s32_args(reader, args, 1, command)) {
        return false;
    }

    mode_impl_t *impl = (mode_impl_t *)user_data;
    if (!impl || !impl->game_state) {
        fprintf(stderr,
                "gds: SetCurrentQuestion called without game state context\n");
        return false;
    }

    int32_t question = args[0];

#ifdef TRACE
    const char *function_name = gds_record_to_string(command);
    printf("gds: %s(question=%d)\n", function_name, question);
#endif

    if (question < 0 || question >= 0x100) {
        question = 0;
    }

    game_state_t *state = impl->game_state;

    if (state->current_question != (int16_t)question) {
        state->question_state = 0;
    }

    state->current_question = (int16_t)question;

    return true;
}

bool gds_func_SolvedQuestion(gds_reader_t *reader, const gds_record_t *command,
                             void *user_data) {
    (void)reader;
    (void)command;

    mode_impl_t *impl = (mode_impl_t *)user_data;
    if (!impl || !impl->game_state) {
        fprintf(stderr,
                "gds: SolvedQuestion called without game state context\n");
        return false;
    }

    game_state_t *state = impl->game_state;

    int32_t question = state->current_question;
    size_t index = (size_t)question;

    if (index >= GDS_MAX_QUESTIONS) {
        state->gds.condition_result = false;
        return true;
    }

#ifdef TRACE
    const char *function_name = gds_record_to_string(command);
    printf("gds: %s(question=%d, index=%zu)\n", function_name, question, index);
#endif

    gds_state_t *gds = &state->gds;
    gds->condition_result = (state->question_states[index] & 0x04) != 0;

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
    case SCRIPT_CMD_ViewedEvent:
        *handler = gds_func_ViewedEvent;
        return true;
    case SCRIPT_CMD_SetEventViewed:
        *handler = gds_func_SetEventViewed;
        return true;
    case SCRIPT_CMD_AddExit:
        *handler = gds_func_AddExit;
        return true;
    case SCRIPT_CMD_BitFlag:
        *handler = gds_func_BitFlag;
        return true;
    case SCRIPT_CMD_SetBitFlag:
        *handler = gds_func_SetBitFlag;
        return true;
    case SCRIPT_CMD_AddEvent:
        *handler = gds_func_AddEvent;
        return true;
    case SCRIPT_CMD_SetCurrentQuestion:
        *handler = gds_func_SetCurrentQuestion;
        return true;
    case SCRIPT_CMD_SolvedQuestion:
        *handler = gds_func_SolvedQuestion;
        return true;
    default:
        *handler = NULL;
        return false;
    }
}
