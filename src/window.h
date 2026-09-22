#ifndef WINDOW_H
#define WINDOW_H

#include <stddef.h>

#include "input.h"

typedef struct window_t window_t;

typedef enum {
    WB_LOGICAL_PRESENTATION_DISABLED = 0,
    WB_LOGICAL_PRESENTATION_STRETCH = 1,
    WB_LOGICAL_PRESENTATION_LETTERBOX = 2,
    WB_LOGICAL_PRESENTATION_OVERSCAN = 3,
    WB_LOGICAL_PRESENTATION_INTEGER_SCALE = 4
} window_logical_presentation;

typedef enum {
    WB_SCALE_MODE_NEAREST = 0,
} window_scale_mode;

typedef struct window_vtable {
    void (*destroy)(window_t *self);
    void (*set_logical_presentation)(window_t *self,
                                     window_logical_presentation presentation);
    void (*set_scale_mode)(window_t *self, window_scale_mode scale_mode);
    void (*convert_event_to_render_coordinates)(window_t *self,
                                                input_event_t *event);
    void *(*as_native_window)(const window_t *self);
    void *(*as_native_renderer)(const window_t *self);
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

static inline void
window_set_logical_presentation(window_t *self,
                                window_logical_presentation presentation) {
    if (self && self->vt && self->vt->set_logical_presentation) {
        self->vt->set_logical_presentation(self, presentation);
    }
}

static inline void window_set_scale_mode(window_t *self,
                                         window_scale_mode scale_mode) {
    if (self && self->vt && self->vt->set_scale_mode) {
        self->vt->set_scale_mode(self, scale_mode);
    }
}

static inline void
window_convert_event_to_render_coordinates(window_t *self,
                                           input_event_t *event) {
    if (self && self->vt && self->vt->convert_event_to_render_coordinates) {
        self->vt->convert_event_to_render_coordinates(self, event);
    }
}

static inline void *window_as_native_window(const window_t *self) {
    if (self && self->vt && self->vt->as_native_window) {
        return self->vt->as_native_window(self);
    }
    return NULL;
}

static inline void *window_as_native_renderer(const window_t *self) {
    if (self && self->vt && self->vt->as_native_renderer) {
        return self->vt->as_native_renderer(self);
    }
    return NULL;
}

#endif // WINDOW_H
