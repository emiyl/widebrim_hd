#ifndef OBJECT_LAYER_H
#define OBJECT_LAYER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "object.h"
#include "renderer.h"
#include "screen.h"

typedef struct {
    renderer_t *renderer;
    object_t **objects;
    size_t count;
    size_t capacity;
} object_layer_t;

void object_layer_init(object_layer_t *layer, renderer_t *renderer);
void object_layer_destroy(object_layer_t *layer);
void object_layer_clear(object_layer_t *layer);
void object_layer_update(object_layer_t *layer, float delta_ms);
void object_layer_add_object(object_layer_t *layer, object_t *object);
bool object_layer_handle_event(object_layer_t *layer,
                               const input_event_t *event);

screen_layer_t object_layer_as_screen_layer(object_layer_t *layer);

#endif // OBJECT_LAYER_H
