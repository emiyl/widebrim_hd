#include "gds_func.h"

#include <stdio.h>

#include "../modes/room/room.h"
#include "gds.h"
#include "gds_branch.h"
#include "gds_opcode.h"
#include "gds_reader.h"
#include "gds_state.h"

GDS_DEF(SetMap) {
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

#ifdef GDS_TRACE
    const char *function_name = gds_record_to_string(command);
    printf("gds: %s(map_text_id=%d, map_background_id=%d, param3=%d, "
           "param4=%d, param5=%d)\n",
           function_name, map_text_id, map_background_id, param3, param4,
           param5);
#endif

    room->set_map(room, map_text_id, map_background_id, param3, param4, param5);
    return true;
}

GDS_DEF(AddTextObj) {
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

#ifdef GDS_TRACE
    const char *function_name = gds_record_to_string(command);
    printf("gds: %s(type_or_flag=%d, x=%d, y=%d, width=%d, height=%d, "
           "text_id=%d, param7=%d)\n",
           function_name, type_or_flag, x, y, width, height, text_id, param7);
#endif

    room->add_text_obj(room, type_or_flag, x, y, width, height, text_id,
                       param7);
    return true;
}

GDS_DEF(AddBGObject) {
    mode_impl_t *impl = (mode_impl_t *)user_data;
    game_state_t *state = impl->state;
    screen_controller_t *sc = impl->controller;
    object_layer_t *ol = sc->object;

    gds_record_t args[3];
    if (!gds_read_args(reader, args, 3, command)) {
        return false;
    }

    int32_t x = args[0].payload.value.s32;
    int32_t y = args[1].payload.value.s32;
    char *filename = (char *)args[2].payload.bytes.data;

#ifdef GDS_TRACE
    const char *function_name = gds_record_to_string(command);
    printf("gds: %s(x=%d, y=%d, filename=%s)\n", function_name, x, y, filename);
#endif

    if (ol && ol->add_bg_object) {
        ol->add_bg_object(ol, state, x, y, filename);
    }

    return true;
}

GDS_DEF(StoryFlag) {
    int32_t args[1];
    if (!gds_read_s32_args(reader, args, 1, command)) {
        return false;
    }

    mode_impl_t *impl = (mode_impl_t *)user_data;
    if (!impl || !impl->state) {
        fprintf(stderr, "gds: StoryFlag called without game state context\n");
        return false;
    }

    int32_t story_flag = args[0];

#ifdef GDS_TRACE
    const char *function_name = gds_record_to_string(command);
    printf("gds: %s(story_flag=%d)\n", function_name, story_flag);
#endif

    gds_state_t *gds = &impl->state->gds;

    gds->condition_result = impl->state->story_flag == (int16_t)story_flag;

    return true;
}

GDS_DEF(SetStoryFlag) {
    int32_t args[1];
    if (!gds_read_s32_args(reader, args, 1, command)) {
        return false;
    }

    mode_impl_t *impl = (mode_impl_t *)user_data;
    if (!impl || !impl->state) {
        fprintf(stderr,
                "gds: SetStoryFlag called without game state context\n");
        return false;
    }

    int32_t story_flag = args[0];

#ifdef GDS_TRACE
    const char *function_name = gds_record_to_string(command);
    printf("gds: %s(story_flag=%d)\n", function_name, story_flag);
#endif

    game_state_t *state = impl->state;
    if (!state->isQuestionCheck)
        state->story_flag = (int16_t)story_flag;

    return true;
}

GDS_DEF(ViewedEvent) {
    int32_t args[1];
    if (!gds_read_s32_args(reader, args, 1, command)) {
        return false;
    }

    mode_impl_t *impl = (mode_impl_t *)user_data;
    if (!impl || !impl->state) {
        fprintf(stderr, "gds: ViewedEvent called without game state context\n");
        return false;
    }

    int32_t event_id = args[0];

#ifdef GDS_TRACE
    const char *function_name = gds_record_to_string(command);
    printf("gds: %s(event_id=%d)\n", function_name, event_id);
#endif

    if (event_id < 0 || event_id >= MAX_EVENT_VIEWED) {
        return false;
    }

    game_state_t *state = impl->state;
    gds_state_t *gds = &state->gds;

    gds->condition_result = (state->event_viewed[event_id] & 1) != 0;

    return true;
}

