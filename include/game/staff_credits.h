#ifndef GAME_STAFF_CREDITS_H
#define GAME_STAFF_CREDITS_H

/* The functions of src/game/staff_credits.c, declared once for every file that calls them. */

#include "common.h"

void ResetSceneSequence(void);   /* the staff credits' scene player (staff_credits.c) */
void StartSequenceScene(void);
void StepSequenceScene(void);
void EndSequenceScene(void);
void StepCameraPath(ModelPart* path, s32 frame);
void DrawCreditImages(void);

typedef struct {
    u16 start;                      /* the scene frame it appears on (gCreditFrame) */
    u16 duration;
    s16 x;
    s16 y;
    s16 width;
    s16 height;
    u16* palette;                   /* 0xC: 16 colours (colour 0 is cleared to transparent) */
    u8* image;                      /* 0x10: CI4 */
} CreditImage;                     /* 0x14: one credits image */
typedef struct {
    u16 timer;                       /* frames left on screen, 0 = free */
    char unk2[2];
    CreditImage* image;
} CreditImageSlots;                     /* 32 image slots */

/* The state src/game/staff_credits.c defines (.bss), in its storage order: the order of these declarations is the
 * unit's .bss layout (include/bss_order.h). */
extern s32 gNextScene;   /* the next scene */
extern s32 gCameraPathFrame;   /* the path frame */
extern s32 gCameraPathFrames;
extern ModelPart* gCameraPath;   /* the camera path being played */
extern s32 gCreditFrame;
extern CreditImage* gNextCreditImage;   /* the next image to show */
extern CreditImageSlots gCreditImageSlots[32];

typedef struct {
    s32 map;
    s32 submap;
    s32 flags;                      /* bit 0: put Brian down at brianX, brianZ, brianYaw (StartSequenceScene) */
    f32 brianX;
    f32 brianZ;
    f32 brianYaw;
    ModelPart* path;             /* 0x18: the camera path */
    CreditImage* images;           /* 0x1C: its credits images */
} SceneEntry;                     /* 0x20: one scene of the sequence */

/* Storage the C does not define yet (assembly or binary), declared here for its type. */
extern SceneEntry gSceneSequence[];   /* the staff credits' fourteen scenes (event_blob_b) */

#endif
