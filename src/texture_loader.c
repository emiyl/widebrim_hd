#include "texture_loader.h"

#include <stdbool.h>
#include <stdint.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

texture_data_t *texture_load_rgba(const char *path) {
    texture_data_t *tex;
    int original_channels;

    if (path == NULL) {
        fprintf(stderr, "widebrim: texture_load_rgba called with NULL path\n");
        return NULL;
    }

    tex = malloc(sizeof(texture_data_t));

    if (tex == NULL) {
        fprintf(stderr, "widebrim: Failed to allocate texture_data_t\n");
        return NULL;
    }

    tex->pixels =
        stbi_load(path, &tex->width, &tex->height, &original_channels, 4);

    if (tex->pixels == NULL) {
        fprintf(stderr, "widebrim: Failed to load image: %s (%s)\n", path,
                stbi_failure_reason());

        free(tex);
        return NULL;
    }

    tex->channels = 4;

    return tex;
}

void texture_free(texture_data_t *tex) {

    if (tex == NULL) {
        return;
    }

    stbi_image_free(tex->pixels);
    free(tex);
}
