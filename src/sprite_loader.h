#ifndef SPRITE_LOADER_H
#define SPRITE_LOADER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "game_state.h"
#include "texture_loader.h"

#define SPRITE_MAX_ANIM_NAME 31U
#define SPRITE_MAX_VAR_NAME 17U
#define SPRITE_VAR_COUNT 16U
#define SPRITE_VAR_LENGTH 8U
#define SPRITE_SUBANIM_NAME_LEN 129U

typedef struct sprite_frame_t {
    int16_t x;
    int16_t y;
    uint16_t width;
    uint16_t height;
} sprite_frame_t;

typedef struct sprite_anim_frame_t {
    uint32_t order;
    uint32_t duration;
    uint32_t image_index;
} sprite_anim_frame_t;

typedef struct sprite_animation_t {
    char name[SPRITE_MAX_ANIM_NAME];
    sprite_anim_frame_t *frames;
    size_t frame_count;
    int16_t sub_x;
    int16_t sub_y;
    uint8_t sub_offset;
} sprite_animation_t;

typedef struct sprite_variable_t {
    char name[SPRITE_MAX_VAR_NAME];
    int16_t values[SPRITE_VAR_LENGTH];
} sprite_variable_t;

typedef struct sprite_sheet_t {
    char *path;
    sprite_frame_t *frames;
    size_t frame_count;
    sprite_animation_t *animations;
    size_t animation_count;
    sprite_variable_t *variables;
    size_t variable_count;
    char sub_anim_name[SPRITE_SUBANIM_NAME_LEN];
} sprite_sheet_t;

sprite_sheet_t *sprite_sheet_load(const char *path);
bool sprite_sheet_load_from_assets(const char *assets_root, language_t language,
                                   const char *rel_path,
                                   sprite_sheet_t **out_sheet);
bool sprite_loader_load(game_state_t *state, const char *rel_path,
                        sprite_sheet_t **out_sheet);
bool sprite_loader_load_frame_rgba(game_state_t *state, const char *rel_path,
                                   size_t frame_index, uint8_t **out_rgba,
                                   int *out_width, int *out_height);
bool sprite_loader_load_animation_rgba(game_state_t *state,
                                       const char *rel_path,
                                       uint8_t ***out_frames,
                                       size_t *out_frame_count, int *out_width,
                                       int *out_height);
bool sprite_sheet_extract_frame_rgba(const sprite_sheet_t *sheet,
                                     const texture_data_t *spritesheet,
                                     size_t frame_index, uint8_t **out_rgba,
                                     int *out_width, int *out_height);
void sprite_sheet_free(sprite_sheet_t *sheet);

#endif // SPRITE_LOADER_H
