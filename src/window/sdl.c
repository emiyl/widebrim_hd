#include "window.h"

#include <SDL3/SDL.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct sdl_window_t {
    SDL_Window *window;
    SDL_Renderer *renderer;
} sdl_window_t;

static void sdl_window_destroy(window_t *self) {
    if (!self) {
        fprintf(stderr, "widebrim: sdl_window_destroy called with NULL self\n");
        return;
    }
    sdl_window_t *sdl_window = (sdl_window_t *)self->impl;
    if (sdl_window) {
        if (sdl_window->renderer) {
            SDL_DestroyRenderer(sdl_window->renderer);
        }
        if (sdl_window->window) {
            SDL_DestroyWindow(sdl_window->window);
        }
        free(sdl_window);
    }
    free(self);
}

static void *sdl_window_as_sdl3_renderer(const window_t *window) {
    sdl_window_t *impl = (sdl_window_t *)window->impl;
    return impl ? (void *)impl->renderer : NULL;
}

static void sdl_window_set_scale(window_t *self, float x_scale, float y_scale) {
    if (!self) {
        return;
    }
    sdl_window_t *sdl_window = (sdl_window_t *)self->impl;
    if (sdl_window && sdl_window->renderer) {
        SDL_SetRenderScale(sdl_window->renderer, x_scale, y_scale);
    }
}

static void
sdl_window_convert_event_to_render_coordinates(window_t *self,
                                               input_event_t *event) {
    if (!self) {
        fprintf(stderr,
                "widebrim: sdl_window_convert_event_to_render_coordinates "
                "called with NULL self\n");
        return;
    }

    sdl_window_t *impl = (sdl_window_t *)self->impl;
    if (!impl || !impl->renderer || !event) {
        fprintf(stderr,
                "widebrim: sdl_window_convert_event_to_render_coordinates "
                "called with NULL impl, renderer, or event\n");
        return;
    }

    SDL_Event sdl_event;
    SDL_WindowID window_id = SDL_GetWindowID(impl->window);

    sdl_event.button.windowID = window_id;
    sdl_event.motion.windowID = window_id;
    sdl_event.key.windowID = window_id;

    switch (event->type) {
    case INPUT_EVENT_QUIT:
        sdl_event.type = SDL_EVENT_QUIT;
        break;
    case INPUT_EVENT_KEY_DOWN:
        sdl_event.type = SDL_EVENT_KEY_DOWN;
        sdl_event.key.key = (SDL_Keycode)event->data.key.key;
        break;
    case INPUT_EVENT_KEY_UP:
        sdl_event.type = SDL_EVENT_KEY_UP;
        sdl_event.key.key = (SDL_Keycode)event->data.key.key;
        break;
    case INPUT_EVENT_MOUSE_MOTION:
        sdl_event.type = SDL_EVENT_MOUSE_MOTION;
        sdl_event.motion.x = event->data.mouse_motion.x;
        sdl_event.motion.y = event->data.mouse_motion.y;
        sdl_event.motion.xrel = event->data.mouse_motion.dx;
        sdl_event.motion.yrel = event->data.mouse_motion.dy;
        break;
    case INPUT_EVENT_MOUSE_BUTTON_DOWN:
        sdl_event.type = SDL_EVENT_MOUSE_BUTTON_DOWN;
        sdl_event.button.button = (Uint8)event->data.mouse_button.button;
        sdl_event.button.x = event->data.mouse_button.x;
        sdl_event.button.y = event->data.mouse_button.y;
        break;
    case INPUT_EVENT_MOUSE_BUTTON_UP:
        sdl_event.type = SDL_EVENT_MOUSE_BUTTON_UP;
        sdl_event.button.button = (Uint8)event->data.mouse_button.button;
        sdl_event.button.x = event->data.mouse_button.x;
        sdl_event.button.y = event->data.mouse_button.y;
        break;
    case INPUT_EVENT_NONE:
        break;
    default:
        fprintf(stderr,
                "widebrim: sdl_window_convert_event_to_render_coordinates "
                "called with unknown event type %d\n",
                event->type);
        break;
    }

    SDL_ConvertEventToRenderCoordinates(impl->renderer, &sdl_event);
    switch (sdl_event.type) {
    case SDL_EVENT_KEY_DOWN:
    case SDL_EVENT_KEY_UP:
        event->data.key.key = (int)sdl_event.key.key;
        break;
    case SDL_EVENT_MOUSE_MOTION:
        event->data.mouse_motion.x = sdl_event.motion.x;
        event->data.mouse_motion.y = sdl_event.motion.y;
        event->data.mouse_motion.dx = sdl_event.motion.xrel;
        event->data.mouse_motion.dy = sdl_event.motion.yrel;
        break;
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
    case SDL_EVENT_MOUSE_BUTTON_UP:
        event->data.mouse_button.button = (int)sdl_event.button.button;
        event->data.mouse_button.x = sdl_event.button.x;
        event->data.mouse_button.y = sdl_event.button.y;
        break;
    default:
        break;
    }
}

static const window_vtable g_sdl_window_vtable = {
    .destroy = sdl_window_destroy,
    .as_sdl3_renderer = sdl_window_as_sdl3_renderer,
    .set_scale = sdl_window_set_scale,
    .convert_event_to_render_coordinates =
        sdl_window_convert_event_to_render_coordinates};

window_t *window_create_sdl(const char *title, int width, int height,
                            unsigned int flags) {
    window_t *window = (window_t *)calloc(1u, sizeof(*window));
    if (!window) {
        fprintf(stderr, "widebrim: failed to allocate memory for SDL window\n");
        return NULL;
    }
    sdl_window_t *impl = (sdl_window_t *)calloc(1u, sizeof(*impl));

    if (!impl) {
        fprintf(stderr, "widebrim: failed to allocate memory for SDL window "
                        "implementation\n");
        free(window);
        return NULL;
    }

    if (!SDL_CreateWindowAndRenderer(title, width, height, flags, &impl->window,
                                     &impl->renderer)) {
        fprintf(stderr, "widebrim: failed to create SDL window and renderer\n");
        free(impl);
        free(window);
        return NULL;
    }

    window->impl = impl;
    window->vt = &g_sdl_window_vtable;
    return window;
}
