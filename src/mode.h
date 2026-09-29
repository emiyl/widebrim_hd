#ifndef MODE_H
#define MODE_H

#include <stdbool.h>
#include <string.h>

#include "screen.h"

// value, name, display string
#define GAME_MODES(X)                                                          \
    X(0, MODE_RESET, "Reset")                                                  \
    X(1, MODE_QUESTION, "Question")                                            \
    X(2, MODE_TITLE, "Title")                                                  \
    X(3, MODE_ROOM, "Room")                                                    \
    X(5, MODE_EVENT, "Event")                                                  \
    X(7, MODE_MOVIE, "Movie")                                                  \
    X(8, MODE_JIGSAW, "Jigsaw")                                                \
    X(9, MODE_HOTEL_ROOM, "Hotel Room")                                        \
    X(10, MODE_ROBOT_DOG, "Robot Dog")                                         \
    X(11, MODE_TAIKEN, "Taiken")                                               \
    X(27, MODE_CHALLENGE_MODE, "Challenge Mode")                               \
    X(33, MODE_EVENT_MOVIE, "Event Movie")                                     \
    X(37, MODE_STAFF_ROLL, "Staff Roll")                                       \
    X(41, MODE_NAZOBA_MODE, "Nazoba Mode")                                     \
    X(255, MODE_INVALID, "Invalid")

typedef enum {
#define X(value, name, str) name = value,
    GAME_MODES(X)
#undef X
} game_mode_t;

static inline const char *game_mode_to_string(game_mode_t mode) {
    switch (mode) {
#define X(value, name, str)                                                    \
    case name:                                                                 \
        return str;
        GAME_MODES(X)
#undef X
    }
    return "UNKNOWN";
}

static inline game_mode_t string_to_game_mode(const char *input) {

#ifdef _WIN32
#define GAME_MODE_STRICMP _stricmp
#else
#define GAME_MODE_STRICMP strcasecmp
#endif

#define X(value, name, str)                                                    \
    if (GAME_MODE_STRICMP(input, str) == 0)                                    \
        return name;

    GAME_MODES(X)

#undef X
#undef GAME_MODE_STRICMP

    return MODE_INVALID;
}

typedef struct {
    screen_layer_t layer;
    bool (*is_done)(void *impl);
    bool valid;
} mode_handler_t;

#endif // MODE_H
