#include "map.h"

#include "bg_loader.h"
#include "text_loader.h"

void mode_room_load_map_place(mode_room_impl_t *impl) {
    int x, y, w, h;

    if (!impl) {
        return;
    }

    renderer_t *renderer = impl->controller->renderer;
    game_state_t *state = impl->state;

    object_t *map_place = impl->map_place;
    if (!map_place) {
        fprintf(stderr, "widebrim: failed to create map place object\n");
        return;
    }

    object_clear(map_place, renderer);

    sprite_t *spr = map_place->sprite;
    sprite_new(spr, renderer, state, "map_place.spr", 0.0f, false);

    sprite_get_size(spr, renderer, &w, &h);
    x = WB_SCREEN_WIDTH - w;
    y = 0;

    object_set_position(map_place, x, y);
    object_set_size(map_place, w, h);
}

void mode_room_setmap(mode_room_impl_t *self, int32_t map_text_id,
                      int32_t map_background_id, int32_t param3, int32_t param4,
                      int32_t param5) {
    (void)param3;
    (void)param4;
    (void)param5;

    char map_background_path[256];
    char map_text_path[256];
    char text_buffer[4096];

    int text_x, text_y;

    snprintf(map_background_path, sizeof(map_background_path), "bg/map_%d.png",
             map_background_id);

    if (!bg_loader_load(self->state, self->controller, map_background_path,
                        screen_controller_set_bg_main)) {
        fprintf(stderr, "widebrim: failed to load map background: %s\n",
                map_background_path);
    }

    if (map_text_id <= 0) {
        return;
    }

    snprintf(map_text_path, sizeof(map_text_path), "storytext/map%d.txt",
             map_text_id);
    if (!text_loader_load_path(self->state, map_text_path, text_buffer,
                               sizeof(text_buffer))) {
        fprintf(stderr, "widebrim: failed to load map text asset: %s\n",
                map_text_path);
        return;
    }

    if (strlen(text_buffer) > 0U) {
        size_t i;
        rect_t text_rect = {0.0f, 0.0f, 0.0f, 0.0f};
        text_instance_t *text = NULL;

        for (i = 0U; i < strlen(text_buffer); ++i) {
            if (text_buffer[i] == '\r') {
                text_buffer[i] = ' ';
            }
        }

        if (self->map_place) {
            int sprite_w = 0;
            int sprite_h = 0;
            sprite_get_size(self->map_place->sprite, self->controller->renderer,
                            &sprite_w, &sprite_h);
            text_rect.x = (float)self->map_place->x;
            text_rect.y = (float)self->map_place->y;
            text_rect.w = (float)sprite_w;
            text_rect.h = (float)sprite_h;
        } else {
            text_rect.x = 16.0f;
            text_rect.y = 16.0f;
            text_rect.w = (float)WB_SCREEN_WIDTH - 32.0f;
            text_rect.h = (float)WB_SCREEN_HEIGHT - 32.0f;
        }

        text = screen_controller_add_text(self->controller, 0, 0, text_buffer);
        if (text) {
            text_layer_center_text_in_rect(text, &text_rect);
            text_layer_get_text_position(text, &text_x, &text_y);
            text_layer_set_text_position(text, text_x, text_y - 7);
        }
    }
}
