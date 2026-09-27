#include "object_layer.h"

#include <stdlib.h>

static bool object_layer_ensure_capacity(object_layer_t *layer,
                                         size_t required) {
    object_t **next;
    size_t new_capacity;

    if (!layer) {
        return false;
    }

    if (layer->capacity >= required) {
        return true;
    }

    new_capacity = layer->capacity == 0U ? 8U : layer->capacity;
    while (new_capacity < required) {
        if (new_capacity > SIZE_MAX / 2U) {
            return false;
        }
        new_capacity *= 2U;
    }

    next = (object_t **)realloc(layer->objects, new_capacity * sizeof(*next));
    if (!next) {
        return false;
    }

    layer->objects = next;
    layer->capacity = new_capacity;
    return true;
}

void object_layer_init(object_layer_t *layer, renderer_t *renderer) {
    if (!layer) {
        return;
    }

    layer->renderer = renderer;
    layer->objects = NULL;
    layer->count = 0U;
    layer->capacity = 0U;
}

void object_layer_clear(object_layer_t *layer) {
    size_t i;

    if (!layer) {
        return;
    }

    if (!layer->objects) {
        layer->count = 0U;
        return;
    }

    for (i = 0U; i < layer->count; ++i) {
        if (layer->objects[i]) {
            object_clear(layer->objects[i], layer->renderer);
        }
    }

    layer->count = 0U;
}

void object_layer_destroy(object_layer_t *layer) {
    if (!layer) {
        return;
    }

    for (size_t i = 0U; i < layer->count; ++i) {
        if (layer->objects[i]) {
            object_clear(layer->objects[i], layer->renderer);
        }
    }

    free(layer->objects);
    layer->objects = NULL;
    layer->count = 0U;
    layer->capacity = 0U;
    layer->renderer = NULL;
}

void object_layer_add_object(object_layer_t *layer, object_t *object) {
    if (!layer) {
        fprintf(stderr, "widebrim: attempted to add object to NULL layer\n");
        return;
    }

    if (!object) {
        fprintf(stderr, "widebrim: attempted to add NULL object to layer\n");
        return;
    }

    if (!object_layer_ensure_capacity(layer, layer->count + 1U)) {
        fprintf(stderr,
                "widebrim: failed to ensure capacity for object layer\n");
        return;
    }

    layer->objects[layer->count++] = object;
}

void object_layer_update(object_layer_t *layer, float delta_ms) {
    size_t i;

    if (!layer) {
        return;
    }

    for (i = 0U; i < layer->count; ++i) {
        object_t *instance = layer->objects[i];
        if (!instance) {
            continue;
        }

        object_update(instance, layer->renderer, delta_ms);
    }
}

bool object_layer_handle_event(object_layer_t *layer,
                               const input_event_t *event) {
    size_t i;

    if (!layer || !event) {
        return false;
    }

    for (i = layer->count; i > 0U; --i) {
        object_t *object = layer->objects[i - 1U];
        if (object && object_handle_event(object, event)) {
            return true;
        }
    }

    return false;
}

static void object_layer_update_impl(void *impl, float delta_ms) {
    object_layer_update((object_layer_t *)impl, delta_ms);
}

static void object_layer_draw(void *impl, renderer_t *renderer) {
    object_layer_t *layer = (object_layer_t *)impl;
    object_t *sorted = NULL;
    size_t i;

    if (!layer || !renderer) {
        return;
    }

    if (layer->count == 0U) {
        return;
    }

    sorted = (object_t *)malloc(layer->count * sizeof(*sorted));
    if (!sorted) {
        return;
    }

    for (i = 0U; i < layer->count; ++i) {
        sorted[i] = *layer->objects[i];
    }

    for (i = 1U; i < layer->count; ++i) {
        size_t j = i;
        while (j > 0U && sorted[j - 1U].z > sorted[j].z) {
            object_t tmp = sorted[j - 1U];
            sorted[j - 1U] = sorted[j];
            sorted[j] = tmp;
            j -= 1U;
        }
    }

    for (i = 0U; i < layer->count; ++i) {
        object_t *instance = &sorted[i];
        object_draw(instance, renderer);
    }

    free(sorted);
}

screen_layer_t object_layer_as_screen_layer(object_layer_t *layer) {
    screen_layer_t screen_layer;

    screen_layer.impl = (void *)layer;
    screen_layer.update = object_layer_update_impl;
    screen_layer.draw = object_layer_draw;
    screen_layer.handle_event =
        (bool (*)(void *, const input_event_t *))object_layer_handle_event;
    screen_layer.on_quit = NULL;
    screen_layer.destroy = NULL;
    return screen_layer;
}
