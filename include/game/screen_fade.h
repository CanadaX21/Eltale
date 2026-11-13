#ifndef GAME_SCREEN_FADE_H
#define GAME_SCREEN_FADE_H

/* The functions of src/game/screen_fade.c, declared once for every file that calls them. */

#include "common.h"

void BuildColourRamp(void);   /* build the RGBA ramp (screen_fade.c) */
void DrawScreenFade(mkst* arg0);
void StartScreenFade(s32 frames, u16 flags, void* ramp);

#endif
