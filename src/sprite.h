#ifndef SPRITE_H
#define SPRITE_H

#include "game_state.h"
#include "renderer.h"
#include "sprite_loader.h"

#include <stdbool.h>

struct object_t;
typedef struct sprite_t sprite_t;

struct sprite_t {
    renderer_texture_t *tex;
    renderer_texture_t **frames;
    char **frame_names;
    sprite_animation_t *animations;
    size_t animation_count;
    size_t active_animation_index;
    int x;
    int y;
    int width;
    int height;
    int *frame_widths;
    int *frame_heights;
    uint8_t alpha;
    size_t frame_count;
    size_t current_frame;
    float frame_duration_ms;
    float elapsed_ms;
    bool loop;
    bool playing;
};

void sprite_init(sprite_t *sprite);
void sprite_new(sprite_t *sprite, renderer_t *renderer, game_state_t *state,
                const char *sprite_path, float frame_duration_ms, bool loop);
void sprite_clear(sprite_t *sprite, renderer_t *renderer);
void sprite_set_alpha(sprite_t *sprite, uint8_t alpha);
void sprite_apply_alpha(sprite_t *sprite, renderer_t *renderer);
void sprite_update(sprite_t *sprite, float delta_ms);

bool sprite_set_playing(sprite_t *sprite, bool playing);
bool sprite_set_frame(sprite_t *sprite, size_t frame);
bool sprite_set_frame_by_name(sprite_t *sprite, const char *frame_name);

bool sprite_get_position(sprite_t *sprite, int *x, int *y);
bool sprite_set_position(sprite_t *sprite, int x, int y);
bool sprite_get_size(sprite_t *sprite, renderer_t *renderer, int *width,
                     int *height);
bool sprite_set_size(sprite_t *sprite, int width, int height);

bool sprite_take_object_position(sprite_t *sprite, const struct object_t *obj);

#endif // SPRITE_H
