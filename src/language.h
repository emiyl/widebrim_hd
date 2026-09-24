#ifndef LANGUAGE_H
#define LANGUAGE_H

#include <string.h>

typedef enum {
    LANGUAGE_UNKNOWN = -1,
    LANGUAGE_EN,
    LANGUAGE_DE,
    LANGUAGE_ES,
    LANGUAGE_FR,
    LANGUAGE_IT
} language_t;

static inline language_t language_string_as_enum(const char *language) {
    if (!language) {
        return LANGUAGE_UNKNOWN;
    }
    if (strcmp(language, "en") == 0) {
        return LANGUAGE_EN;
    }
    if (strcmp(language, "de") == 0) {
        return LANGUAGE_DE;
    }
    if (strcmp(language, "es") == 0) {
        return LANGUAGE_ES;
    }
    if (strcmp(language, "fr") == 0) {
        return LANGUAGE_FR;
    }
    if (strcmp(language, "it") == 0) {
        return LANGUAGE_IT;
    }
    return LANGUAGE_UNKNOWN;
}

#endif // LANGUAGE_H
