#include "gds/gds_exec.h"
#include "gds/gds.h"
#include "gds/gds_state.h"

#include "game_state.h"
#include "modes/mode_impl.h"

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

static void print_usage(const char *program_name) {
    fprintf(stderr, "Usage: %s <script.gsc|script.gds>\n", program_name);
}

static bool has_suffix(const char *path, const char *suffix) {
    size_t path_len = strlen(path);
    size_t suffix_len = strlen(suffix);

    if (path_len < suffix_len) {
        return false;
    }

    return strcmp(path + path_len - suffix_len, suffix) == 0;
}

static bool compile_gsc_to_temp(const char *source_path, char *temp_path,
                                size_t temp_path_size) {
    char compiler_candidate[PATH_MAX];
    char temp_template[] = "/tmp/gds_exec_XXXXXX";
    int fd;
    pid_t child;
    int status;

    if (source_path == NULL || temp_path == NULL || temp_path_size == 0U) {
        return false;
    }

    snprintf(compiler_candidate, sizeof(compiler_candidate), "%s",
             "./build/gds_compile");
    if (access(compiler_candidate, X_OK) != 0) {
        snprintf(compiler_candidate, sizeof(compiler_candidate), "%s",
                 "./gds_compile");
    }
    if (access(compiler_candidate, X_OK) != 0) {
        fprintf(stderr, "gds_exec: could not find gds_compile; expected it "
                        "next to the build output or in ./build\n");
        return false;
    }

    fd = mkstemp(temp_template);
    if (fd < 0) {
        fprintf(stderr, "gds_exec: could not create temporary output: %s\n",
                strerror(errno));
        return false;
    }
    close(fd);

    if (snprintf(temp_path, temp_path_size, "%s", temp_template) >=
        (int)temp_path_size) {
        unlink(temp_template);
        return false;
    }

    child = fork();
    if (child == -1) {
        fprintf(stderr, "gds_exec: fork failed: %s\n", strerror(errno));
        unlink(temp_template);
        return false;
    }

    if (child == 0) {
        execl(compiler_candidate, "gds_compile", source_path, temp_template,
              (char *)NULL);
        fprintf(stderr, "gds_exec: failed to run %s: %s\n", compiler_candidate,
                strerror(errno));
        _exit(EXIT_FAILURE);
    }

    if (waitpid(child, &status, 0) < 0) {
        fprintf(stderr, "gds_exec: waitpid failed: %s\n", strerror(errno));
        unlink(temp_template);
        return false;
    }

    if (!WIFEXITED(status) || WEXITSTATUS(status) != EXIT_SUCCESS) {
        fprintf(stderr, "gds_exec: failed to compile source script: %s\n",
                source_path);
        unlink(temp_template);
        return false;
    }

    return true;
}

int main(int argc, char **argv) {
    const char *input_path = NULL;
    const char *script_path = NULL;
    const uint8_t *payload = NULL;
    size_t payload_size = 0U;
    game_state_t state;
    mode_impl_t impl;
    char temp_script_path[PATH_MAX];
    bool temp_script_used = false;

    if (argc != 2) {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    input_path = argv[1];
    if (strcmp(input_path, "--help") == 0 || strcmp(input_path, "-h") == 0) {
        print_usage(argv[0]);
        return EXIT_SUCCESS;
    }

    if (has_suffix(input_path, ".gsc") || has_suffix(input_path, ".GSC")) {
        memset(temp_script_path, 0, sizeof(temp_script_path));
        if (!compile_gsc_to_temp(input_path, temp_script_path,
                                 sizeof(temp_script_path))) {
            return EXIT_FAILURE;
        }
        script_path = temp_script_path;
        temp_script_used = true;
    } else {
        script_path = input_path;
    }

    memset(&state, 0, sizeof(state));
    game_state_reset(&state);

    impl.state = &state;
    impl.controller = NULL;
    impl.done = false;

    if (!gds_load_from_file_path(script_path, &payload, &payload_size)) {
        fprintf(stderr, "gds_exec: failed to load script: %s\n", script_path);
        if (temp_script_used) {
            unlink(script_path);
        }
        return EXIT_FAILURE;
    }

    if (!gds_execute_script(payload, payload_size, &impl, &state.gds)) {
        fprintf(stderr, "gds_exec: failed to execute script: %s\n",
                script_path);
        gds_free_payload(payload);
        if (temp_script_used) {
            unlink(script_path);
        }
        return EXIT_FAILURE;
    }

    printf("gds_exec: completed %s\n", input_path);
    gds_free_payload(payload);
    if (temp_script_used) {
        unlink(script_path);
    }
    return EXIT_SUCCESS;
}
