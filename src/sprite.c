#include "sprite.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "object.h"
#include "sprite_loader.h"

static char *sprite_strdup(const char *text) {
    size_t length;
    char *copy;

    if (!text) {
        return NULL;
    }

    length = strlen(text) + 1U;
    copy = malloc(length);
    if (!copy) {
        return NULL;
    }

    memcpy(copy, text, length);
    return copy;
}

static char *sprite_frame_name_for_index(size_t index) {
    char buffer[32];
    size_t written;

    written = (size_t)snprintf(buffer, sizeof(buffer), "%zu", index);
    if (written >= sizeof(buffer)) {
        return NULL;
    }

    return sprite_strdup(buffer);
}

static void sprite_clear_frame_names(sprite_t *sprite) {
    if (!sprite || !sprite->frame_names) {
        return;
    }

    for (size_t i = 0; i < sprite->frame_count; ++i) {
        free(sprite->frame_names[i]);
        sprite->frame_names[i] = NULL;
    }

    free(sprite->frame_names);
    sprite->frame_names = NULL;
}

void sprite_clear(sprite_t *sprite, renderer_t *renderer) {
    if (!sprite || !renderer) {
        return;
    }

    if (sprite->frames) {
        for (size_t i = 0; i < sprite->frame_count; ++i) {
            if (sprite->frames[i]) {
                if (sprite->tex == sprite->frames[i]) {
                    sprite->tex = NULL;
                }
                renderer_destroy_texture(renderer, sprite->frames[i]);
                sprite->frames[i] = NULL;
            }
        }
        free(sprite->frames);
        sprite->frames = NULL;
    }

    sprite_clear_frame_names(sprite);

    if (sprite->tex) {
        renderer_destroy_texture(renderer, sprite->tex);
        sprite->tex = NULL;
    }

    sprite->alpha = 255;
    sprite->frame_count = 0;
    sprite->current_frame = 0;
    sprite->frame_duration_ms = 0;
    sprite->elapsed_ms = 0;
    sprite->loop = false;
    sprite->playing = false;
}

void sprite_apply_alpha(sprite_t *sprite, renderer_t *renderer) {
    size_t i;

    if (!sprite || !renderer) {
        return;
    }

    if (sprite->frames) {
        for (i = 0; i < sprite->frame_count; ++i) {
            if (sprite->frames[i]) {
                renderer_set_texture_alpha(renderer, sprite->frames[i],
                                           sprite->alpha);
            }
        }
    } else if (sprite->tex) {
        renderer_set_texture_alpha(renderer, sprite->tex, sprite->alpha);
    }
}

void sprite_set_alpha(sprite_t *sprite, uint8_t alpha) {
    if (!sprite) {
        return;
    }

    sprite->alpha = alpha;
}

bool sprite_set_position(sprite_t *sprite, int x, int y) {
    if (!sprite) {
        return false;
    }

    sprite->x = x;
    sprite->y = y;
    return true;
}

bool sprite_take_object_position(sprite_t *sprite, const object_t *obj) {
    if (!sprite || !obj) {
        return false;
    }

    sprite->x = obj->x;
    sprite->y = obj->y;
    return true;
}

bool sprite_get_position(sprite_t *sprite, int *x, int *y) {
    if (!sprite || !x || !y) {
        return false;
    }

    *x = sprite->x;
    *y = sprite->y;
    return true;
}

bool sprite_set_size(sprite_t *sprite, int width, int height) {
    if (!sprite || width <= 0 || height <= 0) {
        return false;
    }

    sprite->width = width;
    sprite->height = height;
    return true;
}

bool sprite_get_size(sprite_t *sprite, renderer_t *renderer, int *width,
                     int *height) {
    if (!sprite || !renderer || !width || !height) {
        return false;
    }

    if (sprite->tex) {
        renderer_get_texture_size(renderer, sprite->tex, width, height);
    } else {
        *width = 0;
        *height = 0;
    }

    return true;
}

bool sprite_set_playing(sprite_t *sprite, bool playing) {
    if (!sprite) {
        return false;
    }

    sprite->playing = playing;
    if (sprite->frame_count > 0 && sprite->frames &&
        sprite->current_frame < sprite->frame_count) {
        sprite->tex = sprite->frames[sprite->current_frame];
    }
    return true;
}

