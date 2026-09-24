#include "game_state.h"

#include <stdio.h>
#include <string.h>

int game_state_init(game_state_t *state, const char *assets_root,
                    language_t language) {
    if (!state) {
        fprintf(stderr,
                "widebrim: game_state_init called with NULL state pointer\n");
        return -1;
    }
    game_state_reset(state);
    state->assets_root = assets_root;
    state->language = language;
    return 0;
}

void game_state_destroy(game_state_t *state) {
    if (!state) {
        fprintf(
            stderr,
            "widebrim: game_state_destroy called with NULL state pointer\n");
        return;
    }
    memset(state, 0, sizeof(game_state_t));
}

void game_state_reset(game_state_t *state) {
    const char *assets_root;
    language_t language = LANGUAGE_EN;

    if (!state) {
        fprintf(stderr,
                "widebrim: game_state_reset called with NULL state pointer\n");
        return;
    }

    assets_root = state->assets_root;
    language = state->language;
    memset(state, 0, sizeof(game_state_t));
    state->assets_root = assets_root;
    state->language = language;
    state->current_mode = MODE_INVALID;
    state->next_mode = MODE_INVALID;
    state->place_num = 0;
    state->event_id = 0;
    state->first_touch_enabled = false;
    state->party_flags = 0;
    state->hint_coint_state.encountered = 0;
    state->hint_coint_state.available = 0;
    state->font_event_loaded = false;
}

bool game_state_party_member_active(const game_state_t *state,
                                    int member_index) {
    if (!state) {
        fprintf(stderr, "widebrim: game_state_party_member_active called with "
                        "NULL state pointer\n");
        return false;
    }
    if (member_index < 0 || member_index > 3) {
        fprintf(stderr,
                "widebrim: game_state_party_member_active called with invalid "
                "member_index %d\n",
                member_index);
        return false;
    }
    return (state->party_flags & (1u << (member_index - 2))) != 0u;
}

void game_state_party_member_set_active(game_state_t *state, int member_index,
                                        bool active) {
    uint8_t mask;
    if (!state) {
        fprintf(stderr,
                "widebrim: game_state_party_member_set_active called with "
                "NULL state pointer\n");
        return;
    }
    if (member_index < 0 || member_index > 3) {
        fprintf(
            stderr,
            "widebrim: game_state_party_member_set_active called with invalid "
            "member_index %d\n",
            member_index);
        return;
    }

    mask = (uint8_t)(1u << (member_index - 2));
    if (active) {
        state->party_flags |= mask;
    } else {
        state->party_flags &= (uint8_t)(~mask);
    }
}

bool game_state_room_hint_coin_found(const game_state_t *state, int room_num,
                                     int coin_index) {
    const uint8_t *packed;
    uint8_t value;

    if (!state) {
        fprintf(stderr, "widebrim: game_state_room_hint_coin_found called with "
                        "NULL state pointer\n");
        return false;
    }

    if (room_num < 0 || room_num >= 128) {
        fprintf(stderr,
                "widebrim: game_state_room_hint_coin_found called with invalid "
                "room_num %d\n",
                room_num);
        return false;
    }

    if (coin_index < 0 || coin_index >= 4) {
        fprintf(stderr,
                "widebrim: game_state_room_hint_coin_found called with invalid "
                "coin_index %d\n",
                coin_index);
        return false;
    }

    packed = &state->hint_coint_state.room_data[room_num / 2];
    value = (room_num & 1) ? ((packed[0] >> 4) & 0x0f) : (packed[0] & 0x0f);
    return (value & (1u << coin_index)) != 0u;
}

void game_state_room_hint_coin_set_found(game_state_t *state, int room_num,
                                         int coin_index) {
    uint8_t *packed;
    uint8_t current_value;
    uint8_t mask;

    if (!state) {
        fprintf(stderr,
                "widebrim: game_state_room_hint_coin_set_found called with "
                "NULL state pointer\n");
        return;
    }

    if (room_num < 0 || room_num >= 128) {
        fprintf(
            stderr,
            "widebrim: game_state_room_hint_coin_set_found called with invalid "
            "room_num %d\n",
            room_num);
        return;
    }

    if (coin_index < 0 || coin_index >= 4) {
        fprintf(
            stderr,
            "widebrim: game_state_room_hint_coin_set_found called with invalid "
            "coin_index %d\n",
            coin_index);
        return;
    }

    packed = &state->hint_coint_state.room_data[room_num / 2];
    mask = (uint8_t)(1u << coin_index);
    if (room_num & 1) {
        current_value = (*packed >> 4) & 0x0f;
        *packed = (uint8_t)((*packed & 0x0f) |
                            (((current_value | mask) & 0x0f) << 4));
    } else {
        current_value = *packed & 0x0f;
        *packed = (uint8_t)((*packed & 0xf0) | (current_value | mask));
    }
}

void game_state_hint_coin_mark_found(game_state_t *state, int room_num,
                                     int coin_index) {
    game_state_room_hint_coin_set_found(state, room_num, coin_index);
    state->hint_coint_state.encountered++;
    state->hint_coint_state.available++;
}

game_mode_t game_state_get_mode(const game_state_t *state) {
    if (!state) {
        fprintf(
            stderr,
            "widebrim: game_state_get_mode called with NULL state pointer\n");
        return MODE_INVALID;
    }
    return state->current_mode;
}

void game_state_set_mode(game_state_t *state, game_mode_t mode) {
    if (!state) {
        fprintf(
            stderr,
            "widebrim: game_state_set_mode called with NULL state pointer\n");
        return;
    }
    state->current_mode = mode;
}

game_mode_t game_state_get_next_mode(const game_state_t *state) {
    if (!state) {
        fprintf(stderr, "widebrim: game_state_get_next_mode called with NULL "
                        "state pointer\n");
        return MODE_INVALID;
    }
    return state->next_mode;
}

void game_state_set_next_mode(game_state_t *state, game_mode_t mode) {
    if (!state) {
        fprintf(stderr, "widebrim: game_state_set_next_mode called with NULL "
                        "state pointer\n");
        return;
    }
    state->next_mode = mode;
}
game_mode_t game_state_consume_mode_next(game_state_t *state) {
    if (!state) {
        fprintf(stderr,
                "widebrim: game_state_consume_mode_next called with NULL "
                "state pointer\n");
        return MODE_INVALID;
    }
    game_mode_t next_mode = state->next_mode;
    state->next_mode = MODE_INVALID;
    return next_mode;
}

int game_state_get_place_num(const game_state_t *state) {
    if (!state) {
        fprintf(stderr, "widebrim: game_state_get_place_num called with NULL "
                        "state pointer\n");
        return -1;
    }
    return state->place_num;
}

void game_state_set_place_num(game_state_t *state, int place_num) {
    if (!state) {
        fprintf(stderr, "widebrim: game_state_set_place_num called with NULL "
                        "state pointer\n");
        return;
    }
    state->place_num = place_num;
}

int game_state_get_event_id(const game_state_t *state) {
    if (!state) {
        fprintf(stderr, "widebrim: game_state_get_event_id called with NULL "
                        "state pointer\n");
        return -1;
    }
    return state->event_id;
}

void game_state_set_event_id(game_state_t *state, int event_id) {
    if (!state) {
        fprintf(stderr, "widebrim: game_state_set_event_id called with NULL "
                        "state pointer\n");
        return;
    }
    state->event_id = event_id;
}
