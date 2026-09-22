#ifndef FADER_LAYER_H
#define FADER_LAYER_H

#include <fader_layer.h>

#include <SDL3/SDL.h>

#include "screen.h"

typedef void (*fader_callback)(void *user);

typedef struct {
    float alpha;
    float start;
    float target;
    float duration_ms;
    float elapsed_ms;
    bool active;
    bool flash_white;
    fader_callback callback;
    void *callback_user;
} fader_timeline_t;

typedef struct {
    fader_timeline_t main_fade;
    fader_timeline_t sub_fade;
    float wait_remaining_ms;
    bool wait_can_be_skipped;
} fader_layer_t;

#define FADER_DEFAULT_DURATION_MS 250.0f

void fader_layer_init(fader_layer_t *layer);

void fader_layer_fade_out_main(fader_layer_t *layer, float duration_ms,
                               fader_callback cb, void *user);
void fader_layer_fade_out_sub(fader_layer_t *layer, float duration_ms,
                              fader_callback cb, void *user);
void fader_layer_flash_main(fader_layer_t *layer, float duration_ms,
                            fader_callback cb, void *user);
void fader_layer_flash_sub(fader_layer_t *layer, float duration_ms,
                           fader_callback cb, void *user);
void fader_layer_fade_in_main(fader_layer_t *layer, float duration_ms,
                              fader_callback cb, void *user);
void fader_layer_fade_in_sub(fader_layer_t *layer, float duration_ms,
                             fader_callback cb, void *user);
void fader_layer_fade_out(fader_layer_t *layer, float duration_ms,
                          fader_callback cb, void *user);
void fader_layer_fade_in(fader_layer_t *layer, float duration_ms,
                         fader_callback cb, void *user);

void fader_layer_set_wait_duration(fader_layer_t *layer, float duration_ms,
                                   bool can_be_skipped);

bool fader_layer_is_fading(const fader_layer_t *layer);
bool fader_layer_is_view_obscured(const fader_layer_t *layer);

screen_layer_t fader_layer_as_screen_layer(fader_layer_t *layer);

#endif // FADER_LAYER_H
