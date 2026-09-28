#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

static void print_usage(const char *argv0) {
    fprintf(stderr,
            "Usage: %s --input <input.spr> --output <output.spr> --scale <N>\n",
            argv0);
    fprintf(stderr, "       %s <input.spr> <output.spr> <N>\n", argv0);
}

static uint32_t read_u32_le(const uint8_t *bytes) {
    return (uint32_t)bytes[0] | ((uint32_t)bytes[1] << 8U) |
           ((uint32_t)bytes[2] << 16U) | ((uint32_t)bytes[3] << 24U);
}

static int16_t read_i16_le(const uint8_t *bytes) {
    return (int16_t)((uint16_t)bytes[0] | ((uint16_t)bytes[1] << 8U));
}

static uint16_t read_u16_le(const uint8_t *bytes) {
    return (uint16_t)bytes[0] | ((uint16_t)bytes[1] << 8U);
}

static void write_i16_le(uint8_t *bytes, int16_t value) {
    bytes[0] = (uint8_t)((uint16_t)value & 0xFFU);
    bytes[1] = (uint8_t)(((uint16_t)value >> 8U) & 0xFFU);
}

static void write_u16_le(uint8_t *bytes, uint16_t value) {
    bytes[0] = (uint8_t)(value & 0xFFU);
    bytes[1] = (uint8_t)((value >> 8U) & 0xFFU);
}

static bool read_file_bytes(const char *path, uint8_t **buffer_out,
                            size_t *size_out) {
    FILE *file;
    long file_size;
    uint8_t *buffer;

    if (path == NULL || buffer_out == NULL || size_out == NULL) {
        return false;
    }

    file = fopen(path, "rb");
    if (file == NULL) {
        fprintf(stderr, "failed to open %s: %s\n", path, strerror(errno));
        return false;
    }

    if (fseek(file, 0L, SEEK_END) != 0) {
        fprintf(stderr, "failed to seek %s\n", path);
        fclose(file);
        return false;
    }

    file_size = ftell(file);
    if (file_size < 0L) {
        fprintf(stderr, "failed to determine size of %s\n", path);
        fclose(file);
        return false;
    }

    if (fseek(file, 0L, SEEK_SET) != 0) {
        fprintf(stderr, "failed to rewind %s\n", path);
        fclose(file);
        return false;
    }

    buffer = malloc((size_t)file_size == 0U ? 1U : (size_t)file_size);
    if (buffer == NULL) {
        fprintf(stderr, "failed to allocate %ld bytes for %s\n", file_size,
                path);
        fclose(file);
        return false;
    }

    if (file_size > 0L &&
        fread(buffer, 1U, (size_t)file_size, file) != (size_t)file_size) {
        fprintf(stderr, "failed to read %s\n", path);
        free(buffer);
        fclose(file);
        return false;
    }

    fclose(file);
    *buffer_out = buffer;
    *size_out = (size_t)file_size;
    return true;
}

static bool write_file_bytes(const char *path, const uint8_t *buffer,
                             size_t size) {
    char tmp[4096];
    char *slash;
    FILE *file;

    if (path == NULL || buffer == NULL || size == 0U) {
        return false;
    }

    if (strlen(path) >= sizeof(tmp)) {
        fprintf(stderr, "output path is too long: %s\n", path);
        return false;
    }

    strcpy(tmp, path);
    slash = strrchr(tmp, '/');
    if (slash != NULL) {
        *slash = '\0';
        if (tmp[0] != '\0') {
            char *cursor;
            for (cursor = tmp + 1; *cursor != '\0'; ++cursor) {
                if (*cursor == '/') {
                    *cursor = '\0';
                    if (mkdir(tmp, 0777) != 0 && errno != EEXIST) {
                        fprintf(stderr, "mkdir(%s): %s\n", tmp,
                                strerror(errno));
                        return false;
                    }
                    *cursor = '/';
                }
            }
            if (mkdir(tmp, 0777) != 0 && errno != EEXIST) {
                fprintf(stderr, "mkdir(%s): %s\n", tmp, strerror(errno));
                return false;
            }
        }
    }

    file = fopen(path, "wb");
    if (file == NULL) {
        fprintf(stderr, "failed to open %s for writing: %s\n", path,
                strerror(errno));
        return false;
    }

    if (fwrite(buffer, 1U, size, file) != size) {
        fprintf(stderr, "failed to write %s\n", path);
        fclose(file);
        return false;
    }

    fclose(file);
    return true;
}

static bool parse_scale(const char *text, uint32_t *scale_out) {
    char *end = NULL;
    unsigned long value;

    if (text == NULL || scale_out == NULL) {
        return false;
    }

    errno = 0;
    value = strtoul(text, &end, 10);
    if (errno != 0 || end == text || *end != '\0' || value == 0UL ||
        value > UINT32_MAX) {
        return false;
    }

    *scale_out = (uint32_t)value;
    return true;
}

