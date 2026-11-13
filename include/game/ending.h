#ifndef GAME_ENDING_H
#define GAME_ENDING_H

/* The functions of src/game/ending.c, declared once for every file that calls them. */

#include "common.h"

void ResetEndingScenes(void);   /* the scene player (ending.c) */
void StartScene(void);
void StepEndingScene(void);
void EndEndingScene(void);
void StepCameraPath(ModelPart* path, s32 frame);
void DrawCreditImages(void);

#endif
