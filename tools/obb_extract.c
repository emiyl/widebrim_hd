#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#define ARRAY_LEN(x) (sizeof(x) / sizeof((x)[0]))

typedef struct {
    char *name;
    uint32_t offset;
    uint32_t size;
} FileEntry;

static uint32_t read_le32(const uint8_t *p) {
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) |
           ((uint32_t)p[3] << 24);
}

static void decrypt_buffer(const uint8_t *in, uint8_t *out, size_t size,
                           uint32_t offset) {
    uint32_t x = (offset + 0x45243u) & 0xffffffffu;

    for (size_t i = 0; i < size; ++i) {
        x = (x * 0x41C64E6Du + 0x3039u) & 0xffffffffu;
        out[i] = in[i] ^ (uint8_t)(x >> 24);
    }
}

static int starts_with_prefix(const char *name, const char *prefix) {
    size_t prefix_len = strlen(prefix);
    if (prefix_len == 0) {
        return 1;
    }

    size_t name_len = strlen(name);
    if (name_len < prefix_len) {
        return 0;
    }

    return memcmp(name, prefix, prefix_len) == 0;
}

static char *normalize_slashes(const char *path) {
    char *copy = strdup(path);
    if (copy == NULL) {
        return NULL;
    }

    for (char *p = copy; *p != '\0'; ++p) {
        if (*p == '\\') {
            *p = '/';
        }
    }

    return copy;
}

static int ensure_directory(const char *path) {
    char tmp[4096];
    size_t len = strlen(path);
    if (len >= sizeof(tmp)) {
        fprintf(stderr, "Path too long: %s\n", path);
        return -1;
    }

    memcpy(tmp, path, len + 1);

    for (char *p = tmp + 1; *p != '\0'; ++p) {
        if (*p == '/') {
            *p = '\0';
            if (mkdir(tmp, 0777) != 0 && errno != EEXIST) {
                fprintf(stderr, "mkdir(%s): %s\n", tmp, strerror(errno));
                return -1;
            }
            *p = '/';
        }
    }

    if (mkdir(tmp, 0777) != 0 && errno != EEXIST) {
        fprintf(stderr, "mkdir(%s): %s\n", tmp, strerror(errno));
        return -1;
    }

    return 0;
}

static int write_output_file(const char *output_root, const char *entry_name,
                             const uint8_t *data, size_t size) {
    char normalized_name[4096];
    char full_path[4096];

    if (strlen(entry_name) >= sizeof(normalized_name)) {
        fprintf(stderr, "Entry name too long: %s\n", entry_name);
        return -1;
    }

    snprintf(normalized_name, sizeof(normalized_name), "%s", entry_name);
    for (char *p = normalized_name; *p != '\0'; ++p) {
        if (*p == '\\') {
            *p = '/';
        }
    }

    if (snprintf(full_path, sizeof(full_path), "%s/%s", output_root,
                 normalized_name) >= (int)sizeof(full_path)) {
        fprintf(stderr, "Output path is too long for %s\n", entry_name);
        return -1;
    }

    char parent[4096];
    snprintf(parent, sizeof(parent), "%s", full_path);
    char *slash = strrchr(parent, '/');
    if (slash == NULL) {
        fprintf(stderr, "Invalid output path: %s\n", full_path);
        return -1;
    }

    *slash = '\0';
    if (ensure_directory(parent) != 0) {
        return -1;
    }

    FILE *out = fopen(full_path, "wb");
    if (out == NULL) {
        fprintf(stderr, "fopen(%s): %s\n", full_path, strerror(errno));
        return -1;
    }

    if (size > 0 && fwrite(data, 1, size, out) != size) {
        fprintf(stderr, "fwrite(%s): %s\n", full_path, strerror(errno));
        fclose(out);
        return -1;
    }

    fclose(out);
    return 0;
}

