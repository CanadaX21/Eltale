#ifndef GAME_ENDING_TEXT_H
#define GAME_ENDING_TEXT_H

/* The functions of src/game/ending_text.c, declared once for every file that calls them. */

#include "common.h"

void LoadEndingScene(s32 index);
s32 StepEndingText(void);
void FadeSceneMusic(u32 index);
void CopyEndingSpeakerName(u8* text, u8* name);

typedef struct {
    u8* text;                       /* MESSAGE_TEXT_SIZE bytes for gTextBuffer */
    u8* name;                       /* 0x20 bytes for gSpeakerName */
    u8 window;                      /* message window to open (OpenMessageWindow) */
} TimedTextPage;                     /* 0xC: one timed text page */

/* The state src/game/ending_text.c defines (.bss), in its storage order: the order of these declarations is the
 * unit's .bss layout (include/bss_order.h). */
extern u8 D_8007A8C0[0x20];   /* unreferenced */
extern s32 gTextPageTimer;   /* frames until the next page */
extern s32 gTextPagesLeft;   /* pages left */
extern TimedTextPage* gNextTextPage;   /* the next page */

typedef struct {
    s32 map;               /* map (gNextMap) */
    s32 submap;               /* submap (gNextSubmap) */
    f32 cameraX;               /* camera position x, y, z */
    f32 cameraY;
    f32 cameraZ;
    f32 cameraPitch;              /* the camera's pitch and yaw, degrees (negated into the view rotation) */
    f32 cameraYaw;
    f32 viewConeHalf;              /* gViewConeHalf */
    f32 brianX;              /* Brian's spawn: x, z and yaw (gBrianSpawnX, gBrianSpawnZ, gBrianSpawnYaw) */
    f32 brianZ;
    f32 brianYaw;
    NpcRecord* npcs;       /* the scene's NPC records (SpawnNPCs): each must name the scene's submap */
    s32 npcCount;
    TimedTextPage* textPages;    /* the text pages */
    s32 textPageCount;              /* how many */
    u16 flags;              /* bit 0: Brian appears in the scene (else GAME_STATE_HIDE_BRIAN) */
    s16 music;              /* music; FadeSceneMusic fades it out when it changes between neighbours */
} EndingScene;             /* 0x40; 7 of them at gEndingScenes (LoadEndingScene loads one) */

/* Storage the C does not define yet (assembly or binary), declared here for its type. */
extern EndingScene gEndingScenes[7];   /* in the ending's data, event_blob_a: a link name (undefined_syms.jp.txt) */

#endif
