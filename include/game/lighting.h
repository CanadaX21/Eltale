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

/* The state src/game/lighting.c defines (.bss), in its storage order: the order of these declarations is the
 * unit's .bss layout (include/bss_order.h). */
extern s32 gNextDayPeriod;   /* the next period */
extern s32 gFirstDayPeriodEnd;   /* the first period's end */
extern u16 gPlayTimeTicks;   /* ticks toward the next play-time minute (1501 each) */
extern s32 gPaletteFadeSteps;   /* palette fade steps left (counted down by the caller) */
extern s32 gPaletteFadeLength;   /* steps in the fade */
extern u16* gPaletteFadeTarget;   /* the colours it fades towards */
extern u16* gPaletteFadeWork;   /* the palette being worked on */
extern s32 D_8008434C;   /* unreferenced */
extern u16 gPaletteFadeSource[256];   /* its original colours */
extern MtxF gLightMtx;
extern Lights3 gFrameLights;   /* the two frames' lights */
extern Lights3 gFrameLights1;
extern Lights3 gLightAnimLights0[8];
extern Lights3 gLightAnimLights1[8];
extern s32 gTimeOfDay;   /* time of day */
extern s32 gDayLength;
extern s32 gLightTime;   /* time of day for the lights */
extern s32 gFramesPerDay;   /* frames per day */
extern u16 gDayCount;
extern u16 gPlayTime;   /* play time in minutes: +1 every 1501 gPlayTimeTicks, capped at 59939; saved */
extern MapLight* gSubmapLight;   /* the map's light data */
extern LightAnim gLightAnims[8];   /* the light animators (LightAnim) */
extern s32 gLightColours[12];   /* the map's light colours (SetupLights reads 9; ResetTimeOfDay clears 12) */
extern s32 gLightDirections[6];   /* and directions */
extern s32 gSkyColour;   /* the sky colour (map.c) */
extern s32 gSkyColourG;
extern s32 gSkyColourB;
extern s32 gFogColourR;   /* (effects.c) */
extern s32 gFogColourG;
extern s32 gFogColourB;
extern FogRange gFogNow;   /* and fog range */

/* The data src/game/lighting.c defines. */
extern DayPeriod gDayPalettePeriods[4];   /* the day's palette periods: the frame of the day each starts, its palette, the fade's frames (lighting.c's .data; where the extern stood, so the declarations keep their count) */

#endif
