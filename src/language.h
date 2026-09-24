#ifndef LANGUAGE_H
#define LANGUAGE_H

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

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

static inline const char *language_to_data_dir(language_t language) {
    switch (language) {
    case LANGUAGE_DE:
        return "data-de";
    case LANGUAGE_ES:
        return "data-es";
    case LANGUAGE_FR:
        return "data-fr";
    case LANGUAGE_IT:
        return "data-it";
    case LANGUAGE_EN:
    case LANGUAGE_UNKNOWN:
    default:
        return "data-en";
    }
}

static inline bool asset_path_resolve(const char *assets_root,
                                      language_t language, const char *rel_path,
                                      char *out_path, size_t out_path_size) {
    static const char *const dir_candidates[] = {"data", "data-EU"};
    char candidate[4096];
    const char *normalized = rel_path;
    const char *lang_dir = language_to_data_dir(language);
    size_t i;

    if (!assets_root || !rel_path || !out_path || out_path_size == 0U) {
        return false;
    }

    while (*normalized == '/') {
        ++normalized;
    }

    if (strncmp(normalized, "data/", 5U) == 0) {
        normalized += 5U;
    } else if (strncmp(normalized, "data-EU/", 8U) == 0) {
        normalized += 8U;
    } else if (strncmp(normalized, "data-", 5U) == 0) {
        const char *slash = strchr(normalized + 5U, '/');
        if (slash) {
            normalized = slash + 1;
        }
    }

    for (i = 0U; i < sizeof(dir_candidates) / sizeof(dir_candidates[0]); ++i) {
        int len = snprintf(candidate, sizeof(candidate), "%s/%s/%s",
                           assets_root, dir_candidates[i], normalized);
        if (len < 0 || (size_t)len >= sizeof(candidate)) {
            continue;
        }
        if (access(candidate, F_OK) == 0) {
            snprintf(out_path, out_path_size, "%s", candidate);
            return true;
        }
    }

    {
        int len = snprintf(candidate, sizeof(candidate), "%s/%s/%s",
                           assets_root, lang_dir, normalized);
        if (len >= 0 && (size_t)len < sizeof(candidate) &&
            access(candidate, F_OK) == 0) {
            snprintf(out_path, out_path_size, "%s", candidate);
            return true;
        }
    }

    {
        int len = snprintf(candidate, sizeof(candidate), "%s/data/%s",
                           assets_root, normalized);
        if (len >= 0 && (size_t)len < sizeof(candidate)) {
            snprintf(out_path, out_path_size, "%s", candidate);
        }
    }

    return false;
}

#endif // LANGUAGE_H
