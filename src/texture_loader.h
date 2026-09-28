#ifndef TEXTURE_LOADER_H
#define TEXTURE_LOADER_H

#include <stdbool.h>

typedef struct texture_data_t {
    unsigned char *pixels;
    int width;
    int height;
    int channels;
    bool is_webp;
} texture_data_t;

texture_data_t *texture_load_rgba(const char *path);
void texture_free(texture_data_t *tex);

#endif // TEXTURE_LOADER_H