static bool scale_sprite_file(const char *input_path, const char *output_path,
                              uint32_t scale) {
    uint8_t *input = NULL;
    uint8_t *output = NULL;
    size_t size = 0U;
    uint32_t frame_count;
    size_t offset;

    if (input_path == NULL || output_path == NULL) {
        return false;
    }

    if (!read_file_bytes(input_path, &input, &size)) {
        return false;
    }

    if (size < 4U) {
        fprintf(stderr, "%s is too small to be a .spr file\n", input_path);
        free(input);
        return false;
    }

    frame_count = read_u32_le(input);
    offset = 4U;
    if ((size_t)frame_count > (SIZE_MAX - 4U) / 8U) {
        fprintf(stderr, "frame count in %s is absurdly large\n", input_path);
        free(input);
        return false;
    }
    if (offset + (size_t)frame_count * 8U > size) {
        fprintf(stderr, "%s is truncated; frame table is incomplete\n",
                input_path);
        free(input);
        return false;
    }

    output = malloc(size);
    if (output == NULL) {
        fprintf(stderr, "failed to allocate output buffer for %s\n",
                output_path);
        free(input);
        return false;
    }
    memcpy(output, input, size);

    for (uint32_t frame_index = 0U; frame_index < frame_count; ++frame_index) {
        size_t frame_offset = offset + (size_t)frame_index * 8U;
        int16_t x = read_i16_le(input + frame_offset);
        int16_t y = read_i16_le(input + frame_offset + 2U);
        uint16_t width = read_u16_le(input + frame_offset + 4U);
        uint16_t height = read_u16_le(input + frame_offset + 6U);
        int64_t scaled_x = (int64_t)x * (int64_t)scale;
        int64_t scaled_y = (int64_t)y * (int64_t)scale;
        int64_t scaled_width = (int64_t)width * (int64_t)scale;
        int64_t scaled_height = (int64_t)height * (int64_t)scale;

        if (scaled_x < INT16_MIN || scaled_x > INT16_MAX ||
            scaled_y < INT16_MIN || scaled_y > INT16_MAX || scaled_width < 0 ||
            scaled_width > UINT16_MAX || scaled_height < 0 ||
            scaled_height > UINT16_MAX) {
            fprintf(stderr,
                    "frame %u in %s exceeds int16/uint16 range after scaling "
                    "by %u\n",
                    frame_index, input_path, scale);
            free(input);
            free(output);
            return false;
        }

        write_i16_le(output + frame_offset, (int16_t)scaled_x);
        write_i16_le(output + frame_offset + 2U, (int16_t)scaled_y);
        write_u16_le(output + frame_offset + 4U, (uint16_t)scaled_width);
        write_u16_le(output + frame_offset + 6U, (uint16_t)scaled_height);
    }

    if (!write_file_bytes(output_path, output, size)) {
        free(input);
        free(output);
        return false;
    }

    free(input);
    free(output);
    return true;
}

int main(int argc, char **argv) {
    const char *input_path = NULL;
    const char *output_path = NULL;
    uint32_t scale = 0U;
    bool used_option_scale = false;

    if (argc == 2 &&
        (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0)) {
        print_usage(argv[0]);
        return 0;
    }

    for (int index = 1; index < argc; ++index) {
        if (strcmp(argv[index], "--input") == 0 ||
            strcmp(argv[index], "-i") == 0) {
            if (index + 1 >= argc) {
                print_usage(argv[0]);
                return 1;
            }
            input_path = argv[++index];
        } else if (strcmp(argv[index], "--output") == 0 ||
                   strcmp(argv[index], "-o") == 0) {
            if (index + 1 >= argc) {
                print_usage(argv[0]);
                return 1;
            }
            output_path = argv[++index];
        } else if (strcmp(argv[index], "--scale") == 0 ||
                   strcmp(argv[index], "-s") == 0) {
            if (index + 1 >= argc) {
                print_usage(argv[0]);
                return 1;
            }
            if (!parse_scale(argv[++index], &scale)) {
                fprintf(stderr, "invalid scale: %s\n", argv[index]);
                print_usage(argv[0]);
                return 1;
            }
            used_option_scale = true;
        } else if (argv[index][0] == '-') {
            fprintf(stderr, "unknown option: %s\n", argv[index]);
            print_usage(argv[0]);
            return 1;
        } else if (input_path == NULL) {
            input_path = argv[index];
        } else if (output_path == NULL) {
            output_path = argv[index];
        } else if (!used_option_scale) {
            if (!parse_scale(argv[index], &scale)) {
                fprintf(stderr, "invalid scale: %s\n", argv[index]);
                print_usage(argv[0]);
                return 1;
            }
            used_option_scale = true;
        } else {
            fprintf(stderr, "unexpected argument: %s\n", argv[index]);
            print_usage(argv[0]);
            return 1;
        }
    }

    if (input_path == NULL || output_path == NULL || scale == 0U) {
        print_usage(argv[0]);
        return 1;
    }

    if (!scale_sprite_file(input_path, output_path, scale)) {
        fprintf(stderr, "failed to scale sprite metadata in %s\n", input_path);
        return 1;
    }

    fprintf(stdout, "scaled %s -> %s by %u\n", input_path, output_path, scale);
    return 0;
}
