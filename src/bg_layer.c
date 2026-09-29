#include "bg_layer.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "texture_loader.h"

static void bg_layer_texture_set_rgba(bg_layer_texture_t *tex,
                                      renderer_t *renderer, const uint8_t *rgba,
                                      int width, int height);
static void bg_layer_texture_set_darkness(bg_layer_texture_t *tex,
                                          uint8_t darkness);

static bool bg_layer_load_texture(bg_layer_t *bg_layer, game_state_t *state,
                                  const char *bg_name,
                                  bg_layer_texture_t *texture_slot,
                                  const char *texture_name) {
    texture_data_t *texture = NULL;
    char bg_path[1024];
    char full_bg_path[1024];

    if (!bg_layer || !bg_layer->renderer || !bg_name || !texture_slot ||
        !texture_name) {
        return false;
    }

    snprintf(bg_path, sizeof(bg_path), "bg/%s", bg_name);

    if (!asset_path_resolve(state, bg_path, full_bg_path,
                            sizeof(full_bg_path))) {
        fprintf(stderr,
                "widebrim: Failed to resolve path for background image '%s'\n",
                bg_path);
        return false;
    }

    texture = texture_load_rgba(full_bg_path);
    if (!texture) {
        fprintf(stderr, "widebrim: failed to load %s '%s'\n", texture_name,
                bg_name);
        return false;
    }

    bg_layer_texture_set_rgba(texture_slot, bg_layer->renderer, texture->pixels,
                              texture->width, texture->height);
    bg_layer_texture_set_darkness(texture_slot, 0);
    texture_free(texture);

    return true;
}

static bool bg_layer_load(bg_layer_t *bg_layer, game_state_t *state,
                          const char *bg_name) {
    return bg_layer_load_texture(bg_layer, state, bg_name, &bg_layer->tex_main,
                                 "background");
}

static bool bg_layer_load_sub(bg_layer_t *bg_layer, game_state_t *state,
                              const char *bg_name) {
    return bg_layer_load_texture(bg_layer, state, bg_name, &bg_layer->tex_sub,
                                 "sub background");
}

static bool bg_layer_load_sub2(bg_layer_t *bg_layer, game_state_t *state,
                               const char *bg_name) {
    return bg_layer_load_texture(bg_layer, state, bg_name, &bg_layer->tex_sub2,
                                 "sub background");
}

static void bg_layer_texture_init(bg_layer_texture_t *tex) {
    tex->tex = NULL;
    tex->darkness = 0;
    tex->shake_remaining_ms = 0.0f;
    tex->repeating = false;
    tex->scroll_x = 0.0f;
    tex->scroll_speed_x = 0.0f;
}

static void bg_layer_texture_destroy(bg_layer_texture_t *tex,
                                     renderer_t *renderer) {
    if (!tex || !renderer || !tex->tex) {
        return;
    }
    renderer_destroy_texture(renderer, tex->tex);
    tex->tex = NULL;
}

void bg_layer_init(bg_layer_t *bg, renderer_t *renderer) {
    if (!bg) {
        return;
    }

    bg->renderer = renderer;
    bg->load = bg_layer_load;
    bg->load_sub = bg_layer_load_sub;
    bg->load_sub2 = bg_layer_load_sub2;
    bg->set_scroll = bg_layer_set_main_scroll;
    bg->set_sub_scroll = bg_layer_set_sub_scroll;
    bg->set_sub2_scroll = bg_layer_set_sub2_scroll;

    if (bg->tex_main.tex)
        bg_layer_texture_destroy(&bg->tex_main, bg->renderer);
    if (bg->tex_sub.tex)
        bg_layer_texture_destroy(&bg->tex_sub, bg->renderer);
    if (bg->tex_sub2.tex)
        bg_layer_texture_destroy(&bg->tex_sub2, bg->renderer);

    bg_layer_texture_init(&bg->tex_main);
    bg_layer_texture_init(&bg->tex_sub);
    bg_layer_texture_init(&bg->tex_sub2);
    bg->touch_pending = false;
    bg->touch_start_x = 0;
    bg->touch_start_y = 0;
    bg->touch_last_x = 0;
    bg->touch_last_y = 0;
    bg->touch_dragged = false;
    bg->touch_callback = NULL;
    bg->touch_user = NULL;
}

