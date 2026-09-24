#include "mode_spawner.h"

#include <stdio.h>

#include "mode_reset.h"
#include "mode_title.h"

static mode_handler_t
mode_spawner_create_handler(game_mode_t mode, game_state_t *state,
                            screen_controller_t *controller) {
    (void)state;
    (void)controller;
    mode_handler_t invalid;

    switch (mode) {
    case MODE_RESET:
        return mode_reset_create(state, controller);
    case MODE_TITLE:
        return mode_title_create(state, controller);
    default:
        invalid.layer.impl = NULL;
        invalid.layer.update = NULL;
        invalid.layer.draw = NULL;
        invalid.layer.handle_event = NULL;
        invalid.layer.on_quit = NULL;
        invalid.layer.destroy = NULL;
        invalid.is_done = NULL;
        invalid.valid = false;
        return invalid;
    }
}

static void mode_spawner_void_mode(mode_spawner_t *spawner) {
    if (!spawner) {
        fprintf(stderr, "widebrim: spawner is NULL\n");
        return;
    }

    if (spawner->has_active_mode) {
        screen_layer_t removed =
            screen_collection_remove_at(&spawner->layers, 1);
        if (removed.destroy) {
            removed.destroy(removed.impl);
        }
    }

    spawner->has_active_mode = false;
    spawner->current_active_mode = MODE_INVALID;
}

static void mode_spawner_load_mode(mode_spawner_t *spawner, game_mode_t mode) {
    screen_layer_t fader_slot;
    mode_handler_t handler;

    if (!spawner) {
        fprintf(stderr, "widebrim: spawner is NULL\n");
        return;
    }

    fprintf(stderr, "widebrim: loading mode %d\n", (int)mode);
    game_state_set_mode(spawner->state, mode);
    spawner->current_active_mode = mode;

    // Pop fader so new mode is inserted beneath it
    fader_slot = screen_collection_pop(&spawner->layers);

    handler =
        mode_spawner_create_handler(mode, spawner->state, &spawner->controller);
    if (handler.valid) {
        screen_collection_add(&spawner->layers, handler.layer);
        spawner->active_mode_handler = handler;
        spawner->has_active_mode = true;
    } else {
        fprintf(stderr, "widebrim: no handler registered for mode %d\n",
                (int)mode);
        spawner->has_active_mode = false;
    }

    // Add fader back on top
    screen_collection_add(&spawner->layers, fader_slot);
}

static void mode_spawner_ready_switch(mode_spawner_t *spawner,
                                      game_mode_t target);

static void mode_spawner_on_fade_out_complete(void *user) {
    mode_spawner_t *spawner = (mode_spawner_t *)user;
    if (!spawner) {
        fprintf(stderr, "widebrim: spawner is NULL\n");
        return;
    }

    spawner->switch_pending = false;
    mode_spawner_ready_switch(spawner, spawner->pending_target_mode);
}

static void mode_spawner_ready_switch(mode_spawner_t *spawner,
                                      game_mode_t target) {
    if (!spawner) {
        fprintf(stderr, "widebrim: spawner is NULL\n");
        return;
    }

    if (fader_layer_is_view_obscured(&spawner->fader)) {
        spawner->switch_pending = true;
        mode_spawner_void_mode(spawner);
        mode_spawner_load_mode(spawner, target);
    } else if (!spawner->switch_pending) {
        spawner->switch_pending = true;
        spawner->pending_target_mode = target;
        fader_layer_fade_out(&spawner->fader, FADER_DEFAULT_DURATION_MS,
                             mode_spawner_on_fade_out_complete, spawner);
    }
}

void mode_spawner_init(mode_spawner_t *spawner, game_state_t *state,
                       renderer_t *renderer) {
    spawner->state = state;
    spawner->has_active_mode = false;
    spawner->current_active_mode = MODE_INVALID;
    spawner->pending_target_mode = MODE_INVALID;
    spawner->switch_pending = false;
    spawner->should_quit = false;

    spawner->controller.renderer = renderer;
    spawner->controller.bg = &spawner->bg;
    spawner->controller.fader = &spawner->fader;
    bg_layer_init(&spawner->bg, spawner->controller.renderer);
    fader_layer_init(&spawner->fader);

    screen_collection_init(&spawner->layers);
    screen_collection_add(&spawner->layers,
                          bg_layer_as_screen_layer(&spawner->bg));
    screen_collection_add(&spawner->layers,
                          fader_layer_as_screen_layer(&spawner->fader));
}

void mode_spawner_destroy(mode_spawner_t *spawner) {
    if (!spawner) {
        fprintf(stderr, "widebrim: spawner is NULL\n");
        return;
    }

    bg_layer_destroy(&spawner->bg);
    screen_collection_free(&spawner->layers);
    if (spawner->controller.renderer) {
        renderer_destroy(spawner->controller.renderer);
        spawner->controller.renderer = NULL;
    }
}

void mode_spawner_update(mode_spawner_t *spawner, float delta_ms) {
    bool mode_done;

    screen_collection_update(&spawner->layers, delta_ms);

    mode_done = spawner->has_active_mode &&
                spawner->active_mode_handler.is_done &&
                spawner->active_mode_handler.is_done(
                    spawner->active_mode_handler.layer.impl);

    if (spawner->has_active_mode) {
        if (mode_done) {
            mode_spawner_ready_switch(spawner,
                                      game_state_get_mode(spawner->state));
        }
    } else {
        game_mode_t current_mode = game_state_get_mode(spawner->state);
        if (spawner->current_active_mode != current_mode) {
            mode_spawner_ready_switch(spawner, current_mode);
        } else if (game_state_get_next_mode(spawner->state) != current_mode) {
            mode_spawner_ready_switch(spawner,
                                      game_state_get_next_mode(spawner->state));
        } else {
            spawner->should_quit = true;
        }
    }
}

void mode_spawner_draw(mode_spawner_t *spawner, renderer_t *renderer) {
    screen_collection_draw(&spawner->layers, renderer);
}

bool mode_spawner_handle_event(mode_spawner_t *spawner,
                               const input_event_t *event) {
    return screen_collection_handle_event(&spawner->layers, event);
}

void mode_spawner_on_quit(mode_spawner_t *spawner) {
    spawner->should_quit = true;
}
