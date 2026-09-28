#ifndef OBJECT_H
#define OBJECT_H

#include "input.h"
#include "sprite.h"

typedef struct object_t object_t;

typedef bool (*object_event_callback_t)(void *user, const input_event_t *event,
                                        object_t *object);

typedef enum object_kind_t {
    OBJECT_KIND_NONE = 0,
    OBJECT_KIND_BTN,
    OBJECT_KIND_TOBJ,
    OBJECT_KIND_BG,
    OBJECT_KIND_EXIT,
    OBJECT_KIND_EVENT
} object_kind_t;

struct object_t {
    object_kind_t kind;
    sprite_t *sprite;
    int x;
    int y;
    int z;
    int width;
    int height;
    uint8_t alpha;
    float fade_duration_ms;
    float fade_elapsed_ms;
    bool fading_in;
    bool fading_out;
    bool interactive;
    bool visible;
    bool clicked;
    void *user;
    void *self_vars;
    object_event_callback_t on_event;
};

bool object_reset_fade(object_t *object, renderer_t *renderer);
bool object_fade_in(object_t *object, renderer_t *renderer, float duration_ms);
bool object_fade_out(object_t *object, renderer_t *renderer, float duration_ms);
bool object_center_position(object_t *object, int screen_width,
                            int screen_height);
bool object_set_position(object_t *object, int x, int y);
bool object_set_size(object_t *object, int width, int height);
bool object_contains_point(object_t *inst, int x, int y);

bool object_set_interaction_callback(object_t *inst,
                                     object_event_callback_t on_event,
                                     void *user);
bool object_set_interactive(object_t *inst, bool interactive);
bool object_set_visible(object_t *object, bool visible);

void object_init(object_t *object);
void object_init_kind(object_t *object, object_kind_t kind);
void object_clear(object_t *object, renderer_t *renderer);
void object_destroy(object_t *object, renderer_t *renderer);
void object_update(object_t *object, renderer_t *renderer, float delta_ms);
bool object_handle_event(object_t *object, const input_event_t *event);
void object_draw(object_t *object, renderer_t *renderer);

#endif // OBJECT_H
