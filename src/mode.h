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

static inline const char *game_mode_to_string(game_mode_t mode) {
    switch (mode) {
    case MODE_RESET:
        return "RESET";
    case MODE_ROOM:
        return "ROOM";
    case MODE_DRAMA_EVENT:
        return "DRAMA_EVENT";
    case MODE_MOVIE:
        return "MOVIE";
    case MODE_START_PUZZLE:
        return "START_PUZZLE";
    case MODE_END_PUZZLE:
        return "END_PUZZLE";
    case MODE_STAY_PUZZLE:
        return "STAY_PUZZLE";
    case MODE_PUZZLE:
        return "PUZZLE";
    case MODE_UNK_NAZO:
        return "UNK_NAZO";
    case MODE_TITLE:
        return "TITLE";
    case MODE_NARRATION:
        return "NARRATION";
    case MODE_SUB_CAMERA:
        return "SUB_CAMERA";
    case MODE_SUB_HERB_TEA:
        return "SUB_HERB_TEA";
    case MODE_SUB_HAMSTER:
        return "SUB_HAMSTER";
    case MODE_BAG:
        return "BAG";
    case MODE_NAME:
        return "NAME";
    case MODE_JITEN_BAG:
        return "JITEN_BAG";
    case MODE_MYSTERY:
        return "MYSTERY";
    case MODE_STAFF:
        return "STAFF";
    case MODE_JITEN_WIFI:
        return "JITEN_WIFI";
    case MODE_MEMO:
        return "MEMO";
    case MODE_CHALLENGE:
        return "CHALLENGE";
    case MODE_EVENT_TEA:
        return "EVENT_TEA";
    case MODE_UNK_SUB_PHOTO0:
        return "UNK_SUB_PHOTO0";
    case MODE_UNK_SUB_PHOTO1:
        return "UNK_SUB_PHOTO1";
    case MODE_SECRET_MENU:
        return "SECRET_MENU";
    case MODE_WIFI_SECRET_MENU:
        return "WIFI_SECRET_MENU";
    case MODE_TOP_SECRET_MENU:
        return "TOP_SECRET_MENU";
    case MODE_JITEN_SECRET:
        return "JITEN_SECRET";
    case MODE_ART_MODE:
        return "ART_MODE";
    case MODE_CHR_VIEW_MODE:
        return "CHR_VIEW_MODE";
    case MODE_MUSIC_MODE:
        return "MUSIC_MODE";
    case MODE_VOICE_MODE:
        return "VOICE_MODE";
    case MODE_MOVIE_VIEW_MODE:
        return "MOVIE_VIEW_MODE";
    case MODE_HAMSTER_NAME:
        return "HAMSTER_NAME";
    case MODE_NINTENDO_WFC_SETUP:
        return "NINTENDO_WFC_SETUP";
    case MODE_WIFI_DOWNLOAD_PUZZLE:
        return "WIFI_DOWNLOAD_PUZZLE";
    case MODE_PASSCODE:
        return "PASSCODE";
    case MODE_CODE_INPUT_PANDORA:
        return "CODE_INPUT_PANDORA";
    case MODE_CODE_INPUT_FUTURE:
        return "CODE_INPUT_FUTURE";
    case MODE_DIARY:
        return "DIARY";
    case MODE_NAZOBA:
        return "NAZOBA";
    case MODE_INVALID:
        return "INVALID";
    }
}

typedef struct {
    screen_layer_t layer;
    bool (*is_done)(void *impl);
    bool valid;
} mode_handler_t;

#endif // MODE_H
