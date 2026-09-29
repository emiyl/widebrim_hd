#include "event.h"

#include <stdio.h>
#include <stdlib.h>

static void mode_event_on_fade_out_done(void *user) {
    if (!user) {
        fprintf(stderr, "widebrim: mode_event_on_fade_out_done called with "
                        "NULL user pointer\n");
        return;
    }

    mode_event_impl_t *impl = (mode_event_impl_t *)user;
    game_state_set_mode(impl->base.state, MODE_TITLE);
    game_state_set_place_num(impl->base.state, 3);
    impl->base.done = true;
}

static bool mode_event_is_done(void *user) {
    if (!user) {
        fprintf(stderr,
                "widebrim: mode_event_is_done called with NULL user pointer\n");
        return false;
    }

    mode_event_impl_t *impl_ptr = (mode_event_impl_t *)user;
    return impl_ptr->base.done;
}
static void mode_event_destroy(void *user) {
    if (!user) {
        fprintf(stderr,
                "widebrim: mode_event_destroy called with NULL user pointer\n");
        return;
    }

    free(user);
}

mode_handler_t mode_event_create(game_state_t *state,
                                 screen_controller_t *screen_controller) {
    mode_handler_t handler;
    mode_event_impl_t *impl =
        (mode_event_impl_t *)malloc(sizeof(mode_event_impl_t));

    if (!state || !screen_controller) {
        fprintf(stderr,
                "widebrim: mode_event_create called with invalid state or "
                "screen_controller pointers\n");
        exit(EXIT_FAILURE);
    }

    impl->base.state = state;
    impl->base.controller = screen_controller;
    impl->base.done = false;
    game_state_reset(state);
    screen_controller_fade_out(screen_controller, FADER_DEFAULT_DURATION_MS,
                               mode_event_on_fade_out_done, impl);

    handler.layer.impl = impl;
    handler.layer.update = NULL;
    handler.layer.draw = NULL;
    handler.layer.handle_event = NULL;
    handler.layer.on_quit = NULL;
    handler.layer.destroy = mode_event_destroy;
    handler.is_done = mode_event_is_done;
    handler.valid = true;
    return handler;
}
