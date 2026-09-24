#include "sprite_layer.h"

#include <stdlib.h>

static void sprite_instance_clear(sprite_layer_t *layer,
                                  sprite_instance_t *instance) {
    size_t i;

    if (!layer || !instance) {
        return;
    }

    if (instance->frames) {
        for (i = 0U; i < instance->frame_count; ++i) {
            if (instance->frames[i]) {
                renderer_destroy_texture(layer->renderer, instance->frames[i]);
                instance->frames[i] = NULL;
            }
        }
        free(instance->frames);
        instance->frames = NULL;
        instance->tex = NULL;
    } else if (instance->tex) {
        renderer_destroy_texture(layer->renderer, instance->tex);
        instance->tex = NULL;
    }

    instance->x = 0;
    instance->y = 0;
    instance->z = 0;
    instance->width = 0;
    instance->height = 0;
    instance->alpha = 255;
    instance->interactive = false;
    instance->user = NULL;
    instance->on_event = NULL;
    instance->frame_count = 0U;
    instance->current_frame = 0U;
    instance->frame_duration_ms = 0.0f;
    instance->elapsed_ms = 0.0f;
    instance->loop = true;
    instance->playing = false;
}

static bool sprite_layer_ensure_capacity(sprite_layer_t *layer,
                                         size_t required) {
    sprite_instance_t *next;
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

    next = (sprite_instance_t *)realloc(layer->sprites,
                                        new_capacity * sizeof(*next));
    if (!next) {
        return false;
    }

    layer->sprites = next;
    layer->capacity = new_capacity;
    return true;
}

void sprite_layer_init(sprite_layer_t *layer, renderer_t *renderer) {
    if (!layer) {
        return;
    }

    layer->renderer = renderer;
    layer->sprites = NULL;
    layer->count = 0U;
    layer->capacity = 0U;
}

void sprite_layer_clear(sprite_layer_t *layer) {
    size_t i;

    if (!layer) {
        return;
    }

    if (!layer->sprites) {
        layer->count = 0U;
        return;
    }

    for (i = 0U; i < layer->count; ++i) {
        sprite_instance_clear(layer, &layer->sprites[i]);
    }

    layer->count = 0U;
}

void sprite_layer_destroy(sprite_layer_t *layer) {
    if (!layer) {
        return;
    }

    sprite_layer_clear(layer);
    free(layer->sprites);
    layer->sprites = NULL;
    layer->count = 0U;
    layer->capacity = 0U;
    layer->renderer = NULL;
}

sprite_instance_t *sprite_layer_add_rgba(sprite_layer_t *layer,
                                         const uint8_t *rgba, int width,
                                         int height, int x, int y,
                                         uint8_t alpha) {
    return sprite_layer_add_rgba_z(layer, rgba, width, height, x, y, 0, alpha);
}

sprite_instance_t *sprite_layer_add_rgba_z(sprite_layer_t *layer,
                                           const uint8_t *rgba, int width,
                                           int height, int x, int y, int z,
                                           uint8_t alpha) {
    sprite_instance_t *instance;
    renderer_texture_t *tex;

    if (!layer || !layer->renderer || !rgba || width <= 0 || height <= 0) {
        return NULL;
    }

    if (!sprite_layer_ensure_capacity(layer, layer->count + 1U)) {
        return NULL;
    }

    tex =
        renderer_create_texture_from_rgba(layer->renderer, rgba, width, height);
    if (!tex) {
        return NULL;
    }

    if (alpha != 255U) {
        renderer_set_texture_alpha(layer->renderer, tex, alpha);
    }

    instance = &layer->sprites[layer->count];
    instance->tex = tex;
    instance->frames = NULL;
    instance->frame_count = 0U;
    instance->current_frame = 0U;
    instance->frame_duration_ms = 0.0f;
    instance->elapsed_ms = 0.0f;
    instance->loop = true;
    instance->playing = false;
    instance->x = x;
    instance->y = y;
    instance->z = z;
    instance->width = width;
    instance->height = height;
    instance->alpha = alpha;
    layer->count += 1U;
    return instance;
}

sprite_instance_t *
sprite_layer_add_animation(sprite_layer_t *layer, const uint8_t *const *frames,
                           size_t frame_count, int width, int height, int x,
                           int y, int z, uint8_t alpha, float frame_duration_ms,
                           bool loop) {
    sprite_instance_t *instance;
    size_t i;

    if (!layer || !layer->renderer || !frames || frame_count == 0U ||
        width <= 0 || height <= 0) {
        return NULL;
    }

    if (!sprite_layer_ensure_capacity(layer, layer->count + 1U)) {
        return NULL;
    }

    instance = &layer->sprites[layer->count];
    instance->frames =
        (renderer_texture_t **)calloc(frame_count, sizeof(*instance->frames));
    if (!instance->frames) {
        return NULL;
    }

    for (i = 0U; i < frame_count; ++i) {
        instance->frames[i] = renderer_create_texture_from_rgba(
            layer->renderer, frames[i], width, height);
        if (!instance->frames[i]) {
            sprite_instance_clear(layer, instance);
            return NULL;
        }
        if (alpha != 255U) {
            renderer_set_texture_alpha(layer->renderer, instance->frames[i],
                                       alpha);
        }
    }

    instance->tex = instance->frames[0];
    instance->frame_count = frame_count;
    instance->current_frame = 0U;
    instance->frame_duration_ms = frame_duration_ms;
    instance->elapsed_ms = 0.0f;
    instance->loop = loop;
    instance->playing = true;
    instance->x = x;
    instance->y = y;
    instance->z = z;
    instance->width = width;
    instance->height = height;
    instance->alpha = alpha;
    instance->interactive = false;
    instance->user = NULL;
    instance->on_event = NULL;
    layer->count += 1U;
    return instance;
}

