#include "text_layer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint16_t text_layer_read_u16_le(const uint8_t *ptr) {
    return (uint16_t)((uint16_t)ptr[0] | ((uint16_t)ptr[1] << 8U));
}

static uint32_t text_layer_read_u32_le(const uint8_t *ptr) {
    return (uint32_t)ptr[0] | ((uint32_t)ptr[1] << 8U) |
           ((uint32_t)ptr[2] << 16U) | ((uint32_t)ptr[3] << 24U);
}

static void text_layer_clear_glyphs(text_layer_t *layer) {
    size_t i;

    if (!layer) {
        return;
    }

    for (i = 0U; i < layer->glyph_count; ++i) {
        if (layer->glyph_textures && layer->glyph_textures[i]) {
            renderer_destroy_texture(layer->renderer, layer->glyph_textures[i]);
            layer->glyph_textures[i] = NULL;
        }
    }

    free(layer->glyph_codes);
    free(layer->glyph_widths);
    free(layer->glyph_advances);
    free(layer->glyph_indices);
    free(layer->glyph_textures);

    layer->glyph_codes = NULL;
    layer->glyph_widths = NULL;
    layer->glyph_advances = NULL;
    layer->glyph_indices = NULL;
    layer->glyph_textures = NULL;
    layer->glyph_count = 0U;
    layer->cell_height = 0;
    layer->pad_x = 0;
}

static renderer_texture_t *
text_layer_build_glyph_texture(text_layer_t *layer, const uint8_t *atlas,
                               int atlas_width, int atlas_height, int offset_x,
                               int offset_y, int glyph_width, int glyph_height,
                               int pad_x) {
    uint8_t *rgba = NULL;
    renderer_texture_t *tex = NULL;
    int x;
    int y;

    (void)atlas_height;

    if (!layer || !atlas || glyph_width <= 0 || glyph_height <= 0) {
        return NULL;
    }

    rgba = (uint8_t *)calloc((size_t)glyph_width * (size_t)glyph_height * 4U,
                             sizeof(uint8_t));
    if (!rgba) {
        fprintf(stderr, "widebrim: failed to allocate glyph RGBA buffer\n");
        return NULL;
    }

    for (y = 0; y < glyph_height; ++y) {
        const int atlas_y = offset_y + y;
        for (x = 0; x < glyph_width; ++x) {
            const int atlas_x = offset_x + pad_x + x;
            const int src_index = atlas_y * atlas_width + atlas_x;
            const int dst_index = (y * glyph_width + x) * 4;
            const uint8_t pixel = atlas[src_index];
            if (pixel > 0U) {
                rgba[dst_index + 0U] = 0U;
                rgba[dst_index + 1U] = 0U;
                rgba[dst_index + 2U] = 0U;
                rgba[dst_index + 3U] = pixel;
            }
        }
    }

    tex = renderer_create_texture_from_rgba(layer->renderer, rgba, glyph_width,
                                            glyph_height);
    free(rgba);
    if (!tex) {
        fprintf(stderr, "widebrim: failed to create glyph texture\n");
    }
    return tex;
}

