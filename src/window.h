#ifndef WINDOW_H
#define WINDOW_H

#include <stddef.h>

#include "input.h"

typedef struct window_t window_t;

typedef struct window_vtable {
    void (*destroy)(window_t *self);
    void *(*as_sdl3_renderer)(const window_t *self);
    void (*set_scale)(window_t *self, float x_scale, float y_scale);
    void (*convert_event_to_render_coordinates)(window_t *self,
                                                input_event_t *event);
} window_vtable;

struct window_t {
    void *impl;
    const window_vtable *vt;
};

window_t *window_create_sdl(const char *title, int width, int height,
                            unsigned int flags);

static inline void window_destroy(window_t *self) {
    if (self && self->vt && self->vt->destroy) {
        self->vt->destroy(self);
    }
}

static inline void window_set_scale(window_t *self, float x_scale,
                                    float y_scale) {
    if (self && self->vt && self->vt->set_scale) {
        self->vt->set_scale(self, x_scale, y_scale);
    }
}

static inline void
window_convert_event_to_render_coordinates(window_t *self,
                                           input_event_t *event) {
    if (self && self->vt && self->vt->convert_event_to_render_coordinates) {
        self->vt->convert_event_to_render_coordinates(self, event);
    }
}

static inline void *window_as_sdl3_renderer(const window_t *self) {
    if (self && self->vt && self->vt->as_sdl3_renderer) {
        return self->vt->as_sdl3_renderer(self);
    }
    return NULL;
}

#endif // WINDOW_H