static int parse_file_table(const uint8_t *table, size_t table_size,
                            FileEntry **entries_out, size_t *count_out) {
    if (table_size < 4) {
        return -1;
    }

    uint32_t file_count = read_le32(table);
    FileEntry *entries = calloc(file_count ? file_count : 1, sizeof(*entries));
    if (entries == NULL) {
        fprintf(stderr, "calloc failed\n");
        return -1;
    }

    size_t count = 0;
    for (uint32_t i = 0; i < file_count; ++i) {
        size_t base = 4u + (size_t)i * 12u;
        if (base + 12u > table_size) {
            fprintf(stderr, "Malformed file table\n");
            free(entries);
            return -1;
        }

        uint32_t name_offset = read_le32(table + base);
        uint32_t file_offset = read_le32(table + base + 4u);
        uint32_t file_size = read_le32(table + base + 8u);

        if (name_offset >= table_size) {
            fprintf(stderr, "Name offset out of range\n");
            free(entries);
            return -1;
        }

        size_t name_len = 0;
        while (name_offset + name_len < table_size &&
               table[name_offset + name_len] != 0) {
            ++name_len;
        }

        if (name_len >= 4096) {
            fprintf(stderr, "File name too long in table\n");
            free(entries);
            return -1;
        }

        char *name = malloc(name_len + 1u);
        if (name == NULL) {
            fprintf(stderr, "malloc name failed\n");
            free(entries);
            return -1;
        }

        memcpy(name, table + name_offset, name_len);
        name[name_len] = '\0';

        entries[count].name = name;
        entries[count].offset = file_offset;
        entries[count].size = file_size;
        ++count;
    }

    *entries_out = entries;
    *count_out = count;
    return 0;
}

static int matches_any_prefix(const char *name, const char *const *prefixes,
                              size_t prefix_count) {
    if (prefix_count == 0) {
        return 1;
    }

    for (size_t i = 0; i < prefix_count; ++i) {
        const char *prefix = prefixes[i];
        if (prefix == NULL || prefix[0] == '\0') {
            continue;
        }

        size_t prefix_len = strlen(prefix);
        char *prefix_copy = normalize_slashes(prefix);
        if (prefix_copy == NULL) {
            return 0;
        }

        size_t normalized_len = strlen(prefix_copy);
        if (normalized_len != prefix_len) {
            /* This is harmless in practice; the comparison below is still
             * valid. */
        }

        char *name_copy = normalize_slashes(name);
        if (name_copy == NULL) {
            free(prefix_copy);
            return 0;
        }

        int ok = 0;
        if (prefix_copy[normalized_len - 1] == '/') {
            ok = starts_with_prefix(name_copy, prefix_copy);
        } else {
            char prefixed[4096];
            snprintf(prefixed, sizeof(prefixed), "%s/", prefix_copy);
            ok = starts_with_prefix(name_copy, prefixed);
        }

        free(prefix_copy);
        free(name_copy);

        if (ok) {
            return 1;
        }
    }

    return 0;
}