static int text_layer_utf8_decode(const uint8_t **cursor) {
    const uint8_t *p = *cursor;
    uint32_t codepoint = 0U;

    if (!p || !*p) {
        return 0;
    }

    if ((p[0] & 0x80U) == 0U) {
        codepoint = p[0];
        *cursor = p + 1U;
        return (int)codepoint;
    }

    if ((p[0] & 0xE0U) == 0xC0U && (p[1] & 0xC0U) == 0x80U) {
        codepoint = ((uint32_t)(p[0] & 0x1FU) << 6U) | (uint32_t)(p[1] & 0x3FU);
        *cursor = p + 2U;
        return (int)codepoint;
    }

    if ((p[0] & 0xF0U) == 0xE0U && (p[1] & 0xC0U) == 0x80U &&
        (p[2] & 0xC0U) == 0x80U) {
        codepoint = ((uint32_t)(p[0] & 0x0FU) << 12U) |
                    ((uint32_t)(p[1] & 0x3FU) << 6U) | (uint32_t)(p[2] & 0x3FU);
        *cursor = p + 3U;
        return (int)codepoint;
    }

    if ((p[0] & 0xF8U) == 0xF0U && (p[1] & 0xC0U) == 0x80U &&
        (p[2] & 0xC0U) == 0x80U && (p[3] & 0xC0U) == 0x80U) {
        codepoint = ((uint32_t)(p[0] & 0x07U) << 18U) |
                    ((uint32_t)(p[1] & 0x3FU) << 12U) |
                    ((uint32_t)(p[2] & 0x3FU) << 6U) | (uint32_t)(p[3] & 0x3FU);
        *cursor = p + 4U;
        return (int)codepoint;
    }

    *cursor = p + 1U;
    return 0;
}

static int text_layer_find_glyph_index(text_layer_t *layer,
                                       uint32_t codepoint) {
    size_t i;

    if (!layer) {
        return -1;
    }

    for (i = 0U; i < layer->glyph_count; ++i) {
        if (layer->glyph_codes[i] == codepoint) {
            return (int)i;
        }
    }
    return -1;
}

static void text_layer_measure_text_bounds(text_layer_t *layer,
                                           const char *text, int *width,
                                           int *height) {
    const uint8_t *cursor;
    int line_width = 0;
    int max_line_width = 0;
    int line_count = 1;

    if (!layer || !width || !height) {
        return;
    }

    *width = 0;
    *height = 0;
    if (!text || !text[0]) {
        return;
    }

    cursor = (const uint8_t *)text;
    while (*cursor != '\0') {
        int codepoint = text_layer_utf8_decode(&cursor);
        int glyph_index;

        if (codepoint == 0) {
            continue;
        }

        if (codepoint == '\n') {
            if (line_width > max_line_width) {
                max_line_width = line_width;
            }
            line_width = 0;
            line_count += 1;
            continue;
        }

        glyph_index = text_layer_find_glyph_index(layer, (uint32_t)codepoint);
        if (glyph_index < 0) {
            line_width += 1;
            continue;
        }

        line_width += layer->glyph_advances[glyph_index];
    }

    if (line_width > max_line_width) {
        max_line_width = line_width;
    }

    *width = max_line_width;
    *height = line_count * layer->cell_height;
}

static bool text_layer_ensure_instance_capacity(text_layer_t *layer,
                                                size_t required_count) {
    text_instance_t *next = NULL;
    size_t new_capacity = 0U;

    if (!layer) {
        return false;
    }

    if (required_count <= layer->instance_capacity) {
        return true;
    }

    new_capacity =
        layer->instance_capacity == 0U ? 4U : layer->instance_capacity;
    while (new_capacity < required_count) {
        new_capacity *= 2U;
    }

    next = (text_instance_t *)realloc(layer->instances,
                                      new_capacity * sizeof(*next));
    if (!next) {
        return false;
    }

    layer->instances = next;
    layer->instance_capacity = new_capacity;
    return true;
}

void text_layer_init(text_layer_t *layer, renderer_t *renderer) {
    if (!layer) {
        return;
    }

    layer->renderer = renderer;
    layer->glyph_codes = NULL;
    layer->glyph_widths = NULL;
    layer->glyph_advances = NULL;
    layer->glyph_indices = NULL;
    layer->glyph_textures = NULL;
    layer->glyph_count = 0U;
    layer->cell_height = 0;
    layer->pad_x = 0;
    layer->text_x = 0;
    layer->text_y = 0;
    layer->text[0] = '\0';
    layer->has_text = false;
    layer->instances = NULL;
    layer->instance_count = 0U;
    layer->instance_capacity = 0U;
}