bool sprite_set_frame(sprite_t *sprite, size_t frame_index) {
    if (!sprite || !sprite->frames || sprite->frame_count == 0) {
        return false;
    }

    if (frame_index >= sprite->frame_count) {
        return false;
    }

    sprite->current_frame = frame_index;
    sprite->elapsed_ms = 0;
    sprite->tex = sprite->frames[frame_index];
    return true;
}

bool sprite_set_frame_by_name(sprite_t *sprite, const char *frame_name) {
    char *end = NULL;
    long index;

    if (!sprite || !frame_name || !*frame_name) {
        return false;
    }

    if (!sprite->frames || sprite->frame_count == 0U) {
        return strcmp(frame_name, "default") == 0 && sprite->tex != NULL;
    }

    if (!sprite->frame_names) {
        sprite->frame_names =
            calloc(sprite->frame_count, sizeof(*sprite->frame_names));
        if (!sprite->frame_names) {
            return false;
        }

        for (size_t i = 0; i < sprite->frame_count; ++i) {
            sprite->frame_names[i] = sprite_frame_name_for_index(i);
        }
    }

    for (size_t i = 0; i < sprite->frame_count; ++i) {
        if (sprite->frame_names[i] &&
            strcmp(sprite->frame_names[i], frame_name) == 0) {
            return sprite_set_frame(sprite, i);
        }
    }

    errno = 0;
    index = strtol(frame_name, &end, 10);
    if (errno == 0 && frame_name != end && *end == '\0' && index >= 0L &&
        (size_t)index < sprite->frame_count) {
        return sprite_set_frame(sprite, (size_t)index);
    }

    return false;
}

void sprite_init(sprite_t *sprite) {
    if (!sprite) {
        return;
    }

    sprite->x = 0;
    sprite->y = 0;
    sprite->width = 0;
    sprite->height = 0;
    sprite->frames = NULL;
    sprite->frame_names = NULL;
    sprite->frame_count = 0;
    sprite->current_frame = 0;
    sprite->frame_duration_ms = 0;
    sprite->elapsed_ms = 0;
    sprite->loop = false;
    sprite->playing = false;
    sprite->tex = NULL;
    sprite->alpha = 255;
}

void sprite_new_rgba(sprite_t *sprite, renderer_t *renderer,
                     const uint8_t *rgba, int width, int height) {
    if (!sprite || !rgba || width <= 0 || height <= 0) {
        return;
    }

    sprite_init(sprite);
    sprite->tex =
        renderer_create_texture_from_rgba(renderer, rgba, width, height);
}

void sprite_new_animation(sprite_t *sprite, renderer_t *renderer,
                          const uint8_t *const *frames, size_t frame_count,
                          const int *frame_widths, const int *frame_heights,
                          float frame_duration_ms, bool loop) {
    size_t i;

    if (!sprite) {
        fprintf(stderr,
                "widebrim: sprite_new_animation called with NULL sprite\n");
        return;
    }

    if (!frames) {
        fprintf(stderr,
                "widebrim: sprite_new_animation called with NULL frames\n");
        return;
    }

    if (frame_count == 0U) {
        fprintf(stderr, "widebrim: sprite_new_animation called with zero "
                        "frame_count\n");
        return;
    }

    sprite_init(sprite);
    sprite->frames =
        (renderer_texture_t **)calloc(frame_count, sizeof(*sprite->frames));
    if (!sprite->frames) {
        fprintf(stderr,
                "widebrim: failed to allocate memory for sprite frames\n");
        return;
    }

    for (i = 0; i < frame_count; ++i) {
        int width = 0;
        int height = 0;

        if (frame_widths != NULL && frame_heights != NULL) {
            width = frame_widths[i];
            height = frame_heights[i];
        } else if (i == 0U) {
            width = 1;
            height = 1;
        }

        if (width <= 0 || height <= 0) {
            fprintf(stderr,
                    "widebrim: invalid frame dimensions for sprite frame %zu: "
                    "width=%d, height=%d\n",
                    i, width, height);
            for (size_t j = 0; j < i; ++j) {
                if (sprite->frames[j]) {
                    renderer_destroy_texture(renderer, sprite->frames[j]);
                    sprite->frames[j] = NULL;
                }
            }
            free(sprite->frames);
            sprite->frames = NULL;
            return;
        }

        sprite->frames[i] = renderer_create_texture_from_rgba(
            renderer, frames[i], width, height);
        if (!sprite->frames[i]) {
            for (size_t j = 0; j < i; ++j) {
                if (sprite->frames[j]) {
                    renderer_destroy_texture(renderer, sprite->frames[j]);
                    sprite->frames[j] = NULL;
                }
            }
            free(sprite->frames);
            sprite->frames = NULL;
            fprintf(stderr,
                    "widebrim: failed to create texture for sprite frame %zu\n",
                    i);
            return;
        }
    }

    sprite->frame_count = frame_count;
    sprite->frame_names = calloc(frame_count, sizeof(*sprite->frame_names));
    if (!sprite->frame_names) {
        for (size_t j = 0; j < frame_count; ++j) {
            if (sprite->frames[j]) {
                renderer_destroy_texture(renderer, sprite->frames[j]);
                sprite->frames[j] = NULL;
            }
        }
        free(sprite->frames);
        sprite->frames = NULL;
        sprite->frame_count = 0U;
        fprintf(
            stderr,
            "widebrim: failed to allocate sprite frame names for animation\n");
        return;
    }

    for (size_t j = 0; j < frame_count; ++j) {
        sprite->frame_names[j] = sprite_frame_name_for_index(j);
    }

    sprite->current_frame = 0;
    sprite->frame_duration_ms = frame_duration_ms;
    sprite->elapsed_ms = 0;
    sprite->loop = loop;
    sprite->playing = true;
    sprite->tex = sprite->frames[0];
    if (frame_widths != NULL && frame_heights != NULL) {
        sprite->width = frame_widths[0];
        sprite->height = frame_heights[0];
    }
}

