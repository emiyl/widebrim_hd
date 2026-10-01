#include "script.h"

#include <stdio.h>

#include "gds/gds_exec.h"

bool script_load_and_execute(game_state_t *state, const char *relative_path,
                             void *user_data, gds_state_t *gds_state) {
    char resolved_path[1024];
    const uint8_t *script_payload = NULL;
    size_t script_payload_size = 0U;

    if (!state || !relative_path || !gds_state) {
        fprintf(
            stderr,
            "widebrim: script_load_and_execute called with invalid state or "
            "path\n");
        return false;
    }

    if (!asset_path_resolve(state, relative_path, resolved_path,
                            sizeof(resolved_path))) {
        fprintf(stderr, "widebrim: Failed to resolve asset path for %s\n",
                relative_path);
        return false;
    }

    fprintf(stderr, "widebrim: Loading script: %s\n", relative_path);
    if (!gds_load_from_file_path(resolved_path, &script_payload,
                                 &script_payload_size)) {
        fprintf(stderr, "widebrim: Failed to load script: %s\n", relative_path);
        return false;
    }

    if (!gds_execute_script(script_payload, script_payload_size, user_data,
                            gds_state)) {
        fprintf(stderr, "widebrim: Failed to execute script: %s\n",
                relative_path);
        gds_free_payload(script_payload);
        return false;
    }

    gds_free_payload(script_payload);
    return true;
}
