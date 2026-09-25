#ifndef TEXT_LAYER_H
#define TEXT_LAYER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "renderer.h"
#include "screen.h"

typedef struct text_layer_t text_layer_t;

typedef struct text_instance_t {
    int x;
    int y;
    int width;
    int height;
    bool visible;
    text_layer_t *layer;
    char text[4096];
} text_instance_t;

struct text_layer_t {
    renderer_t *renderer;
    uint32_t *glyph_codes;
    int *glyph_widths;
    int *glyph_advances;
    int *glyph_indices;
    renderer_texture_t **glyph_textures;
    size_t glyph_count;
    int cell_height;
    int pad_x;
    int text_x;
    int text_y;
    char text[4096];
    bool has_text;
    text_instance_t *instances;
    size_t instance_count;
    size_t instance_capacity;
};

void text_layer_init(text_layer_t *layer, renderer_t *renderer);
void text_layer_destroy(text_layer_t *layer);
void text_layer_clear(text_layer_t *layer);
text_instance_t *text_layer_add_text(text_layer_t *layer, int x, int y,
                                     const char *text);
bool text_layer_get_text_position(text_instance_t *instance, int *x, int *y);
bool text_layer_set_text_position(text_instance_t *instance, int x, int y);
bool text_layer_center_text_in_rect(text_instance_t *instance,
                                    const rect_t *rect);
bool text_layer_set_text_contents(text_instance_t *instance, const char *text);
bool text_layer_set_visible(text_instance_t *instance, bool visible);
bool text_layer_load_font_file(text_layer_t *layer, const char *font_path);
bool text_layer_set_text(text_layer_t *layer, int x, int y, const char *text);
bool text_layer_draw_text(text_layer_t *layer, int x, int y, const char *text);
bool text_layer_draw_text_instance(text_layer_t *layer,
                                   text_instance_t *instance);
screen_layer_t text_layer_as_screen_layer(text_layer_t *layer);

#endif // TEXT_LAYER_H
