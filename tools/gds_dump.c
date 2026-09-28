#include "gds/gds_dump.h"
#include "gds/gds.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void print_usage(const char *program_name) {
    fprintf(stderr, "Usage: %s [--raw|-r] <gds-file>\n", program_name);
}

int main(int argc, char **argv) {
    const uint8_t *payload = NULL;
    const char *file_path = NULL;
    bool raw_mode = false;
    size_t payload_size = 0;
    int i;

    if (argc < 2) {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--raw") == 0 || strcmp(argv[i], "-r") == 0) {
            raw_mode = true;
            continue;
        }

        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            print_usage(argv[0]);
            return EXIT_SUCCESS;
        }

        if (file_path != NULL) {
            print_usage(argv[0]);
            return EXIT_FAILURE;
        }

        file_path = argv[i];
    }

    if (file_path == NULL) {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    if (!gds_load_from_file_path(file_path, &payload, &payload_size)) {
        fprintf(stderr, "Failed to load GDS script: %s\n", file_path);
        return EXIT_FAILURE;
    }

    if (raw_mode) {
        if (!dump_gds_raw(payload, payload_size)) {
            fprintf(stderr, "Failed to dump raw GDS script: %s\n", file_path);
            gds_free_payload(payload);
            return EXIT_FAILURE;
        }
    } else if (!dump_gds(payload, payload_size)) {
        fprintf(stderr, "Failed to dump GDS script: %s\n", file_path);
        gds_free_payload(payload);
        return EXIT_FAILURE;
    }

    gds_free_payload(payload);
    return EXIT_SUCCESS;
}
