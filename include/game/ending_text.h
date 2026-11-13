#ifndef GAME_ENDING_TEXT_H
#define GAME_ENDING_TEXT_H

/* The functions of src/game/ending_text.c, declared once for every file that calls them. */

#include "common.h"

void LoadEndingScene(s32 index);
s32 StepEndingText(void);
void FadeSceneMusic(u32 index);
void CopyEndingSpeakerName(u8* text, u8* name);

#endif
