#include "text_loader.h"

#include <stdio.h>
#include <string.h>

bool text_loader_load_path(game_state_t *state, const char *rel_path,
                           char *out_text, size_t out_text_size) {
    char resolved_path[4096];
    FILE *file = NULL;
    size_t bytes_read = 0U;

    if (!state || !rel_path || !out_text || out_text_size == 0U) {
        return false;
    }

    if (!asset_path_resolve(state->assets_root, state->language, rel_path,
                            resolved_path, sizeof(resolved_path))) {
        fprintf(stderr, "widebrim: failed to resolve text asset: %s\n",
                rel_path);
        return false;
    }

    file = fopen(resolved_path, "rb");
    if (!file) {
        fprintf(stderr, "widebrim: failed to open text asset: %s\n",
                resolved_path);
        return false;
    }

    bytes_read = fread(out_text, 1U, out_text_size - 1U, file);
    fclose(file);
    out_text[bytes_read] = '\0';

    for (size_t i = 0U; i < bytes_read; ++i) {
        if (out_text[i] == '\r') {
            out_text[i] = ' ';
        }
    }

    return true;
}

bool text_loader_load_room_text(game_state_t *state, int32_t text_id,
                                char *out_text, size_t out_text_size) {
    char room_text_path[256];

    if (!state || !out_text || out_text_size == 0U) {
        return false;
    }

    snprintf(room_text_path, sizeof(room_text_path), "room/tobj/t_%d.txt",
             text_id);
    return text_loader_load_path(state, room_text_path, out_text,
                                 out_text_size);
}
