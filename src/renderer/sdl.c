#include "renderer.h"

#include <SDL3/SDL.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct sdl_renderer_t {
    SDL_Renderer *renderer;
    SDL_Texture **texture_registry;
    size_t texture_registry_size;
    size_t texture_registry_capacity;
} sdl_renderer_t;

struct renderer_texture_t {
    SDL_Texture *texture;
};

static void sdl_renderer_destroy(renderer_t *self) {
    sdl_renderer_t *impl = (sdl_renderer_t *)self->impl;
    size_t i;

    for (i = 0; i < impl->texture_registry_size; ++i) {
        if (impl->texture_registry[i]) {
            SDL_DestroyTexture(impl->texture_registry[i]);
        }
    }
    free(impl->texture_registry);
    SDL_DestroyRenderer(impl->renderer);
    free(impl);
    free(self);
}

static void sdl_renderer_register_texture(sdl_renderer_t *self,
                                          SDL_Texture *tex) {

    SDL_Texture **grown;
    size_t new_capacity;

    if (!tex) {
        fprintf(stderr, "widebrim: attempted to register a NULL texture\n");
        return;
    }
    if (self->texture_registry_size == self->texture_registry_capacity) {
        new_capacity = self->texture_registry_capacity * 2;
        grown = realloc(self->texture_registry,
                        new_capacity * sizeof(SDL_Texture *));
        if (!grown) {
            fprintf(stderr, "widebrim: failed to grow texture registry\n");
            return;
        }
        self->texture_registry = grown;
        self->texture_registry_capacity = new_capacity;
    }
    self->texture_registry[self->texture_registry_size++] = tex;
}

static void sdl_renderer_clear(renderer_t *self, uint8_t r, uint8_t g,
                               uint8_t b, uint8_t a) {
    sdl_renderer_t *impl = (sdl_renderer_t *)self->impl;
    SDL_SetRenderDrawColor(impl->renderer, r, g, b, a);
    SDL_RenderClear(impl->renderer);
}

static void sdl_renderer_present(renderer_t *self) {
    sdl_renderer_t *impl = (sdl_renderer_t *)self->impl;
    SDL_RenderPresent(impl->renderer);
}

static void sdl_renderer_destroy_texture(renderer_t *_r,
                                         renderer_texture_t *tex) {
    (void)_r;
    if (!tex) {
        fprintf(stderr, "widebrim: attempted to destroy a NULL texture\n");
        return;
    }
    if (tex->texture) {
        SDL_DestroyTexture(tex->texture);
        tex->texture = NULL;
    } else {
        fprintf(stderr, "widebrim: attempted to destroy a texture with a NULL "
                        "SDL_Texture\n");
    }
    free(tex);
}

static bool sdl_renderer_texture_exists(renderer_t *self,
                                        renderer_texture_t *tex) {
    sdl_renderer_t *impl = (sdl_renderer_t *)self->impl;
    if (!tex || !tex->texture) {
        return false;
    }
    for (size_t i = 0; i < impl->texture_registry_size; ++i) {
        if (impl->texture_registry[i] == tex->texture) {
            return true;
        }
    }
    return false;
}

static renderer_texture_t *
sdl_renderer_create_texture_from_rgba(renderer_t *self, const uint8_t *rgba,
                                      int width, int height) {
    sdl_renderer_t *impl = (sdl_renderer_t *)self->impl;
    renderer_texture_t *texture =
        (renderer_texture_t *)malloc(sizeof(renderer_texture_t));
    SDL_Texture *sdl_tex;

    if (!texture) {
        fprintf(stderr,
                "widebrim: failed to allocate memory for renderer_texture_t\n");
        return NULL;
    }
    if (!rgba) {
        fprintf(stderr, "widebrim: attempted to create a texture with a NULL "
                        "RGBA data\n");
        free(texture);
        return NULL;
    }

    sdl_tex = SDL_CreateTexture(impl->renderer, SDL_PIXELFORMAT_RGBA32,
                                SDL_TEXTUREACCESS_STATIC, width, height);
    if (!sdl_tex) {
        fprintf(stderr, "widebrim: failed to create SDL_Texture\n");
        free(texture);
        return NULL;
    }

    SDL_SetTextureBlendMode(sdl_tex, SDL_BLENDMODE_BLEND);
    SDL_SetTextureScaleMode(sdl_tex, SDL_SCALEMODE_LINEAR);
    if (!SDL_UpdateTexture(sdl_tex, NULL, rgba, width * 4)) {
        fprintf(stderr,
                "widebrim: failed to update SDL_Texture with RGBA data\n");
        SDL_DestroyTexture(sdl_tex);
        free(texture);
        return NULL;
    }

    sdl_renderer_register_texture(impl, sdl_tex);
    texture->texture = sdl_tex;
    return texture;
}

