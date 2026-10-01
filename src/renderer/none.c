
#include <stdio.h>
#include <stdlib.h>

#include "../renderer.h"
#include "../safe.h"

typedef struct none_renderer_t {
    void *renderer;
    int *texture_registry;
    size_t texture_registry_size;
    size_t texture_registry_capacity;
} none_renderer_t;
struct renderer_texture_t {
    int texture;
};

static void none_renderer_destroy(renderer_t *self) {
    none_renderer_t *impl;

    if (!self) {
        return;
    }

    impl = (none_renderer_t *)self->impl;
    if (impl) {
        free(impl->texture_registry);
        free(impl);
    }
    free(self);
}

static void none_renderer_register_texture(none_renderer_t *self,
                                           renderer_texture_t *tex) {
    int *grown;
    size_t new_capacity;

    if (!tex) {
        fprintf(stderr, "Error: Attempted to register a NULL texture.\n");
        return;
    }
    if (self->texture_registry_size >= self->texture_registry_capacity) {
        new_capacity = self->texture_registry_capacity == 0
                           ? 4
                           : self->texture_registry_capacity * 2;
        grown = realloc(self->texture_registry, new_capacity * sizeof(int));
        if (!grown) {
            fprintf(stderr, "Error: Failed to grow texture registry.\n");
            return;
        }
        self->texture_registry = grown;
        self->texture_registry_capacity = new_capacity;
    }
    self->texture_registry[self->texture_registry_size++] = tex->texture;
}

static void none_renderer_unregister_texture(none_renderer_t *self,
                                             renderer_texture_t *texture) {
    size_t i;

    if (!self || !texture) {
        fprintf(stderr, "Error: Attempted to unregister a NULL texture or from "
                        "a NULL renderer.\n");
        return;
    }

    for (i = 0; i < self->texture_registry_size; ++i) {
        if (self->texture_registry[i] == texture->texture) {
            self->texture_registry[i] =
                self->texture_registry[--self->texture_registry_size];
            return;
        }
    }
}

static void none_renderer_destroy_texture(renderer_t *self,
                                          renderer_texture_t *tex) {
    none_renderer_t *none_self = (none_renderer_t *)self->impl;
    if (!none_self || !tex) {
        fprintf(stderr, "Error: Attempted to destroy a NULL texture or from a "
                        "NULL renderer.\n");
        return;
    }

    if (tex->texture >= 0) {
        none_renderer_unregister_texture(none_self, tex);
        tex->texture = -1;
    } else {
        fprintf(stderr, "Warning: Attempted to destroy a texture that was not "
                        "registered.\n");
    }

    free(tex);
}

static bool none_renderer_texture_exists(renderer_t *self,
                                         renderer_texture_t *texture) {
    none_renderer_t *none_self = (none_renderer_t *)self->impl;
    if (!none_self || !texture) {
        return false;
    }

    for (size_t i = 0; i < none_self->texture_registry_size; ++i) {
        if (none_self->texture_registry[i] == texture->texture) {
            return true;
        }
    }
    return false;
}

static renderer_texture_t *
none_renderer_create_texture_from_rgba(renderer_t *self, const uint8_t *rgba,
                                       int width, int height) {
    (void)rgba;
    (void)width;
    (void)height;
    none_renderer_t *impl = (none_renderer_t *)self->impl;
    renderer_texture_t *tex = malloc(sizeof(renderer_texture_t));
    if (!tex) {
        fprintf(stderr, "Error: Failed to allocate memory for texture.\n");
        return NULL;
    }
    tex->texture = impl->texture_registry_size;
    none_renderer_register_texture(impl, tex);
    return tex;
}

static void none_renderer_draw_texture(renderer_t *self,
                                       renderer_texture_t *texture,
                                       const rect_t *rect) {
    (void)self;
    (void)texture;
    (void)rect;
}

static void none_renderer_draw_rect(renderer_t *self, const rect_t *rect,
                                    uint8_t r, uint8_t g, uint8_t b,
                                    uint8_t a) {
    (void)self;
    (void)rect;
    (void)r;
    (void)g;
    (void)b;
    (void)a;
}

static void none_renderer_fill_rect(renderer_t *self, const rect_t *rect,
                                    uint8_t r, uint8_t g, uint8_t b,
                                    uint8_t a) {
    (void)self;
    (void)rect;
    (void)r;
    (void)g;
    (void)b;
    (void)a;
}

static void none_renderer_set_texture_alpha(renderer_t *self,
                                            renderer_texture_t *texture,
                                            uint8_t alpha) {
    (void)self;
    (void)texture;
    (void)alpha;
}

static void none_renderer_get_texture_size(renderer_t *self,
                                           renderer_texture_t *texture,
                                           int *width, int *height) {
    (void)self;
    (void)texture;
    if (width) {
        *width = 0;
    }
    if (height) {
        *height = 0;
    }
}

static void none_renderer_clear(renderer_t *self, uint8_t r, uint8_t g,
                                uint8_t b, uint8_t a) {
    (void)self;
    (void)r;
    (void)g;
    (void)b;
    (void)a;
}

static void none_renderer_present(renderer_t *self) { (void)self; }

static const renderer_vtable_t g_none_renderer_vtable = {
    .destroy = none_renderer_destroy,
    .clear = none_renderer_clear,
    .present = none_renderer_present,
    .create_texture_from_rgba = none_renderer_create_texture_from_rgba,
    .destroy_texture = none_renderer_destroy_texture,
    .texture_exists = none_renderer_texture_exists,
    .draw_texture = none_renderer_draw_texture,
    .draw_rect = none_renderer_draw_rect,
    .fill_rect = none_renderer_fill_rect,
    .set_texture_alpha = none_renderer_set_texture_alpha,
    .get_texture_size = none_renderer_get_texture_size,
};

renderer_t *renderer_create_none(void *none_renderer) {
    renderer_t *renderer = (renderer_t *)smalloc(sizeof(renderer_t));
    none_renderer_t *impl = (none_renderer_t *)smalloc(sizeof(none_renderer_t));

    impl->renderer = none_renderer;
    impl->texture_registry = NULL;
    impl->texture_registry_size = 0U;
    impl->texture_registry_capacity = 0U;
    renderer->impl = impl;
    renderer->vt = &g_none_renderer_vtable;

    return renderer;
}
