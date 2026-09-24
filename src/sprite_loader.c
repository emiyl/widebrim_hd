#include "sprite_loader.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SPRITE_PATH_MAX 4096

static char *sprite_strdup(const char *text) {
    size_t length;
    char *copy;

    if (text == NULL) {
        return NULL;
    }

    length = strlen(text) + 1U;
    copy = malloc(length);
    if (copy == NULL) {
        return NULL;
    }

    memcpy(copy, text, length);
    return copy;
}

static int16_t sprite_s16_from_bytes(const uint8_t *bytes, size_t offset) {
    return (int16_t)((uint16_t)bytes[offset] |
                     ((uint16_t)bytes[offset + 1U] << 8U));
}

static bool sprite_read_exact(FILE *file, void *buffer, size_t size) {
    if (file == NULL || buffer == NULL || size == 0U) {
        return false;
    }

    return fread(buffer, 1U, size, file) == size;
}

static uint32_t sprite_read_u32_le(FILE *file) {
    uint8_t bytes[4];

    if (!sprite_read_exact(file, bytes, sizeof(bytes))) {
        return 0U;
    }

    return ((uint32_t)bytes[0]) | ((uint32_t)bytes[1] << 8U) |
           ((uint32_t)bytes[2] << 16U) | ((uint32_t)bytes[3] << 24U);
}

static bool sprite_sheet_parse_file(FILE *file, sprite_sheet_t *sheet) {
    uint32_t frame_count;
    uint32_t index;
    long file_size;

    if (sheet == NULL || file == NULL) {
        return false;
    }

    if (fseek(file, 0L, SEEK_END) != 0) {
        fprintf(stderr, "widebrim: failed to seek sprite file to end\n");
        return false;
    }

    file_size = ftell(file);
    if (file_size < 0L) {
        fprintf(stderr, "widebrim: failed to determine sprite file size\n");
        return false;
    }

    if (fseek(file, 0L, SEEK_SET) != 0) {
        fprintf(stderr, "widebrim: failed to rewind sprite file\n");
        return false;
    }

    frame_count = sprite_read_u32_le(file);
    sheet->frames =
        calloc(frame_count == 0U ? 1U : frame_count, sizeof(*sheet->frames));
    if (sheet->frames == NULL && frame_count > 0U) {
        fprintf(stderr, "widebrim: failed to allocate %u sprite frames\n",
                frame_count);
        return false;
    }
    sheet->frame_count = (size_t)frame_count;

    for (index = 0U; index < frame_count; ++index) {
        uint8_t raw[8];

        if (!sprite_read_exact(file, raw, sizeof(raw))) {
            fprintf(stderr, "widebrim: failed to read sprite frame %u\n",
                    index);
            return false;
        }

        sheet->frames[index].x = sprite_s16_from_bytes(raw, 0U);
        sheet->frames[index].y = sprite_s16_from_bytes(raw, 2U);
        sheet->frames[index].width =
            (uint16_t)raw[4] | ((uint16_t)raw[5] << 8U);
        sheet->frames[index].height =
            (uint16_t)raw[6] | ((uint16_t)raw[7] << 8U);
    }

    /*
     * The real .spr files in this project store frame rectangles at the front
     * of the file. The optional animation metadata that follows is not
     * consistent across every asset, and the project only needs the frame
     * geometry for rendering. Keep the sprite-sheet parser tolerant: load the
     * frames we need, ignore the trailing metadata block if it is not present
     * or does not match the expected layout.
     */
    sheet->animation_count = 0U;
    sheet->animations = NULL;
    sheet->variable_count = 0U;
    sheet->variables = NULL;
    sheet->sub_anim_name[0] = '\0';

    (void)file_size;
    return true;
}

sprite_sheet_t *sprite_sheet_load(const char *path) {
    FILE *file;
    sprite_sheet_t *sheet;

    if (path == NULL) {
        fprintf(stderr, "widebrim: sprite_sheet_load called with NULL path\n");
        return NULL;
    }

    file = fopen(path, "rb");
    if (file == NULL) {
        fprintf(stderr, "widebrim: failed to open sprite file '%s': %s\n", path,
                strerror(errno));
        return NULL;
    }

    sheet = calloc(1U, sizeof(*sheet));
    if (sheet == NULL) {
        fprintf(stderr, "widebrim: failed to allocate sprite_sheet_t\n");
        fclose(file);
        return NULL;
    }

    sheet->path = sprite_strdup(path);
    if (sheet->path == NULL) {
        fprintf(stderr, "widebrim: failed to duplicate sprite path\n");
        free(sheet);
        fclose(file);
        return NULL;
    }

    if (!sprite_sheet_parse_file(file, sheet)) {
        sprite_sheet_free(sheet);
        fclose(file);
        return NULL;
    }

    fclose(file);
    return sheet;
}