GDS_DEF(SetEventViewed) {
    int32_t args[1];
    if (!gds_read_s32_args(reader, args, 1, command)) {
        return false;
    }

    mode_impl_t *impl = (mode_impl_t *)user_data;
    if (!impl || !impl->state) {
        fprintf(stderr,
                "gds: SetEventViewed called without game state context\n");
        return false;
    }

    int32_t event_id = args[0];

#ifdef GDS_TRACE
    const char *function_name = gds_record_to_string(command);
    printf("gds: %s(event_id=%d)\n", function_name, event_id);
#endif

    if (event_id < 0 || event_id >= MAX_EVENT_VIEWED) {
        return false;
    }

    game_state_t *state = impl->state;
    if (!state->isQuestionCheck) {
        state->event_viewed[event_id] |= 1;
    }

    return true;
}

GDS_DEF(AddExit) {
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

#ifdef GDS_TRACE
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

GDS_DEF(BitFlag) {
    int32_t args[1];
    if (!gds_read_s32_args(reader, args, 1, command)) {
        return false;
    }

#ifdef GDS_TRACE
    const char *function_name = gds_record_to_string(command);
    printf("gds: %s(flag=%d)\n", function_name, args[0]);
#endif

    mode_impl_t *impl = (mode_impl_t *)user_data;
    if (!impl || !impl->state) {
        fprintf(stderr, "gds: BitFlag called without game state context\n");
        return false;
    }

    game_state_t *state = impl->state;
    gds_state_t *gds = &state->gds;

    gds->condition_result = state->bit_flag(state, args[0]);

    return true;
}

GDS_DEF(SetBitFlag) {
    int32_t args[2];
    if (!gds_read_s32_args(reader, args, 2, command)) {
        return false;
    }

    mode_impl_t *impl = (mode_impl_t *)user_data;
    if (!impl || !impl->state) {
        fprintf(stderr, "gds: SetBitFlag called without game state context\n");
        return false;
    }

    int32_t flag = args[0];
    bool value = args[1] != 0;

#ifdef GDS_TRACE
    const char *function_name = gds_record_to_string(command);
    printf("gds: %s(flag=%d, value=%d)\n", function_name, args[0], args[1]);
#endif

    game_state_t *state = impl->state;

    state->set_bit_flag(state, flag, value);

    return true;
}

GDS_DEF(AddEvent) {
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

#ifdef GDS_TRACE
    const char *function_name = gds_record_to_string(command);
    printf(
        "gds: %s(x=%d, y=%d, width=%d, height=%d, sprite_id=%d, event_id=%d)\n",
        function_name, x, y, width, height, sprite_id, event_id);
#endif

    return room->add_event(room, x, y, width, height, sprite_id, event_id);
}

GDS_DEF(SetCurrentQuestion) {
    int32_t args[1];
    if (!gds_read_s32_args(reader, args, 1, command)) {
        return false;
    }

    mode_impl_t *impl = (mode_impl_t *)user_data;
    if (!impl || !impl->state) {
        fprintf(stderr,
                "gds: SetCurrentQuestion called without game state context\n");
        return false;
    }

    int32_t question = args[0];

#ifdef GDS_TRACE
    const char *function_name = gds_record_to_string(command);
    printf("gds: %s(question=%d)\n", function_name, question);
#endif

    if (question < 0 || question >= 0x100) {
        question = 0;
    }

    game_state_t *state = impl->state;

    if (state->current_question != (int16_t)question) {
        state->question_state = 0;
    }

    state->current_question = (int16_t)question;

    return true;
}

GDS_DEF(SolvedQuestion) {
    (void)reader;
    (void)command;

    mode_impl_t *impl = (mode_impl_t *)user_data;
    if (!impl || !impl->state) {
        fprintf(stderr,
                "gds: SolvedQuestion called without game state context\n");
        return false;
    }

    game_state_t *state = impl->state;

    int32_t question = state->current_question;
    size_t index = (size_t)question;

    if (index >= GDS_MAX_QUESTIONS) {
        state->gds.condition_result = false;
        return true;
    }

#ifdef GDS_TRACE
    const char *function_name = gds_record_to_string(command);
    printf("gds: %s(question=%d, index=%zu)\n", function_name, question, index);
#endif

    gds_state_t *gds = &state->gds;
    gds->condition_result = (state->question_states[index] & 0x04) != 0;

    return true;
}

GDS_DEF(AddSubSprite) {
    mode_impl_t *impl = (mode_impl_t *)user_data;
    game_state_t *state = impl->state;
    screen_controller_t *sc = impl->controller;
    object_layer_t *ol = sc->object;

    int32_t int_args[2];
    const char *string_args[2];

    if (!gds_read_s32_args(reader, int_args, 2, command)) {
        return false;
    }

    if (!gds_read_string_args(reader, string_args, 2, command)) {
        return false;
    }

    int32_t x = int_args[0];
    int32_t y = int_args[1];
    const char *sprite_name = string_args[0];
    const char *animation_name = string_args[1];

#ifdef GDS_TRACE
    const char *function_name = gds_record_to_string(command);
    printf("gds: %s(x=%d, y=%d, sprite_name=\"%s\", animation_name=\"%s\")\n",
           function_name, x, y, sprite_name, animation_name);
#endif

    if (ol && ol->add_sub_sprite) {
        ol->add_sub_sprite(ol, state, x, y, sprite_name, animation_name);
    }

    return true;
}

GDS_DEF(SetGameMode) {
    mode_impl_t *impl = (mode_impl_t *)user_data;
    game_state_t *state = impl->state;

    const char *event_mode_str = NULL;
    const char *string_args[1];
    if (!gds_read_string_args(reader, string_args, 1, command)) {
        return false;
    }
    event_mode_str = string_args[0];

#ifdef GDS_TRACE
    const char *function_name = gds_record_to_string(command);
    printf("gds: %s(event_mode=\"%s\")\n", function_name, event_mode_str);
#endif

    game_mode_t mode = string_to_game_mode(event_mode_str);
    if (mode == MODE_INVALID) {
        fprintf(stderr,
                "widebrim: SetGameMode called with invalid mode "
                "string \"%s\"\n",
                event_mode_str);
        return false;
    }

    if (impl) {
        impl->done = true;
    }
    state->set_next_mode(state, mode);

    return true;
}

GDS_DEF(SetCurrentEvent) {
    mode_impl_t *impl = (mode_impl_t *)user_data;
    game_state_t *state = impl->state;

    int32_t event_id;
    int32_t args[1];
    if (!gds_read_s32_args(reader, args, sizeof(args) / sizeof(args[0]),
                           command)) {
        return false;
    }
    event_id = args[0];

#ifdef GDS_TRACE
    const char *function_name = gds_record_to_string(command);
    printf("gds: %s(event_id=%d)\n", function_name, event_id);
#endif

    state->set_current_event(state, event_id);

    return true;
}

GDS_DEF(AddHintCoin) {
    int32_t args[6];
    if (!gds_read_s32_args(reader, args, sizeof(args) / sizeof(args[0]),
                           command)) {
        return false;
    }

    int32_t coin_id = args[0];
    int32_t x = args[1];
    int32_t y = args[2];
    int32_t width = args[3];
    int32_t height = args[4];
    int32_t arg6 = args[5];

#ifdef GDS_TRACE
    const char *function_name = gds_record_to_string(command);
    printf(
        "gds: STUB %s(coin_id=%d, x=%d, y=%d, width=%d, height=%d, arg6=%d)\n",
        function_name, coin_id, x, y, width, height, arg6);
#else
    (void)coin_id;
    (void)x;
    (void)y;
    (void)width;
    (void)height;
    (void)arg6;
#endif

    return true;
}

GDS_DEF(AddDogCoin) {
    int32_t args[6];
    if (!gds_read_s32_args(reader, args, sizeof(args) / sizeof(args[0]),
                           command)) {
        return false;
    }

    int32_t coin_id = args[0];
    int32_t x = args[1];
    int32_t y = args[2];
    bool flag = args[3] != 0;
    int32_t distance_or_size = args[4];
    int32_t variant = args[5];

#ifdef GDS_TRACE
    const char *function_name = gds_record_to_string(command);
    printf("gds: STUB %s(coin_id=%d, x=%d, y=%d, flag=%d, distance_or_size=%d, "
           "variant=%d)\n",
           function_name, coin_id, x, y, flag, distance_or_size, variant);
#else
    (void)coin_id;
    (void)x;
    (void)y;
    (void)flag;
    (void)distance_or_size;
    (void)variant;
#endif

    return true;
}

GDS_DEF(AddSecretCoin) {
    int32_t args[6];
    if (!gds_read_s32_args(reader, args, sizeof(args) / sizeof(args[0]),
                           command)) {
        return false;
    }

    int32_t coin_id = args[0];
    int32_t x = args[1];
    int32_t y = args[2];
    int32_t arg4 = args[3];
    int32_t arg5 = args[4];
    int32_t arg6 = args[5];

#ifdef GDS_TRACE
    const char *function_name = gds_record_to_string(command);
    printf("gds: STUB %s(coin_id=%d, x=%d, y=%d, arg4=%d, arg5=%d, arg6=%d)\n",
           function_name, coin_id, x, y, arg4, arg5, arg6);
#else
    (void)coin_id;
    (void)x;
    (void)y;
    (void)arg4;
    (void)arg5;
    (void)arg6;
#endif

    return true;
}

GDS_DEF(SetExitSound) {
    int32_t args[2];
    if (!gds_read_s32_args(reader, args, sizeof(args) / sizeof(args[0]),
                           command)) {
        return false;
    }

    int32_t exit_id = args[0];
    int32_t sound_id = args[1];

#ifdef GDS_TRACE
    const char *function_name = gds_record_to_string(command);
    printf("gds: STUB %s(exit_id=%d, sound_id=%d)\n", function_name, exit_id,
           sound_id);
#else
    (void)exit_id;
    (void)sound_id;
#endif

    return true;
}

GDS_DEF(ExitScript) {
    (void)reader;
    (void)command;

#ifdef GDS_TRACE
    const char *function_name = gds_record_to_string(command);
    printf("gds: %s()\n", function_name);
#endif

    mode_impl_t *impl = (mode_impl_t *)user_data;
    if (impl) {
        impl->done = true;
    }

    return true;
}

GDS_DEF(LoadBG) {
    mode_impl_t *impl = (mode_impl_t *)user_data;
    game_state_t *state = impl->state;
    bg_layer_t *bg = impl->controller->bg;

    const char *string_args[1];
    int32_t int_args[1];
    if (!gds_read_string_args(reader, string_args,
                              sizeof(string_args) / sizeof(string_args[0]),
                              command)) {
        return false;
    }
    if (!gds_read_s32_args(reader, int_args,
                           sizeof(int_args) / sizeof(int_args[0]), command)) {
        return false;
    }

    const char *bg_name = string_args[0];
    int32_t layer = int_args[0];

#ifdef GDS_TRACE
    const char *function_name = gds_record_to_string(command);
    printf("gds: %s(bg_name=\"%s\", layer=%d)\n", function_name, bg_name,
           layer);
#else
    (void)bg_name;
    (void)layer;
#endif

    bg->load_main(bg, state, bg_name);

    return true;
}

GDS_DEF(LoadSubBG) {
    mode_impl_t *impl = (mode_impl_t *)user_data;
    game_state_t *state = impl->state;
    bg_layer_t *bg = impl->controller->bg;

    const char *string_args[1];
    int32_t int_args[1];
    if (!gds_read_string_args(reader, string_args,
                              sizeof(string_args) / sizeof(string_args[0]),
                              command)) {
        return false;
    }
    if (!gds_read_s32_args(reader, int_args,
                           sizeof(int_args) / sizeof(int_args[0]), command)) {
        return false;
    }

    const char *bg_name = string_args[0];
    int32_t layer = int_args[0];

#ifdef GDS_TRACE
    const char *function_name = gds_record_to_string(command);
    printf("gds: %s(bg_name=\"%s\", layer=%d)\n", function_name, bg_name,
           layer);
#else
    (void)bg_name;
    (void)layer;
#endif

    bg->load_sub(bg, state, bg_name);

    return true;
}

GDS_DEF(ModifyBGPal) {
    int32_t args[4];
    if (!gds_read_s32_args(reader, args, sizeof(args) / sizeof(args[0]),
                           command)) {
        return false;
    }

    mode_impl_t *impl = (mode_impl_t *)user_data;
    if (!impl || !impl->controller || !impl->controller->bg) {
        fprintf(stderr, "gds: ModifyBGPal called without background context\n");
        return false;
    }

    int32_t red = args[0];
    int32_t green = args[1];
    int32_t blue = args[2];
    int32_t intensity = args[3];

#ifdef GDS_TRACE
    const char *function_name = gds_record_to_string(command);
    printf("gds: %s(red=%d, green=%d, blue=%d, intensity=%d)\n", function_name,
           red, green, blue, intensity);
#endif

    if (!impl->state || impl->state->isQuestionCheck) {
        return true;
    }

    if (impl->controller->bg->set_main_palette) {
        impl->controller->bg->set_main_palette(impl->controller->bg, red, green,
                                               blue, intensity);
    }
    return true;
}

GDS_DEF(ModifySubBGPal) {
    int32_t args[4];
    if (!gds_read_s32_args(reader, args, sizeof(args) / sizeof(args[0]),
                           command)) {
        return false;
    }

    mode_impl_t *impl = (mode_impl_t *)user_data;
    if (!impl || !impl->controller || !impl->controller->bg) {
        fprintf(stderr,
                "gds: ModifySubBGPal called without background context\n");
        return false;
    }

    int32_t red = args[0];
    int32_t green = args[1];
    int32_t blue = args[2];
    int32_t intensity = args[3];

#ifdef GDS_TRACE
    const char *function_name = gds_record_to_string(command);
    printf("gds: %s(red=%d, green=%d, blue=%d, intensity=%d)\n", function_name,
           red, green, blue, intensity);
#endif

    if (!impl->state || impl->state->isQuestionCheck) {
        return true;
    }

    if (impl->controller->bg->set_sub_palette) {
        impl->controller->bg->set_sub_palette(impl->controller->bg, red, green,
                                              blue, intensity);
    }
    return true;
}

GDS_DEF(AddSprite) {
    mode_impl_t *impl = (mode_impl_t *)user_data;
    game_state_t *state = impl ? impl->state : NULL;
    screen_controller_t *sc = impl ? impl->controller : NULL;
    object_layer_t *ol = sc ? sc->object : NULL;

    int32_t int_args[2];
    const char *str_args[2];

    if (!gds_read_s32_args(reader, int_args,
                           sizeof(int_args) / sizeof(int_args[0]), command)) {
        return false;
    }
    if (!gds_read_string_args(reader, str_args, 2, command)) {
        return false;
    }

    int32_t x = int_args[0];
    int32_t y = int_args[1];
    const char *sprite_name = str_args[0];
    const char *animation_name = str_args[1];

#ifdef GDS_TRACE
    const char *function_name = gds_record_to_string(command);
    printf("gds: %s(x=%d, y=%d, sprite_name=\"%s\", animation_name=\"%s\")\n",
           function_name, x, y, sprite_name, animation_name);
#endif

    if (ol && ol->add_sprite) {
        ol->add_sprite(ol, state, x, y, sprite_name, animation_name);
    }

    return true;
}

GDS_DEF(Print) {
    const char *str_args[1];
    if (!gds_read_string_args(reader, str_args, 1, command)) {
        return false;
    }

#ifdef GDS_TRACE
    const char *function_name = gds_record_to_string(command);
    printf("gds: %s(str=\"%s\")\n", function_name, str_args[0]);
#endif

    const char *str = str_args[0];
    printf("%s\n", str);

    return true;
}

bool gds_func_lookup(gds_opcode_t opcode, gds_command_handler_fn *handler) {
    if (handler == NULL) {
        return false;
    }

    if (gds_branch_lookup(opcode, handler)) {
        return true;
    }

    switch (opcode) {
        GDS_FUNC_LOOKUP_TABLE(GDS_FUNC_LOOKUP_CASE)
    default:
        *handler = NULL;
        return false;
    }
}
