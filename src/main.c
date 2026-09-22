#include <stdio.h>
#include <string.h>

static void print_usage(const char* argv0) {
    printf("Usage: %s\n --assets <assets_path> [--language en]\n", argv0);
}

int main(int argc, char* argv[]) {
    const char *assets_root = NULL;
    const char* language = "en";

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--assets") == 0 && i + 1 < argc) {
            assets_root = argv[i + 1];
        } else if (strcmp(argv[i], "--language") == 0 && i + 1 < argc) {
            language = argv[i + 1];
        } else {
            print_usage(argv[0]);
            return 1;
        }
    }

    if (assets_root == NULL) {
        fprintf(stderr, "widebrim: --assets <assets_path> is required.\n");
        print_usage(argv[0]);
        return 1;
    }

    fprintf(stderr, "widebrim: exiting\n");
    return 0;
}