#ifndef INPUT_H
#define INPUT_H

#include <stdbool.h>

typedef enum {
    INPUT_EVENT_NONE = 0,
    INPUT_EVENT_QUIT,
    INPUT_EVENT_KEY_DOWN,
    INPUT_EVENT_KEY_UP,
    INPUT_EVENT_MOUSE_MOTION,
    INPUT_EVENT_MOUSE_BUTTON_DOWN,
    INPUT_EVENT_MOUSE_BUTTON_UP
} input_event_type_t;

typedef struct {
    int x;
    int y;
    int dx;
    int dy;
} input_mouse_motion_event_t;

typedef struct {
    int x;
    int y;
    int button;
} input_mouse_button_event_t;

typedef struct {
    int key;
} input_key_event_t;

typedef struct {
    input_event_type_t type;
    union {
        input_mouse_motion_event_t mouse_motion;
        input_mouse_button_event_t mouse_button;
        input_key_event_t key;
    } data;
} input_event_t;

typedef struct input_t input_t;

typedef struct input_vtable {
    void (*destroy)(input_t *self);
    bool (*poll_event)(input_t *self, input_event_t *event);
} input_vtable_t;

struct input_t {
    void *impl;
    const input_vtable_t *vt;
};

input_t *input_create_sdl(void);

static inline void input_destroy(input_t *self) {
    if (self && self->vt && self->vt->destroy) {
        self->vt->destroy(self);
    }
}

static inline bool input_poll_event(input_t *self, input_event_t *event) {
    if (self && self->vt && self->vt->poll_event) {
        return self->vt->poll_event(self, event);
    }
    return false;
}

#endif // INPUT_H
