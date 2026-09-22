#ifndef RENDERER_H
#define RENDERER_H

#include <stddef.h>
#include <stdint.h>

typedef struct {
    float x;
    float y;
    float w;
    float h;
} rect_t;

typedef struct renderer_texture_t renderer_texture_t;
typedef struct renderer_t renderer_t;

typedef struct renderer_vtable {
    void (*destroy)(renderer_t *self);
    void (*clear)(renderer_t *self, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
    void (*present)(renderer_t *self);
    renderer_texture_t *(*create_texture_from_rgba)(renderer_t *self,
                                                    const uint8_t *rgba,
                                                    int width, int height);
    void (*destroy_texture)(renderer_t *self, renderer_texture_t *texture);
    void (*draw_texture)(renderer_t *self, renderer_texture_t *texture,
                         const rect_t *dst);
    void (*draw_rect)(renderer_t *self, const rect_t *rect, uint8_t r,
                      uint8_t g, uint8_t b, uint8_t a);
    void (*fill_rect)(renderer_t *self, const rect_t *rect, uint8_t r,
                      uint8_t g, uint8_t b, uint8_t a);
    void (*set_texture_alpha)(renderer_t *self, renderer_texture_t *texture,
                              uint8_t alpha);
    void (*get_texture_size)(renderer_t *self, renderer_texture_t *texture,
                             int *width, int *height);
} renderer_vtable_t;

struct renderer_t {
    void *impl;
    const renderer_vtable_t *vt;
};

renderer_t *renderer_create_sdl(void *renderer);

static inline void renderer_destroy(renderer_t *self) {
    if (self && self->vt && self->vt->destroy) {
        self->vt->destroy(self);
    }
}

static inline void renderer_clear(renderer_t *self, uint8_t r, uint8_t g,
                                  uint8_t b, uint8_t a) {
    if (self && self->vt && self->vt->clear) {
        self->vt->clear(self, r, g, b, a);
    }
}

static inline void renderer_present(renderer_t *self) {
    if (self && self->vt && self->vt->present) {
        self->vt->present(self);
    }
}

static inline renderer_texture_t *
renderer_create_texture_from_rgba(renderer_t *self, const uint8_t *rgba,
                                  int width, int height) {
    if (self && self->vt && self->vt->create_texture_from_rgba) {
        return self->vt->create_texture_from_rgba(self, rgba, width, height);
    }
    return NULL;
}

static inline void renderer_draw_texture(renderer_t *self,
                                         renderer_texture_t *texture,
                                         const rect_t *dst) {
    if (self && self->vt && self->vt->draw_texture) {
        self->vt->draw_texture(self, texture, dst);
    }
}

static inline void renderer_destroy_texture(renderer_t *self,
                                            renderer_texture_t *texture) {
    if (self && self->vt && self->vt->destroy_texture) {
        self->vt->destroy_texture(self, texture);
    }
}

static inline void renderer_draw_rect(renderer_t *self, const rect_t *rect,
                                      uint8_t r, uint8_t g, uint8_t b,
                                      uint8_t a) {
    if (self && self->vt && self->vt->draw_rect) {
        self->vt->draw_rect(self, rect, r, g, b, a);
    }
}

static inline void renderer_fill_rect(renderer_t *self, const rect_t *rect,
                                      uint8_t r, uint8_t g, uint8_t b,
                                      uint8_t a) {
    if (self && self->vt && self->vt->fill_rect) {
        self->vt->fill_rect(self, rect, r, g, b, a);
    }
}

static inline void renderer_set_texture_alpha(renderer_t *self,
                                              renderer_texture_t *texture,
                                              uint8_t alpha) {
    if (self && self->vt && self->vt->set_texture_alpha) {
        self->vt->set_texture_alpha(self, texture, alpha);
    }
}

static inline void renderer_get_texture_size(renderer_t *self,
                                             renderer_texture_t *texture,
                                             int *width, int *height) {
    if (self && self->vt && self->vt->get_texture_size) {
        self->vt->get_texture_size(self, texture, width, height);
    }
}

#endif // RENDERER_H
