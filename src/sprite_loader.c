#include "sprite_loader.h"

#include <ctype.h>
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

static uint32_t sprite_read_u32_le_at(const uint8_t *bytes, size_t offset) {
    return (uint32_t)bytes[offset] | ((uint32_t)bytes[offset + 1U] << 8U) |
           ((uint32_t)bytes[offset + 2U] << 16U) |
           ((uint32_t)bytes[offset + 3U] << 24U);
}

static bool sprite_sheet_parse_animation_data(const uint8_t *bytes,
                                              size_t length,
                                              sprite_sheet_t *sheet) {
    size_t offset = 0U;
    uint32_t animation_count = 0U;

    if (sheet == NULL || bytes == NULL || length == 0U) {
        return false;
    }

    if (length < 4U + sheet->frame_count * 8U) {
        return true;
    }

    offset = 4U + sheet->frame_count * 8U;
    if (offset + 30U + 4U > length) {
        return true;
    }

    offset += 30U;
    animation_count = sprite_read_u32_le_at(bytes, offset);
    offset += 4U;

    if (animation_count == 0U) {
        sheet->animation_count = 0U;
        sheet->animations = NULL;
        return true;
    }

    sheet->animations = calloc(animation_count, sizeof(*sheet->animations));
    if (sheet->animations == NULL) {
        fprintf(stderr, "widebrim: Failed to allocate %u sprite animations\n",
                animation_count);
        return false;
    }
    sheet->animation_count = (size_t)animation_count;

    for (uint32_t anim_index = 0U; anim_index < animation_count; ++anim_index) {
        size_t name_len = 0U;

        if (offset + 30U > length) {
            return false;
        }

        while (name_len < 30U && bytes[offset + name_len] != '\0') {
            ++name_len;
        }
        if (name_len >= sizeof(sheet->animations[anim_index].name)) {
            name_len = sizeof(sheet->animations[anim_index].name) - 1U;
        }
        memcpy(sheet->animations[anim_index].name, bytes + offset, name_len);
        sheet->animations[anim_index].name[name_len] = '\0';
        offset += 30U;
    }

    for (uint32_t anim_index = 0U; anim_index < animation_count; ++anim_index) {
        uint32_t keyframe_count = 0U;
        uint32_t *ordering = NULL;
        uint32_t *durations = NULL;
        uint32_t *image_indices = NULL;
        size_t *ordered_indices = NULL;

        if (offset + 4U > length) {
            return false;
        }
        keyframe_count = sprite_read_u32_le_at(bytes, offset);
        offset += 4U;

        if (keyframe_count == 0U) {
            sheet->animations[anim_index].frames = NULL;
            sheet->animations[anim_index].frame_count = 0U;
            continue;
        }

        ordering = calloc(keyframe_count, sizeof(*ordering));
        durations = calloc(keyframe_count, sizeof(*durations));
        image_indices = calloc(keyframe_count, sizeof(*image_indices));
        ordered_indices = calloc(keyframe_count, sizeof(*ordered_indices));
        if (ordering == NULL || durations == NULL || image_indices == NULL ||
            ordered_indices == NULL) {
            free(ordering);
            free(durations);
            free(image_indices);
            free(ordered_indices);
            return false;
        }

        for (uint32_t frame_index = 0U; frame_index < keyframe_count;
             ++frame_index) {
            if (offset + 4U > length) {
                free(ordering);
                free(durations);
                free(image_indices);
                free(ordered_indices);
                return false;
            }
            ordering[frame_index] = sprite_read_u32_le_at(bytes, offset);
            offset += 4U;
        }
        for (uint32_t frame_index = 0U; frame_index < keyframe_count;
             ++frame_index) {
            if (offset + 4U > length) {
                free(ordering);
                free(durations);
                free(image_indices);
                free(ordered_indices);
                return false;
            }
            durations[frame_index] = sprite_read_u32_le_at(bytes, offset);
            offset += 4U;
        }
        for (uint32_t frame_index = 0U; frame_index < keyframe_count;
             ++frame_index) {
            if (offset + 4U > length) {
                free(ordering);
                free(durations);
                free(image_indices);
                free(ordered_indices);
                return false;
            }
            image_indices[frame_index] = sprite_read_u32_le_at(bytes, offset);
            offset += 4U;
        }

        for (size_t i = 0U; i < keyframe_count; ++i) {
            ordered_indices[i] = i;
        }
        for (size_t i = 1U; i < keyframe_count; ++i) {
            size_t j = i;
            while (j > 0U && ordering[ordered_indices[j - 1U]] >
                                 ordering[ordered_indices[j]]) {
                size_t tmp = ordered_indices[j - 1U];
                ordered_indices[j - 1U] = ordered_indices[j];
                ordered_indices[j] = tmp;
                --j;
            }
        }

        sheet->animations[anim_index].frames = calloc(
            keyframe_count, sizeof(*sheet->animations[anim_index].frames));
        sheet->animations[anim_index].frame_count = (size_t)keyframe_count;
        if (sheet->animations[anim_index].frames == NULL) {
            free(ordering);
            free(durations);
            free(image_indices);
            free(ordered_indices);
            return false;
        }

        for (size_t frame_index = 0U; frame_index < keyframe_count;
             ++frame_index) {
            size_t ordered_index = ordered_indices[frame_index];
            sheet->animations[anim_index].frames[frame_index].order =
                ordering[ordered_index];
            sheet->animations[anim_index].frames[frame_index].duration =
                durations[ordered_index];
            sheet->animations[anim_index].frames[frame_index].image_index =
                image_indices[ordered_index];
        }

        free(ordering);
        free(durations);
        free(image_indices);
        free(ordered_indices);
    }

    sheet->variable_count = 0U;
    sheet->variables = NULL;
    sheet->sub_anim_name[0] = '\0';
    return true;
}