void text_layer_clear(text_layer_t *layer) {
    if (!layer) {
        return;
    }

    free(layer->instances);
    layer->instances = NULL;
    layer->instance_count = 0U;
    layer->instance_capacity = 0U;
    layer->text_x = 0;
    layer->text_y = 0;
    layer->text[0] = '\0';
    layer->has_text = false;
}

void text_layer_destroy(text_layer_t *layer) {
    if (!layer) {
        return;
    }

    text_layer_clear_glyphs(layer);
    text_layer_clear(layer);
    layer->renderer = NULL;
}

bool text_layer_load_font_file(text_layer_t *layer, const char *font_path) {
    FILE *file = NULL;
    uint8_t *data = NULL;
    long file_size = 0L;
    size_t i;
    size_t glyphs_loaded = 0U;
    const uint8_t *cursor;
    uint32_t symbol_count = 0U;
    uint16_t weight = 0U;
    uint16_t pad_x = 0U;
    uint16_t atlas_width = 0U;
    uint16_t atlas_height = 0U;
    int cell_size = 0;
    const uint8_t *atlas = NULL;
    size_t atlas_offset;

    if (!layer || !font_path) {
        fprintf(stderr,
                "widebrim: text_layer_load_font_file called with NULL args\n");
        return false;
    }

    file = fopen(font_path, "rb");
    if (!file) {
        fprintf(stderr, "widebrim: failed to open font file: %s\n", font_path);
        return false;
    }

    if (fseek(file, 0L, SEEK_END) != 0) {
        fprintf(stderr, "widebrim: failed to seek font file: %s\n", font_path);
        fclose(file);
        return false;
    }

    file_size = ftell(file);
    if (file_size <= 0L) {
        fprintf(stderr, "widebrim: empty font file: %s\n", font_path);
        fclose(file);
        return false;
    }

    if (fseek(file, 0L, SEEK_SET) != 0) {
        fprintf(stderr, "widebrim: failed to rewind font file: %s\n",
                font_path);
        fclose(file);
        return false;
    }

    data = (uint8_t *)malloc((size_t)file_size);
    if (!data) {
        fprintf(stderr, "widebrim: failed to allocate font data\n");
        fclose(file);
        return false;
    }

    if (fread(data, 1U, (size_t)file_size, file) != (size_t)file_size) {
        fprintf(stderr, "widebrim: failed to read font file: %s\n", font_path);
        free(data);
        fclose(file);
        return false;
    }
    fclose(file);

    if (file_size < 12L) {
        fprintf(stderr, "widebrim: font file too small: %s\n", font_path);
        free(data);
        return false;
    }

    symbol_count = text_layer_read_u32_le(data);
    weight = text_layer_read_u16_le(data + 4U);
    pad_x = text_layer_read_u16_le(data + 6U);
    atlas_width = text_layer_read_u16_le(data + 8U);
    atlas_height = text_layer_read_u16_le(data + 10U);
    if (symbol_count == 0U || atlas_width == 0U || atlas_height == 0U) {
        fprintf(stderr, "widebrim: invalid font metadata in %s\n", font_path);
        free(data);
        return false;
    }

    cell_size = (int)weight + (int)pad_x * 2;
    atlas_offset = 12U + (size_t)symbol_count * 4U;
    if (atlas_offset > (size_t)file_size) {
        fprintf(stderr, "widebrim: truncated font table in %s\n", font_path);
        free(data);
        return false;
    }

    text_layer_clear_glyphs(layer);
    layer->glyph_codes =
        (uint32_t *)calloc(symbol_count, sizeof(*layer->glyph_codes));
    layer->glyph_widths =
        (int *)calloc(symbol_count, sizeof(*layer->glyph_widths));
    layer->glyph_advances =
        (int *)calloc(symbol_count, sizeof(*layer->glyph_advances));
    layer->glyph_indices =
        (int *)calloc(symbol_count, sizeof(*layer->glyph_indices));
    layer->glyph_textures = (renderer_texture_t **)calloc(
        symbol_count, sizeof(*layer->glyph_textures));
    if (!layer->glyph_codes || !layer->glyph_widths || !layer->glyph_advances ||
        !layer->glyph_indices || !layer->glyph_textures) {
        fprintf(stderr, "widebrim: failed to allocate glyph tables\n");
        free(data);
        text_layer_clear_glyphs(layer);
        return false;
    }

    cursor = data + 12U;
    for (i = 0U; i < symbol_count; ++i) {
        uint16_t codepoint = text_layer_read_u16_le(cursor);
        uint16_t glyph_width = text_layer_read_u16_le(cursor + 2U);
        cursor += 4U;

        if (codepoint == 0U) {
            continue;
        }

        layer->glyph_codes[glyphs_loaded] = (uint32_t)codepoint;
        layer->glyph_widths[glyphs_loaded] = (int)glyph_width;
        layer->glyph_advances[glyphs_loaded] = (int)glyph_width;
        layer->glyph_indices[glyphs_loaded] = (int)i;
        layer->glyph_textures[glyphs_loaded] = NULL;
        ++glyphs_loaded;
    }

    layer->glyph_count = (size_t)glyphs_loaded;
    layer->cell_height = cell_size;
    layer->pad_x = (int)pad_x;

    atlas = data + (ptrdiff_t)atlas_offset;
    for (i = 0U; i < layer->glyph_count; ++i) {
        const int glyph_width = layer->glyph_widths[i];
        const int glyph_height = layer->cell_height;
        const int slot_index = layer->glyph_indices[i];
        const int glyph_col = slot_index % 26;
        const int glyph_row = slot_index / 26;
        const int offset_x = glyph_col * cell_size;
        const int offset_y = glyph_row * cell_size;

        layer->glyph_textures[i] = text_layer_build_glyph_texture(
            layer, atlas, atlas_width, atlas_height, offset_x, offset_y,
            glyph_width, glyph_height, layer->pad_x);
    }

    free(data);
    return layer->glyph_count > 0U;
}

