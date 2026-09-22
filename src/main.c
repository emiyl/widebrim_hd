#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "runtime.h"

static void print_usage(const char *argv0) {
    printf("Usage: %s\n --assets <assets_path> [--language en]\n", argv0);
}

int main(int argc, char *argv[]) {
    const char *assets_root = NULL;
    const char *language = "en";

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--assets") == 0 && i + 1 < argc) {
            assets_root = argv[i + 1];
        } else if (strcmp(argv[i], "--language") == 0 && i + 1 < argc) {
            language = argv[i + 1];
        } else if (strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 1;
        }
    }

    if (assets_root == NULL) {
        assets_root = "assets";
        fprintf(stderr,
                "widebrim: --assets <assets_path> not specified, defaulting to "
                "'%s'\n",
                assets_root);
    }

    if (access(assets_root, F_OK) != 0) {
        fprintf(stderr, "widebrim: assets directory '%s' does not exist.\n",
                assets_root);
        return 1;
    }

    runtime_t runtime;
    runtime_init(&runtime, assets_root, language);

    fprintf(stderr, "widebrim: exiting\n");
    return 0;
}