bool sprite_layer_set_sprite_position(sprite_instance_t *sprite, int x, int y) {
    if (!sprite) {
        return false;
    }

    sprite->x = x;
    sprite->y = y;
    return true;
}

bool sprite_layer_center_sprite(sprite_instance_t *sprite, int area_width,
                                int area_height) {
    if (!sprite || area_width <= 0 || area_height <= 0) {
        return false;
    }

    sprite->x = (area_width - sprite->width) / 2;
    sprite->y = (area_height - sprite->height) / 2;
    return true;
}

bool sprite_layer_set_frame(sprite_instance_t *sprite, size_t frame_index) {
    if (!sprite || !sprite->frames || sprite->frame_count == 0U) {
        return false;
    }

    if (frame_index >= sprite->frame_count) {
        return false;
    }

    sprite->current_frame = frame_index;
    sprite->elapsed_ms = 0.0f;
    sprite->tex = sprite->frames[frame_index];
    return true;
}

bool sprite_layer_set_playing(sprite_instance_t *sprite, bool playing) {
    if (!sprite) {
        return false;
    }

    sprite->playing = playing;
    if (sprite->frame_count > 0U && sprite->frames &&
        sprite->current_frame < sprite->frame_count) {
        sprite->tex = sprite->frames[sprite->current_frame];
    }
    return true;
}

bool sprite_layer_set_interactive(sprite_instance_t *sprite, bool interactive,
                                  sprite_event_callback_t on_event,
                                  void *user) {
    if (!sprite) {
        return false;
    }

    sprite->interactive = interactive;
    sprite->user = user;
    sprite->on_event = on_event;
    return true;
}

bool sprite_layer_contains_point(sprite_instance_t *sprite, int x, int y) {
    if (!sprite) {
        return false;
    }

    return x >= sprite->x && x < sprite->x + sprite->width && y >= sprite->y &&
           y < sprite->y + sprite->height;
}

void sprite_layer_update(sprite_layer_t *layer, float delta_ms) {
    size_t i;

    if (!layer) {
        return;
    }

    for (i = 0U; i < layer->count; ++i) {
        sprite_instance_t *instance = &layer->sprites[i];
        if (!instance || !instance->playing || instance->frame_count < 2U ||
            instance->frame_duration_ms <= 0.0f) {
            continue;
        }

        instance->elapsed_ms += delta_ms;
        while (instance->elapsed_ms >= instance->frame_duration_ms) {
            instance->elapsed_ms -= instance->frame_duration_ms;
            if (instance->current_frame + 1U < instance->frame_count) {
                instance->current_frame += 1U;
            } else if (instance->loop) {
                instance->current_frame = 0U;
            } else {
                instance->playing = false;
                instance->current_frame = instance->frame_count - 1U;
                break;
            }
        }

        instance->tex = instance->frames[instance->current_frame];
    }
}

bool sprite_layer_handle_event(sprite_layer_t *layer,
                               const input_event_t *event) {
    size_t i;

    if (!layer || !event) {
        return false;
    }

    for (i = layer->count; i > 0U; --i) {
        sprite_instance_t *sprite = &layer->sprites[i - 1U];

        if (!sprite->interactive || !sprite->on_event ||
            !sprite_layer_contains_point(sprite, event->data.mouse_button.x,
                                         event->data.mouse_button.y)) {
            continue;
        }

        return sprite->on_event(sprite->user, event, sprite);
    }

    return false;
}

static void sprite_layer_update_impl(void *impl, float delta_ms) {
    sprite_layer_update((sprite_layer_t *)impl, delta_ms);
}

static void sprite_layer_draw(void *impl, renderer_t *renderer) {
    sprite_layer_t *layer = (sprite_layer_t *)impl;
    sprite_instance_t *sorted = NULL;
    size_t i;

    if (!layer || !renderer) {
        return;
    }

    if (layer->count == 0U) {
        return;
    }

    sorted = (sprite_instance_t *)malloc(layer->count * sizeof(*sorted));
    if (!sorted) {
        return;
    }

    for (i = 0U; i < layer->count; ++i) {
        sorted[i] = layer->sprites[i];
    }

    for (i = 1U; i < layer->count; ++i) {
        size_t j = i;
        while (j > 0U && sorted[j - 1U].z > sorted[j].z) {
            sprite_instance_t tmp = sorted[j - 1U];
            sorted[j - 1U] = sorted[j];
            sorted[j] = tmp;
            j -= 1U;
        }
    }

    for (i = 0U; i < layer->count; ++i) {
        sprite_instance_t *instance = &sorted[i];
        rect_t dst;

        if (!instance || !instance->tex ||
            !renderer_texture_exists(renderer, instance->tex)) {
            continue;
        }

        dst = (rect_t){.x = (float)instance->x,
                       .y = (float)instance->y,
                       .w = (float)instance->width,
                       .h = (float)instance->height};
        renderer_draw_texture(renderer, instance->tex, &dst);
    }

    free(sorted);
}

screen_layer_t sprite_layer_as_screen_layer(sprite_layer_t *layer) {
    screen_layer_t screen_layer;

    screen_layer.impl = (void *)layer;
    screen_layer.update = sprite_layer_update_impl;
    screen_layer.draw = sprite_layer_draw;
    screen_layer.handle_event =
        (bool (*)(void *, const input_event_t *))sprite_layer_handle_event;
    screen_layer.on_quit = NULL;
    screen_layer.destroy = NULL;
    return screen_layer;
}
