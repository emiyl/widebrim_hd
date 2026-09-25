#ifndef TEXT_LAYER_H
#define TEXT_LAYER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "renderer.h"
#include "screen.h"

typedef struct text_layer_t {
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
} text_layer_t;

void text_layer_init(text_layer_t *layer, renderer_t *renderer);
void text_layer_destroy(text_layer_t *layer);
bool text_layer_load_font_file(text_layer_t *layer, const char *font_path);
bool text_layer_set_text(text_layer_t *layer, int x, int y, const char *text);
bool text_layer_draw_text(text_layer_t *layer, int x, int y, const char *text);
screen_layer_t text_layer_as_screen_layer(text_layer_t *layer);

#endif // TEXT_LAYER_H