text_instance_t *text_layer_add_text(text_layer_t *layer, int x, int y,
                                     const char *text) {
    text_instance_t *instance = NULL;

    if (!layer) {
        return NULL;
    }

    if (!text_layer_ensure_instance_capacity(layer,
                                             layer->instance_count + 1U)) {
        return NULL;
    }

    instance = &layer->instances[layer->instance_count];
    instance->x = x;
    instance->y = y;
    instance->width = 0;
    instance->height = 0;
    instance->visible = true;
    instance->layer = layer;
    instance->text[0] = '\0';
    if (text) {
        snprintf(instance->text, sizeof(instance->text), "%s", text);
    }
    text_layer_measure_text_bounds(layer, instance->text, &instance->width,
                                   &instance->height);
    layer->instance_count += 1U;
    layer->has_text = true;
    return instance;
}

bool text_layer_get_text_position(text_instance_t *instance, int *x, int *y) {
    if (!instance || !x || !y) {
        return false;
    }

    *x = instance->x;
    *y = instance->y;
    return true;
}

bool text_layer_set_text_position(text_instance_t *instance, int x, int y) {
    if (!instance) {
        return false;
    }

    instance->x = x;
    instance->y = y;
    return true;
}

bool text_layer_center_text_in_rect(text_instance_t *instance,
                                    const rect_t *rect) {
    if (!instance || !rect) {
        return false;
    }

    instance->x =
        (int)((rect->x + (rect->w / 2.0f)) - (instance->width / 2.0f));
    instance->y =
        (int)((rect->y + (rect->h / 2.0f)) - (instance->height / 2.0f));
    return true;
}

