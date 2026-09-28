#include "texture_loader.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef HAVE_LIBWEBP
#include <webp/decode.h>
#endif

#ifndef HAVE_LIBWEBP
#define HAVE_LIBWEBP 0
#endif

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#if HAVE_LIBWEBP
static bool texture_load_webp(const char *path, texture_data_t *tex) {
    FILE *file = NULL;
    uint8_t *buffer = NULL;
    uint8_t *pixels = NULL;
    long file_size = 0L;
    size_t read_size;

    if (path == NULL || tex == NULL) {
        return false;
    }

    file = fopen(path, "rb");
    if (file == NULL) {
        fprintf(stderr, "widebrim: Failed to open WebP image: %s\n", path);
        return false;
    }

    if (fseek(file, 0L, SEEK_END) != 0) {
        fprintf(stderr, "widebrim: Failed to seek WebP image: %s\n", path);
        fclose(file);
        return false;
    }

    file_size = ftell(file);
    if (file_size < 0L) {
        fprintf(stderr, "widebrim: Failed to determine WebP image size: %s\n",
                path);
        fclose(file);
        return false;
    }

    if (fseek(file, 0L, SEEK_SET) != 0) {
        fprintf(stderr, "widebrim: Failed to rewind WebP image: %s\n", path);
        fclose(file);
        return false;
    }

    buffer = malloc((size_t)file_size);
    if (buffer == NULL) {
        fprintf(stderr, "widebrim: Failed to allocate WebP buffer for %s\n",
                path);
        fclose(file);
        return false;
    }

    read_size = fread(buffer, 1U, (size_t)file_size, file);
    fclose(file);

    if (read_size != (size_t)file_size) {
        fprintf(stderr, "widebrim: Failed to read WebP image: %s\n", path);
        free(buffer);
        return false;
    }

    pixels =
        WebPDecodeRGBA(buffer, (size_t)file_size, &tex->width, &tex->height);
    free(buffer);

    if (pixels == NULL) {
        fprintf(stderr, "widebrim: libwebp failed to decode WebP image: %s\n",
                path);
        return false;
    }

    tex->pixels = pixels;
    tex->channels = 4;
    tex->is_webp = true;
    return true;
}
#else
static bool texture_load_webp(const char *path, texture_data_t *tex) {
    (void)path;
    (void)tex;
    return false;
}
#endif

texture_data_t *texture_load_rgba(const char *path) {
    texture_data_t *tex;
    int original_channels;
    const char *ext;

    if (path == NULL) {
        fprintf(stderr, "widebrim: texture_load_rgba called with NULL path\n");
        return NULL;
    }

    tex = calloc(1U, sizeof(*tex));
    if (tex == NULL) {
        fprintf(stderr, "widebrim: Failed to allocate texture_data_t\n");
        return NULL;
    }

    ext = strrchr(path, '.');
    if (ext != NULL &&
        (strcmp(ext, ".webp") == 0 || strcmp(ext, ".WEBP") == 0)) {
        if (!texture_load_webp(path, tex)) {
#if HAVE_LIBWEBP
            free(tex);
            return NULL;
#else
            fprintf(
                stderr,
                "widebrim: WebP decoding support was not compiled in for %s\n",
                path);
            free(tex);
            return NULL;
#endif
        }
        return tex;
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
    tex->is_webp = false;
    return tex;
}

void texture_free(texture_data_t *tex) {
    if (tex == NULL) {
        return;
    }

    if (tex->pixels != NULL) {
#if HAVE_LIBWEBP
        if (tex->is_webp) {
            WebPFree(tex->pixels);
        } else {
            stbi_image_free(tex->pixels);
        }
#else
        stbi_image_free(tex->pixels);
#endif
    }
    free(tex);
}
