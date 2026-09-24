#ifndef GAME_STATE_H
#define GAME_STATE_H

#include <stdbool.h>
#include <stdint.h>

#include "mode.h"

typedef struct {
    uint8_t room_data[64];
    uint16_t encountered;
    uint16_t available;
} hint_coin_state_t;

typedef struct {
    const char *assets_root;
    game_mode_t current_mode;
    game_mode_t next_mode;
    int place_num;
    int event_id;
    bool first_touch_enabled;
    hint_coin_state_t hint_coint_state;
    uint8_t party_flags;
    bool font_event_loaded;
} game_state_t;

int game_state_init(game_state_t *state, const char *assets_root);
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

#endif // GAME_STATE_H
