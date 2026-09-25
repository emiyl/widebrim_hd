#include "gds_func.h"

#include <stdio.h>

#include "mode_room.h"

static bool gds_func_setmap(gds_reader_t *reader, const gds_record_t *command,
                            void *user_data) {
    (void)command;
    int32_t argv[5];

    if (!gds_read_s32_args(reader, argv, 5, "SetMap")) {
        return false;
    }

    int32_t map_text_id = argv[0];
    int32_t map_background_id = argv[1];
    int32_t param3 = argv[2];
    int32_t param4 = argv[3];
    int32_t param5 = argv[4];

    mode_room_impl_t *impl = (mode_room_impl_t *)user_data;
    if (impl->setmap)
        impl->setmap(impl, map_text_id, map_background_id, param3, param4,
                     param5);

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
