#ifndef GDS_FUNC_H
#define GDS_FUNC_H

#include <stdbool.h>

#include "gds.h"
#include "gds_opcode.h"

#define GDS_ARGS                                                               \
    gds_reader_t *reader, const gds_record_t *command, void *user_data
#define GDS_DEF(name) static bool gds_func_##name(GDS_ARGS)

#define GDS_OPCODE_HANDLER_LIST(X)                                             \
    X(SetMap)                                                                  \
    X(AddTextObj)                                                              \
    X(AddBGObject)                                                             \
    X(StoryFlag)                                                               \
    X(SetStoryFlag)                                                            \
    X(ViewedEvent)                                                             \
    X(SetEventViewed)                                                          \
    X(AddExit)                                                                 \
    X(BitFlag)                                                                 \
    X(SetBitFlag)                                                              \
    X(AddEvent)                                                                \
    X(SetCurrentQuestion)                                                      \
    X(SolvedQuestion)                                                          \
    X(AddSubSprite)                                                            \
    X(ExitScript)                                                              \
    X(SetGameMode)                                                             \
    X(SetCurrentEvent)                                                         \
    X(AddHintCoin)                                                             \
    X(AddDogCoin)                                                              \
    X(AddSecretCoin)                                                           \
    X(SetExitSound)                                                            \
    X(LoadBG)                                                                  \
    X(LoadSubBG)                                                               \
    X(ModifyBGPal)                                                             \
    X(Print)

#define GDS_FUNC_LOOKUP_TABLE(X) GDS_OPCODE_HANDLER_LIST(X)

#define GDS_FUNC_LOOKUP_CASE(name)                                             \
    case SCRIPT_CMD_##name:                                                    \
        *handler = gds_func_##name;                                            \
        return true;

typedef bool (*gds_command_handler_fn)(gds_reader_t *reader,
                                       const gds_record_t *command,
                                       void *user_data);

bool gds_func_lookup(gds_opcode_t opcode, gds_command_handler_fn *handler);

#endif // GDS_FUNC_H