static int extract_obb(const char *obb_path, const char *output_root,
                       const char *const *prefixes, size_t prefix_count) {
    size_t file_size = 0;
    uint8_t *file_data = NULL;
    FILE *f = fopen(obb_path, "rb");
    if (f == NULL) {
        fprintf(stderr, "fopen(%s): %s\n", obb_path, strerror(errno));
        return 1;
    }

    if (fseek(f, 0, SEEK_END) != 0) {
        fprintf(stderr, "fseek(%s): %s\n", obb_path, strerror(errno));
        fclose(f);
        return 1;
    }

    long file_len = ftell(f);
    if (file_len < 0) {
        fprintf(stderr, "ftell(%s): %s\n", obb_path, strerror(errno));
        fclose(f);
        return 1;
    }

    rewind(f);
    file_size = (size_t)file_len;
    file_data = malloc(file_size == 0 ? 1 : file_size);
    if (file_data == NULL) {
        fprintf(stderr, "malloc for OBB failed\n");
        fclose(f);
        return 1;
    }

    if (file_size > 0 && fread(file_data, 1, file_size, f) != file_size) {
        fprintf(stderr, "fread(%s): %s\n", obb_path, strerror(errno));
        free(file_data);
        fclose(f);
        return 1;
    }
    fclose(f);

    uint8_t *header_dec = malloc(0x14u);
    if (header_dec == NULL) {
        fprintf(stderr, "malloc for header failed\n");
        free(file_data);
        return 1;
    }

    decrypt_buffer(file_data, header_dec, 0x14u, 0u);
    if (memcmp(header_dec, "ARC1", 4) != 0) {
        fprintf(stderr, "Invalid OBB header. Expected ARC1.\n");
        free(header_dec);
        free(file_data);
        return 1;
    }

    uint32_t table_offset = read_le32(header_dec + 8u);
    uint32_t table_size = read_le32(header_dec + 12u);
    uint32_t file_table_end = table_offset + table_size;
    if (table_offset > file_size || file_table_end < table_offset ||
        file_table_end > file_size) {
        fprintf(stderr, "Table offset/size out of range for OBB\n");
        free(header_dec);
        free(file_data);
        return 1;
    }

    uint8_t *table_dec = malloc(table_size == 0 ? 1u : table_size);
    if (table_dec == NULL) {
        fprintf(stderr, "malloc for table failed\n");
        free(header_dec);
        free(file_data);
        return 1;
    }

    decrypt_buffer(file_data + table_offset, table_dec, table_size,
                   table_offset);

    FileEntry *entries = NULL;
    size_t entry_count = 0;
    if (parse_file_table(table_dec, table_size, &entries, &entry_count) != 0) {
        fprintf(stderr, "Could not parse file table\n");
        free(table_dec);
        free(header_dec);
        free(file_data);
        return 1;
    }

    if (ensure_directory(output_root) != 0) {
        free(table_dec);
        free(header_dec);
        free(file_data);
        for (size_t i = 0; i < entry_count; ++i) {
            free(entries[i].name);
        }
        free(entries);
        return 1;
    }

    for (size_t i = 0; i < entry_count; ++i) {
        FileEntry *entry = &entries[i];
        if (!matches_any_prefix(entry->name, prefixes, prefix_count)) {
            continue;
        }

        uint32_t end = entry->offset + entry->size;
        if (entry->offset > file_size || end < entry->offset ||
            end > file_size) {
            fprintf(stderr, "Entry out of range: %s\n", entry->name);
            continue;
        }

        uint8_t *data = NULL;
        size_t size = entry->size;
        if (size == 0) {
            data = NULL;
        } else {
            data = malloc(size);
            if (data == NULL) {
                fprintf(stderr, "malloc for payload failed\n");
                continue;
            }
        }

        if (size > 0) {
            if (strstr(entry->name, ".mp4") == NULL) {
                decrypt_buffer(file_data + entry->offset, data, size,
                               entry->offset);
            } else {
                memcpy(data, file_data + entry->offset, size);
            }
        }

        if (write_output_file(output_root, entry->name, data, size) != 0) {
            fprintf(stderr, "Failed writing %s\n", entry->name);
            free(data);
            continue;
        }

        free(data);
        printf("Extracted %s\n", entry->name);
    }

    for (size_t i = 0; i < entry_count; ++i) {
        free(entries[i].name);
    }
    free(entries);
    free(table_dec);
    free(header_dec);
    free(file_data);
    return 0;
}

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <obb_file> <output_dir> [prefix ...]\n",
                argv[0]);
        fprintf(
            stderr,
            "Example: %s assets/main.obb assets/extracted place script sound\n",
            argv[0]);
        return 1;
    }

    const char *obb_path = argv[1];
    const char *output_root = argv[2];
    const char **prefixes = NULL;
    size_t prefix_count = (size_t)(argc - 3);

    if (argc > 3) {
        prefixes = (const char **)(argv + 3);
    }

    int rc = extract_obb(obb_path, output_root, prefixes, prefix_count);
    return rc;
}
