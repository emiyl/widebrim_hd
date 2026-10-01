#include <stdio.h>

#include "../safe.h"
#include "../window.h"

typedef struct none_window_t {
    void *window;
    void *renderer;
} none_window_t;

static void none_window_destroy(window_t *self) {
    none_window_t *impl;

    if (!self) {
        fprintf(stderr, "Error: NULL window passed to none_window_destroy\n");
        return;
    }

    impl = (none_window_t *)self->impl;
    if (impl) {
        free(impl);
    }
    free(self);
}

static void *none_window_as_sdl3_renderer(const window_t *self) {
    (void)self;
    return NULL;
}

static void none_window_set_scale(window_t *self, float x_scale,
                                  float y_scale) {
    (void)self;
    (void)x_scale;
    (void)y_scale;
}

static void
none_window_convert_event_to_render_coordinates(window_t *self,
                                                input_event_t *event) {
    (void)self;
    (void)event;
}

static const window_vtable g_none_window_vtable = {
    .destroy = none_window_destroy,
    .as_sdl3_renderer = none_window_as_sdl3_renderer,
    .set_scale = none_window_set_scale,
    .convert_event_to_render_coordinates =
        none_window_convert_event_to_render_coordinates,
};

window_t *window_create_none(void) {
    window_t *window = (window_t *)smalloc(sizeof(window_t));
    none_window_t *impl = (none_window_t *)smalloc(sizeof(none_window_t));
    window->impl = impl;
    window->vt = &g_none_window_vtable;
    return window;
}
