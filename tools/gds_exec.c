#include "../src/gds/gds_exec.h"
#include "../src/gds/gds.h"

#include "../src/game_state.h"
#include "../src/modes/mode_impl.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#ifdef _WIN32
#include <io.h>
#include <process.h>
#define access _access
#define unlink _unlink
#ifndef X_OK
#define X_OK 1
#endif
#else
#include <sys/wait.h>
#include <unistd.h>
#endif

#define GDS_EXEC_PATH_MAX 4096

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

static bool resolve_tool_path(const char *argv0, const char *tool_name,
                              char *out_path, size_t out_path_size) {
    const char *last_slash = strrchr(argv0, '/');
    char dir_candidate[GDS_EXEC_PATH_MAX];
    const char *candidate_names[4];
    size_t candidate_count = 0U;
    size_t i;

    if (argv0 == NULL || tool_name == NULL || out_path == NULL ||
        out_path_size == 0U) {
        return false;
    }

    if (last_slash != NULL) {
        size_t dir_len = (size_t)(last_slash - argv0);
        size_t tool_len = strlen(tool_name);
        size_t candidate_len;

        if (dir_len >= sizeof(dir_candidate)) {
            return false;
        }

        candidate_len = dir_len + 1U + tool_len + 1U;
        if (candidate_len > out_path_size) {
            return false;
        }

        memcpy(dir_candidate, argv0, dir_len);
        dir_candidate[dir_len] = '\0';
        memcpy(out_path, dir_candidate, dir_len);
        out_path[dir_len] = '/';
        memcpy(out_path + dir_len + 1U, tool_name, tool_len + 1U);
        if (access(out_path, X_OK) == 0) {
            return true;
        }
    }

    candidate_names[candidate_count++] = "./build/gds_compile";
    candidate_names[candidate_count++] = "./gds_compile";
    candidate_names[candidate_count++] = tool_name;

    for (i = 0U; i < candidate_count; ++i) {
        size_t candidate_len = strlen(candidate_names[i]);

        if (candidate_len >= out_path_size) {
            continue;
        }

        memcpy(out_path, candidate_names[i], candidate_len + 1U);
        if (access(out_path, X_OK) == 0) {
            return true;
        }
    }

    return false;
}

static bool compile_gsc_to_temp(const char *source_path,
                                const char *compiler_path, char *temp_path,
                                size_t temp_path_size) {
    char temp_template[GDS_EXEC_PATH_MAX];
    int fd;
#ifndef _WIN32
    pid_t child;
    int status;
#endif
    static unsigned long long temp_counter = 0ULL;

    if (source_path == NULL || compiler_path == NULL || temp_path == NULL ||
        temp_path_size == 0U) {
        return false;
    }

    if (access(compiler_path, X_OK) != 0) {
        fprintf(stderr, "gds_exec: could not find gds_compile; expected it "
                        "next to the build output or in ./build\n");
        return false;
    }

    do {
        snprintf(temp_template, sizeof(temp_template), "/tmp/gds_exec_%llu_%ld",
                 temp_counter++, (long)getpid());
        fd = open(temp_template, O_RDWR | O_CREAT | O_EXCL, 0600);
    } while (fd < 0 && errno == EEXIST);

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

#ifdef _WIN32
    {
        const char *args[] = {"gds_compile", source_path, temp_template, NULL};
        int rc = _spawnv(_P_WAIT, compiler_path, args);
        if (rc == -1) {
            fprintf(stderr, "gds_exec: failed to run %s: %s\n", compiler_path,
                    strerror(errno));
            unlink(temp_template);
            return false;
        }

        if (rc != EXIT_SUCCESS) {
            fprintf(stderr, "gds_exec: failed to compile source script: %s\n",
                    source_path);
            unlink(temp_template);
            return false;
        }
    }
#else
    child = fork();
    if (child == -1) {
        fprintf(stderr, "gds_exec: fork failed: %s\n", strerror(errno));
        unlink(temp_template);
        return false;
    }

    if (child == 0) {
        execl(compiler_path, "gds_compile", source_path, temp_template,
              (char *)NULL);
        fprintf(stderr, "gds_exec: failed to run %s: %s\n", compiler_path,
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
#endif

    return true;
}

int main(int argc, char **argv) {
    const char *input_path = NULL;
    const char *script_path = NULL;
    const uint8_t *payload = NULL;
    size_t payload_size = 0U;
    game_state_t state;
    mode_impl_t impl;
    char compiler_path[GDS_EXEC_PATH_MAX];
    char temp_script_path[GDS_EXEC_PATH_MAX];
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
        memset(compiler_path, 0, sizeof(compiler_path));
        if (!resolve_tool_path(argv[0], "gds_compile", compiler_path,
                               sizeof(compiler_path))) {
            fprintf(stderr, "gds_exec: could not find gds_compile; expected it "
                            "next to the build output or in ./build\n");
            return EXIT_FAILURE;
        }

        memset(temp_script_path, 0, sizeof(temp_script_path));
        if (!compile_gsc_to_temp(input_path, compiler_path, temp_script_path,
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
