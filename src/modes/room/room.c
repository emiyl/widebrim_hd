#include "room.h"

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

#include "bg_loader.h"
#include "text_loader.h"

#include "gds/gds.h"
#include "gds/gds_exec.h"

#define MOVE_MODE_ICON_SIZE 84
#define MOVE_MODE_TRANSITION 250.0f

static void toggle_move_mode(mode_room_impl_t *impl) {
    if (!impl) {
        return;
    }

    impl->in_move_mode = !impl->in_move_mode;
    if (impl->in_move_mode) {
        object_layer_fade_out(impl->move_mode_sprite, MOVE_MODE_TRANSITION);
    } else {
        object_layer_fade_in(impl->move_mode_sprite, MOVE_MODE_TRANSITION);
    }
}

static bool mode_room_on_move_mode_icon_click(void *user,
                                              const input_event_t *event,
                                              sprite_instance_t *sprite) {
    (void)sprite;
    if (!user) {
        fprintf(stderr, "widebrim: mode_room_on_move_mode_icon_click called "
                        "with NULL user pointer\n");
        return false;
    }

    mode_room_impl_t *impl = (mode_room_impl_t *)user;
    static bool clicked = false;
    static bool sprite_is_offset = false;
    const int clicked_offset = 5;

    switch (event->type) {
    case INPUT_EVENT_MOUSE_BUTTON_DOWN:
        clicked = true;
        if (!sprite_is_offset) {
            object_layer_set_sprite_position(
                impl->move_mode_sprite,
                impl->move_mode_sprite->x + clicked_offset,
                impl->move_mode_sprite->y + clicked_offset);
            sprite_is_offset = true;
        }
        break;
    case INPUT_EVENT_MOUSE_BUTTON_UP:
        if (sprite_is_offset) {
            object_layer_set_sprite_position(
                impl->move_mode_sprite,
                impl->move_mode_sprite->x - clicked_offset,
                impl->move_mode_sprite->y - clicked_offset);
            sprite_is_offset = false;
        }
        if (clicked) {
            clicked = false;
            toggle_move_mode(impl);
            return true;
        }
        break;
    default:
        break;
    }

    return false;
}

static void mode_room_load_move_mode_sprite(mode_room_impl_t *impl,
                                            game_state_t *state,
                                            screen_controller_t *controller) {
    if (!impl) {
        return;
    }

    impl->move_mode_sprite = screen_controller_add_sprite_asset(
        controller, state, "ani/movemode.spr", 0, 0, 0, 255, 0.0f, false);
    if (!impl->move_mode_sprite) {
        fprintf(stderr, "widebrim: failed to load move mode sprite\n");
        return;
    }

    int x, y;
    object_layer_get_sprite_size(impl->move_mode_sprite, &x, &y);
    x = WB_SCREEN_WIDTH - x - 20;
    y = WB_SCREEN_HEIGHT * 2 - y - 20;

    object_layer_set_sprite_position(impl->move_mode_sprite, x, y);
    object_layer_set_interactive(impl->move_mode_sprite, true,
                                 mode_room_on_move_mode_icon_click, impl);
}

static void mode_room_load_map_place_sprite(mode_room_impl_t *impl,
                                            game_state_t *state,
                                            screen_controller_t *controller) {
    int x, y, w, h;

    if (!impl) {
        return;
    }

    impl->map_place_sprite = screen_controller_add_sprite_asset(
        controller, state, "ani/map_place.spr", 0, 0, 0, 255, 0.0f, false);
    if (!impl->map_place_sprite) {
        fprintf(stderr, "widebrim: failed to load map place sprite\n");
        return;
    }

    object_layer_get_sprite_size(impl->map_place_sprite, &w, &h);
    x = WB_SCREEN_WIDTH - w;
    y = 0;

    object_layer_set_sprite_position(impl->map_place_sprite, x, y);
}

static bool mode_room_load_and_execute_script(mode_room_impl_t *impl,
                                              int room_num) {
    char script_path[256];
    char resolved_path[1024];
    const uint8_t *script_payload = NULL;
    size_t script_payload_size = 0U;

    snprintf(script_path, sizeof(script_path), "script/rooms/room%d_param.gds",
             room_num);

    if (!asset_path_resolve(impl->state->assets_root, impl->state->language,
                            script_path, resolved_path,
                            sizeof(resolved_path))) {
        fprintf(stderr, "widebrim: Failed to resolve asset path for %s\n",
                script_path);
        return false;
    }

    fprintf(stderr, "widebrim: Loading script for room %d: %s\n", room_num,
            script_path);
    if (!gds_load_from_file_path(resolved_path, &script_payload,
                                 &script_payload_size)) {
        fprintf(stderr, "widebrim: Failed to load script for room %d: %s\n",
                room_num, script_path);
        return false;
    }

    if (!gds_execute_script(script_payload, script_payload_size, impl)) {
        fprintf(stderr, "widebrim: failed to execute script for room %d: %s\n",
                room_num, script_path);
        gds_free_payload(script_payload);
        return false;
    }

    gds_free_payload(script_payload);
    return true;
}

