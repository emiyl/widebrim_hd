#ifndef SAFE_H
#define SAFE_H

#include <stdio.h>
#include <stdlib.h>

static inline void *smalloc(size_t size) {
    void *ptr = malloc(size);
    if (!ptr) {
        fprintf(stderr, "widebrim: malloc failed to allocate %zu bytes\n",
                (size_t)(size));
        exit(EXIT_FAILURE);
    }
    return ptr;
}

#endif // SAFE_H
