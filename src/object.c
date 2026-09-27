#include "object.h"

#include <stdlib.h>

void object_init(object_t *inst) {
    if (!inst) {
        fprintf(stderr, "widebrim: object_init called with NULL instance\n");
        return;
    }

    if (!inst->sprite) {
        inst->sprite = malloc(sizeof(sprite_t));
        if (!inst->sprite) {
            fprintf(
                stderr,
                "widebrim: object_init failed to allocate memory for sprite\n");
            return;
        }
    }
    sprite_init(inst->sprite);

    inst->x = 0;
    inst->y = 0;
    inst->z = 0;
    inst->width = 0;
    inst->height = 0;
    inst->alpha = 255;
    inst->fade_duration_ms = 0.0f;
    inst->fade_elapsed_ms = 0.0f;
    inst->fading_in = false;
    inst->fading_out = false;
    inst->interactive = false;
    inst->visible = true;
    inst->user = NULL;
    inst->on_event = NULL;
}

void object_clear(object_t *inst, renderer_t *renderer) {
    if (!renderer || !inst) {
        return;
    }

    sprite_clear(inst->sprite, renderer);
    object_init(inst);
}

void object_destroy(object_t *inst, renderer_t *renderer) {
    if (!inst) {
        return;
    }

    if (inst->sprite) {
        sprite_clear(inst->sprite, renderer);
        free(inst->sprite);
        inst->sprite = NULL;
    }
}

bool object_contains_point(object_t *inst, int x, int y) {
    if (!inst) {
        return false;
    }

    return x >= inst->x && x <= inst->x + inst->width && y >= inst->y &&
           y <= inst->y + inst->height;
}

bool object_set_interactive(object_t *inst, bool interactive,
                            object_event_callback_t on_event, void *user) {
    if (!inst) {
        return false;
    }

    inst->interactive = interactive;
    inst->user = user;
    inst->on_event = on_event;
    return true;
}

bool object_set_visible(object_t *inst, bool visible) {
    if (!inst) {
        return false;
    }

    inst->visible = visible;
    if (!visible) {
        inst->fading_in = false;
        inst->fading_out = false;
        inst->fade_duration_ms = 0.0f;
        inst->fade_elapsed_ms = 0.0f;
    }
    return true;
}

bool object_fade_in(object_t *inst, renderer_t *renderer, float duration_ms) {
    if (!inst) {
        return false;
    }

    if (duration_ms < 0.0f) {
        duration_ms = 0.0f;
    }

    if (inst->alpha == 0U) {
        inst->alpha = 255U;
    }

    inst->fade_duration_ms = duration_ms;
    inst->fade_elapsed_ms = 0.0f;
    inst->fading_in = true;
    inst->fading_out = false;
    inst->visible = true;

    if (duration_ms <= 0.0f) {
        inst->visible = true;
        inst->fade_duration_ms = 0.0f;
        inst->fade_elapsed_ms = 0.0f;
        inst->fading_in = false;

        inst->sprite->alpha = inst->alpha;
        sprite_apply_alpha(inst->sprite, renderer);
    }

    return true;
}

bool object_fade_out(object_t *inst, renderer_t *renderer, float duration_ms) {
    if (!inst) {
        return false;
    }

    if (duration_ms < 0.0f) {
        duration_ms = 0.0f;
    }

    if (inst->sprite->alpha > 0U) {
        inst->alpha = inst->sprite->alpha;
    }

    inst->fade_duration_ms = duration_ms;
    inst->fade_elapsed_ms = 0.0f;
    inst->fading_in = false;
    inst->fading_out = true;
    inst->visible = true;

    if (duration_ms <= 0.0f) {
        inst->visible = false;
        inst->fade_duration_ms = 0.0f;
        inst->fade_elapsed_ms = 0.0f;
        inst->fading_out = false;

        inst->sprite->alpha = 0U;
        sprite_apply_alpha(inst->sprite, renderer);
    }

    return true;
}

bool object_center_position(object_t *inst, int area_width, int area_height) {
    if (!inst || area_width <= 0 || area_height <= 0) {
        return false;
    }

    inst->x = (area_width - inst->width) / 2;
    inst->y = (area_height - inst->height) / 2;
    return true;
}

bool object_set_position(object_t *inst, int x, int y) {
    if (!inst) {
        return false;
    }

    inst->x = x;
    inst->y = y;
    return true;
}

bool object_set_size(object_t *inst, int width, int height) {
    if (!inst || width <= 0 || height <= 0) {
        return false;
    }

    inst->width = width;
    inst->height = height;
    return true;
}

void object_update(object_t *inst, renderer_t *renderer, float delta_ms) {
    if (!inst) {
        return;
    }

    do {
        if (inst->fading_in || inst->fading_out) {
            inst->fade_elapsed_ms += delta_ms;

            if (inst->fade_duration_ms <= 0.0f ||
                inst->fade_elapsed_ms >= inst->fade_duration_ms) {
                if (inst->fading_in) {
                    inst->sprite->alpha = inst->alpha;
                    inst->visible = true;
                } else {
                    inst->sprite->alpha = 0U;
                    inst->visible = false;
                }
                sprite_apply_alpha(inst->sprite, renderer);

                inst->fading_in = false;
                inst->fading_out = false;
                inst->fade_elapsed_ms = 0.0f;
                inst->fade_duration_ms = 0.0f;
                break;
            }

            if (inst->fade_duration_ms > 0.0f) {
                const float t = inst->fade_elapsed_ms / inst->fade_duration_ms;
                if (inst->fading_in) {
                    inst->sprite->alpha = (uint8_t)((float)inst->alpha * t);
                } else {
                    inst->sprite->alpha =
                        (uint8_t)((float)inst->alpha * (1.0f - t));
                }
                inst->visible = true;
                sprite_apply_alpha(inst->sprite, renderer);
            }
        }
    } while (0);

    sprite_update(inst->sprite, delta_ms);
}

bool object_handle_event(object_t *inst, const input_event_t *event) {
    if (!inst || !event) {
        return false;
    }

    if (!inst->interactive || !inst->on_event) {
        return false;
    }

    if (!object_contains_point(inst, event->data.mouse_button.x,
                               event->data.mouse_button.y)) {
        return false;
    }

    return inst->on_event(inst->user, event, inst);
}

void object_draw(object_t *inst, renderer_t *renderer) {
    if (!inst) {
        fprintf(stderr, "widebrim: object instance is NULL in object_draw\n");
        return;
    }

    if (!inst->visible) {
        return;
    }

    if (!inst->sprite) {
        fprintf(stderr, "widebrim: object sprite is NULL in object_draw\n");
        return;
    }

    if (!renderer_texture_exists(renderer, inst->sprite->tex)) {
        fprintf(stderr,
                "widebrim: object texture does not exist in object_draw\n");
        return;
    }

    rect_t dst = {.x = (float)inst->x,
                  .y = (float)inst->y,
                  .w = (float)inst->width,
                  .h = (float)inst->height};
    renderer_draw_texture(renderer, inst->sprite->tex, &dst);
}
