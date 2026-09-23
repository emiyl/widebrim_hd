#ifndef MODE_H
#define MODE_H

#include <stdbool.h>

#include "screen.h"

typedef enum {
    MODE_RESET = 0,
    MODE_ROOM = 1,
    MODE_DRAMA_EVENT = 3,
    MODE_MOVIE = 6,
    MODE_START_PUZZLE = 7,
    MODE_END_PUZZLE = 8,
    MODE_STAY_PUZZLE = 9,
    MODE_PUZZLE = 10,
    MODE_UNK_NAZO = 11,
    MODE_TITLE = 12,
    MODE_NARRATION = 13,
    MODE_SUB_CAMERA = 14,
    MODE_SUB_HERB_TEA = 15,
    MODE_SUB_HAMSTER = 16,
    MODE_BAG = 17,
    MODE_NAME = 18,
    MODE_JITEN_BAG = 19,
    MODE_MYSTERY = 20,
    MODE_STAFF = 21,
    MODE_JITEN_WIFI = 22,
    MODE_MEMO = 23,
    MODE_CHALLENGE = 24,
    MODE_EVENT_TEA = 25,
    MODE_UNK_SUB_PHOTO0 = 26,
    MODE_UNK_SUB_PHOTO1 = 27,
    MODE_SECRET_MENU = 28,
    MODE_WIFI_SECRET_MENU = 29,
    MODE_TOP_SECRET_MENU = 30,
    MODE_JITEN_SECRET = 31,
    MODE_ART_MODE = 32,
    MODE_CHR_VIEW_MODE = 33,
    MODE_MUSIC_MODE = 34,
    MODE_VOICE_MODE = 35,
    MODE_MOVIE_VIEW_MODE = 36,
    MODE_HAMSTER_NAME = 37,
    MODE_NINTENDO_WFC_SETUP = 38,
    MODE_WIFI_DOWNLOAD_PUZZLE = 39,
    MODE_PASSCODE = 40,
    MODE_CODE_INPUT_PANDORA = 41,
    MODE_CODE_INPUT_FUTURE = 42,
    MODE_DIARY = 43,
    MODE_NAZOBA = 44,
    MODE_INVALID = 255
} game_mode_t;

typedef struct {
    screen_layer_t layer;
    bool (*is_done)(void *impl);
    bool valid;
} mode_handler_t;

#endif // MODE_H
