#include "../input.h"

#include <stdlib.h>

typedef struct none_input_t {
    bool unused;
} none_input_t;

static void none_input_destroy(input_t *self) {
    none_input_t *impl;

    if (!self) {
        return;
    }

    impl = (none_input_t *)self->impl;
    if (impl) {
        free(impl);
    }
    free(self);
}

static bool none_input_poll_event(input_t *self, input_event_t *event) {
    (void)self;
    (void)event;
    return false;
}

static const input_vtable_t g_none_input_vtable = {
    .destroy = none_input_destroy,
    .poll_event = none_input_poll_event,
};

input_t *input_create_none(void) {
    input_t *input = (input_t *)malloc(sizeof(input_t));
    none_input_t *impl = (none_input_t *)malloc(sizeof(none_input_t));
    input->impl = impl;
    input->vt = &g_none_input_vtable;
    return input;
}
