#include "bg_loader.h"

#include <stdio.h>
#include <stdlib.h>

#include "language.h"
#include "texture_loader.h"

bool bg_loader_load(game_state_t *state, screen_controller_t *controller,
                    const char *rel_path,
                    void (*setter)(screen_controller_t *, const uint8_t *, int,
                                   int)) {
    if (!state) {
        fprintf(stderr,
                "widebrim: bg_loader_load called with NULL state pointer\n");
        return false;
    }
    if (!state->assets_root) {
        fprintf(
            stderr,
            "widebrim: bg_loader_load called with NULL assets_root in state\n");
        return false;
    }
    if (!controller) {
        fprintf(
            stderr,
            "widebrim: bg_loader_load called with NULL controller pointer\n");
        return false;
    }
    if (!rel_path) {
        fprintf(stderr,
                "widebrim: bg_loader_load called with NULL rel_path pointer\n");
        return false;
    }
    if (!setter) {
        fprintf(stderr,
                "widebrim: bg_loader_load called with NULL setter pointer\n");
        return false;
    }

    char full_path[1024];

    if (!asset_path_resolve(state->assets_root, state->language, rel_path,
                            full_path, sizeof(full_path))) {
        fprintf(stderr,
                "widebrim: Failed to resolve path for background image '%s'\n",
                rel_path);
        return false;
    }

    texture_data_t *texture = texture_load_rgba(full_path);

    if (texture == NULL) {
        return false;
    }

    setter(controller, texture->pixels, texture->width, texture->height);
    texture_free(texture);
    return true;
}
