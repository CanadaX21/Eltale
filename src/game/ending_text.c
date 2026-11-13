#include "common.h"
#include "game/ending_text.h"
#include "game/audio.h"
#include "game/camera.h"
#include "game/enemy.h"
#include "game/map.h"
#include "game/math.h"
#include "game/message.h"
#include "game/player.h"
#include "game/sound.h"

typedef struct {
    u8* text;                       /* 0x4B0 bytes for gTextBuffer */
    u8* name;                       /* 0x20 bytes for gSpeakerName */
    u8 window;                      /* message window to open (OpenMessageWindow) */
} TimedTextPage;                     /* 0xC: one timed text page */
typedef struct {
    s32 unk0;               /* map (gNextMap) */
    s32 unk4;               /* submap (gNextSubmap) */
    f32 unk8;               /* camera position x, y, z */
    f32 unkC;
    f32 unk10;
    f32 unk14;              /* camera angles, degrees (negated into the view rotation) */
    f32 unk18;
    f32 unk1C;              /* gViewConeHalf */
    f32 unk20;              /* Brian's position x, y, z */
    f32 unk24;
    f32 unk28;
    s32 unk2C;              /* SpawnNPCs's two arguments */
    s32 unk30;
    TimedTextPage* unk34;    /* the text pages */
    s32 unk38;              /* how many */
    u16 unk3C;              /* bit 0: keep game flag 0x10000 clear */
    s16 unk3E;              /* music; FadeSceneMusic fades it out when it changes between neighbours */
} EndingScene;             /* 0x40; 7 of them at D_801C14E4 (LoadEndingScene loads one) */
extern EndingScene D_801C14E4[7];
extern s32 gGameState;               /* game state flags */
extern s32 gModeCountdown;
extern u16 gTransitionStyle;
extern u16 gFadeOutAt;
extern s32 gTextPageTimer;               /* frames until the next page */
extern s32 gTextPagesLeft;               /* pages left */
extern TimedTextPage* gNextTextPage;     /* the next page */
extern s32 gMessageWindowsOpen;
extern s32 gCurrentMap;               /* current map */
extern s32 gNextMap;               /* next map */
extern s32 gNextSubmap;               /* next submap */
extern PendingWarp gPendingWarp;
extern f32 gBrianSpawnX;               /* Brian's start position */
extern f32 gBrianSpawnZ;
extern f32 gBrianSpawnYaw;
extern s32 gArrivalFlags;
extern PlayerState gPlayerState;
extern BrianData1 gBrianStats;
extern Submap* gSubmapHeader;        /* the current submap */
extern s32 gNpcCount;
extern s32 gEnemyCount;
extern Camera gCamera;     /* the camera */
extern f32 gViewConeHalf;               /* half the view cone, in degrees */

void LoadEndingScene(s32 index) {
    EndingScene* entry = &D_801C14E4[index];
    MtxF mtx;

    gNextTextPage = entry->unk34;
    gTextPagesLeft = entry->unk38;
    gTextPageTimer = 60;
    gCurrentMap = -1;
    gNextMap = entry->unk0;
    gNextSubmap = entry->unk4;
    ResetMessageWindows(0);
    gMessageWindowsOpen = 0;
    gPendingWarp.entrance = -1;
    LoadMap();
    gBrianSpawnX = entry->unk20;
    gBrianSpawnZ = entry->unk24;
    gBrianSpawnYaw = entry->unk28;
    gArrivalFlags = 0x108;
    gPlayerState.animCue = 0;
    gBrianStats.currHP = gBrianStats.maxHP;
    SpawnBrian();
    if (!(entry->unk3C & 1)) {
        gGameState |= 0x10000;
    }
    gNpcCount = 0;
    LoadNPCArea(gSubmapHeader->area);
    SpawnNPCs(entry->unk2C, entry->unk30);
    gEnemyCount = 0;
    InitRoomCamera();
    gCamera.mode = 3;
    gCamera.cam_x = entry->unk8;
    gCamera.cam_y = entry->unkC;
    gCamera.cam_z = entry->unk10;
    BuildRotationMatrix(&mtx, -entry->unk14 * 0.017453292519943295, -entry->unk18 * 0.017453292519943295, 0.0f);
    TransformVecByCols(&mtx, 0.0f, 0.0f, -100.0f, &gCamera.focus_x, &gCamera.focus_y, &gCamera.focus_z);
    gCamera.focus_x += gCamera.cam_x;
    gCamera.focus_y += gCamera.cam_y;
    gCamera.focus_z = gCamera.cam_z - gCamera.focus_z;
    gViewConeHalf = entry->unk1C;
    RequestMusic(entry->unk3E);
}
s32 StepEndingText(void) {
    s32 ret = 0;

    if (gTextPageTimer == 0) {
        if (gTextPagesLeft != 0) {
            CopyEndingSpeakerName(gNextTextPage->text, gNextTextPage->name);
            OpenMessageWindow(gNextTextPage->window);
            gTextPageTimer = 0xF;
            gNextTextPage++;
            gTextPagesLeft--;
        } else {
            gGameState |= 0x80;
            gModeCountdown = 0x31;
            gTransitionStyle = 2;
            gFadeOutAt = 0x2F;
            gTextPageTimer = 0x32;
            ret = 1;
        }
    } else if (gMessageWindowsOpen == 0) {
        gTextPageTimer--;
    }
    return ret;
}
void FadeSceneMusic(u32 index) {
    s16* music;

    if (index >= 7) {
        StartMusicFadeOut(0x40);
    } else {
        music = &D_801C14E4[index].unk3E;
        /* the previous entry's unk3E: one entry back (the entry[-1].unk3E form loads the two in the other order) */
        if (music[-(s32)(sizeof(EndingScene) / sizeof(s16))] != *music) {
            StartMusicFadeOut(0x40);
        }
    }
}
extern u8 gTextBuffer[];
extern u8 gSpeakerName[];
void CopyEndingSpeakerName(u8* text, u8* name) {
    s32 i;
    u8* dst;

    dst = gTextBuffer;
    for (i = 0x4B0; i != 0; i--) {
        *dst++ = *text++;
    }
    dst = gSpeakerName;
    for (i = 0x20; i != 0; i--) {
        *dst++ = *name++;
    }
}
