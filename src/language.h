#ifndef LANGUAGE_H
#define LANGUAGE_H

#include <string.h>

typedef enum {
    LANGUAGE_EN = 0,
    LANGUAGE_DE,
    LANGUAGE_ES,
    LANGUAGE_FR,
    LANGUAGE_IT
} language_t;

static inline language_t language_string_as_enum(const char *language) {
    if (!language) {
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
    return LANGUAGE_EN;
}

#endif // LANGUAGE_H
