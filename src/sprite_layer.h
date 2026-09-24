#ifndef SPRITE_LAYER_H
#define SPRITE_LAYER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "renderer.h"
#include "screen.h"

typedef struct sprite_instance_t sprite_instance_t;

typedef bool (*sprite_event_callback_t)(void *user, const input_event_t *event,
                                        sprite_instance_t *sprite);

struct sprite_instance_t {
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
    sprite_event_callback_t on_event;
};

typedef struct {
    renderer_t *renderer;
    sprite_instance_t *sprites;
    size_t count;
    size_t capacity;
} sprite_layer_t;

void sprite_layer_init(sprite_layer_t *layer, renderer_t *renderer);
void sprite_layer_destroy(sprite_layer_t *layer);
void sprite_layer_clear(sprite_layer_t *layer);
void sprite_layer_update(sprite_layer_t *layer, float delta_ms);

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
bool sprite_layer_set_sprite_position(sprite_instance_t *sprite, int x, int y);
bool sprite_layer_center_sprite(sprite_instance_t *sprite, int area_width,
                                int area_height);
bool sprite_layer_set_frame(sprite_instance_t *sprite, size_t frame_index);
bool sprite_layer_set_playing(sprite_instance_t *sprite, bool playing);
bool sprite_layer_set_interactive(sprite_instance_t *sprite, bool interactive,
                                  sprite_event_callback_t on_event, void *user);
bool sprite_layer_set_visible(sprite_instance_t *sprite, bool visible);
bool sprite_layer_fade_in(sprite_instance_t *sprite, float duration_ms);
bool sprite_layer_fade_out(sprite_instance_t *sprite, float duration_ms);
bool sprite_layer_contains_point(sprite_instance_t *sprite, int x, int y);
bool sprite_layer_handle_event(sprite_layer_t *layer,
                               const input_event_t *event);

screen_layer_t sprite_layer_as_screen_layer(sprite_layer_t *layer);

#endif // SPRITE_LAYER_H
