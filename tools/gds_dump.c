#include "gds/gds_dump.h"
#include "gds/gds.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void print_usage(const char *program_name) {
    fprintf(stderr, "Usage: %s <gds-file>\n", program_name);
}

int main(int argc, char **argv) {
    const uint8_t *payload = NULL;
    size_t payload_size = 0;

    if (argc != 2) {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
        print_usage(argv[0]);
        return EXIT_SUCCESS;
    }

    if (!gds_load_from_file_path(argv[1], &payload, &payload_size)) {
        fprintf(stderr, "Failed to load GDS script: %s\n", argv[1]);
        return EXIT_FAILURE;
    }

    if (!dump_gds(payload, payload_size)) {
        fprintf(stderr, "Failed to dump GDS script: %s\n", argv[1]);
        gds_free_payload(payload);
        return EXIT_FAILURE;
    }

    gds_free_payload(payload);
    return EXIT_SUCCESS;
}