static void sdl_renderer_draw_texture(renderer_t *self, renderer_texture_t *tex,
                                      const rect_t *rect) {
    sdl_renderer_t *impl = (sdl_renderer_t *)self->impl;
    if (!tex || !tex->texture) {
        fprintf(stderr, "widebrim: attempted to draw a NULL texture\n");
        return;
    }
    if (!rect) {
        fprintf(stderr,
                "widebrim: attempted to draw a texture with a NULL rect\n");
        return;
    }
    SDL_FRect dst_rect = {rect->x, rect->y, rect->w, rect->h};
    SDL_RenderTexture(impl->renderer, tex->texture, NULL, &dst_rect);
}

static void sdl_renderer_draw_rect(renderer_t *self, const rect_t *rect,
                                   uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    sdl_renderer_t *impl = (sdl_renderer_t *)self->impl;
    if (!rect) {
        fprintf(stderr, "widebrim: attempted to draw a NULL rect\n");
        return;
    }
    SDL_FRect dst_rect = {rect->x, rect->y, rect->w, rect->h};

    uint8_t prev_r, prev_g, prev_b, prev_a;
    SDL_GetRenderDrawColor(impl->renderer, &prev_r, &prev_g, &prev_b, &prev_a);
    SDL_SetRenderDrawColor(impl->renderer, r, g, b, a);
    SDL_RenderRect(impl->renderer, &dst_rect);
    SDL_SetRenderDrawColor(impl->renderer, prev_r, prev_g, prev_b, prev_a);
}

static void sdl_renderer_fill_rect(renderer_t *self, const rect_t *rect,
                                   uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    sdl_renderer_t *impl = (sdl_renderer_t *)self->impl;
    if (!rect) {
        fprintf(stderr, "widebrim: attempted to fill a NULL rect\n");
        return;
    }
    SDL_FRect dst_rect = {rect->x, rect->y, rect->w, rect->h};

    uint8_t prev_r, prev_g, prev_b, prev_a;
    SDL_GetRenderDrawColor(impl->renderer, &prev_r, &prev_g, &prev_b, &prev_a);
    SDL_SetRenderDrawBlendMode(impl->renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(impl->renderer, r, g, b, a);
    SDL_RenderFillRect(impl->renderer, &dst_rect);
    SDL_SetRenderDrawColor(impl->renderer, prev_r, prev_g, prev_b, prev_a);
}

static void sdl_renderer_set_texture_alpha(renderer_t *self,
                                           renderer_texture_t *tex,
                                           uint8_t alpha) {
    (void)self;
    if (!tex || !tex->texture) {
        fprintf(stderr, "widebrim: attempted to set alpha on a NULL texture\n");
        return;
    }
    SDL_SetTextureAlphaMod(tex->texture, alpha);
}

static void sdl_renderer_get_texture_size(renderer_t *self,
                                          renderer_texture_t *texture,
                                          int *width, int *height) {
    (void)self;
    if (!texture || !texture->texture) {
        fprintf(stderr, "widebrim: attempted to get size of a NULL texture\n");
        return;
    }

    float fw = 0.0f, fh = 0.0f;

    if (width)
        *width = 0;
    if (height)
        *height = 0;
    if (SDL_GetTextureSize(texture->texture, &fw, &fh)) {
        if (width)
            *width = (int)fw;
        if (height)
            *height = (int)fh;
    } else {
        fprintf(stderr, "widebrim: failed to get texture size\n");
    }
}

static const renderer_vtable_t g_sdl_renderer_vtable = {
    .destroy = sdl_renderer_destroy,
    .clear = sdl_renderer_clear,
    .present = sdl_renderer_present,
    .create_texture_from_rgba = sdl_renderer_create_texture_from_rgba,
    .destroy_texture = sdl_renderer_destroy_texture,
    .texture_exists = sdl_renderer_texture_exists,
    .draw_texture = sdl_renderer_draw_texture,
    .draw_rect = sdl_renderer_draw_rect,
    .fill_rect = sdl_renderer_fill_rect,
    .set_texture_alpha = sdl_renderer_set_texture_alpha,
    .get_texture_size = sdl_renderer_get_texture_size,
};

renderer_t *renderer_create_sdl(void *sdl_Renderer) {
    renderer_t *renderer = (renderer_t *)malloc(sizeof(renderer_t));
    if (!renderer) {
        fprintf(stderr,
                "widebrim: failed to allocate memory for SDL renderer\n");
        return NULL;
    }

    sdl_renderer_t *impl = (sdl_renderer_t *)malloc(sizeof(sdl_renderer_t));
    if (!impl) {
        fprintf(stderr, "widebrim: failed to allocate memory for SDL renderer "
                        "implementation\n");
        free(renderer);
        return NULL;
    }

    impl->renderer = (SDL_Renderer *)sdl_Renderer;
    renderer->impl = impl;
    renderer->vt = &g_sdl_renderer_vtable;

    return renderer;
}