bool sprite_sheet_load_from_assets(const char *assets_root, language_t language,
                                   const char *rel_path,
                                   sprite_sheet_t **out_sheet) {
    char normalized[SPRITE_PATH_MAX];
    char path[SPRITE_PATH_MAX];
    const char *suffix;
    sprite_sheet_t *sheet;

    if (assets_root == NULL || rel_path == NULL || out_sheet == NULL) {
        return false;
    }

    if (strncmp(rel_path, "ani/", 4U) == 0 ||
        strstr(rel_path, "/ani/") != NULL) {
        snprintf(normalized, sizeof(normalized), "%s", rel_path);
    } else {
        snprintf(normalized, sizeof(normalized), "ani/%s", rel_path);
    }

    suffix = strrchr(normalized, '.');
    if (suffix != NULL && strcmp(suffix, ".spr") != 0 &&
        strcmp(suffix, ".png") != 0) {
        suffix = NULL;
    }

    if (!asset_path_resolve(assets_root, language, normalized, path,
                            sizeof(path))) {
        return false;
    }

    if (suffix != NULL && strcmp(suffix, ".png") == 0) {
        char *resolved_suffix = strrchr(path, '.');
        if (resolved_suffix != NULL && strcmp(resolved_suffix, ".png") == 0) {
            strcpy(resolved_suffix, ".spr");
        }
    }

    sheet = sprite_sheet_load(path);
    if (sheet == NULL) {
        return false;
    }

    *out_sheet = sheet;
    return true;
}

bool sprite_loader_load(game_state_t *state, const char *rel_path,
                        sprite_sheet_t **out_sheet) {
    char full_path[SPRITE_PATH_MAX];

    if (!state) {
        fprintf(
            stderr,
            "widebrim: sprite_loader_load called with NULL state pointer\n");
        return false;
    }
    if (!state->assets_root) {
        fprintf(stderr, "widebrim: sprite_loader_load called with NULL "
                        "assets_root in state\n");
        return false;
    }
    if (!rel_path) {
        fprintf(
            stderr,
            "widebrim: sprite_loader_load called with NULL rel_path pointer\n");
        return false;
    }
    if (!out_sheet) {
        fprintf(stderr, "widebrim: sprite_loader_load called with NULL "
                        "out_sheet pointer\n");
        return false;
    }

    if (!asset_path_resolve(state->assets_root, state->language, rel_path,
                            full_path, sizeof(full_path))) {
        fprintf(stderr,
                "widebrim: Failed to resolve path for sprite asset '%s'\n",
                rel_path);
        return false;
    }

    if (strncmp(rel_path, "ani/", 4U) == 0 ||
        strstr(rel_path, "/ani/") != NULL || strstr(rel_path, ".spr") != NULL ||
        strstr(rel_path, ".png") != NULL) {
        *out_sheet = sprite_sheet_load(full_path);
        return *out_sheet != NULL;
    }

    return sprite_sheet_load_from_assets(state->assets_root, state->language,
                                         rel_path, out_sheet);
}

bool sprite_sheet_extract_frame_rgba(const sprite_sheet_t *sheet,
                                     const texture_data_t *spritesheet,
                                     size_t frame_index, uint8_t **out_rgba,
                                     int *out_width, int *out_height) {
    const sprite_frame_t *frame;
    uint8_t *rgba;
    size_t x;
    size_t y;

    if (!sheet || !spritesheet || !out_rgba || !out_width || !out_height) {
        return false;
    }
    if (frame_index >= sheet->frame_count) {
        return false;
    }

    frame = &sheet->frames[frame_index];
    if (frame->width == 0U || frame->height == 0U) {
        return false;
    }

    rgba = (uint8_t *)calloc((size_t)frame->width * (size_t)frame->height * 4U,
                             sizeof(uint8_t));
    if (!rgba) {
        return false;
    }

    for (y = 0U; y < frame->height; ++y) {
        int src_y = (int)frame->y + (int)y;
        if (src_y < 0 || src_y >= spritesheet->height) {
            continue;
        }

        for (x = 0U; x < frame->width; ++x) {
            int src_x = (int)frame->x + (int)x;
            int dst_index = (int)(y * frame->width + x) * 4;
            int src_index;

            if (src_x < 0 || src_x >= spritesheet->width) {
                continue;
            }

            src_index = (src_y * spritesheet->width + src_x) * 4;
            rgba[dst_index + 0U] = spritesheet->pixels[src_index + 0U];
            rgba[dst_index + 1U] = spritesheet->pixels[src_index + 1U];
            rgba[dst_index + 2U] = spritesheet->pixels[src_index + 2U];
            rgba[dst_index + 3U] = spritesheet->pixels[src_index + 3U];
        }
    }

    *out_rgba = rgba;
    *out_width = (int)frame->width;
    *out_height = (int)frame->height;
    return true;
}

