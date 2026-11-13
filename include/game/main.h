#ifndef GAME_MAIN_H
#define GAME_MAIN_H

/* The functions of src/game/main.c, declared once for every file that calls them. */

#include "common.h"

void mainproc(s32 arg);
void InitNewGame(void);
void InitGraphics(void);
void InitFrameTask(GfxFrame* frame);
void BuildFrame(GfxFrame* frame);   /* build the frame's display list */
void FieldLoop(void);
void EndingLoop(void);
void AfterCreditsLoop(void);
void SaveModeLoop(void);
void TitleLoop(void);
void IntroLoop(void);
void CreditsLoop(void);
void WaitForScheduler(s16 type);   /* wait for the scheduler */
void FadeScreenIn(void);
void FadeScreenOut(void);

#endif
