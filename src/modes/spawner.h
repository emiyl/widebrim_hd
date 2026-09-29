#ifndef MODE_SPAWNER_H
#define MODE_SPAWNER_H

#include <stdbool.h>

#include "bg_layer.h"
#include "fader_layer.h"
#include "game_state.h"
#include "input.h"
#include "mode.h"
#include "object_layer.h"
#include "renderer.h"
#include "screen.h"
#include "screen_controller.h"
#include "text_layer.h"

typedef struct {
    screen_collection_t layers;
    bg_layer_t bg;
    object_layer_t object;
    text_layer_t text;
    fader_layer_t fader;
    screen_controller_t controller;
    game_state_t *state;

    bool has_active_mode;
    mode_handler_t active_mode_handler;
    game_mode_t current_active_mode;
    game_mode_t pending_target_mode;
    bool switch_pending;

    bool should_quit;
} mode_spawner_t;

void mode_spawner_init(mode_spawner_t *spawner, game_state_t *state,
                       renderer_t *renderer);
void mode_spawner_destroy(mode_spawner_t *spawner);

void mode_spawner_update(mode_spawner_t *spawner, float dt_ms);
void mode_spawner_draw(mode_spawner_t *spawner, renderer_t *renderer);
bool mode_spawner_handle_event(mode_spawner_t *spawner,
                               const input_event_t *event);
void mode_spawner_on_quit(mode_spawner_t *spawner);

#endif // MODE_SPAWNER_H