bool text_layer_set_text_contents(text_instance_t *instance, const char *text) {
    if (!instance || !instance->layer) {
        return false;
    }

    if (!text) {
        instance->text[0] = '\0';
        instance->width = 0;
        instance->height = 0;
        return true;
    }

    snprintf(instance->text, sizeof(instance->text), "%s", text);
    text_layer_measure_text_bounds(instance->layer, instance->text,
                                   &instance->width, &instance->height);
    return true;
}

bool text_layer_set_visible(text_instance_t *instance, bool visible) {
    if (!instance) {
        return false;
    }

    instance->visible = visible;
    return true;
}

bool text_layer_set_text(text_layer_t *layer, int x, int y, const char *text) {
    text_instance_t *instance = NULL;

    if (!layer || !text) {
        return false;
    }

    if (layer->instance_count == 0U) {
        instance = text_layer_add_text(layer, x, y, text);
        return instance != NULL;
    }

    instance = &layer->instances[0];
    instance->x = x;
    instance->y = y;
    instance->visible = true;
    instance->layer = layer;
    snprintf(instance->text, sizeof(instance->text), "%s", text);
    text_layer_measure_text_bounds(layer, instance->text, &instance->width,
                                   &instance->height);
    layer->has_text = true;
    return true;
}

bool text_layer_draw_text(text_layer_t *layer, int x, int y, const char *text) {
    const uint8_t *cursor;
    int cursor_x = x;
    int cursor_y = y;

    if (!layer || !layer->renderer || !text) {
        return false;
    }

    cursor = (const uint8_t *)text;
    while (*cursor != '\0') {
        int codepoint = text_layer_utf8_decode(&cursor);
        int glyph_index;
        rect_t dst;

        if (codepoint == 0) {
            continue;
        }

        if (codepoint == '\n') {
            cursor_x = x;
            cursor_y += layer->cell_height;
            continue;
        }

        glyph_index = text_layer_find_glyph_index(layer, (uint32_t)codepoint);
        if (glyph_index < 0) {
            cursor_x += 1;
            continue;
        }

        if (layer->glyph_textures[glyph_index]) {
            dst = (rect_t){.x = (float)cursor_x,
                           .y = (float)cursor_y,
                           .w = (float)layer->glyph_widths[glyph_index],
                           .h = (float)layer->cell_height};
            renderer_draw_texture(layer->renderer,
                                  layer->glyph_textures[glyph_index], &dst);
        }

        cursor_x += layer->glyph_advances[glyph_index];
    }

    return true;
}

bool text_layer_draw_text_instance(text_layer_t *layer,
                                   text_instance_t *instance) {
    if (!layer || !instance || !instance->visible || !instance->text[0]) {
        return false;
    }

    return text_layer_draw_text(layer, instance->x, instance->y,
                                instance->text);
}

static void text_layer_draw(void *impl, renderer_t *renderer) {
    text_layer_t *layer = (text_layer_t *)impl;
    size_t i;

    if (!layer || !renderer) {
        return;
    }

    for (i = 0U; i < layer->instance_count; ++i) {
        if (layer->instances[i].visible) {
            text_layer_draw_text_instance(layer, &layer->instances[i]);
        }
    }
}

static void text_layer_update(void *impl, float dt_ms) {
    (void)impl;
    (void)dt_ms;
}

static bool text_layer_handle_event(void *impl, const input_event_t *event) {
    (void)impl;
    (void)event;
    return false;
}

static void text_layer_on_quit(void *impl) { (void)impl; }

static void text_layer_destroy_void(void *impl) {
    text_layer_destroy((text_layer_t *)impl);
}

screen_layer_t text_layer_as_screen_layer(text_layer_t *layer) {
    screen_layer_t result;

    result.impl = layer;
    result.update = text_layer_update;
    result.draw = text_layer_draw;
    result.handle_event = text_layer_handle_event;
    result.on_quit = text_layer_on_quit;
    result.destroy = text_layer_destroy_void;
    return result;
}