void bg_layer_destroy(bg_layer_t *bg) {
    if (!bg) {
        return;
    }

    bg_layer_texture_destroy(&bg->tex_main, bg->renderer);
    bg_layer_texture_destroy(&bg->tex_sub, bg->renderer);
    bg_layer_texture_destroy(&bg->tex_sub2, bg->renderer);
    bg->renderer = NULL;
}

void bg_layer_set_touch_callback(bg_layer_t *bg_layer,
                                 bg_layer_touch_callback_t callback,
                                 void *user) {
    if (!bg_layer) {
        return;
    }

    bg_layer->touch_callback = callback;
    bg_layer->touch_user = user;
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

static void bg_layer_texture_set_scroll(bg_layer_texture_t *tex,
                                        float pixels_per_second,
                                        bool repeating) {
    tex->scroll_speed_x = pixels_per_second;
    tex->repeating = repeating;
    if (!repeating) {
        tex->scroll_x = 0.0f;
    }
}

void bg_layer_set_main_rgba(bg_layer_t *bg, const uint8_t *rgba, int width,
                            int height) {
    bg_layer_texture_set_rgba(&bg->tex_main, bg->renderer, rgba, width, height);
}

void bg_layer_set_sub_rgba(bg_layer_t *bg, const uint8_t *rgba, int width,
                           int height) {
    bg_layer_texture_set_rgba(&bg->tex_sub, bg->renderer, rgba, width, height);
}

void bg_layer_set_sub2_rgba(bg_layer_t *bg, const uint8_t *rgba, int width,
                            int height) {
    bg_layer_texture_set_rgba(&bg->tex_sub2, bg->renderer, rgba, width, height);
}

void bg_layer_set_main_darkness(bg_layer_t *bg, uint8_t darkness) {
    bg_layer_texture_set_darkness(&bg->tex_main, darkness);
}

void bg_layer_set_sub_darkness(bg_layer_t *bg, uint8_t darkness) {
    bg_layer_texture_set_darkness(&bg->tex_sub, darkness);
}

void bg_layer_set_sub2_darkness(bg_layer_t *bg, uint8_t darkness) {
    bg_layer_texture_set_darkness(&bg->tex_sub2, darkness);
}

void bg_layer_set_main_shake(bg_layer_t *bg, float shake_remaining_ms) {
    bg_layer_texture_set_shake(&bg->tex_main, shake_remaining_ms);
}

void bg_layer_set_sub_shake(bg_layer_t *bg, float shake_remaining_ms) {
    bg_layer_texture_set_shake(&bg->tex_sub, shake_remaining_ms);
}

void bg_layer_set_sub2_shake(bg_layer_t *bg, float shake_remaining_ms) {
    bg_layer_texture_set_shake(&bg->tex_sub2, shake_remaining_ms);
}

void bg_layer_set_main_scroll(bg_layer_t *bg, float pixels_per_second,
                              bool repeating) {
    bg_layer_texture_set_scroll(&bg->tex_main, pixels_per_second, repeating);
}

void bg_layer_set_sub_scroll(bg_layer_t *bg, float pixels_per_second,
                             bool repeating) {
    bg_layer_texture_set_scroll(&bg->tex_sub, pixels_per_second, repeating);
}

void bg_layer_set_sub2_scroll(bg_layer_t *bg, float pixels_per_second,
                              bool repeating) {
    bg_layer_texture_set_scroll(&bg->tex_sub2, pixels_per_second, repeating);
}

static void bg_layer_texture_update(bg_layer_texture_t *tex, float delta_ms) {
    if (tex->shake_remaining_ms > 0.0f) {
        tex->shake_remaining_ms -= delta_ms;
        if (tex->shake_remaining_ms < 0.0f) {
            tex->shake_remaining_ms = 0.0f;
        }
    }

    if (tex->repeating && tex->tex) {
        tex->scroll_x += (tex->scroll_speed_x * delta_ms) / 1000.0f;
    }
}

static void bg_layer_update_impl(void *impl, float delta_ms) {
    bg_layer_t *bg = (bg_layer_t *)impl;
    bg_layer_texture_update(&bg->tex_main, delta_ms);
    bg_layer_texture_update(&bg->tex_sub, delta_ms);
    bg_layer_texture_update(&bg->tex_sub2, delta_ms);
}

static void bg_layer_texture_draw(renderer_t *renderer, bg_layer_texture_t *tex,
                                  int y_offset) {
    rect_t dst;
    int shake_x = 0, shake_y = 0;
    int tex_w = 0, tex_h = 0;
    float x = 0.0f;

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

    if (tex && tex->tex) {
        if (tex->repeating) {
            renderer_get_texture_size(renderer, tex->tex, &tex_w, &tex_h);
            if (tex_w > 0) {
                float tile_w = (float)tex_w;
                if (tile_w > (float)WB_SCREEN_WIDTH) {
                    tile_w = (float)WB_SCREEN_WIDTH;
                }

                float offset_x = fmodf(tex->scroll_x, tile_w);
                if (offset_x < 0.0f) {
                    offset_x += tile_w;
                }

                for (x = -offset_x; x < WB_SCREEN_WIDTH + tile_w; x += tile_w) {
                    rect_t tile = {.x = x + (float)shake_x,
                                   .y = (float)y_offset + (float)shake_y,
                                   .w = tile_w,
                                   .h = (float)WB_SCREEN_HEIGHT};
                    renderer_draw_texture(renderer, tex->tex, &tile);
                }
            } else {
                renderer_draw_texture(renderer, tex->tex, &dst);
            }
        } else {
            renderer_draw_texture(renderer, tex->tex, &dst);
        }
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
    if (renderer_texture_exists(renderer, bg->tex_sub2.tex)) {
        bg_layer_texture_draw(renderer, &bg->tex_sub2, WB_SCREEN_HEIGHT);
    }
}

static bool bg_layer_handle_event(void *impl, const input_event_t *event) {
    bg_layer_t *bg = (bg_layer_t *)impl;
    if (!bg || !event) {
        return false;
    }

    switch (event->type) {
    case INPUT_EVENT_MOUSE_BUTTON_DOWN:
        bg->touch_pending = true;
        bg->touch_dragged = false;
        bg->touch_start_x = event->data.mouse_button.x;
        bg->touch_start_y = event->data.mouse_button.y;
        bg->touch_last_x = bg->touch_start_x;
        bg->touch_last_y = bg->touch_start_y;
        return false;

    case INPUT_EVENT_MOUSE_MOTION:
        if (!bg->touch_pending) {
            return false;
        }

        bg->touch_last_x = event->data.mouse_motion.x;
        bg->touch_last_y = event->data.mouse_motion.y;

        if (abs(bg->touch_last_x - bg->touch_start_x) > 18 ||
            abs(bg->touch_last_y - bg->touch_start_y) > 18) {
            bg->touch_dragged = true;
            bg->touch_pending = false;
            if (bg->touch_callback) {
                bg->touch_callback(bg->touch_user, BG_TOUCH_KIND_DRAG,
                                   bg->touch_last_x, bg->touch_last_y);
            }
        }
        return false;

    case INPUT_EVENT_MOUSE_BUTTON_UP:
        if (!bg->touch_pending) {
            return false;
        }

        if (!bg->touch_dragged && bg->touch_callback) {
            bg->touch_callback(bg->touch_user, BG_TOUCH_KIND_TAP,
                               bg->touch_start_x, bg->touch_start_y);
        }

        bg->touch_pending = false;
        bg->touch_dragged = false;
        return false;

    default:
        return false;
    }
}

screen_layer_t bg_layer_as_screen_layer(bg_layer_t *bg) {
    screen_layer_t layer;
    layer.impl = (void *)bg;
    layer.update = bg_layer_update_impl;
    layer.draw = bg_layer_draw;
    layer.handle_event = bg_layer_handle_event;
    layer.on_quit = NULL;
    layer.destroy = NULL;
    return layer;
}
