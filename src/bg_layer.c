#include "bg_layer.h"

#include <stdlib.h>

static void bg_layer_texture_init(bg_layer_texture_t *tex) {
    tex->tex = NULL;
    tex->darkness = 0;
    tex->shake_remaining_ms = 0.0f;
}

static void bg_layer_texture_destroy(bg_layer_texture_t *tex,
                                     renderer_t *renderer) {
    if (tex->tex) {
        renderer_destroy_texture(renderer, tex->tex);
        tex->tex = NULL;
    }
}

void bg_layer_init(bg_layer_t *bg, renderer_t *renderer) {
    bg->renderer = renderer;
    bg_layer_texture_init(&bg->tex_main);
    bg_layer_texture_init(&bg->tex_sub);
}

void bg_layer_destroy(bg_layer_t *bg) {
    bg_layer_texture_destroy(&bg->tex_main, bg->renderer);
    bg_layer_texture_destroy(&bg->tex_sub, bg->renderer);
    bg->renderer = NULL;
}

static void bg_layer_texture_set_rgba(bg_layer_texture_t *tex,
                                      renderer_t *renderer, const uint8_t *rgba,
                                      int width, int height) {
    if (tex->tex) {
        renderer_destroy_texture(renderer, tex->tex);
        tex->tex = NULL;
    }
    tex->tex = renderer_create_texture_from_rgba(renderer, rgba, width, height);
}

static void bg_layer_texture_set_darkness(bg_layer_texture_t *tex,
                                          uint8_t darkness) {
    tex->darkness = darkness;
}

static void bg_layer_texture_set_shake(bg_layer_texture_t *tex,
                                       float shake_remaining_ms) {
    tex->shake_remaining_ms = shake_remaining_ms;
}

void bg_layer_set_main_rgba(bg_layer_t *bg, const uint8_t *rgba, int width,
                            int height) {
    bg_layer_texture_set_rgba(&bg->tex_main, bg->renderer, rgba, width, height);
}

void bg_layer_set_sub_rgba(bg_layer_t *bg, const uint8_t *rgba, int width,
                           int height) {
    bg_layer_texture_set_rgba(&bg->tex_sub, bg->renderer, rgba, width, height);
}

void bg_layer_set_main_darkness(bg_layer_t *bg, uint8_t darkness) {
    bg_layer_texture_set_darkness(&bg->tex_main, darkness);
}

void bg_layer_set_sub_darkness(bg_layer_t *bg, uint8_t darkness) {
    bg_layer_texture_set_darkness(&bg->tex_sub, darkness);
}

void bg_layer_set_main_shake(bg_layer_t *bg, float shake_remaining_ms) {
    bg_layer_texture_set_shake(&bg->tex_main, shake_remaining_ms);
}

void bg_layer_set_sub_shake(bg_layer_t *bg, float shake_remaining_ms) {
    bg_layer_texture_set_shake(&bg->tex_sub, shake_remaining_ms);
}

static void bg_layer_texture_update(bg_layer_texture_t *tex, float delta_ms) {
    if (tex->shake_remaining_ms > 0.0f) {
        tex->shake_remaining_ms -= delta_ms;
        if (tex->shake_remaining_ms < 0.0f) {
            tex->shake_remaining_ms = 0.0f;
        }
    }
}

static void bg_layer_update_impl(void *impl, float delta_ms) {
    bg_layer_t *bg = (bg_layer_t *)impl;
    bg_layer_texture_update(&bg->tex_main, delta_ms);
    bg_layer_texture_update(&bg->tex_sub, delta_ms);
}

static void bg_layer_texture_draw(renderer_t *renderer, bg_layer_texture_t *tex,
                                  int y_offset) {
    rect_t dst;
    int shake_x = 0, shake_y = 0;

    dst = (rect_t){.x = 0.0f,
                   .y = (float)y_offset,
                   .w = (float)WB_SCREEN_WIDTH,
                   .h = (float)WB_SCREEN_HEIGHT};

    if (tex->shake_remaining_ms > 0.0f) {
        shake_x = (rand() % 5) - 2;
        shake_y = (rand() % 5) - 2;
        dst.x += (float)shake_x;
        dst.y += (float)shake_y;
    }

    if (tex) {
        renderer_draw_texture(renderer, tex->tex, &dst);
    }

    if (tex->darkness > 0) {
        rect_t overlay = {.x = 0.0f,
                          .y = (float)y_offset,
                          .w = (float)WB_SCREEN_WIDTH,
                          .h = (float)WB_SCREEN_HEIGHT};
        renderer_fill_rect(renderer, &overlay, 0, 0, 0, tex->darkness);
    }
}

static void bg_layer_draw(void *impl, renderer_t *renderer) {
    bg_layer_t *bg = (bg_layer_t *)impl;
    if (renderer_texture_exists(renderer, bg->tex_main.tex)) {
        bg_layer_texture_draw(renderer, &bg->tex_main, 0);
    }
    if (renderer_texture_exists(renderer, bg->tex_sub.tex)) {
        bg_layer_texture_draw(renderer, &bg->tex_sub, WB_SCREEN_HEIGHT);
    }
}

screen_layer_t bg_layer_as_screen_layer(bg_layer_t *bg) {
    screen_layer_t layer;
    layer.impl = (void *)bg;
    layer.update = bg_layer_update_impl;
    layer.draw = bg_layer_draw;
    layer.handle_event = NULL;
    layer.on_quit = NULL;
    layer.destroy = NULL;
    return layer;
}
