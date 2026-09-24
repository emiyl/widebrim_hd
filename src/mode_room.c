#include "mode_room.h"

#include <stdio.h>
#include <stdlib.h>

#include "bg_loader.h"

typedef struct {
    game_state_t *state;
    screen_controller_t *controller;
    bool done;
    bool in_move_mode;
} mode_room_impl_t;

static bool mode_room_is_done(void *user) {
    if (!user) {
        fprintf(stderr,
                "widebrim: mode_room_is_done called with NULL user pointer\n");
        return false;
    }

    mode_room_impl_t *impl = (mode_room_impl_t *)user;
    return impl->done;
}

static void mode_room_destroy(void *user) {
    if (!user) {
        fprintf(stderr,
                "widebrim: mode_room_destroy called with NULL user pointer\n");
        return;
    }

    free(user);
}

mode_handler_t mode_room_create(game_state_t *state,
                                screen_controller_t *controller) {
    mode_handler_t handler;
    mode_room_impl_t *impl = malloc(sizeof(mode_room_impl_t));
    if (!impl) {
        fprintf(stderr, "Failed to allocate memory for mode_room_impl_t\n");
        exit(EXIT_FAILURE);
    }

    int room_num = game_state_get_place_num(state);
    char bg_sub_path[256];
    snprintf(bg_sub_path, sizeof(bg_sub_path), "bg/room_%d_bg.png", room_num);

    bg_loader_load(state, controller, bg_sub_path,
                   screen_controller_set_bg_sub);

    screen_controller_fade_in(controller, FADER_DEFAULT_DURATION_MS, NULL,
                              NULL);

    impl->state = state;
    impl->controller = controller;
    impl->done = false;
    impl->in_move_mode = false;

    handler.layer.impl = impl;
    handler.layer.update = NULL;
    handler.layer.draw = NULL;
    handler.layer.handle_event = NULL;
    handler.layer.on_quit = NULL;
    handler.layer.destroy = mode_room_destroy;
    handler.is_done = mode_room_is_done;
    handler.valid = true;
    return handler;
}
