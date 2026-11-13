#ifndef GAME_LIGHTING_H
#define GAME_LIGHTING_H

/* The functions of src/game/lighting.c, declared once for every file that calls them. */

#include "common.h"

void ResetTimeOfDay(void);
void InitDayPalette(void);
void UpdateTimeOfDay(void);
void SetTimeOfDay(s32 time);
void UpdateMapLights(s32 time, MapLight* lights);
void UpdateFrameLights(void);
void StartPaletteFade(s32 steps, u16* target, u16* palette);
void StepPaletteFade(s32 left);
void ResetLightAnims(void);
s32 AllocLightAnim(void);
void FreeLightAnim(s32 n);
void SetLightAnimColours(s32 n, s32 mode, s32 count, s32* keys);
void SetLightAnimLights(s32 n, s32 mode, s32 count, s32* keys);
void SetLightAnimDirections(s32 n, s32* dirs);
void StepLightAnim(s32 index);
void SetupLights(Lights3* lights, s32* colors, s32* dirs);

#endif
