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

static inline bool asset_path_resolve_roots(const char *assets_root,
                                            const char *resource_pack_root,
                                            language_t language,
                                            const char *rel_path,
                                            char *out_path,
                                            size_t out_path_size) {
    static const char *const dir_candidates[] = {"data", "data-EU"};
    char candidate[4096];
    const char *normalized = rel_path;
    const char *lang_dir = language_to_data_dir(language);
    const char *roots[2];
    size_t root_count = 0U;

    if (!rel_path || !out_path || out_path_size == 0U) {
        return false;
    }

    if (resource_pack_root && resource_pack_root[0] != '\0') {
        roots[root_count++] = resource_pack_root;
    }
    if (assets_root && assets_root[0] != '\0') {
        roots[root_count++] = assets_root;
    }
    if (root_count == 0U) {
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

    for (size_t i = 0U; i < root_count; ++i) {
        const char *root = roots[i];
        char stem[4096];
        const char *suffix = strrchr(normalized, '.');
        size_t stem_len = strlen(normalized);
        char candidate_paths[5][4096];
        size_t candidate_count = 0U;

        if (suffix != NULL &&
            (strcmp(suffix, ".png") == 0 || strcmp(suffix, ".jpg") == 0 ||
             strcmp(suffix, ".jpeg") == 0 || strcmp(suffix, ".bgx") == 0)) {
            stem_len = (size_t)(suffix - normalized);
        }

        if (stem_len >= sizeof(stem)) {
            stem_len = sizeof(stem) - 1U;
        }
        memcpy(stem, normalized, stem_len);
        stem[stem_len] = '\0';

        static const char *const extension_variants[] = {".jpg", ".jpeg",
                                                         ".png", ".bgx"};
        for (size_t v = 0U;
             v < sizeof(extension_variants) / sizeof(extension_variants[0]);
             ++v) {
            int len =
                snprintf(candidate_paths[candidate_count],
                         sizeof(candidate_paths[candidate_count]), "%.*s%s",
                         (int)stem_len, stem, extension_variants[v]);
            if (len >= 0 &&
                (size_t)len < sizeof(candidate_paths[candidate_count])) {
                ++candidate_count;
            }
        }

        if (candidate_count == 0U ||
            strcmp(candidate_paths[candidate_count - 1], normalized) != 0) {
            int len = snprintf(candidate_paths[candidate_count],
                               sizeof(candidate_paths[candidate_count]), "%s",
                               normalized);
            if (len >= 0 &&
                (size_t)len < sizeof(candidate_paths[candidate_count])) {
                ++candidate_count;
            }
        }

        for (size_t j = 0U;
             j < sizeof(dir_candidates) / sizeof(dir_candidates[0]); ++j) {
            for (size_t k = 0U; k < candidate_count; ++k) {
                int len = snprintf(candidate, sizeof(candidate), "%s/%s/%s",
                                   root, dir_candidates[j], candidate_paths[k]);
                if (len < 0 || (size_t)len >= sizeof(candidate)) {
                    continue;
                }
                if (access(candidate, F_OK) == 0) {
                    snprintf(out_path, out_path_size, "%s", candidate);
                    return true;
                }
            }
        }

        for (size_t k = 0U; k < candidate_count; ++k) {
            int len = snprintf(candidate, sizeof(candidate), "%s/%s/%s", root,
                               lang_dir, candidate_paths[k]);
            if (len >= 0 && (size_t)len < sizeof(candidate) &&
                access(candidate, F_OK) == 0) {
                snprintf(out_path, out_path_size, "%s", candidate);
                return true;
            }
        }

        for (size_t k = 0U; k < candidate_count; ++k) {
            int len = snprintf(candidate, sizeof(candidate), "%s/data/%s", root,
                               candidate_paths[k]);
            if (len >= 0 && (size_t)len < sizeof(candidate) &&
                access(candidate, F_OK) == 0) {
                snprintf(out_path, out_path_size, "%s", candidate);
                return true;
            }
        }
    }

    return false;
}

#endif // LANGUAGE_H
