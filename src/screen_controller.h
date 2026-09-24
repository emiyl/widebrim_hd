#ifndef SCREEN_CONTROLLER_H
#define SCREEN_CONTROLLER_H

#include <stdlib.h>

#include "bg_layer.h"
#include "fader_layer.h"
#include "renderer.h"
#include "sprite_layer.h"
#include "sprite_loader.h"

typedef struct {
    bg_layer_t *bg;
    sprite_layer_t *sprite;
    fader_layer_t *fader;
    renderer_t *renderer;
} screen_controller_t;

static inline void screen_controller_set_bg_main(screen_controller_t *sc,
                                                 const uint8_t *rgba, int w,
                                                 int h) {
    bg_layer_set_main_rgba(sc->bg, rgba, w, h);
    bg_layer_set_main_darkness(sc->bg, 0);
}

static inline void screen_controller_set_bg_sub(screen_controller_t *sc,
                                                const uint8_t *rgba, int w,
                                                int h) {
    bg_layer_set_sub_rgba(sc->bg, rgba, w, h);
    bg_layer_set_sub_darkness(sc->bg, 0);
}

static inline void screen_controller_set_bg_sub2(screen_controller_t *sc,
                                                 const uint8_t *rgba, int w,
                                                 int h) {
    bg_layer_set_sub2_rgba(sc->bg, rgba, w, h);
    bg_layer_set_sub2_darkness(sc->bg, 0);
}

static inline void screen_controller_set_bg_main_scroll(screen_controller_t *sc,
                                                        float pixels_per_second,
                                                        bool repeating) {
    bg_layer_set_main_scroll(sc->bg, pixels_per_second, repeating);
}

static inline void screen_controller_set_bg_sub_scroll(screen_controller_t *sc,
                                                       float pixels_per_second,
                                                       bool repeating) {
    bg_layer_set_sub_scroll(sc->bg, pixels_per_second, repeating);
}

static inline void screen_controller_set_bg_sub2_scroll(screen_controller_t *sc,
                                                        float pixels_per_second,
                                                        bool repeating) {
    bg_layer_set_sub2_scroll(sc->bg, pixels_per_second, repeating);
}

static inline void screen_controller_clear_bg_layer(screen_controller_t *sc) {
    if (!sc || !sc->bg) {
        return;
    }
    bg_layer_init(sc->bg, sc->renderer);
}

static inline sprite_instance_t *
screen_controller_add_sprite_z(screen_controller_t *sc, const uint8_t *rgba,
                               int w, int h, int x, int y, int z,
                               uint8_t alpha) {
    if (!sc || !sc->sprite) {
        return NULL;
    }
    return sprite_layer_add_rgba_z(sc->sprite, rgba, w, h, x, y, z, alpha);
}

static inline sprite_instance_t *
screen_controller_add_sprite(screen_controller_t *sc, const uint8_t *rgba,
                             int w, int h, int x, int y) {
    return screen_controller_add_sprite_z(sc, rgba, w, h, x, y, 0, 255U);
}

static inline sprite_instance_t *screen_controller_add_sprite_animation(
    screen_controller_t *sc, const uint8_t *const *frames, size_t frame_count,
    int w, int h, int x, int y, int z, uint8_t alpha, float frame_duration_ms,
    bool loop) {
    if (!sc || !sc->sprite) {
        return NULL;
    }
    return sprite_layer_add_animation(sc->sprite, frames, frame_count, w, h, x,
                                      y, z, alpha, frame_duration_ms, loop);
}

static inline sprite_instance_t *screen_controller_add_sprite_asset(
    screen_controller_t *sc, game_state_t *state, const char *rel_path, int x,
    int y, int z, uint8_t alpha, float frame_duration_ms, bool loop) {
    uint8_t **frames = NULL;
    size_t frame_count = 0U;
    int frame_w = 0;
    int frame_h = 0;
    size_t i;
    sprite_instance_t *sprite = NULL;

    if (!sc || !sc->sprite || !state || !rel_path) {
        fprintf(stderr, "widebrim: one or more required arguments are NULL\n");
        return NULL;
    }

    if (!sprite_loader_load_animation_rgba(state, rel_path, &frames,
                                           &frame_count, &frame_w, &frame_h)) {
        uint8_t *single_frame = NULL;
        int single_w = 0;
        int single_h = 0;

        if (sprite_loader_load_frame_rgba(state, rel_path, 0U, &single_frame,
                                          &single_w, &single_h)) {
            sprite = screen_controller_add_sprite_z(sc, single_frame, single_w,
                                                    single_h, x, y, z, alpha);
            free(single_frame);
            return sprite;
        }

        fprintf(stderr,
                "widebrim: sprite_loader_load_animation_rgba failed for asset: "
                "%s\n",
                rel_path);
        return NULL;
    }

    sprite = sprite_layer_add_animation(
        sc->sprite, (const uint8_t *const *)frames, frame_count, frame_w,
        frame_h, x, y, z, alpha, frame_duration_ms, loop);

    for (i = 0U; i < frame_count; ++i) {
        free(frames[i]);
    }
    free(frames);
    return sprite;
}

static inline void
screen_controller_clear_sprite_layer(screen_controller_t *sc) {
    if (!sc || !sc->sprite) {
        return;
    }
    sprite_layer_clear(sc->sprite);
}

static inline bool
screen_controller_set_sprite_position(screen_controller_t *sc,
                                      sprite_instance_t *sprite, int x, int y) {
    (void)sc;
    return sprite_layer_set_sprite_position(sprite, x, y);
}

static inline bool screen_controller_center_sprite(screen_controller_t *sc,
                                                   sprite_instance_t *sprite,
                                                   int area_width,
                                                   int area_height) {
    (void)sc;
    return sprite_layer_center_sprite(sprite, area_width, area_height);
}

static inline void screen_controller_fade_in(screen_controller_t *sc,
                                             float duration_ms,
                                             fader_callback cb, void *user) {
    fader_layer_fade_in(sc->fader, duration_ms, cb, user);
}

static inline void screen_controller_fade_out(screen_controller_t *sc,
                                              float duration_ms,
                                              fader_callback cb, void *user) {
    fader_layer_fade_out(sc->fader, duration_ms, cb, user);
}

static inline bool
screen_controller_is_view_obscured(const screen_controller_t *sc) {
    return fader_layer_is_view_obscured(sc->fader);
}

#endif // SCREEN_CONTROLLER_H
