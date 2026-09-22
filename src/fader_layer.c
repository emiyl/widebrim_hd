#include "fader_layer.h"

#include "bg_layer.h"
#include "renderer.h"

static void fader_timeline_start(fader_timeline_t *tl, float target,
                                 float duration_ms, bool flash_white,
                                 fader_callback cb, void *user) {
    tl->start = tl->alpha;
    tl->target = target;
    tl->duration_ms = duration_ms > 0.0f ? duration_ms : 1.0f;
    tl->elapsed_ms = 0.0f;
    tl->active = true;
    tl->flash_white = flash_white;
    tl->callback = cb;
    tl->callback_user = user;
}

static void fader_timeline_update(fader_timeline_t *tl, float delta_ms) {
    float t;
    fader_callback cb;
    void *user;

    if (!tl->active) {
        return;
    }

    tl->elapsed_ms += delta_ms;
    t = tl->elapsed_ms / tl->duration_ms;
    if (t >= 1.0f) {
        tl->alpha = tl->target;
        tl->active = false;
        cb = tl->callback;
        user = tl->callback_user;
        tl->callback = NULL;
        tl->callback_user = NULL;
        if (cb) {
            cb(user);
        }
    } else {
        tl->alpha = tl->start + (tl->target - tl->start) * t;
    }
}

void fader_layer_init(fader_layer_t *fader) {
    fader->main_fade.alpha = 0.0f;
    fader->main_fade.active = false;
    fader->main_fade.callback = NULL;
    fader->main_fade.callback_user = NULL;
    fader->sub_fade = fader->main_fade;
    fader->wait_remaining_ms = 0.0f;
    fader->wait_can_be_skipped = false;
}

void fader_layer_fade_out_main(fader_layer_t *fader, float duration_ms,
                               fader_callback cb, void *user) {
    fader_timeline_start(&fader->main_fade, 255.0f, duration_ms, false, cb,
                         user);
}

void fader_layer_fade_in_main(fader_layer_t *fader, float duration_ms,
                              fader_callback cb, void *user) {
    fader_timeline_start(&fader->main_fade, 0.0f, duration_ms, false, cb, user);
}

void fader_layer_flash_main(fader_layer_t *fader, float duration_ms,
                            fader_callback cb, void *user) {
    fader_timeline_start(&fader->main_fade, 255.0f, duration_ms, true, cb,
                         user);
}

void fader_layer_flash_sub(fader_layer_t *fader, float duration_ms,
                           fader_callback cb, void *user) {
    fader_timeline_start(&fader->sub_fade, 255.0f, duration_ms, true, cb, user);
}

void fader_layer_fade_out_sub(fader_layer_t *fader, float duration_ms,
                              fader_callback cb, void *user) {
    fader_timeline_start(&fader->sub_fade, 255.0f, duration_ms, false, cb,
                         user);
}

void fader_layer_fade_in_sub(fader_layer_t *fader, float duration_ms,
                             fader_callback cb, void *user) {
    fader_timeline_start(&fader->sub_fade, 0.0f, duration_ms, false, cb, user);
}

void fader_layer_fade_out(fader_layer_t *fader, float duration_ms,
                          fader_callback cb, void *user) {
    fader_layer_fade_out_sub(fader, duration_ms, NULL, NULL);
    fader_layer_fade_out_main(fader, duration_ms, cb, user);
}

void fader_layer_fade_in(fader_layer_t *fader, float duration_ms,
                         fader_callback cb, void *user) {
    fader_layer_fade_in_sub(fader, duration_ms, NULL, NULL);
    fader_layer_fade_in_main(fader, duration_ms, cb, user);
}

void fader_layer_set_wait_duration(fader_layer_t *fader, float duration_ms,
                                   bool can_be_skipped) {
    fader->wait_remaining_ms = duration_ms;
    fader->wait_can_be_skipped = can_be_skipped;
}

bool fader_layer_is_fading(const fader_layer_t *fader) {
    return fader->main_fade.active || fader->sub_fade.active;
}

bool fader_layer_is_view_obscured(const fader_layer_t *fader) {
    return fader->main_fade.alpha >= 255.0f && fader->sub_fade.alpha >= 255.0f;
}

static void fader_layer_update_impl(void *impl, float dt_ms) {
    fader_layer_t *fader = (fader_layer_t *)impl;
    fader_timeline_update(&fader->main_fade, dt_ms);
    fader_timeline_update(&fader->sub_fade, dt_ms);
    if (fader->wait_remaining_ms > 0.0f) {
        fader->wait_remaining_ms -= dt_ms;
    }
}

static void fader_layer_draw_rect(renderer_t *renderer_instance,
                                  const fader_timeline_t *tl, int y_offset) {
    rect_t rect;
    uint8_t alpha;

    if (tl->alpha <= 0.0f) {
        return;
    }

    alpha = tl->alpha >= 255.0f ? 255 : (uint8_t)tl->alpha;
    rect.x = 0.0f;
    rect.y = (float)y_offset;
    rect.w = (float)WB_SCREEN_WIDTH;
    rect.h = (float)WB_SCREEN_HEIGHT;

    if (tl->flash_white) {
        renderer_fill_rect(renderer_instance, &rect, 255, 255, 255, alpha);
    } else {
        renderer_fill_rect(renderer_instance, &rect, 0, 0, 0, alpha);
    }
}

static void fader_layer_draw_impl(void *impl, renderer_t *renderer_instance) {
    fader_layer_t *fader = (fader_layer_t *)impl;
    fader_layer_draw_rect(renderer_instance, &fader->sub_fade, 0);
    fader_layer_draw_rect(renderer_instance, &fader->main_fade,
                          WB_SCREEN_HEIGHT);
}

static bool fader_layer_handle_event_impl(void *impl,
                                          const input_event_t *event) {
    fader_layer_t *fader = (fader_layer_t *)impl;
    if (fader->wait_remaining_ms > 0.0f && fader->wait_can_be_skipped &&
        event && event->type == INPUT_EVENT_MOUSE_BUTTON_DOWN) {
        fader->wait_remaining_ms = 0.0f;
        return true;
    }
    return false;
}

screen_layer_t fader_layer_as_screen_layer(fader_layer_t *fader) {
    screen_layer_t layer;
    layer.impl = fader;
    layer.update = fader_layer_update_impl;
    layer.draw = fader_layer_draw_impl;
    layer.handle_event = fader_layer_handle_event_impl;
    layer.on_quit = NULL;
    layer.destroy = NULL;
    return layer;
}
