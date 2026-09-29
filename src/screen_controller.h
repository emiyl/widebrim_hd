#ifndef SCREEN_CONTROLLER_H
#define SCREEN_CONTROLLER_H

#include <stdlib.h>

#include "bg_layer.h"
#include "fader_layer.h"
#include "game_state.h"
#include "object_layer.h"
#include "renderer.h"
#include "text_layer.h"

typedef struct {
    bg_layer_t *bg;
    object_layer_t *object;
    text_layer_t *text;
    fader_layer_t *fader;
    renderer_t *renderer;
} screen_controller_t;

static inline void screen_controller_clear_bg_layer(screen_controller_t *sc) {
    if (!sc || !sc->bg) {
        return;
    }
    bg_layer_init(sc->bg, sc->renderer);
}

static inline void
screen_controller_clear_object_layer(screen_controller_t *sc) {
    if (!sc || !sc->object) {
        return;
    }
    object_layer_clear(sc->object);
}

static inline bool screen_controller_load_font(screen_controller_t *sc,
                                               game_state_t *state,
                                               const char *rel_path) {
    char resolved[4096];

    if (!sc || !sc->text || !state || !rel_path) {
        return false;
    }

    if (!asset_path_resolve(state, rel_path, resolved, sizeof(resolved))) {
        fprintf(stderr, "widebrim: failed to resolve font path for asset: %s\n",
                rel_path);
        return false;
    }

    return text_layer_load_font_file(sc->text, resolved);
}

static inline bool screen_controller_load_default_font(screen_controller_t *sc,
                                                       game_state_t *state) {
    return screen_controller_load_font(sc, state, "data/font/font.dat");
}

static inline text_instance_t *
screen_controller_add_text(screen_controller_t *sc, int x, int y,
                           const char *text) {
    if (!sc || !sc->text) {
        return NULL;
    }
    return text_layer_add_text(sc->text, x, y, text);
}

static inline bool screen_controller_set_text_position(screen_controller_t *sc,
                                                       text_instance_t *text,
                                                       int x, int y) {
    (void)sc;
    return text_layer_set_text_position(text, x, y);
}

static inline bool screen_controller_set_text_contents(screen_controller_t *sc,
                                                       text_instance_t *text,
                                                       const char *string) {
    (void)sc;
    return text_layer_set_text_contents(text, string);
}

static inline bool screen_controller_set_text_visible(screen_controller_t *sc,
                                                      text_instance_t *text,
                                                      bool visible) {
    (void)sc;
    return text_layer_set_visible(text, visible);
}

static inline bool screen_controller_draw_text(screen_controller_t *sc, int x,
                                               int y, const char *text) {
    if (!sc || !sc->text || !text) {
        return false;
    }
    return text_layer_set_text(sc->text, x, y, text);
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
