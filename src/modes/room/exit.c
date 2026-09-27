#include "exit.h"

#include <stdlib.h>

#include "object.h"
#include "object_layer.h"
#include "room.h"
#include "safe.h"
#include "sprite.h"

void room_add_exit(mode_room_impl_t *impl, int32_t exit_sprite_id,
                   int32_t target_map_id, int32_t x, int32_t y, int32_t width,
                   int32_t height, int32_t param7, int32_t param8) {
    (void)param7;
    (void)param8;

    room_exit_t *new_exit = &impl->exits[impl->exit_count];
    new_exit->exit_sprite_id = exit_sprite_id;
    new_exit->target_map_id = target_map_id;

    new_exit->object = smalloc(sizeof(object_t));
    object_t *object = new_exit->object;
    object_init(object);

    if (!object) {
        fprintf(stderr,
                "widebrim: [room_add_exit] failed to allocate object\n");
        return;
    }

    object->x = x;
    object->y = y + WB_SCREEN_HEIGHT;
    object->width = width;
    object->height = height;

    sprite_t *sprite = object->sprite;
    if (!sprite) {
        fprintf(stderr,
                "widebrim: [room_add_exit] failed to allocate sprite\n");
        return;
    }

    renderer_t *renderer = impl->controller->renderer;
    game_state_t *state = impl->state;

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
        sprite_take_object_position(sprite, object);
    }

    object_set_visible(object, false);
    object_layer_add_object(impl->controller->object, new_exit->object);

    impl->exit_count++;
}
