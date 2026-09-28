#ifndef GAME_STATE_H
#define GAME_STATE_H

#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "gds/gds_state.h"
#include "language.h"
#include "mode.h"

typedef struct {
    uint8_t room_data[64];
    uint16_t encountered;
    uint16_t available;
} hint_coin_state_t;

#define GDS_MAX_QUESTIONS 256
#define MAX_EVENT_VIEWED 0x1DF
#define BIT_FLAG_COUNT 0x400
#define BIT_FLAG_BYTES (BIT_FLAG_COUNT / 8)

typedef struct game_state_t {
    gds_state_t gds;
    const char *assets_root;
    const char *resource_pack_root;
    language_t language;
    game_mode_t current_mode;
    game_mode_t next_mode;

    int16_t story_flag;
    uint8_t memo_flag;
    uint8_t event_viewed[MAX_EVENT_VIEWED];
    uint8_t bit_flags[BIT_FLAG_BYTES];

    int16_t current_question;
    uint8_t question_state;
    uint8_t question_states[256];
    bool isQuestionCheck;
    bool isQuestionExist;

    bool (*bit_flag)(const struct game_state_t *state, int flag);
    void (*set_bit_flag)(struct game_state_t *state, int flag, bool value);

    int place_num;
    int event_id;
    bool first_touch_enabled;
    hint_coin_state_t hint_coint_state;
    uint8_t party_flags;
    bool font_event_loaded;
} game_state_t;

int game_state_init(game_state_t *state, const char *assets_root,
                    const char *resource_pack_root, language_t language);
void game_state_destroy(game_state_t *state);

void game_state_reset(game_state_t *state);

game_mode_t game_state_get_mode(const game_state_t *state);
void game_state_set_mode(game_state_t *state, game_mode_t mode);
game_mode_t game_state_get_next_mode(const game_state_t *state);
void game_state_set_next_mode(game_state_t *state, game_mode_t mode);
game_mode_t game_state_consume_mode_next(game_state_t *state);

int game_state_get_place_num(const game_state_t *state);
void game_state_set_place_num(game_state_t *state, int place_num);

int game_state_get_event_id(const game_state_t *state);
void game_state_set_event_id(game_state_t *state, int event_id);

bool game_state_party_member_active(const game_state_t *state,
                                    int member_index);
void game_state_party_member_set_active(game_state_t *state, int member_index,
                                        bool active);

bool game_state_room_hint_coin_found(const game_state_t *state, int room_num,
                                     int coin_index);
void game_state_room_hint_coin_set_found(game_state_t *state, int room_num,
                                         int coin_index);
void game_state_hint_coin_mark_found(game_state_t *state, int room_num,
                                     int coin_index);

static inline uint32_t game_state_scale_from_root(const char *root) {
    char scale_path[4096];
    FILE *file;
    char line[64];
    char *end = NULL;
    unsigned long value;

    if (!root || root[0] == '\0') {
        return 1U;
    }

    if (snprintf(scale_path, sizeof(scale_path), "%s/scale.txt", root) >=
        (int)sizeof(scale_path)) {
        return 1U;
    }

    file = fopen(scale_path, "r");
    if (file == NULL) {
        return 1U;
    }

    if (fgets(line, sizeof(line), file) == NULL) {
        fclose(file);
        return 1U;
    }
    fclose(file);

    errno = 0;
    value = strtoul(line, &end, 10);
    if (errno != 0 || end == line || value == 0UL || value > UINT32_MAX) {
        return 1U;
    }

    while (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r') {
        ++end;
    }
    if (*end != '\0') {
        return 1U;
    }

    return (uint32_t)value;
}

static inline bool
game_state_path_is_in_resource_pack(const game_state_t *state,
                                    const char *path) {
    size_t root_len;

    if (!state || !state->resource_pack_root || !path ||
        state->resource_pack_root[0] == '\0') {
        return false;
    }

    root_len = strlen(state->resource_pack_root);
    if (root_len == 0U) {
        return false;
    }

    if (strncmp(path, state->resource_pack_root, root_len) != 0) {
        return false;
    }

    return path[root_len] == '\0' || path[root_len] == '/';
}

static inline uint32_t
game_state_resource_pack_scale_for_path(const game_state_t *state,
                                        const char *path) {
    if (!state || !state->resource_pack_root ||
        state->resource_pack_root[0] == '\0') {
        return 1U;
    }

    if (path && !game_state_path_is_in_resource_pack(state, path)) {
        return 1U;
    }

    return game_state_scale_from_root(state->resource_pack_root);
}

static inline uint32_t
game_state_resource_pack_scale(const game_state_t *state) {
    return game_state_resource_pack_scale_for_path(state, NULL);
}

static inline int game_state_scale_dimension_for_path(const game_state_t *state,
                                                      int value,
                                                      const char *path) {
    uint32_t scale = game_state_resource_pack_scale_for_path(state, path);

    if (scale <= 1U || value <= 0) {
        return value;
    }

    if (!game_state_path_is_in_resource_pack(state, path)) {
        return value;
    }

    return value / (int)scale;
}

static inline int game_state_scale_dimension(const game_state_t *state,
                                             int value) {
    return game_state_scale_dimension_for_path(state, value, NULL);
}

static inline bool asset_path_resolve(const game_state_t *state,
                                      const char *rel_path, char *out_path,
                                      size_t out_path_size) {
    if (!state) {
        return false;
    }

    return asset_path_resolve_roots(state->assets_root,
                                    state->resource_pack_root, state->language,
                                    rel_path, out_path, out_path_size);
}

#endif // GAME_STATE_H
