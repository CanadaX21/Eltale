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
void EndingLoop(void);   /* game mode 6: the ending's seven scenes */
void StaffCreditsLoop(void);   /* game mode 7: the staff credits */
void MenuLoop(void);
void TitleLoop(void);
void IntroLoop(void);
void EpilogueLoop(void);   /* game mode 5: the epilogue (its overlay's segment: credits_overlay) */
void WaitForScheduler(s16 type);   /* wait for the scheduler */
void FadeScreenIn(void);
void FadeScreenOut(void);

/* gFieldMode: how the field was entered */
#define FIELD_MODE_NORMAL     0  /* any field start (FieldLoop sets it once the place is loaded) */
#define FIELD_MODE_NEW_GAME   1  /* a new game's first field start (InitNewGame): SetupNPCs starts the opening speech
                                    (gOpeningSpeechText) */
#define FIELD_MODE_LOADED     2  /* the first field start after a load from the save menu (MenuLoop, with Brian at his
                                    save point); no code tests for it */
#define FIELD_MODE_LAST_BOSS  4  /* the last boss is beaten (OnBossDefeated): the countdown leads to the ending
                                    (FieldLoop), the HUD is not drawn (BuildFrame), the map's music does not return */

/* gTransitionStyle: how FadeScreenOut and FadeScreenIn cross a change of place */
#define TRANSITION_BLACK       0  /* through black, 16 frames: a door or warp (FieldLoop) */
#define TRANSITION_WHITE       1  /* through white, 16 frames: the Wings (UseWings) */
#define TRANSITION_BLACK_SLOW  2  /* through black, 32 frames: the inn, a defeat, between the ending's scenes
                                     (StepEndingText), the staff credits' scenes */
#define TRANSITION_WHITE_SLOW  3  /* through white, 32 frames: the last boss beaten, into the ending (EndingLoop) */
#define TRANSITION_GREY_SLOW   4  /* through the grey ramp (gFadeRamp), 256 frames: no code selects it */

/* gTitleChoice: the title's choice, in its high nibble (BuildFrame reads it when TitleScreen_Update returns 0) */
#define TITLE_CHOICE_MASK      0xF0
#define TITLE_CHOICE_NEW_GAME  0x10  /* a new game: the field */
#define TITLE_CHOICE_CONTINUE  0x20  /* the save pages, to load a game (EnterSaveMode(SAVE_MODE_LOAD)) */
#define TITLE_CHOICE_DELETE    0x30  /* the save pages, to delete one (EnterSaveMode(SAVE_MODE_DELETE)) */

/* The state src/game/main.c defines (.bss), in its storage order: the order of these declarations is the
 * unit's .bss layout (include/bss_order.h). */
extern u16 gGameMode;   /* game mode (read with lhu by TitleLoop) */
extern u16 gFieldMode;
extern s32 gModeCountdown;   /* the mode's countdown */
extern u16 gTransitionStyle;
extern u16 gFadeOutAt;
extern s32 gCurrentFrame;   /* which frame is current */
extern s32 gMatrixHead;   /* matrices used this frame */
extern Gfx* gDisplayListHead;   /* the display list being written */
extern s32 gSkyFillR;   /* the sky fill colour: red, then gSkyFillG / gSkyFillB */
extern s32 gSkyFillG;
extern u32 gSkyFillB;
extern s32 D_8007A194;   /* unreferenced (three words: IDO would 8-align an array this size) */
extern s32 D_8007A198;   /* unreferenced */
extern s32 D_8007A19C;   /* unreferenced */
extern s16 gFogNear;   /* fog near (lighting.c) */
extern s16 gFogFar;   /* fog far */
extern s32 D_8007A1A4;   /* unreferenced */
extern OSMesgQueue gDmaQueue;
extern OSMesg gDmaMesgBuf[1];
extern s32 gSpareLightAnim;   /* a light animator allocated with the other two, never used */
extern s32 gMapLightAnim;   /* the map and sky light animator (an index into gLightAnims) */
extern u8 gTitleChoice;   /* the title's choice, in its high nibble (TITLE_CHOICE_) */
extern u8 D_8007A1CD;
extern s32 gFrameCount;   /* frames sent since the loop started */
extern s32 gLongestDisplayList;   /* the longest display list so far */
extern u8* gSegment1Data;
extern NNSched gScheduler;   /* the scheduler (0x680) */
extern OSMesgQueue* gGfxTaskQueue;   /* the scheduler's graphics task queue */
extern s32 D_8007A864;   /* unreferenced */
extern NNScClient gMainSchedClient;   /* the main thread's scheduler client */
extern OSMesgQueue gSchedDoneQueue;   /* the scheduler's task-done queue */
extern OSMesg gSchedDoneMesgBuf[8];
extern u64 gLastFrameTime;   /* osGetTime() when the last frame went out */
extern u64 gFrameTime;   /* osGetTime() this frame */
extern u64 gFrameDeltaMicros;   /* the microseconds in between */

#endif
