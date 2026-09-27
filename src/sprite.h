#ifndef SPRITE_H
#define SPRITE_H

#include "game_state.h"
#include "renderer.h"

#include <stdbool.h>

typedef struct sprite_t sprite_t;

struct sprite_t {
    renderer_texture_t *tex;
    renderer_texture_t **frames;
    int width;
    int height;
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
void sprite_apply_alpha(sprite_t *sprite, renderer_t *renderer);
void sprite_update(sprite_t *sprite, float delta_ms);
void sprite_get_size(sprite_t *sprite, renderer_t *renderer, int *width,
                     int *height);

bool sprite_set_playing(sprite_t *sprite, bool playing);
bool sprite_set_frame(sprite_t *sprite, size_t frame);

#endif // SPRITE_H
