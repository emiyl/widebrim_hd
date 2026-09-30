#ifndef GDS_OPCODE_H
#define GDS_OPCODE_H

#include <stdbool.h>
#include <string.h>

#define GDS_OPCODE_LIST(X)                                                     \
    X(SCRIPT_CMD_Invalid, "Invalid")                                           \
    X(SCRIPT_CMD_FadeIn, "FadeIn")                                             \
    X(SCRIPT_CMD_FadeInOnly, "FadeInOnly")                                     \
    X(SCRIPT_CMD_FadeInOnlySub, "FadeInOnlySub")                               \
    X(SCRIPT_CMD_FadeInOnlyMain, "FadeInOnlyMain")                             \
    X(SCRIPT_CMD_FadeOut, "FadeOut")                                           \
    X(SCRIPT_CMD_FadeOutOnly, "FadeOutOnly")                                   \
    X(SCRIPT_CMD_FadeOutOnlySub, "FadeOutOnlySub")                             \
    X(SCRIPT_CMD_FadeOutOnlyMain, "FadeOutOnlyMain")                           \
    X(SCRIPT_CMD_WaitPenTouch, "WaitPenTouch")                                 \
    X(SCRIPT_CMD_WaitInput, "WaitInput")                                       \
    X(SCRIPT_CMD_LoadBG, "LoadBG")                                             \
    X(SCRIPT_CMD_LoadSubBG, "LoadSubBG")                                       \
    X(SCRIPT_CMD_WaitVSyncOrPenTouch, "WaitVSyncOrPenTouch")                   \
    X(SCRIPT_CMD_UnloadAllGfx, "UnloadAllGfx")                                 \
    X(SCRIPT_CMD_VSyncProcess, "VSyncProcess")                                 \
    X(SCRIPT_CMD_PlaySound, "PlaySound")                                       \
    X(SCRIPT_CMD_PlayBGM, "PlayBGM")                                           \
    X(SCRIPT_CMD_IF, "IF")                                                     \
    X(SCRIPT_CMD_CheckRoomNumber, "CheckRoomNumber")                           \
    X(SCRIPT_CMD_Loop, "Loop")                                                 \
    X(SCRIPT_CMD_WHILE, "WHILE")                                               \
    X(SCRIPT_CMD_ELSEIF, "ELSEIF")                                             \
    X(SCRIPT_CMD_ELSE, "ELSE")                                                 \
    X(SCRIPT_CMD_PressingStart, "PressingStart")                               \
    X(SCRIPT_CMD_TRUE, "TRUE")                                                 \
    X(SCRIPT_CMD_FALSE, "FALSE")                                               \
    X(SCRIPT_CMD_CreateQuestion, "CreateQuestion")                             \
    X(SCRIPT_CMD_AddHints, "AddHints")                                         \
    X(SCRIPT_CMD_AddButtons, "AddButtons")                                     \
    X(SCRIPT_CMD_SetCorrect, "SetCorrect")                                     \
    X(SCRIPT_CMD_SetQuestionEndBG, "SetQuestionEndBG")                         \
    X(SCRIPT_CMD_AddHint, "AddHint")                                           \
    X(SCRIPT_CMD_AddQuestionButton, "AddQuestionButton")                       \
    X(SCRIPT_CMD_AddExit, "AddExit")                                           \
    X(SCRIPT_CMD_AddChr, "AddChr")                                             \
    X(SCRIPT_CMD_SetNumberAnswer, "SetNumberAnswer")                           \
    X(SCRIPT_CMD_AddCoin, "AddCoin")                                           \
    X(SCRIPT_CMD_AddCoinSolution, "AddCoinSolution")                           \
    X(SCRIPT_CMD_SetNumTouch, "SetNumTouch")                                   \
    X(SCRIPT_CMD_GridAddBlock, "GridAddBlock")                                 \
    X(SCRIPT_CMD_GridAddLetter, "GridAddLetter")                               \
    X(SCRIPT_CMD_AddMatch, "AddMatch")                                         \
    X(SCRIPT_CMD_AddMatchSolution, "AddMatchSolution")                         \
    X(SCRIPT_CMD_SetQuestionEvent, "SetQuestionEvent")                         \
    X(SCRIPT_CMD_AddWeights, "AddWeights")                                     \
    X(SCRIPT_CMD_RandomLightWeight, "RandomLightWeight")                       \
    X(SCRIPT_CMD_RandomHeavyWeight, "RandomHeavyWeight")                       \
    X(SCRIPT_CMD_RandomLightOrHeavyWeight, "RandomLightOrHeavyWeight")         \
    X(SCRIPT_CMD_AddChicken, "AddChicken")                                     \
    X(SCRIPT_CMD_AddWolf, "AddWolf")                                           \
    X(SCRIPT_CMD_NewShape, "NewShape")                                         \
    X(SCRIPT_CMD_AddVertex, "AddVertex")                                       \
    X(SCRIPT_CMD_AddTriangle, "AddTriangle")                                   \
    X(SCRIPT_CMD_SetShapePosition, "SetShapePosition")                         \
    X(SCRIPT_CMD_SetShapeRotation, "SetShapeRotation")                         \
    X(SCRIPT_CMD_SetShapeSolutionPosition, "SetShapeSolutionPosition")         \
    X(SCRIPT_CMD_SetShapeSolutionRotation, "SetShapeSolutionRotation")         \
    X(SCRIPT_CMD_AddCup, "AddCup")                                             \
    X(SCRIPT_CMD_SetBoard, "SetBoard")                                         \
    X(SCRIPT_CMD_AddQueens, "AddQueens")                                       \
    X(SCRIPT_CMD_AddGoldQueen, "AddGoldQueen")                                 \
    X(SCRIPT_CMD_SetQueenCheckMode, "SetQueenCheckMode")                       \
    X(SCRIPT_CMD_SetFillPos, "SetFillPos")                                     \
    X(SCRIPT_CMD_AddInPoint, "AddInPoint")                                     \
    X(SCRIPT_CMD_AddOutPoint, "AddOutPoint")                                   \
    X(SCRIPT_CMD_SetFontUserColor, "SetFontUserColor")                         \
    X(SCRIPT_CMD_AddTextObj, "AddTextObj")                                     \
    X(SCRIPT_CMD_TextWindow, "TextWindow")                                     \
    X(SCRIPT_CMD_SetTextWindowLeft, "SetTextWindowLeft")                       \
    X(SCRIPT_CMD_SetTextWindowRight, "SetTextWindowRight")                     \
    X(SCRIPT_CMD_SetWinNum, "SetWinNum")                                       \
    X(SCRIPT_CMD_SetCurrentQuestion, "SetCurrentQuestion")                     \
    X(SCRIPT_CMD_FoundQuestion, "FoundQuestion")                               \
    X(SCRIPT_CMD_EventModeStart, "EventModeStart")                             \
    X(SCRIPT_CMD_EventModeFinish, "EventModeFinish")                           \
    X(SCRIPT_CMD_FailQuestion, "FailQuestion")                                 \
    X(SCRIPT_CMD_CorrectQuestion, "CorrectQuestion")                           \
    X(SCRIPT_CMD_SolvedQuestion, "SolvedQuestion")                             \
    X(SCRIPT_CMD_ExitScript, "ExitScript")                                     \
    X(SCRIPT_CMD_AddEvent, "AddEvent")                                         \
    X(SCRIPT_CMD_SetGameMode, "SetGameMode")                                   \
    X(SCRIPT_CMD_SetQuestionEndGameMode, "SetQuestionEndGameMode")             \
    X(SCRIPT_CMD_SetCurrentRoom, "SetCurrentRoom")                             \
    X(SCRIPT_CMD_CorrectQuestionN, "CorrectQuestionN")                         \
    X(SCRIPT_CMD_SetEventFinished, "SetEventFinished")                         \
    X(SCRIPT_CMD_DoPrizeScreen, "DoPrizeScreen")                               \
    X(SCRIPT_CMD_DoStockScreen, "DoStockScreen")                               \
    X(SCRIPT_CMD_ViewedEvent, "ViewedEvent")                                   \
    X(SCRIPT_CMD_PlayBridgeSound, "PlayBridgeSound")                           \
    X(SCRIPT_CMD_SetMap, "SetMap")                                             \
    X(SCRIPT_CMD_SetExitSound, "SetExitSound")                                 \
    X(SCRIPT_CMD_AddBGObject, "AddBGObject")                                   \
    X(SCRIPT_CMD_AddOnOffButton, "AddOnOffButton")                             \
    X(SCRIPT_CMD_SetTarget, "SetTarget")                                       \
    X(SCRIPT_CMD_UnloadMainGfx, "UnloadMainGfx")                               \
    X(SCRIPT_CMD_SetCurrentEvent, "SetCurrentEvent")                           \
    X(SCRIPT_CMD_DoSaveScreen, "DoSaveScreen")                                 \
    X(SCRIPT_CMD_SetStoryFlag, "SetStoryFlag")                                 \
    X(SCRIPT_CMD_StoryFlag, "StoryFlag")                                       \
    X(SCRIPT_CMD_ForceTutorial, "ForceTutorial")                               \
    X(SCRIPT_CMD_SetTextWindowCenter, "SetTextWindowCenter")                   \
    X(SCRIPT_CMD_PuzzleSolverLayton, "PuzzleSolverLayton")                     \
    X(SCRIPT_CMD_PuzzleSolverLuke, "PuzzleSolverLuke")                         \
    X(SCRIPT_CMD_AddHintCoin, "AddHintCoin")                                   \
    X(SCRIPT_CMD_FadeOutBGM, "FadeOutBGM")                                     \
    X(SCRIPT_CMD_FadeInBGM, "FadeInBGM")                                       \
    X(SCRIPT_CMD_WaitFrame, "WaitFrame")                                       \
    X(SCRIPT_CMD_AddSprite, "AddSprite")                                       \
    X(SCRIPT_CMD_AddSpriteChild, "AddSpriteChild")                             \
    X(SCRIPT_CMD_SetSpriteAnimation, "SetSpriteAnimation")                     \
    X(SCRIPT_CMD_SetSpriteAnimationChild, "SetSpriteAnimationChild")           \
    X(SCRIPT_CMD_SetSpritePosition, "SetSpritePosition")                       \
    X(SCRIPT_CMD_SpriteOn, "SpriteOn")                                         \
    X(SCRIPT_CMD_SpriteOff, "SpriteOff")                                       \
    X(SCRIPT_CMD_AddTile, "AddTile")                                           \
    X(SCRIPT_CMD_AddPoint, "AddPoint")                                         \
    X(SCRIPT_CMD_AddTileSolution, "AddTileSolution")                           \
    X(SCRIPT_CMD_SetNumSolution, "SetNumSolution")                             \
    X(SCRIPT_CMD_NumQuestionsSolved, "NumQuestionsSolved")                     \
    X(SCRIPT_CMD_SetSpriteFade, "SetSpriteFade")                               \
    X(SCRIPT_CMD_SetSpriteAlpha, "SetSpriteAlpha")                             \
    X(SCRIPT_CMD_DrawFrames, "DrawFrames")                                     \
    X(SCRIPT_CMD_ModifyBGPal, "ModifyBGPal")                                   \
    X(SCRIPT_CMD_ModifySubBGPal, "ModifySubBGPal")                             \
    X(SCRIPT_CMD_FreeEventAniMemory, "FreeEventAniMemory")                     \
    X(SCRIPT_CMD_AddSubSprite, "AddSubSprite")                                 \
    X(SCRIPT_CMD_SetSubSpriteAnimation, "SetSubSpriteAnimation")               \
    X(SCRIPT_CMD_SetSubSpritePosition, "SetSubSpritePosition")                 \
    X(SCRIPT_CMD_SubSpriteOn, "SubSpriteOn")                                   \
    X(SCRIPT_CMD_SubSpriteOff, "SubSpriteOff")                                 \
    X(SCRIPT_CMD_AddCoinType, "AddCoinType")                                   \
    X(SCRIPT_CMD_AddCoinSolutionType, "AddCoinSolutionType")                   \
    X(SCRIPT_CMD_AddItem, "AddItem")                                           \
    X(SCRIPT_CMD_CheckItem, "CheckItem")                                       \
    X(SCRIPT_CMD_ShakeBG, "ShakeBG")                                           \
    X(SCRIPT_CMD_ShakeSubBG, "ShakeSubBG")                                     \
    X(SCRIPT_CMD_AddMan, "AddMan")                                             \
    X(SCRIPT_CMD_AddCabbage, "AddCabbage")                                     \
    X(SCRIPT_CMD_AddSheep, "AddSheep")                                         \
    X(SCRIPT_CMD_SetRiverCrossMode, "SetRiverCrossMode")                       \
    X(SCRIPT_CMD_BitFlag, "BitFlag")                                           \
    X(SCRIPT_CMD_SetBitFlag, "SetBitFlag")                                     \
    X(SCRIPT_CMD_SetSpriteShake, "SetSpriteShake")                             \
    X(SCRIPT_CMD_SetSpriteState, "SetSpriteState")                             \
    X(SCRIPT_CMD_SetSpriteTargetPosition, "SetSpriteTargetPosition")           \
    X(SCRIPT_CMD_SetSpriteSpeed, "SetSpriteSpeed")                             \
    X(SCRIPT_CMD_SetSubSpriteShake, "SetSubSpriteShake")                       \
    X(SCRIPT_CMD_SetSubSpriteState, "SetSubSpriteState")                       \
    X(SCRIPT_CMD_SetSubSpriteTargetPosition, "SetSubSpriteTargetPosition")     \
    X(SCRIPT_CMD_SetSubSpriteSpeed, "SetSubSpriteSpeed")                       \
    X(SCRIPT_CMD_SetSpriteType, "SetSpriteType")                               \
    X(SCRIPT_CMD_TextWindowK, "TextWindowK")                                   \
    X(SCRIPT_CMD_TextWindowR, "TextWindowR")                                   \
    X(SCRIPT_CMD_TextWindowL, "TextWindowL")                                   \
    X(SCRIPT_CMD_TextWindowM, "TextWindowM")                                   \
    X(SCRIPT_CMD_TextWindowKR, "TextWindowKR")                                 \
    X(SCRIPT_CMD_TextWindowKL, "TextWindowKL")                                 \
    X(SCRIPT_CMD_SetMemoFlag, "SetMemoFlag")                                   \
    X(SCRIPT_CMD_SetGridTypeRange, "SetGridTypeRange")                         \
    X(SCRIPT_CMD_AddTouchPoint, "AddTouchPoint")                               \
    X(SCRIPT_CMD_AddCheckLine, "AddCheckLine")                                 \
    X(SCRIPT_CMD_EnableNaname, "EnableNaname")                                 \
    X(SCRIPT_CMD_SetGridPosition, "SetGridPosition")                           \
    X(SCRIPT_CMD_SetGridSize, "SetGridSize")                                   \
    X(SCRIPT_CMD_SetBlockSize, "SetBlockSize")                                 \
    X(SCRIPT_CMD_AddBlock, "AddBlock")                                         \
    X(SCRIPT_CMD_SetKatakanaAnswer, "SetKatakanaAnswer")                       \
    X(SCRIPT_CMD_SetInputType, "SetInputType")                                 \
    X(SCRIPT_CMD_SetAlphabetAnswer, "SetAlphabetAnswer")                       \
    X(SCRIPT_CMD_SetType, "SetType")                                           \
    X(SCRIPT_CMD_OnHintMedal, "OnHintMedal")                                   \
    X(SCRIPT_CMD_SetLineColor, "SetLineColor")                                 \
    X(SCRIPT_CMD_SetPenColor, "SetPenColor")                                   \
    X(SCRIPT_CMD_LoadBGSetFadeIn, "LoadBGSetFadeIn")                           \
    X(SCRIPT_CMD_DoSpriteFadeIn, "DoSpriteFadeIn")                             \
    X(SCRIPT_CMD_DoSpriteFadeInFast, "DoSpriteFadeInFast")                     \
    X(SCRIPT_CMD_AddShapeSolutionType, "AddShapeSolutionType")                 \
    X(SCRIPT_CMD_SetShapeType, "SetShapeType")                                 \
    X(SCRIPT_CMD_DoSpriteFadeOut, "DoSpriteFadeOut")                           \
    X(SCRIPT_CMD_SetSpriteFlip, "SetSpriteFlip")                               \
    X(SCRIPT_CMD_AddRotateBox, "AddRotateBox")                                 \
    X(SCRIPT_CMD_AddStoryScript, "AddStoryScript")                             \
    X(SCRIPT_CMD_SetQuestionSolved, "SetQuestionSolved")                       \
    X(SCRIPT_CMD_SetEventViewed, "SetEventViewed")                             \
    X(SCRIPT_CMD_SetSingleNumberAnswer, "SetSingleNumberAnswer")               \
    X(SCRIPT_CMD_SetPuzzleTitle, "SetPuzzleTitle")                             \
    X(SCRIPT_CMD_SetShapeSolutionMirror, "SetShapeSolutionMirror")             \
    X(SCRIPT_CMD_AddLaytonFurniture, "AddLaytonFurniture")                     \
    X(SCRIPT_CMD_AddLukeFurniture, "AddLukeFurniture")                         \
    X(SCRIPT_CMD_DisableResetButton, "DisableResetButton")                     \
    X(SCRIPT_CMD_SetLiquidColor, "SetLiquidColor")                             \
    X(SCRIPT_CMD_SetQuestionFailBG, "SetQuestionFailBG")                       \
    X(SCRIPT_CMD_PenTouched, "PenTouched")                                     \
    X(SCRIPT_CMD_AddDogPart, "AddDogPart")                                     \
    X(SCRIPT_CMD_SetQuestionCarot, "SetQuestionCarot")                         \
    X(SCRIPT_CMD_DoDogItemScreen, "DoDogItemScreen")                           \
    X(SCRIPT_CMD_DoJigsawScreen, "DoJigsawScreen")                             \
    X(SCRIPT_CMD_AddLaytonItemText, "AddLaytonItemText")                       \
    X(SCRIPT_CMD_AddLaytonItemTextParent, "AddLaytonItemTextParent")           \
    X(SCRIPT_CMD_AddLukeItemText, "AddLukeItemText")                           \
    X(SCRIPT_CMD_AddLukeItemTextParent, "AddLukeItemTextParent")               \
    X(SCRIPT_CMD_AddLaytonHint, "AddLaytonHint")                               \
    X(SCRIPT_CMD_AddLukeHint, "AddLukeHint")                                   \
    X(SCRIPT_CMD_DoFurnitureScreen, "DoFurnitureScreen")                       \
    X(SCRIPT_CMD_TextWindowKM, "TextWindowKM")                                 \
    X(SCRIPT_CMD_RemoveItem, "RemoveItem")                                     \
    X(SCRIPT_CMD_SetItemName, "SetItemName")                                   \
    X(SCRIPT_CMD_SetTraceCorrectZone, "SetTraceCorrectZone")                   \
    X(SCRIPT_CMD_AddDogEvent, "AddDogEvent")                                   \
    X(SCRIPT_CMD_AddDogCoin, "AddDogCoin")                                     \
    X(SCRIPT_CMD_SetMovieNum, "SetMovieNum")                                   \
    X(SCRIPT_CMD_AddTracePoint, "AddTracePoint")                               \
    X(SCRIPT_CMD_ChoiceWindow3, "ChoiceWindow3")                               \
    X(SCRIPT_CMD_SetChoiceText1, "SetChoiceText1")                             \
    X(SCRIPT_CMD_SetChoiceText2, "SetChoiceText2")                             \
    X(SCRIPT_CMD_SetChoiceText3, "SetChoiceText3")                             \
    X(SCRIPT_CMD_SetChoiceQuestion, "SetChoiceQuestion")                       \
    X(SCRIPT_CMD_OnChoice, "OnChoice")                                         \
    X(SCRIPT_CMD_ChoiceWindow2, "ChoiceWindow2")                               \
    X(SCRIPT_CMD_SetQuestionInfo, "SetQuestionInfo")                           \
    X(SCRIPT_CMD_PlayMovieDual, "PlayMovieDual")                               \
    X(SCRIPT_CMD_SaveTextureMemoryState, "SaveTextureMemoryState")             \
    X(SCRIPT_CMD_SetButtonAnswerWifi, "SetButtonAnswerWifi")                   \
    X(SCRIPT_CMD_SetNumberAnswerWifi, "SetNumberAnswerWifi")                   \
    X(SCRIPT_CMD_SetLetterAnswerWifi, "SetLetterAnswerWifi")                   \
    X(SCRIPT_CMD_SetHiraganaAnswerWifi, "SetHiraganaAnswerWifi")               \
    X(SCRIPT_CMD_SetKatakanaAnswerWifi, "SetKatakanaAnswerWifi")               \
    X(SCRIPT_CMD_AddBabaQuestion, "AddBabaQuestion")                           \
    X(SCRIPT_CMD_SetBabaParam, "SetBabaParam")                                 \
    X(SCRIPT_CMD_LoadBabaData, "LoadBabaData")                                 \
    X(SCRIPT_CMD_DoBabaAddScreen, "DoBabaAddScreen")                           \
    X(SCRIPT_CMD_DoHukamaruAddScreen, "DoHukamaruAddScreen")                   \
    X(SCRIPT_CMD_SetAraSujiEventNumber, "SetAraSujiEventNumber")               \
    X(SCRIPT_CMD_DoAraSujiEvent, "DoAraSujiEvent")                             \
    X(SCRIPT_CMD_PressingX, "PressingX")                                       \
    X(SCRIPT_CMD_FadeToVolumeBGM, "FadeToVolumeBGM")                           \
    X(SCRIPT_CMD_LoadPlayBGM, "LoadPlayBGM")                                   \
    X(SCRIPT_CMD_LoadOtherSoundGroup, "LoadOtherSoundGroup")                   \
    X(SCRIPT_CMD_PlayBGMWait, "PlayBGMWait")                                   \
    X(SCRIPT_CMD_FadeBGMWait, "FadeBGMWait")                                   \
    X(SCRIPT_CMD_PlayMovieSound, "PlayMovieSound")                             \
    X(SCRIPT_CMD_SetMaxDist, "SetMaxDist")                                     \
    X(SCRIPT_CMD_PlaySoundDirect, "PlaySoundDirect")                           \
    X(SCRIPT_CMD_LoadEventSoundGroup, "LoadEventSoundGroup")                   \
    X(SCRIPT_CMD_SetChangeAnswerKomoji, "SetChangeAnswerKomoji")               \
    X(SCRIPT_CMD_NoTutorial, "NoTutorial")                                     \
    X(SCRIPT_CMD_FadeOutBGMScript, "FadeOutBGMScript")                         \
    X(SCRIPT_CMD_FadeInBGMScript, "FadeInBGMScript")                           \
    X(SCRIPT_CMD_FadeOutBGMQuick, "FadeOutBGMQuick")                           \
    X(SCRIPT_CMD_SetAnswerBox, "SetAnswerBox")                                 \
    X(SCRIPT_CMD_SetAnswer, "SetAnswer")                                       \
    X(SCRIPT_CMD_SetDrawInputBG, "SetDrawInputBG")                             \
    X(SCRIPT_CMD_SetSubTitle, "SetSubTitle")                                   \
    X(SCRIPT_CMD_SetMovie, "SetMovie")                                         \
    X(SCRIPT_CMD_SetFullScreen, "SetFullScreen")                               \
    X(SCRIPT_CMD_DoNoChargedScreen, "DoNoChargedScreen")                       \
    X(SCRIPT_CMD_SetLaytonChallenge, "SetLaytonChallenge")                     \
    X(SCRIPT_CMD_DoDownload, "DoDownload")                                     \
    X(SCRIPT_CMD_SetBandType, "SetBandType")                                   \
    X(SCRIPT_CMD_AddSecretCoin, "AddSecretCoin")                               \
    X(SCRIPT_CMD_OnSecretMedal, "OnSecretMedal")                               \
    X(SCRIPT_CMD_SetCharmPoint, "SetCharmPoint")                               \
    X(SCRIPT_CMD_SetL5iDPoint, "SetL5iDPoint")                                 \
    X(SCRIPT_CMD_SetTopSecretPoint, "SetTopSecretPoint")                       \
    X(SCRIPT_CMDX_Print, "Print")

