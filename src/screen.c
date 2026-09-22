#include "screen.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void screen_collection_init(screen_collection_t *sc) {
    sc->layers = NULL;
    sc->count = 0;
    sc->capacity = 0;
}

void screen_collection_free(screen_collection_t *sc) {
    if (sc->layers) {
        for (size_t i = 0; i < sc->count; ++i) {
            if (sc->layers[i].destroy) {
                sc->layers[i].destroy(sc->layers[i].impl);
            }
        }
        free(sc->layers);
    }
    sc->layers = NULL;
    sc->count = 0;
    sc->capacity = 0;
}

int screen_collection_add(screen_collection_t *sc, screen_layer_t layer) {
    if (sc->count >= sc->capacity) {
        size_t new_capacity = sc->capacity == 0 ? 4 : sc->capacity * 2;
        screen_layer_t *new_layers =
            realloc(sc->layers, new_capacity * sizeof(screen_layer_t));
        if (!new_layers) {
            fprintf(stderr, "Failed to allocate memory for screen layers\n");
            return -1;
        }
        sc->layers = new_layers;
        sc->capacity = new_capacity;
    }
    sc->layers[sc->count++] = layer;
    return 0;
}

screen_layer_t screen_collection_remove_at(screen_collection_t *sc,
                                           size_t index) {
    if (index >= sc->count) {
        return (screen_layer_t){0};
    }
    screen_layer_t removed = sc->layers[index];
    memmove(&sc->layers[index], &sc->layers[index + 1],
            (sc->count - index - 1) * sizeof(screen_layer_t));
    --sc->count;
    return removed;
}

screen_layer_t screen_collection_pop(screen_collection_t *sc) {
    return screen_collection_remove_at(sc, sc->count - 1);
}

void screen_collection_update(screen_collection_t *sc, float dt_ms) {
    for (size_t i = 0; i < sc->count; ++i) {
        if (sc->layers[i].update) {
            sc->layers[i].update(sc->layers[i].impl, dt_ms);
        }
    }
}

void screen_collection_draw(screen_collection_t *sc, renderer_t *renderer) {
    for (size_t i = 0; i < sc->count; ++i) {
        if (sc->layers[i].draw) {
            sc->layers[i].draw(sc->layers[i].impl, renderer);
        }
    }
}

bool screen_collection_handle_event(screen_collection_t *sc,
                                    input_event_t event) {
    for (size_t i = 0; i < sc->count; ++i) {
        if (sc->layers[i].handle_event &&
            sc->layers[i].handle_event(sc->layers[i].impl, event)) {
            return true;
        }
    }
    return false;
}

void screen_collection_on_quit(screen_collection_t *sc) {
    for (size_t i = 0; i < sc->count; ++i) {
        if (sc->layers[i].on_quit) {
            sc->layers[i].on_quit(sc->layers[i].impl);
        }
    }
}
