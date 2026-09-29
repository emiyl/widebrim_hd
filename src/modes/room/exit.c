#include "exit.h"

#include <stdlib.h>

#include "object.h"
#include "object_layer.h"
#include "room.h"
#include "safe.h"
#include "sprite.h"

bool room_exit_on_event(void *user, const input_event_t *event,
                        object_t *object) {
    if (!user || !event || !object) {
        return false;
    }

    mode_room_impl_t *impl = (mode_room_impl_t *)user;
    if (!impl) {
        return false;
    }

    switch (event->type) {
    case INPUT_EVENT_MOUSE_BUTTON_DOWN:
        printf("widebrim: room_exit_on_event mouse button down\n");
        object->clicked = true;
        break;
    case INPUT_EVENT_MOUSE_BUTTON_UP:
        if (object->clicked) {
            object->clicked = false;

            room_exit_impl_t *exit_data = (room_exit_impl_t *)object->self_vars;
            if (!exit_data) {
                fprintf(stderr, "widebrim: [room_exit_on_event] missing exit "
                                "target data\n");
                return true;
            }

            printf("widebrim: exit clicked -> room %d\n",
                   exit_data->target_map_id);
            game_state_set_place_num(impl->base.state,
                                     exit_data->target_map_id);
            mode_room_reload_room(impl);
            return true;
        }
        break;
    default:
        break;
    }

    return true;
}

void room_add_exit(mode_room_impl_t *impl, int32_t exit_sprite_id,
                   int32_t target_map_id, int32_t x, int32_t y, int32_t width,
                   int32_t height, int32_t param7, int32_t param8) {
    (void)param7;
    (void)param8;

    if (impl->exit_count >= 8) {
        fprintf(stderr,
                "widebrim: [room_add_exit] maximum number of exits reached\n");
        return;
    }

    room_exit_impl_t *exit_data = smalloc(sizeof(*exit_data));
    *exit_data = (room_exit_impl_t){.target_map_id = target_map_id};

    object_t *exit = impl->exits[impl->exit_count];
    object_init_kind(exit, OBJECT_KIND_EXIT);
    exit->self_vars = exit_data;

    if (!exit) {
        fprintf(stderr,
                "widebrim: [room_add_exit] failed to allocate object\n");
        return;
    }

    exit->x = x;
    exit->y = y + WB_SCREEN_HEIGHT;
    exit->width = width;
    exit->height = height;

    sprite_t *sprite = exit->sprite;
    if (!sprite) {
        fprintf(stderr,
                "widebrim: [room_add_exit] failed to allocate sprite\n");
        return;
    }

    renderer_t *renderer = impl->base.controller->renderer;
    game_state_t *state = impl->base.state;

    char sprite_filename[14];
    bool has_sprite = false;
    if (exit_sprite_id == 0) {
        snprintf(sprite_filename, sizeof(sprite_filename), "exit_door.spr");
        has_sprite = true;
    } else if (exit_sprite_id < 6) {
        snprintf(sprite_filename, sizeof(sprite_filename), "exit_%d.spr",
                 exit_sprite_id);
        has_sprite = true;
    }

    if (has_sprite) {
        sprite_new(sprite, renderer, state, sprite_filename, 0.0f, false);
        sprite_take_object_position(sprite, exit);
    }

    object_set_visible(exit, false);
    object_set_interaction_callback(exit, room_exit_on_event, impl);
    object_set_interactive(exit, false);
    object_layer_add_object(impl->base.controller->object, exit);

    impl->exit_count++;
}