#define GDS_OPCODE_ENUM(name, string) name,

typedef enum { GDS_OPCODE_LIST(GDS_OPCODE_ENUM) SCRIPT_CMD_MAX } gds_opcode_t;

#undef GDS_OPCODE_ENUM

#define GDS_OPCODE_STRING(name, string) [name] = string,

static const char *const gds_opcode_names[SCRIPT_CMD_MAX] = {
    GDS_OPCODE_LIST(GDS_OPCODE_STRING)};

#undef GDS_OPCODE_STRING

static inline const char *gds_opcode_to_string(gds_opcode_t opcode) {
    if ((unsigned)opcode >= SCRIPT_CMD_MAX)
        return "UNKNOWN_OPCODE";

    return gds_opcode_names[opcode];
}

static inline gds_opcode_t gds_opcode_from_string(const char *string) {
#define GDS_OPCODE_MATCH(name, value)                                          \
    if (strcmp(string, value) == 0)                                            \
        return name;

    GDS_OPCODE_LIST(GDS_OPCODE_MATCH)

#undef GDS_OPCODE_MATCH

    return SCRIPT_CMD_Invalid;
}

static inline bool gds_is_valid_opcode(gds_opcode_t opcode) {
    if (opcode > SCRIPT_CMD_Invalid && opcode < SCRIPT_CMD_MAX) {
        return true;
    }
    return false;
}

#endif // GDS_OPCODE_H
