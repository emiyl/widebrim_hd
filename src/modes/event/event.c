#include "event.h"

#include <stdio.h>
#include <stdlib.h>

#include "safe.h"
#include "script.h"

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

static bool mode_event_load_script(mode_event_impl_t *impl) {
    char script_path[64];

    if (!impl || !impl->base.state) {
        return false;
    }

    snprintf(script_path, sizeof(script_path), "script/event/e%d.gds",
             game_state_get_event_id(impl->base.state));
    return script_load_and_execute(impl->base.state, script_path, impl,
                                   &impl->base.state->gds);
}

static void mode_event_load_center_line(mode_event_impl_t *impl) {
    if (!impl || !impl->base.controller) {
        return;
    }

    renderer_t *renderer = impl->base.controller->renderer;
    game_state_t *state = impl->base.state;

    object_t *line = impl->center_line;

    if (!line)
        return;

    object_clear(line, renderer);

    sprite_t *spr = line->sprite;
    sprite_new(spr, renderer, state, "center_line.spr", 0.0f, false);

    if (!spr) {
        fprintf(stderr, "widebrim: failed to create center line sprite\n");
        return;
    }

    int spr_w, spr_h;
    sprite_get_size(spr, renderer, &spr_w, &spr_h);

    object_set_size(line, spr_w, spr_h);
    object_center_position(line, WB_SCREEN_WIDTH, WB_SCREEN_HEIGHT * 2);
    sprite_take_object_position(spr, line);
}

static void mode_event_update(void *user, float delta_time) {
    (void)delta_time;
    if (!user) {
        return;
    }

    mode_event_impl_t *impl = (mode_event_impl_t *)user;
    if (!impl || impl->base.done) {
        return;
    }

    game_state_t *state = impl->base.state;

    if (!mode_event_load_script(impl)) {
        fprintf(stderr, "widebrim: failed to load event script for event %d\n",
                game_state_get_event_id(state));
    }
}

mode_handler_t mode_event_create(game_state_t *state,
                                 screen_controller_t *screen_controller) {
    mode_handler_t handler;
    mode_event_impl_t *impl =
        (mode_event_impl_t *)smalloc(sizeof(mode_event_impl_t));

    if (!state || !screen_controller) {
        fprintf(stderr,
                "widebrim: mode_event_create called with invalid state or "
                "screen_controller pointers\n");
        exit(EXIT_FAILURE);
    }

    impl->base.state = state;
    impl->base.controller = screen_controller;
    impl->base.done = false;

    impl->center_line = smalloc(sizeof(object_t));
    mode_event_load_center_line(impl);
    object_layer_add_object(impl->base.controller->object, impl->center_line);

    screen_controller_fade_in(screen_controller, FADER_DEFAULT_DURATION_MS,
                              NULL, NULL);

    handler.layer.impl = impl;
    handler.layer.update = mode_event_update;
    handler.layer.draw = NULL;
    handler.layer.handle_event = NULL;
    handler.layer.on_quit = NULL;
    handler.layer.destroy = mode_event_destroy;
    handler.is_done = mode_event_is_done;
    handler.valid = true;
    return handler;
}
