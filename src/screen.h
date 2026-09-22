#ifndef SCREEN_H
#define SCREEN_H

#include <stdbool.h>
#include <stddef.h>

#include "input.h"
#include "renderer.h"

typedef struct {
    void *impl;
    void (*update)(void *impl, renderer_t *renderer);
    void (*draw)(void *impl, renderer_t *renderer);
    bool (*handle_event)(void *impl, input_event_t event);
    void (*on_quit)(void *impl);
    void (*destroy)(void *impl);
} screen_layer_t;

typedef struct {
    screen_layer_t *layers;
    size_t count;
    size_t capacity;
} screen_collection_t;

void screen_collection_init(screen_collection_t *sc);
void screen_collection_free(screen_collection_t *sc);
int screen_collection_add(screen_collection_t *sc, screen_layer_t layer);

screen_layer_t screen_collection_pop(screen_collection_t *sc);
screen_layer_t screen_collection_remove_at(screen_collection_t *sc,
                                           size_t index);

void screen_collection_update(screen_collection_t *sc, float dt_ms);
void screen_collection_draw(screen_collection_t *sc, renderer_t *renderer);
bool screen_collection_handle_event(screen_collection_t *sc,
                                    input_event_t event);
void screen_collection_on_quit(screen_collection_t *sc);

#endif // SCREEN_H
