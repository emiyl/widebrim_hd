#include "gds/gds_func.h"

#include <stdio.h>

static bool gds_func_set_map(gds_reader_t *reader, const gds_record_t *command,
                             void *user_data) {
    (void)command;
    (void)user_data;

    gds_record_t value;
    for (int i = 0; i < 5; ++i) {
        if (!gds_read_record(reader, &value)) {
            fprintf(stderr, "gds: SetMap expected %d more arguments\n", 5 - i);
            return false;
        }
    }

    return true;
}

static bool gds_func_add_text_obj(gds_reader_t *reader,
                                  const gds_record_t *command,
                                  void *user_data) {
    (void)command;
    (void)user_data;

    gds_record_t value;
    for (int i = 0; i < 7; ++i) {
        if (!gds_read_record(reader, &value)) {
            fprintf(stderr, "gds: AddTextObj expected %d more arguments\n",
                    7 - i);
            return false;
        }
    }

    return true;
}

static bool gds_func_add_bg_object(gds_reader_t *reader,
                                   const gds_record_t *command,
                                   void *user_data) {
    (void)command;
    (void)user_data;

    gds_record_t value;
    for (int i = 0; i < 3; ++i) {
        if (!gds_read_record(reader, &value)) {
            fprintf(stderr, "gds: AddBGObject expected %d more arguments\n",
                    3 - i);
            return false;
        }
    }

    return true;
}

bool gds_func_lookup(gds_opcode_t opcode, gds_command_handler_fn *handler) {
    if (handler == NULL) {
        return false;
    }

    switch (opcode) {
    case SCRIPT_CMD_SetMap:
        *handler = gds_func_set_map;
        return true;
    case SCRIPT_CMD_AddTextObj:
        *handler = gds_func_add_text_obj;
        return true;
    case SCRIPT_CMD_AddBGObject:
        *handler = gds_func_add_bg_object;
        return true;
    default:
        *handler = NULL;
        return false;
    }
}
