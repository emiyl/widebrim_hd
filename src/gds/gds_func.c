#include "gds_func.h"

#include <stdio.h>

static bool gds_func_setmap(gds_reader_t *reader, const gds_record_t *command,
                            game_state_t *state) {
    (void)command;
    (void)state;

    gds_record_t record_argv[5];
    for (int i = 0; i < 5; i++) {
        if (!gds_read_record(reader, &record_argv[i])) {
            fprintf(stderr, "gds: SetMap expected 5 arguments\n");
            return false;
        }
    }

    int32_t argv[5];
    for (int i = 0; i < 5; i++) {
        if (record_argv[i].type != GDS_RECORD_VALUE_S32) {
            fprintf(stderr, "gds: SetMap argument %d is not an integer\n", i);
            return false;
        }
        argv[i] = record_argv[i].payload.value.s32;
    }

    printf("gds: SetMap arguments: %d, %d, %d, %d, %d\n", argv[0], argv[1],
           argv[2], argv[3], argv[4]);

    return true;
}
bool gds_func_lookup(gds_opcode_t opcode, gds_command_handler_fn *handler) {
    if (handler == NULL) {
        return false;
    }

    switch (opcode) {
    case SCRIPT_CMD_SetMap:
        *handler = gds_func_setmap;
        return true;
    default:
        *handler = NULL;
        return false;
    }
}