static bool sprite_sheet_parse_file(FILE *file, sprite_sheet_t *sheet) {
    uint32_t frame_count;
    uint32_t index;
    long file_size;
    uint8_t *buffer = NULL;

    if (sheet == NULL || file == NULL) {
        return false;
    }

    if (fseek(file, 0L, SEEK_END) != 0) {
        fprintf(stderr, "widebrim: Failed to seek sprite file to end\n");
        return false;
    }

    file_size = ftell(file);
    if (file_size < 0L) {
        fprintf(stderr, "widebrim: Failed to determine sprite file size\n");
        return false;
    }

    if (fseek(file, 0L, SEEK_SET) != 0) {
        fprintf(stderr, "widebrim: Failed to rewind sprite file\n");
        return false;
    }

    frame_count = sprite_read_u32_le(file);
    sheet->frames =
        calloc(frame_count == 0U ? 1U : frame_count, sizeof(*sheet->frames));
    if (sheet->frames == NULL && frame_count > 0U) {
        fprintf(stderr, "widebrim: Failed to allocate %u sprite frames\n",
                frame_count);
        return false;
    }
    sheet->frame_count = (size_t)frame_count;

    for (index = 0U; index < frame_count; ++index) {
        uint8_t raw[8];

        if (!sprite_read_exact(file, raw, sizeof(raw))) {
            fprintf(stderr, "widebrim: Failed to read sprite frame %u\n",
                    index);
            free(buffer);
            return false;
        }

        sheet->frames[index].x = sprite_s16_from_bytes(raw, 0U);
        sheet->frames[index].y = sprite_s16_from_bytes(raw, 2U);
        sheet->frames[index].width =
            (uint16_t)raw[4] | ((uint16_t)raw[5] << 8U);
        sheet->frames[index].height =
            (uint16_t)raw[6] | ((uint16_t)raw[7] << 8U);
    }

    buffer = malloc((size_t)file_size);
    if (buffer == NULL) {
        fprintf(stderr, "widebrim: Failed to allocate sprite file buffer\n");
        return false;
    }

    if (fseek(file, 0L, SEEK_SET) != 0) {
        free(buffer);
        fprintf(stderr, "widebrim: Failed to rewind sprite file\n");
        return false;
    }

    if (fread(buffer, 1U, (size_t)file_size, file) != (size_t)file_size) {
        free(buffer);
        fprintf(stderr, "widebrim: Failed to read sprite file contents\n");
        return false;
    }

    if (!sprite_sheet_parse_animation_data(buffer, (size_t)file_size, sheet)) {
        free(buffer);
        return false;
    }

    free(buffer);
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
        fprintf(stderr, "widebrim: Failed to open sprite file '%s': %s\n", path,
                strerror(errno));
        return NULL;
    }

    sheet = calloc(1U, sizeof(*sheet));
    if (sheet == NULL) {
        fprintf(stderr, "widebrim: Failed to allocate sprite_sheet_t\n");
        fclose(file);
        return NULL;
    }

    sheet->path = sprite_strdup(path);
    if (sheet->path == NULL) {
        fprintf(stderr, "widebrim: Failed to duplicate sprite path\n");
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

bool sprite_sheet_load_from_assets(const char *assets_root,
                                   const char *resource_pack_root,
                                   language_t language, const char *rel_path,
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

    if (!asset_path_resolve_roots(assets_root, resource_pack_root, language,
                                  normalized, path, sizeof(path))) {
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

    if (!asset_path_resolve(state, rel_path, full_path, sizeof(full_path))) {
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

    return sprite_sheet_load_from_assets(state->assets_root,
                                         state->resource_pack_root,
                                         state->language, rel_path, out_sheet);
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

    if (!asset_path_resolve(state, rel_path, spritesheet_path,
                            sizeof(spritesheet_path))) {
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

static bool sprite_loader_parse_animation_names(const uint8_t *bytes,
                                                size_t length,
                                                char ***out_names,
                                                size_t *out_count) {
    static const char sprite_anim_marker[] = "Create an Animation";
    char **names = NULL;
    size_t count = 0U;
    size_t offset = 0U;
    const uint8_t *start = NULL;

    if (!bytes || !out_names || !out_count) {
        return false;
    }

    for (offset = 0U; offset + strlen(sprite_anim_marker) < length; ++offset) {
        if (memcmp(bytes + offset, sprite_anim_marker,
                   strlen(sprite_anim_marker)) == 0) {
            start = bytes + offset + strlen(sprite_anim_marker);
            break;
        }
    }

    if (!start) {
        return false;
    }

    offset = (size_t)(start - bytes);
    while (offset < length) {
        size_t name_len = 0U;
        char *name;

        while (offset < length && bytes[offset] == '\0') {
            ++offset;
        }

        if (offset >= length) {
            break;
        }

        if (!isprint((unsigned char)bytes[offset])) {
            break;
        }

        while (offset + name_len < length && bytes[offset + name_len] != '\0' &&
               isprint((unsigned char)bytes[offset + name_len])) {
            ++name_len;
        }

        if (name_len == 0U || name_len >= SPRITE_MAX_ANIM_NAME) {
            break;
        }

        name = malloc(name_len + 1U);
        if (!name) {
            return false;
        }

        memcpy(name, bytes + offset, name_len);
        name[name_len] = '\0';

        char **next_names = realloc(names, (count + 1U) * sizeof(*next_names));
        if (!next_names) {
            free(name);
            return false;
        }
        names = next_names;
        names[count++] = name;

        offset += name_len + 1U;
    }

    if (count == 0U) {
        free(names);
        return false;
    }

    *out_names = names;
    *out_count = count;
    return true;
}

bool sprite_loader_load_animation_names(game_state_t *state,
                                        const char *rel_path, char ***out_names,
                                        size_t *out_count) {
    char full_path[SPRITE_PATH_MAX];
    char *suffix;
    FILE *file;
    long size;
    uint8_t *buffer = NULL;
    bool ok = false;
    char **names = NULL;
    size_t count = 0U;

    if (!state || !state->assets_root || !rel_path || !out_names ||
        !out_count) {
        return false;
    }

    if (!asset_path_resolve(state, rel_path, full_path, sizeof(full_path))) {
        return false;
    }

    suffix = strrchr(full_path, '.');
    if (suffix != NULL && strcmp(suffix, ".png") == 0) {
        size_t prefix_len = (size_t)(suffix - full_path);
        if (prefix_len + strlen(".spr") + 1U < sizeof(full_path)) {
            snprintf(full_path + prefix_len, sizeof(full_path) - prefix_len,
                     ".spr");
        }
    }

    file = fopen(full_path, "rb");
    if (!file) {
        return false;
    }

    if (fseek(file, 0L, SEEK_END) != 0) {
        fclose(file);
        return false;
    }

    size = ftell(file);
    if (size < 0L) {
        fclose(file);
        return false;
    }

    if (fseek(file, 0L, SEEK_SET) != 0) {
        fclose(file);
        return false;
    }

    buffer = malloc((size_t)size);
    if (!buffer) {
        fclose(file);
        return false;
    }

    if (fread(buffer, 1U, (size_t)size, file) != (size_t)size) {
        free(buffer);
        fclose(file);
        return false;
    }

    if (sprite_loader_parse_animation_names(buffer, (size_t)size, &names,
                                            &count)) {
        ok = true;
        *out_names = names;
        *out_count = count;
    }

    free(buffer);
    fclose(file);
    return ok;
}

bool sprite_loader_load_animation_rgba(
    game_state_t *state, const char *rel_path, uint8_t ***out_frames,
    size_t *out_frame_count, int **out_frame_widths, int **out_frame_heights,
    int *out_width, int *out_height) {
    char spritesheet_path[SPRITE_PATH_MAX];
    char *suffix;
    texture_data_t *spritesheet = NULL;
    sprite_sheet_t *sheet = NULL;
    uint8_t **frames = NULL;
    int *frame_widths = NULL;
    int *frame_heights = NULL;
    size_t frame_count = 0U;
    int width = 0;
    int height = 0;
    bool ok = false;
    size_t index;

    if (!state || !state->assets_root || !rel_path || !out_frames ||
        !out_frame_count || !out_frame_widths || !out_frame_heights ||
        !out_width || !out_height) {
        fprintf(stderr, "widebrim: invalid arguments to "
                        "sprite_loader_load_animation_rgba\n");
        return false;
    }

    if (!asset_path_resolve(state, rel_path, spritesheet_path,
                            sizeof(spritesheet_path))) {
        fprintf(stderr, "widebrim: Failed to resolve asset path for %s\n",
                rel_path);
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
        fprintf(stderr, "widebrim: Failed to load spritesheet from %s\n",
                spritesheet_path);
        return false;
    }

    if (!sprite_loader_load(state, rel_path, &sheet)) {
        texture_free(spritesheet);
        fprintf(stderr, "widebrim: Failed to load sprite sheet for %s\n",
                rel_path);
        return false;
    }

    frame_count = sheet->frame_count;
    if (frame_count == 0U) {
        sprite_sheet_free(sheet);
        texture_free(spritesheet);
        fprintf(stderr, "widebrim: sprite sheet for %s has no frames\n",
                rel_path);
        return false;
    }

    frames = calloc(frame_count, sizeof(*frames));
    frame_widths = calloc(frame_count, sizeof(*frame_widths));
    frame_heights = calloc(frame_count, sizeof(*frame_heights));
    if (!frames || !frame_widths || !frame_heights) {
        free(frames);
        free(frame_widths);
        free(frame_heights);
        sprite_sheet_free(sheet);
        texture_free(spritesheet);
        fprintf(stderr,
                "widebrim: Failed to allocate memory for sprite frames\n");
        return false;
    }

    for (index = 0U; index < frame_count; ++index) {
        if (!sprite_sheet_extract_frame_rgba(
                sheet, spritesheet, index, &frames[index], &frame_widths[index],
                &frame_heights[index])) {
            goto cleanup;
        }
        if (index == 0U) {
            width = frame_widths[index];
            height = frame_heights[index];
        }
    }

    ok = true;
    *out_frames = frames;
    *out_frame_count = frame_count;
    *out_frame_widths = frame_widths;
    *out_frame_heights = frame_heights;
    *out_width = width;
    *out_height = height;

cleanup:
    if (!ok) {
        for (index = 0U; index < frame_count; ++index) {
            free(frames[index]);
            frames[index] = NULL;
        }
        free(frames);
        free(frame_widths);
        free(frame_heights);
        frames = NULL;
        frame_widths = NULL;
        frame_heights = NULL;
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
