#ifndef SPRITE_LAYER_H
#define SPRITE_LAYER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "renderer.h"
#include "screen.h"

typedef struct object_instance_t object_instance_t;
typedef object_instance_t sprite_instance_t;

typedef bool (*object_event_callback_t)(void *user, const input_event_t *event,
                                        object_instance_t *object);

struct object_instance_t {
    renderer_t *renderer;
    renderer_texture_t *tex;
    renderer_texture_t **frames;
    size_t frame_count;
    size_t current_frame;
    float frame_duration_ms;
    float elapsed_ms;
    bool loop;
    bool playing;
    int x;
    int y;
    int z;
    int width;
    int height;
    uint8_t alpha;
    uint8_t base_alpha;
    float fade_duration_ms;
    float fade_elapsed_ms;
    bool fading_in;
    bool fading_out;
    bool interactive;
    bool visible;
    void *user;
    object_event_callback_t on_event;
};

static inline bool object_instance_has_sprite(const object_instance_t *object) {
    return object != NULL &&
           (object->tex != NULL ||
            (object->frames != NULL && object->frame_count > 0U));
}

static inline bool
object_instance_has_animation(const object_instance_t *object) {
    return object != NULL && object->frames != NULL && object->frame_count > 1U;
}

typedef struct {
    renderer_t *renderer;
    sprite_instance_t *sprites;
    size_t count;
    size_t capacity;
} object_layer_t;

typedef object_layer_t sprite_layer_t;

void object_layer_init(object_layer_t *layer, renderer_t *renderer);
void object_layer_destroy(object_layer_t *layer);
void object_layer_clear(object_layer_t *layer);
void object_layer_update(object_layer_t *layer, float delta_ms);

void sprite_layer_init(sprite_layer_t *layer, renderer_t *renderer);
void sprite_layer_destroy(sprite_layer_t *layer);
void sprite_layer_clear(sprite_layer_t *layer);
void sprite_layer_update(sprite_layer_t *layer, float delta_ms);

sprite_instance_t *object_layer_add_rgba(object_layer_t *layer,
                                         const uint8_t *rgba, int width,
                                         int height, int x, int y,
                                         uint8_t alpha);
sprite_instance_t *object_layer_add_rgba_z(object_layer_t *layer,
                                           const uint8_t *rgba, int width,
                                           int height, int x, int y, int z,
                                           uint8_t alpha);
sprite_instance_t *
object_layer_add_animation(object_layer_t *layer, const uint8_t *const *frames,
                           size_t frame_count, int width, int height, int x,
                           int y, int z, uint8_t alpha, float frame_duration_ms,
                           bool loop);

sprite_instance_t *sprite_layer_add_rgba(sprite_layer_t *layer,
                                         const uint8_t *rgba, int width,
                                         int height, int x, int y,
                                         uint8_t alpha);
sprite_instance_t *sprite_layer_add_rgba_z(sprite_layer_t *layer,
                                           const uint8_t *rgba, int width,
                                           int height, int x, int y, int z,
                                           uint8_t alpha);
sprite_instance_t *
sprite_layer_add_animation(sprite_layer_t *layer, const uint8_t *const *frames,
                           size_t frame_count, int width, int height, int x,
                           int y, int z, uint8_t alpha, float frame_duration_ms,
                           bool loop);

bool object_layer_get_sprite_size(sprite_instance_t *sprite, int *width,
                                  int *height);
bool object_layer_set_sprite_position(sprite_instance_t *sprite, int x, int y);
bool object_layer_center_sprite(sprite_instance_t *sprite, int area_width,
                                int area_height);
bool object_layer_set_frame(sprite_instance_t *sprite, size_t frame_index);
bool object_layer_set_playing(sprite_instance_t *sprite, bool playing);
bool object_layer_set_interactive(object_instance_t *object, bool interactive,
                                  object_event_callback_t on_event, void *user);
bool object_layer_set_visible(sprite_instance_t *sprite, bool visible);
bool object_layer_fade_in(sprite_instance_t *sprite, float duration_ms);
bool object_layer_fade_out(sprite_instance_t *sprite, float duration_ms);
bool object_layer_contains_point(sprite_instance_t *sprite, int x, int y);
bool object_layer_handle_event(object_layer_t *layer,
                               const input_event_t *event);

bool sprite_layer_get_sprite_size(sprite_instance_t *sprite, int *width,
                                  int *height);
bool sprite_layer_set_sprite_position(sprite_instance_t *sprite, int x, int y);
bool sprite_layer_center_sprite(sprite_instance_t *sprite, int area_width,
                                int area_height);
bool sprite_layer_set_frame(sprite_instance_t *sprite, size_t frame_index);
bool sprite_layer_set_playing(sprite_instance_t *sprite, bool playing);
bool sprite_layer_set_interactive(sprite_instance_t *sprite, bool interactive,
                                  object_event_callback_t on_event, void *user);
bool sprite_layer_set_visible(sprite_instance_t *sprite, bool visible);
bool sprite_layer_fade_in(sprite_instance_t *sprite, float duration_ms);
bool sprite_layer_fade_out(sprite_instance_t *sprite, float duration_ms);
bool sprite_layer_contains_point(sprite_instance_t *sprite, int x, int y);
bool sprite_layer_handle_event(sprite_layer_t *layer,
                               const input_event_t *event);

screen_layer_t object_layer_as_screen_layer(object_layer_t *layer);
screen_layer_t sprite_layer_as_screen_layer(sprite_layer_t *layer);

#endif // SPRITE_LAYER_H
