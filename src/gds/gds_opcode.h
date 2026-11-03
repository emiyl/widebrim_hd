#ifndef GDS_OPCODE_H
#define GDS_OPCODE_H

#include <stdbool.h>
#include <string.h>

#define GDS_OPCODE_LIST(X)                                                     \
    X(Invalid)                                                                 \
    X(FadeIn)                                                                  \
    X(FadeInOnly)                                                              \
    X(FadeInOnlySub)                                                           \
    X(FadeInOnlyMain)                                                          \
    X(FadeOut)                                                                 \
    X(FadeOutOnly)                                                             \
    X(FadeOutOnlySub)                                                          \
    X(FadeOutOnlyMain)                                                         \
    X(WaitPenTouch)                                                            \
    X(WaitInput)                                                               \
    X(LoadBG)                                                                  \
    X(LoadSubBG)                                                               \
    X(WaitVSyncOrPenTouch)                                                     \
    X(UnloadAllGfx)                                                            \
    X(VSyncProcess)                                                            \
    X(PlaySound)                                                               \
    X(PlayBGM)                                                                 \
    X(IF)                                                                      \
    X(CheckRoomNumber)                                                         \
    X(Loop)                                                                    \
    X(WHILE)                                                                   \
    X(ELSEIF)                                                                  \
    X(ELSE)                                                                    \
    X(PressingStart)                                                           \
    X(TRUE)                                                                    \
    X(FALSE)                                                                   \
    X(CreateQuestion)                                                          \
    X(AddHints)                                                                \
    X(AddButtons)                                                              \
    X(SetCorrect)                                                              \
    X(SetQuestionEndBG)                                                        \
    X(AddHint)                                                                 \
    X(AddQuestionButton)                                                       \
    X(AddExit)                                                                 \
    X(AddChr)                                                                  \
    X(SetNumberAnswer)                                                         \
    X(AddCoin)                                                                 \
    X(AddCoinSolution)                                                         \
    X(SetNumTouch)                                                             \
    X(GridAddBlock)                                                            \
    X(GridAddLetter)                                                           \
    X(AddMatch)                                                                \
    X(AddMatchSolution)                                                        \
    X(SetQuestionEvent)                                                        \
    X(AddWeights)                                                              \
    X(RandomLightWeight)                                                       \
    X(RandomHeavyWeight)                                                       \
    X(RandomLightOrHeavyWeight)                                                \
    X(AddChicken)                                                              \
    X(AddWolf)                                                                 \
    X(NewShape)                                                                \
    X(AddVertex)                                                               \
    X(AddTriangle)                                                             \
    X(SetShapePosition)                                                        \
    X(SetShapeRotation)                                                        \
    X(SetShapeSolutionPosition)                                                \
    X(SetShapeSolutionRotation)                                                \
    X(AddCup)                                                                  \
    X(SetBoard)                                                                \
    X(AddQueens)                                                               \
    X(AddGoldQueen)                                                            \
    X(SetQueenCheckMode)                                                       \
    X(SetFillPos)                                                              \
    X(AddInPoint)                                                              \
    X(AddOutPoint)                                                             \
    X(SetFontUserColor)                                                        \
    X(AddTextObj)                                                              \
    X(TextWindow)                                                              \
    X(SetTextWindowLeft)                                                       \
    X(SetTextWindowRight)                                                      \
    X(SetWinNum)                                                               \
    X(SetCurrentQuestion)                                                      \
    X(FoundQuestion)                                                           \
    X(EventModeStart)                                                          \
    X(EventModeFinish)                                                         \
    X(FailQuestion)                                                            \
    X(CorrectQuestion)                                                         \
    X(SolvedQuestion)                                                          \
    X(ExitScript)                                                              \
    X(AddEvent)                                                                \
    X(SetGameMode)                                                             \
    X(SetQuestionEndGameMode)                                                  \
    X(SetCurrentRoom)                                                          \
    X(CorrectQuestionN)                                                        \
    X(SetEventFinished)                                                        \
    X(DoPrizeScreen)                                                           \
    X(DoStockScreen)                                                           \
    X(ViewedEvent)                                                             \
    X(PlayBridgeSound)                                                         \
    X(SetMap)                                                                  \
    X(SetExitSound)                                                            \
    X(AddBGObject)                                                             \
    X(AddOnOffButton)                                                          \
    X(SetTarget)                                                               \
    X(UnloadMainGfx)                                                           \
    X(SetCurrentEvent)                                                         \
    X(DoSaveScreen)                                                            \
    X(SetStoryFlag)                                                            \
    X(StoryFlag)                                                               \
    X(ForceTutorial)                                                           \
    X(SetTextWindowCenter)                                                     \
    X(PuzzleSolverLayton)                                                      \
    X(PuzzleSolverLuke)                                                        \
    X(AddHintCoin)                                                             \
    X(FadeOutBGM)                                                              \
    X(FadeInBGM)                                                               \
    X(WaitFrame)                                                               \
    X(AddSprite)                                                               \
    X(AddSpriteChild)                                                          \
    X(SetSpriteAnimation)                                                      \
    X(SetSpriteAnimationChild)                                                 \
    X(SetSpritePosition)                                                       \
    X(SpriteOn)                                                                \
    X(SpriteOff)                                                               \
    X(AddTile)                                                                 \
    X(AddPoint)                                                                \
    X(AddTileSolution)                                                         \
    X(SetNumSolution)                                                          \
    X(NumQuestionsSolved)                                                      \
    X(SetSpriteFade)                                                           \
    X(SetSpriteAlpha)                                                          \
    X(DrawFrames)                                                              \
    X(ModifyBGPal)                                                             \
    X(ModifySubBGPal)                                                          \
    X(FreeEventAniMemory)                                                      \
    X(AddSubSprite)                                                            \
    X(SetSubSpriteAnimation)                                                   \
    X(SetSubSpritePosition)                                                    \
    X(SubSpriteOn)                                                             \
    X(SubSpriteOff)                                                            \
    X(AddCoinType)                                                             \
    X(AddCoinSolutionType)                                                     \
    X(AddItem)                                                                 \
    X(CheckItem)                                                               \
    X(ShakeBG)                                                                 \
    X(ShakeSubBG)                                                              \
    X(AddMan)                                                                  \
    X(AddCabbage)                                                              \
    X(AddSheep)                                                                \
    X(SetRiverCrossMode)                                                       \
    X(BitFlag)                                                                 \
    X(SetBitFlag)                                                              \
    X(SetSpriteShake)                                                          \
    X(SetSpriteState)                                                          \
    X(SetSpriteTargetPosition)                                                 \
    X(SetSpriteSpeed)                                                          \
    X(SetSubSpriteShake)                                                       \
    X(SetSubSpriteState)                                                       \
    X(SetSubSpriteTargetPosition)                                              \
    X(SetSubSpriteSpeed)                                                       \
    X(SetSpriteType)                                                           \
    X(TextWindowK)                                                             \
    X(TextWindowR)                                                             \
    X(TextWindowL)                                                             \
    X(TextWindowM)                                                             \
    X(TextWindowKR)                                                            \
    X(TextWindowKL)                                                            \
    X(SetMemoFlag)                                                             \
    X(SetGridTypeRange)                                                        \
    X(AddTouchPoint)                                                           \
    X(AddCheckLine)                                                            \
    X(EnableNaname)                                                            \
    X(SetGridPosition)                                                         \
    X(SetGridSize)                                                             \
    X(SetBlockSize)                                                            \
    X(AddBlock)                                                                \
    X(SetKatakanaAnswer)                                                       \
    X(SetInputType)                                                            \
    X(SetAlphabetAnswer)                                                       \
    X(SetType)                                                                 \
    X(OnHintMedal)                                                             \
    X(SetLineColor)                                                            \
    X(SetPenColor)                                                             \
    X(LoadBGSetFadeIn)                                                         \
    X(DoSpriteFadeIn)                                                          \
    X(DoSpriteFadeInFast)                                                      \
    X(AddShapeSolutionType)                                                    \
    X(SetShapeType)                                                            \
    X(DoSpriteFadeOut)                                                         \
    X(SetSpriteFlip)                                                           \
    X(AddRotateBox)                                                            \
    X(AddStoryScript)                                                          \
    X(SetQuestionSolved)                                                       \
    X(SetEventViewed)                                                          \
    X(SetSingleNumberAnswer)                                                   \
    X(SetPuzzleTitle)                                                          \
    X(SetShapeSolutionMirror)                                                  \
    X(AddLaytonFurniture)                                                      \
    X(AddLukeFurniture)                                                        \
    X(DisableResetButton)                                                      \
    X(SetLiquidColor)                                                          \
    X(SetQuestionFailBG)                                                       \
    X(PenTouched)                                                              \
    X(AddDogPart)                                                              \
    X(SetQuestionCarot)                                                        \
    X(DoDogItemScreen)                                                         \
    X(DoJigsawScreen)                                                          \
    X(AddLaytonItemText)                                                       \
    X(AddLaytonItemTextParent)                                                 \
    X(AddLukeItemText)                                                         \
    X(AddLukeItemTextParent)                                                   \
    X(AddLaytonHint)                                                           \
    X(AddLukeHint)                                                             \
    X(DoFurnitureScreen)                                                       \
    X(TextWindowKM)                                                            \
    X(RemoveItem)                                                              \
    X(SetItemName)                                                             \
    X(SetTraceCorrectZone)                                                     \
    X(AddDogEvent)                                                             \
    X(AddDogCoin)                                                              \
    X(SetMovieNum)                                                             \
    X(AddTracePoint)                                                           \
    X(ChoiceWindow3)                                                           \
    X(SetChoiceText1)                                                          \
    X(SetChoiceText2)                                                          \
    X(SetChoiceText3)                                                          \
    X(SetChoiceQuestion)                                                       \
    X(OnChoice)                                                                \
    X(ChoiceWindow2)                                                           \
    X(SetQuestionInfo)                                                         \
    X(PlayMovieDual)                                                           \
    X(SaveTextureMemoryState)                                                  \
    X(SetButtonAnswerWifi)                                                     \
    X(SetNumberAnswerWifi)                                                     \
    X(SetLetterAnswerWifi)                                                     \
    X(SetHiraganaAnswerWifi)                                                   \
    X(SetKatakanaAnswerWifi)                                                   \
    X(AddBabaQuestion)                                                         \
    X(SetBabaParam)                                                            \
    X(LoadBabaData)                                                            \
    X(DoBabaAddScreen)                                                         \
    X(DoHukamaruAddScreen)                                                     \
    X(SetAraSujiEventNumber)                                                   \
    X(DoAraSujiEvent)                                                          \
    X(PressingX)                                                               \
    X(FadeToVolumeBGM)                                                         \
    X(LoadPlayBGM)                                                             \
    X(LoadOtherSoundGroup)                                                     \
    X(PlayBGMWait)                                                             \
    X(FadeBGMWait)                                                             \
    X(PlayMovieSound)                                                          \
    X(SetMaxDist)                                                              \
    X(PlaySoundDirect)                                                         \
    X(LoadEventSoundGroup)                                                     \
    X(SetChangeAnswerKomoji)                                                   \
    X(NoTutorial)                                                              \
    X(FadeOutBGMScript)                                                        \
    X(FadeInBGMScript)                                                         \
    X(FadeOutBGMQuick)                                                         \
    X(SetAnswerBox)                                                            \
    X(SetAnswer)                                                               \
    X(SetDrawInputBG)                                                          \
    X(SetSubTitle)                                                             \
    X(SetMovie)                                                                \
    X(SetFullScreen)                                                           \
    X(DoNoChargedScreen)                                                       \
    X(SetLaytonChallenge)                                                      \
    X(DoDownload)                                                              \
    X(SetBandType)                                                             \
    X(AddSecretCoin)                                                           \
    X(OnSecretMedal)                                                           \
    X(SetCharmPoint)                                                           \
    X(SetL5iDPoint)                                                            \
    X(SetTopSecretPoint)                                                       \
    X(Print)

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
    X(Print)

#define GDS_OPCODE_ENUM(name) SCRIPT_CMD_##name,

typedef enum { GDS_OPCODE_LIST(GDS_OPCODE_ENUM) SCRIPT_CMD_MAX } gds_opcode_t;

#undef GDS_OPCODE_ENUM

#define GDS_OPCODE_STRING(name) [SCRIPT_CMD_##name] = #name,

static const char *const gds_opcode_names[SCRIPT_CMD_MAX] = {
    GDS_OPCODE_LIST(GDS_OPCODE_STRING)};

#undef GDS_OPCODE_STRING

static inline const char *gds_opcode_to_string(gds_opcode_t opcode) {
    if ((unsigned)opcode >= SCRIPT_CMD_MAX)
        return "UNKNOWN_OPCODE";

    return gds_opcode_names[opcode];
}

static inline gds_opcode_t gds_opcode_from_string(const char *string) {
#define GDS_OPCODE_MATCH(name)                                                 \
    if (strcmp(string, #name) == 0)                                            \
        return SCRIPT_CMD_##name;

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