static void mode_room_on_background_touch(void *user, bg_touch_kind_t kind,
                                          int x, int y) {
    (void)x;
    (void)y;
    mode_room_impl_t *impl = (mode_room_impl_t *)user;
    if (!impl) {
        return;
    }

    if (kind == BG_TOUCH_KIND_TAP && impl->in_move_mode) {
        toggle_move_mode(impl);
        return;
    }

    switch (kind) {
    case BG_TOUCH_KIND_NONE:
    case BG_TOUCH_KIND_TAP:
    case BG_TOUCH_KIND_DRAG:
        break;
    }
}

static void mode_room_reset_room(mode_room_impl_t *impl) {
    if (!impl || !impl->state || !impl->controller) {
        return;
    }

    int room_num = game_state_get_place_num(impl->state);
    char bg_sub_path[256];
    snprintf(bg_sub_path, sizeof(bg_sub_path), "bg/room_%d_bg.png", room_num);

    if (!bg_loader_load(impl->state, impl->controller, bg_sub_path,
                        screen_controller_set_bg_sub)) {
        fprintf(stderr, "widebrim: failed to reload room %d background\n",
                room_num);
    }

    if (!impl->move_mode_sprite) {
        mode_room_load_move_mode_sprite(impl, impl->state, impl->controller);
    }

    if (!impl->map_place_sprite) {
        mode_room_load_map_place_sprite(impl, impl->state, impl->controller);
    }

    if (!mode_room_load_and_execute_script(impl, room_num)) {
        fprintf(stderr, "widebrim: failed to reset room %d script\n", room_num);
    }

    screen_controller_fade_in(impl->controller, FADER_DEFAULT_DURATION_MS, NULL,
                              NULL);
    impl->done = false;
    impl->in_move_mode = false;
    bg_layer_set_touch_callback(impl->controller->bg,
                                mode_room_on_background_touch, impl);
}

static void mode_room_reload_room_after_fade_out(void *user) {
    mode_room_impl_t *impl = (mode_room_impl_t *)user;
    text_layer_clear(impl->controller->text);
    mode_room_reset_room(impl);
}

static void mode_room_reload_room(mode_room_impl_t *impl) {
    if (!impl || !impl->state || !impl->controller) {
        return;
    }

    screen_controller_fade_out(impl->controller, FADER_DEFAULT_DURATION_MS,
                               mode_room_reload_room_after_fade_out, impl);
}

static bool mode_room_on_textobj_click(void *user, const input_event_t *event,
                                       sprite_instance_t *sprite) {
    typedef struct {
        mode_room_impl_t *impl;
        int32_t text_id;
    } room_text_click_t;

    room_text_click_t *click = (room_text_click_t *)user;
    (void)sprite;

    if (!click || !click->impl || !event) {
        return false;
    }

    if (event->type != INPUT_EVENT_MOUSE_BUTTON_DOWN &&
        event->type != INPUT_EVENT_MOUSE_BUTTON_UP) {
        return false;
    }

    if (click->impl->popup_text && click->impl->popup_text->visible) {
        screen_controller_set_text_visible(click->impl->controller,
                                           click->impl->popup_text, false);
        return true;
    }

    {
        char text_buffer[4096];
        text_instance_t *popup = NULL;
        rect_t rect = {0.0f, 0.0f, (float)WB_SCREEN_WIDTH,
                       (float)WB_SCREEN_HEIGHT};

        if (!text_loader_load_room_text(click->impl->state, click->text_id,
                                        text_buffer, sizeof(text_buffer))) {
            fprintf(
                stderr,
                "widebrim: failed to load room text object asset for id %d\n",
                click->text_id);
            return false;
        }

        if (!click->impl->popup_text) {
            popup = screen_controller_add_text(click->impl->controller, 0, 0,
                                               text_buffer);
            if (!popup) {
                fprintf(stderr,
                        "widebrim: failed to create popup text for id %d\n",
                        click->text_id);
                return false;
            }
            click->impl->popup_text = popup;
        } else {
            popup = click->impl->popup_text;
            screen_controller_set_text_contents(click->impl->controller, popup,
                                                text_buffer);
        }

        text_layer_center_text_in_rect(popup, &rect);
        text_layer_set_visible(popup, true);
        return true;
    }
}

