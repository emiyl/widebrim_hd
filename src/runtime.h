#ifndef RUNTIME_H
#define RUNTIME_H

#include <stdbool.h>

#include "input.h"
#include "window.h"

typedef struct {
    window_t *window;
    input_t *input;
    bool running;
} runtime_t;

int runtime_init(runtime_t *runtime, const char *assets_root,
                 const char *language);
void runtime_destroy(runtime_t *runtime);
void runtime_run(runtime_t *runtime);

#endif // RUNTIME_H
