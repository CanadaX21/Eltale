#include "common.h"
#include "game/ending.h"
#include "game/audio.h"
#include "game/camera.h"
#include "game/map.h"
#include "game/math.h"
#include "game/message.h"
#include "game/player.h"
#include "game/spline.h"

extern s32 gNextScene;               /* the next scene */
extern s32 gTimeOfDay;
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
    u16 unk0;                       /* frames left on screen, 0 = free */
    char unk2[2];
    CreditImage* unk4;
} CreditImageSlots;                     /* 32 image slots */
typedef struct {
    s32 map;
    s32 submap;
    s32 flags;                      /* bit 0: place Brian at x/y/z */
    f32 x;
    f32 y;
    f32 z;
    ModelPart* path;             /* 0x18: the camera path */
    CreditImage* images;           /* 0x1C: its credits images */
} SceneEntry;                     /* 0x20: one scene of the sequence */
extern s32 gCameraPathFrame;               /* the path frame */
extern s32 gCameraPathFrames;
extern ModelPart* gCameraPath;     /* the camera path being played */
extern s32 gModeCountdown;
extern u16 gTransitionStyle;
extern u16 gFadeOutAt;
extern Camera gCamera;     /* the camera */
extern s32 gGameState;   /* game state flags (gGameState) */
extern u16 gGameMode;   /* the game mode (defined in main.c) */
void ResetEndingScenes(void) {
    gNextScene = 0;
    gTimeOfDay = 0x34BC;
}
extern SceneEntry gSceneSequence[];    /* the scene sequence */
extern f32 gBrianSpawnX;               /* Brian's spawn x */
extern f32 gBrianSpawnZ;
extern f32 gBrianSpawnYaw;
extern s32 gArrivalFlags;
extern BrianData1 gBrianStats;        /* Brian's stats */
extern PlayerState gPlayerState;
extern s32 gNpcCount;
extern s32 gEnemyCount;               /* enemy count */
extern s32 gCurrentMap;               /* current map */
extern s32 gNextMap;               /* next map */
extern s32 gNextSubmap;               /* next submap */
extern PendingWarp gPendingWarp;
extern f32 gViewConeHalf;
extern s32 gMessageWindowsOpen;
extern s32 gCreditFrame;
extern CreditImage* gNextCreditImage;     /* the next image to show */
extern CreditImageSlots gCreditImageSlots[32];
void StartScene(void) {
    SceneEntry* scene;
    s32 i;

    scene = &gSceneSequence[gNextScene++];
    gCameraPath = scene->path;
    gCameraPathFrame = 0;
    gCameraPathFrames = gCameraPath->frames;
    gCurrentMap = -1;
    gNextMap = scene->map;
    gNextSubmap = scene->submap;
    ResetMessageWindows(0);
    gMessageWindowsOpen = 0;
    gPendingWarp.entrance = -1;
    LoadMap();
    if (scene->flags & 1) {
        gBrianSpawnX = scene->x;
        gBrianSpawnZ = scene->y;
        gBrianSpawnYaw = scene->z;
        gArrivalFlags = 0x108;
        gBrianStats.currHP = gBrianStats.maxHP;
        gPlayerState.animCue = 0;
        SpawnBrian();
    } else {
        gGameState |= 0x10000;
    }
    InitRoomCamera();
    gCamera.mode = 3;
    gViewConeHalf = 35.0f;
    StepCameraPath(gCameraPath, gCameraPathFrame);
    gNpcCount = 0;
    gEnemyCount = 0;
    gCreditFrame = 0;
    gNextCreditImage = scene->images;
    for (i = 0; i < 32; i++) {
        gCreditImageSlots[i].unk0 = 0;
    }
}
void StepEndingScene(void) {
    StepCameraPath(gCameraPath, gCameraPathFrame++);
    if (gCameraPathFrames - gCameraPathFrame == 0x31 && !(gGameState & 0x80)) {
        gGameState |= 0x80;
        gTransitionStyle = 2;
        if (gNextScene >= 0xE) {
            gModeCountdown = 0x14E;
            gFadeOutAt = 0x14C;
            StartMusicFadeOut(0x12C);
        } else {
            gModeCountdown = 0x31;
            gFadeOutAt = 0x2F;
        }
    }
}
void EndEndingScene(void) {
    if (gNextScene < 0xE) {
        gGameState |= 0x60;
        return;
    }
    gGameMode = 0;
    gGameState |= 0x8240;
}
void StepCameraPath(ModelPart* path, s32 frame) {
    ModelKey key;
    MtxF mtx;
    s32 last;

    last = path->frames;
    if (last < frame) {
        frame = last;
    }
    SampleSpline(path, &key, frame);
    gCamera.cam_x = key.x;
    gCamera.cam_y = key.y;
    gCamera.cam_z = key.z;
    BuildRotationMatrix(&mtx, -key.unk10 * 0.017453292519943295, -key.unkC * 0.017453292519943295, key.unk14 * 0.017453292519943295);
    TransformVecByCols(&mtx, 0.0f, 0.0f, -100.0f, &gCamera.focus_x, &gCamera.focus_y, &gCamera.focus_z);
    gCamera.focus_x += gCamera.cam_x;
    gCamera.focus_y += gCamera.cam_y;
    gCamera.focus_z = gCamera.cam_z - gCamera.focus_z;
}
extern Gfx gCreditImageGfx[];             /* render state for the images */
extern Gfx* gDisplayListHead;
void DrawCreditImages(void) {
    CreditImage* img;
    CreditImageSlots* slot;
    s32 i;
    s32 alpha;
    s32 left;
    s32 elapsed;
    s32 x2;

    i = 31;
    slot = gCreditImageSlots;
    while (gCreditFrame == gNextCreditImage->start) {
        while (i != 0 && slot->unk0 != 0) {
            i--;
            slot++;
        }
        gNextCreditImage->palette[0] = 0;
        slot->unk0 = gNextCreditImage->duration;
        slot->unk4 = gNextCreditImage;
        gNextCreditImage++;
    }
    gSPDisplayList(gDisplayListHead++, gCreditImageGfx);
    slot = gCreditImageSlots;
    i = 32;
    do {
        i--;
        left = slot->unk0;
        if (left != 0) {
            img = slot->unk4;
            if (slot->unk0 < 16) {
                alpha = left * 16;
            } else {
                elapsed = img->duration - left;
                alpha = (elapsed < 32) ? elapsed * 8 : 0xFF;
            }
            gDPLoadTextureTile_4b(gDisplayListHead++, img->image, G_IM_FMT_CI, (img->width + 15) & ~15, img->height, 0, 0,
                                  img->width - 1, img->height - 1, 0, 0, 0, 0, 0, 0, 0);
            gDPLoadTLUT_pal16(gDisplayListHead++, 0, img->palette);
            gDPSetEnvColor(gDisplayListHead++, 0xFF, 0xFF, 0xFF, alpha);
            x2 = (img->x + img->width) & ~1;
            gSPTextureRectangle(gDisplayListHead++, img->x << 2, img->y << 2, x2 << 2, (img->y + img->height) << 2,
                                G_TX_RENDERTILE, 0, 0, 0x400, 0x400);
            slot->unk0--;
        }
        slot++;
    } while (i != 0);
    gCreditFrame++;
}