bool sprite_loader_load_frame_rgba(game_state_t *state, const char *rel_path,
                                   size_t frame_index, uint8_t **out_rgba,
                                   int *out_width, int *out_height) {
    char spritesheet_path[SPRITE_PATH_MAX];
    char *suffix;
    texture_data_t *spritesheet = NULL;
    sprite_sheet_t *sheet = NULL;
    bool ok = false;

    if (!state || !state->assets_root || !rel_path || !out_rgba || !out_width ||
        !out_height) {
        return false;
    }

    if (!asset_path_resolve(state->assets_root, state->language, rel_path,
                            spritesheet_path, sizeof(spritesheet_path))) {
        return false;
    }

    suffix = strrchr(spritesheet_path, '.');
    if (suffix != NULL && strcmp(suffix, ".spr") == 0) {
        size_t prefix_len = (size_t)(suffix - spritesheet_path);
        if (prefix_len + strlen(".png") + 1U < sizeof(spritesheet_path)) {
            snprintf(spritesheet_path + prefix_len,
                     sizeof(spritesheet_path) - prefix_len, ".png");
        }
    }

    spritesheet = texture_load_rgba(spritesheet_path);
    if (!spritesheet) {
        return false;
    }

    if (!sprite_loader_load(state, rel_path, &sheet)) {
        texture_free(spritesheet);
        return false;
    }

    ok = sprite_sheet_extract_frame_rgba(sheet, spritesheet, frame_index,
                                         out_rgba, out_width, out_height);

    sprite_sheet_free(sheet);
    texture_free(spritesheet);
    return ok;
}

bool sprite_loader_load_animation_rgba(game_state_t *state,
                                       const char *rel_path,
                                       uint8_t ***out_frames,
                                       size_t *out_frame_count, int *out_width,
                                       int *out_height) {
    char spritesheet_path[SPRITE_PATH_MAX];
    char *suffix;
    texture_data_t *spritesheet = NULL;
    sprite_sheet_t *sheet = NULL;
    uint8_t **frames = NULL;
    size_t frame_count = 0U;
    int width = 0;
    int height = 0;
    bool ok = false;
    size_t index;

    if (!state || !state->assets_root || !rel_path || !out_frames ||
        !out_frame_count || !out_width || !out_height) {
        return false;
    }

    if (!asset_path_resolve(state->assets_root, state->language, rel_path,
                            spritesheet_path, sizeof(spritesheet_path))) {
        return false;
    }

    suffix = strrchr(spritesheet_path, '.');
    if (suffix != NULL && strcmp(suffix, ".spr") == 0) {
        size_t prefix_len = (size_t)(suffix - spritesheet_path);
        if (prefix_len + strlen(".png") + 1U < sizeof(spritesheet_path)) {
            snprintf(spritesheet_path + prefix_len,
                     sizeof(spritesheet_path) - prefix_len, ".png");
        }
    }

    spritesheet = texture_load_rgba(spritesheet_path);
    if (!spritesheet) {
        return false;
    }

    if (!sprite_loader_load(state, rel_path, &sheet)) {
        texture_free(spritesheet);
        return false;
    }

    frame_count = sheet->frame_count;
    if (frame_count == 0U) {
        sprite_sheet_free(sheet);
        texture_free(spritesheet);
        return false;
    }

    frames = calloc(frame_count, sizeof(*frames));
    if (!frames) {
        sprite_sheet_free(sheet);
        texture_free(spritesheet);
        return false;
    }

    for (index = 0U; index < frame_count; ++index) {
        int frame_w = 0;
        int frame_h = 0;

        if (!sprite_sheet_extract_frame_rgba(sheet, spritesheet, index,
                                             &frames[index], &frame_w,
                                             &frame_h)) {
            goto cleanup;
        }

        if (index == 0U) {
            width = frame_w;
            height = frame_h;
        } else if (frame_w != width || frame_h != height) {
            goto cleanup;
        }
    }

    ok = true;
    *out_frames = frames;
    *out_frame_count = frame_count;
    *out_width = width;
    *out_height = height;

cleanup:
    if (!ok) {
        for (index = 0U; index < frame_count; ++index) {
            free(frames[index]);
            frames[index] = NULL;
        }
        free(frames);
        frames = NULL;
    }

    sprite_sheet_free(sheet);
    texture_free(spritesheet);
    return ok;
}

void sprite_sheet_free(sprite_sheet_t *sheet) {
    size_t index;

    if (sheet == NULL) {
        return;
    }

    if (sheet->frames != NULL) {
        free(sheet->frames);
        sheet->frames = NULL;
    }

    if (sheet->animations != NULL) {
        for (index = 0U; index < sheet->animation_count; ++index) {
            if (sheet->animations[index].frames != NULL) {
                free(sheet->animations[index].frames);
            }
        }
        free(sheet->animations);
        sheet->animations = NULL;
    }

    if (sheet->variables != NULL) {
        free(sheet->variables);
        sheet->variables = NULL;
    }

    if (sheet->path != NULL) {
        free(sheet->path);
        sheet->path = NULL;
    }

    sheet->frame_count = 0U;
    sheet->animation_count = 0U;
    sheet->variable_count = 0U;
}