static sprite_instance_t *
mode_room_add_textobj_area(mode_room_impl_t *impl, int32_t x, int32_t y,
                           int32_t width, int32_t height, int32_t text_id) {
    typedef struct {
        mode_room_impl_t *impl;
        int32_t text_id;
    } room_text_click_t;

    uint8_t *transparent = NULL;
    sprite_instance_t *sprite = NULL;
    room_text_click_t *click = NULL;

    if (!impl || !impl->controller || width <= 0 || height <= 0) {
        return NULL;
    }

    transparent = (uint8_t *)calloc((size_t)width * (size_t)height * 4U,
                                    sizeof(*transparent));
    if (!transparent) {
        fprintf(
            stderr,
            "widebrim: failed to allocate transparent hitbox for textobj\n");
        return NULL;
    }

    click = (room_text_click_t *)malloc(sizeof(*click));
    if (!click) {
        free(transparent);
        fprintf(stderr,
                "widebrim: failed to allocate textobj callback state\n");
        return NULL;
    }

    click->impl = impl;
    click->text_id = text_id;

    sprite = screen_controller_add_sprite_z(impl->controller, transparent,
                                            width, height, x, y, 0, 0U);
    free(transparent);
    if (!sprite) {
        free(click);
        fprintf(stderr, "widebrim: failed to create textobj hitbox\n");
        return NULL;
    }

    object_layer_set_visible(sprite, false);
    object_layer_set_interactive(sprite, true, mode_room_on_textobj_click,
                                 click);
    return sprite;
}

static void mode_room_add_text_obj(mode_room_impl_t *impl, int32_t x, int32_t y,
                                   int32_t width, int32_t height,
                                   int32_t text_id, int32_t param7) {
    (void)param7;

    if (!impl || !impl->controller) {
        return;
    }

    if (width <= 0 || height <= 0) {
        return;
    }

    mode_room_add_textobj_area(impl, x, y, width, height, text_id);
}

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

static bool mode_room_handle_event(void *user, const input_event_t *event) {
    if (!user) {
        return false;
    }

    mode_room_impl_t *impl = (mode_room_impl_t *)user;
    if (!impl) {
        return false;
    }

    if (event->type == INPUT_EVENT_MOUSE_BUTTON_DOWN && impl->popup_text &&
        impl->popup_text->visible) {
        screen_controller_set_text_visible(impl->controller, impl->popup_text,
                                           false);
        return true;
    }

    if (impl->controller && impl->controller->object &&
        object_layer_handle_event(impl->controller->object, event)) {
        return true;
    }

    int place_num = game_state_get_place_num(impl->state);

    switch (event->type) {
    case INPUT_EVENT_KEY_DOWN:
        switch (event->data.key.key) {
        case 'm':
        case 'M':
            toggle_move_mode(impl);
            return true;
        case 'q':
            if (place_num > 1) {
                game_state_set_place_num(impl->state, place_num - 1);
                mode_room_reload_room(impl);
                printf("widebrim: moved to room %d\n", place_num - 1);
                return true;
            }
            break;
        case 'w':
            game_state_set_place_num(impl->state, place_num + 1);
            mode_room_reload_room(impl);
            printf("widebrim: moved to room %d\n", place_num + 1);
            return true;
        default:
            break;
        }
        break;
    default:
        break;
    }

    return false;
}

static void mode_room_setmap(mode_room_impl_t *self, int32_t map_text_id,
                             int32_t map_background_id, int32_t param3,
                             int32_t param4, int32_t param5) {
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

        if (self->map_place_sprite) {
            int sprite_w = 0;
            int sprite_h = 0;
            object_layer_get_sprite_size(self->map_place_sprite, &sprite_w,
                                         &sprite_h);
            text_rect.x = (float)self->map_place_sprite->x;
            text_rect.y = (float)self->map_place_sprite->y;
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

mode_handler_t mode_room_create(game_state_t *state,
                                screen_controller_t *controller) {
    mode_handler_t handler = {0};
    mode_room_impl_t *impl = malloc(sizeof(mode_room_impl_t));
    if (!impl) {
        fprintf(stderr,
                "widebrim: Failed to allocate memory for mode_room_impl_t\n");
        exit(EXIT_FAILURE);
    }

    impl->state = state;
    impl->controller = controller;
    impl->setmap = mode_room_setmap;
    impl->add_text_obj = mode_room_add_text_obj;
    impl->popup_text = NULL;

    mode_room_reset_room(impl);

    handler.layer.impl = impl;
    handler.layer.update = NULL;
    handler.layer.draw = NULL;
    handler.layer.handle_event = mode_room_handle_event;
    handler.layer.on_quit = NULL;
    handler.layer.destroy = mode_room_destroy;
    handler.is_done = mode_room_is_done;
    handler.valid = true;
    return handler;
}