void sprite_new(sprite_t *sprite, renderer_t *renderer, game_state_t *state,
                const char *sprite_name, float frame_duration_ms, bool loop) {
    uint8_t **frames = NULL;
    int *frame_widths = NULL;
    int *frame_heights = NULL;
    char **frame_names = NULL;
    size_t frame_name_count = 0U;
    if (!sprite || !renderer || !state || !sprite_name) {
        return;
    }

    char sprite_path[256];
    snprintf(sprite_path, sizeof(sprite_path), "ani/%s", sprite_name);

    // if sprite path ends with .sbj, replace with .spr
    size_t len = strlen(sprite_path);
    if (len > 4 && strcmp(sprite_path + len - 4, ".sbj") == 0) {
        strcpy(sprite_path + len - 4, ".spr");
    }

    sprite_loader_load_animation_rgba(
        state, sprite_path, &frames, &sprite->frame_count, &frame_widths,
        &frame_heights, &sprite->width, &sprite->height);
    sprite_new_animation(sprite, renderer, (const uint8_t *const *)frames,
                         sprite->frame_count, frame_widths, frame_heights,
                         frame_duration_ms, loop);

    if (sprite_loader_load_animation_names(state, sprite_path, &frame_names,
                                           &frame_name_count)) {
        size_t max_count = frame_name_count < sprite->frame_count
                               ? frame_name_count
                               : sprite->frame_count;
        if (sprite->frame_names) {
            sprite_clear_frame_names(sprite);
        }
        sprite->frame_names =
            calloc(sprite->frame_count, sizeof(*sprite->frame_names));
        if (sprite->frame_names) {
            for (size_t i = 0; i < sprite->frame_count; ++i) {
                if (i < max_count && frame_names[i]) {
                    sprite->frame_names[i] = sprite_strdup(frame_names[i]);
                }
                if (!sprite->frame_names[i]) {
                    sprite->frame_names[i] = sprite_frame_name_for_index(i);
                }
            }
        }
        for (size_t i = 0; i < frame_name_count; ++i) {
            free(frame_names[i]);
        }
        free(frame_names);
    }

    free(frame_widths);
    free(frame_heights);
    if (!sprite->frames && frames != NULL) {
        for (size_t i = 0; i < sprite->frame_count; ++i) {
            free(frames[i]);
        }
        free(frames);
    }
}

void sprite_update(sprite_t *sprite, float delta_ms) {
    if (!sprite) {
        return;
    }

    if (!sprite->playing || sprite->frame_count < 2U ||
        sprite->frame_duration_ms <= 0.0f) {
        return;
    }

    sprite->elapsed_ms += delta_ms;
    while (sprite->elapsed_ms >= sprite->frame_duration_ms) {
        sprite->elapsed_ms -= sprite->frame_duration_ms;
        if (sprite->current_frame + 1U < sprite->frame_count) {
            sprite->current_frame += 1U;
        } else if (sprite->loop) {
            sprite->current_frame = 0U;
        } else {
            sprite->playing = false;
            sprite->current_frame = sprite->frame_count - 1U;
            break;
        }
    }

    sprite->tex = sprite->frames[sprite->current_frame];
}
