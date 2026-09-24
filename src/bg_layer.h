#ifndef BG_LAYER_H
#define BG_LAYER_H

#include <stdint.h>

#include <SDL3/SDL.h>

#include "renderer.h"
#include "screen.h"

#define WB_SCREEN_WIDTH 768
#define WB_SCREEN_HEIGHT 620

typedef struct {
    renderer_texture_t *tex;
    uint8_t darkness;
    float shake_remaining_ms;
    bool repeating;
    float scroll_x;
    float scroll_speed_x;
} bg_layer_texture_t;

typedef enum {
    BG_TOUCH_KIND_NONE = 0,
    BG_TOUCH_KIND_TAP,
    BG_TOUCH_KIND_DRAG
} bg_touch_kind_t;

typedef void (*bg_layer_touch_callback_t)(void *user, bg_touch_kind_t kind,
                                          int x, int y);

typedef struct {
    renderer_t *renderer;
    bg_layer_texture_t tex_main;
    bg_layer_texture_t tex_sub;
    bg_layer_texture_t tex_sub2;
    bool touch_pending;
    int touch_start_x;
    int touch_start_y;
    int touch_last_x;
    int touch_last_y;
    bool touch_dragged;
    bg_layer_touch_callback_t touch_callback;
    void *touch_user;
} bg_layer_t;

void bg_layer_init(bg_layer_t *bg_layer, renderer_t *renderer);
void bg_layer_destroy(bg_layer_t *bg_layer);

void bg_layer_set_touch_callback(bg_layer_t *bg_layer,
                                 bg_layer_touch_callback_t callback,
                                 void *user);

void bg_layer_set_main_rgba(bg_layer_t *bg, const uint8_t *rgba, int width,
                            int height);
void bg_layer_set_sub_rgba(bg_layer_t *bg, const uint8_t *rgba, int width,
                           int height);
void bg_layer_set_sub2_rgba(bg_layer_t *bg, const uint8_t *rgba, int width,
                            int height);
void bg_layer_set_main_darkness(bg_layer_t *bg, uint8_t darkness);
void bg_layer_set_sub_darkness(bg_layer_t *bg, uint8_t darkness);
void bg_layer_set_sub2_darkness(bg_layer_t *bg, uint8_t darkness);
void bg_layer_set_main_shake(bg_layer_t *bg, float shake_remaining_ms);
void bg_layer_set_sub_shake(bg_layer_t *bg, float shake_remaining_ms);
void bg_layer_set_sub2_shake(bg_layer_t *bg, float shake_remaining_ms);
void bg_layer_set_main_scroll(bg_layer_t *bg, float pixels_per_second,
                              bool repeating);
void bg_layer_set_sub_scroll(bg_layer_t *bg, float pixels_per_second,
                             bool repeating);
void bg_layer_set_sub2_scroll(bg_layer_t *bg, float pixels_per_second,
                              bool repeating);

screen_layer_t bg_layer_as_screen_layer(bg_layer_t *bg_layer);

#endif // BG_LAYER_H
