#ifndef SCREEN_CONTROLLER_H
#define SCREEN_CONTROLLER_H

#include "bg_layer.h"
#include "fader_layer.h"
#include "renderer.h"

typedef struct {
    bg_layer_t *bg;
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
