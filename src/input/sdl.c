#include "input.h"

#include <SDL3/SDL.h>

#include <stdlib.h>
#include <string.h>

typedef struct sdl_input_t {
    bool unused;
} sdl_input_t;

static void sdl_input_event_from_sdl(const SDL_Event *sdl_event,
                                     input_event_t *event) {
    if (!event) {
        fprintf(stderr, "widebrim: input event is NULL\n");
        return;
    }
    memset(event, 0, sizeof(*event));

    switch (sdl_event->type) {
    case SDL_EVENT_QUIT:
        event->type = INPUT_EVENT_QUIT;
        break;
    case SDL_EVENT_KEY_DOWN:
        event->type = INPUT_EVENT_KEY_DOWN;
        event->data.key.key = sdl_event->key.key;
        break;
    case SDL_EVENT_KEY_UP:
        event->type = INPUT_EVENT_KEY_UP;
        event->data.key.key = sdl_event->key.key;
        break;
    case SDL_EVENT_MOUSE_MOTION:
        event->type = INPUT_EVENT_MOUSE_MOTION;
        event->data.mouse_motion.x = sdl_event->motion.x;
        event->data.mouse_motion.y = sdl_event->motion.y;
        event->data.mouse_motion.dx = sdl_event->motion.xrel;
        event->data.mouse_motion.dy = sdl_event->motion.yrel;
        break;
    case SDL_EVENT_MOUSE_BUTTON_UP:
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
        if (sdl_event->type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
            event->type = INPUT_EVENT_MOUSE_BUTTON_DOWN;
        } else {
            event->type = INPUT_EVENT_MOUSE_BUTTON_UP;
        }
        event->data.mouse_button.x = sdl_event->button.x;
        event->data.mouse_button.y = sdl_event->button.y;
        event->data.mouse_button.button = sdl_event->button.button;
        break;
    }
}

static void sdl_input_destroy(input_t *input) {
    if (!input) {
        return;
    }
    free(input->impl);
    free(input);
}

static bool sdl_input_poll_event(input_t *input, input_event_t *event) {
    SDL_Event sdl_event;

    (void)input;
    if (SDL_PollEvent(&sdl_event)) {
        sdl_input_event_from_sdl(&sdl_event, event);
        return true;
    }
    return false;
}

static const input_vtable_t g_sdl_input_vtable = {
    .destroy = sdl_input_destroy,
    .poll_event = sdl_input_poll_event,
};

input_t *input_create_sdl(void) {
    input_t *input = (input_t *)malloc(sizeof(input_t));
    if (!input) {
        return NULL;
    }

    sdl_input_t *impl = (sdl_input_t *)malloc(sizeof(sdl_input_t));
    if (!impl) {
        free(input);
        return NULL;
    }

    input->impl = impl;
    input->vt = &g_sdl_input_vtable;
    return input;
}
