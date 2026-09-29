#include "map.h"

#include "bg_loader.h"
#include "text_loader.h"

void mode_room_load_map_place(mode_room_impl_t *impl) {
    int x, y, w, h;

    if (!impl) {
        return;
    }

    renderer_t *renderer = impl->base.controller->renderer;
    game_state_t *state = impl->base.state;

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
    sprite_take_object_position(spr, map_place);
}

void mode_room_load_map_purpose(mode_room_impl_t *impl) {
    int x, y, w, h;
    if (!impl) {
        return;
    }

    renderer_t *renderer = impl->base.controller->renderer;
    game_state_t *state = impl->base.state;

    object_t *map_purpose = impl->map_purpose;
    if (!map_purpose) {
        fprintf(stderr, "widebrim: failed to create map purpose object\n");
        return;
    }

    object_clear(map_purpose, renderer);

    sprite_t *spr = map_purpose->sprite;
    sprite_new(spr, renderer, state, "map_purpose.spr", 0.0f, false);

    sprite_get_size(spr, renderer, &w, &h);
    x = 0;
    y = WB_SCREEN_HEIGHT - h;

    object_set_position(map_purpose, x, y);
    object_set_size(map_purpose, w, h);
    sprite_take_object_position(spr, map_purpose);
}

static void mode_room_draw_map_text(mode_room_impl_t *room, char *text_buffer,
                                    object_t *anchor, int y_offset) {
    size_t i;
    rect_t text_rect = {0.0f, 0.0f, 0.0f, 0.0f};
    text_instance_t *text = NULL;

    if (!room || !text_buffer || text_buffer[0] == '\0') {
        return;
    }

    for (i = 0U; text_buffer[i] != '\0'; ++i) {
        if (text_buffer[i] == '\r') {
            text_buffer[i] = ' ';
        }
    }

    if (anchor) {
        int sprite_w = 0;
        int sprite_h = 0;

        sprite_get_size(anchor->sprite, room->base.controller->renderer,
                        &sprite_w, &sprite_h);
        text_rect.x = (float)anchor->x;
        text_rect.y = (float)anchor->y;
        text_rect.w = (float)sprite_w;
        text_rect.h = (float)sprite_h;
    }

    text = screen_controller_add_text(room->base.controller, 0, 0, text_buffer);
    if (text) {
        int text_x, text_y;

        text_layer_center_text_in_rect(text, &text_rect);
        text_layer_get_text_position(text, &text_x, &text_y);
        text_layer_set_text_position(text, text_x, text_y + y_offset);
    }
}

void mode_room_set_map(mode_room_impl_t *room, int32_t map_text_id,
                       int32_t map_background_id, int32_t param3,
                       int32_t param4, int32_t param5) {
    (void)param3;
    (void)param4;
    (void)param5;

    if (!room || !room->base.state) {
        fprintf(stderr, "widebrim: invalid game state\n");
        return;
    }

    game_state_t *state = room->base.state;

    char map_background_path[256];
    char map_title_path[256];
    char map_title_buffer[256];

    char map_purpose_path[256];
    char map_purpose_buffer[256];

    snprintf(map_background_path, sizeof(map_background_path), "bg/map_%d.bgx",
             map_background_id);

    if (!bg_loader_load(room->base.state, room->base.controller,
                        map_background_path, screen_controller_set_bg_main)) {
        fprintf(stderr, "widebrim: failed to load map background: %s\n",
                map_background_path);
    }

    if (map_text_id <= 0) {
        return;
    }

    snprintf(map_title_path, sizeof(map_title_path), "storytext/map%d.txt",
             map_text_id);
    if (!text_loader_load_path(room->base.state, map_title_path,
                               map_title_buffer, sizeof(map_title_buffer))) {
        fprintf(stderr, "widebrim: failed to load map text asset: %s\n",
                map_title_path);
        return;
    }

    snprintf(map_purpose_path, sizeof(map_purpose_path), "storytext/mt_%d.txt",
             state->memo_flag);
    if (!text_loader_load_path(room->base.state, map_purpose_path,
                               map_purpose_buffer,
                               sizeof(map_purpose_buffer))) {
        fprintf(stderr, "widebrim: failed to load map purpose: %s\n",
                map_purpose_path);
        return;
    }

    mode_room_draw_map_text(room, map_title_buffer, room->map_place, -7);
    mode_room_draw_map_text(room, map_purpose_buffer, room->map_purpose, 0);
}
