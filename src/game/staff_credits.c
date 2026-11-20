#include "common.h"
BSS_ORDER_PAD(0)
#include "game/staff_credits.h"
#include "game/audio.h"
#include "game/camera.h"
#include "game/map.h"
#include "game/math.h"
#include "game/message.h"
#include "game/player.h"
#include "game/spline.h"
#include "game/collision.h"
#include "game/enemy.h"
#include "game/lighting.h"
#include "game/main.h"

/* ---- staff_credits.c's .bss (0x80092ED0-0x80092FF0): the staff credits' scene sequence and its credit images.
   Defined in their storage order, the order its header declares them in: IDO 5.3 lays the unit's
   .bss out by those declarations (include/bss_order.h). ---- */
s32 gNextScene;                          /* the next scene */
s32 gCameraPathFrame;                    /* the path frame */
s32 gCameraPathFrames;
ModelPart* gCameraPath;                  /* the camera path being played */
s32 gCreditFrame;
CreditImage* gNextCreditImage;           /* the next image to show */
CreditImageSlots gCreditImageSlots[32];
void ResetSceneSequence(void) {
    gNextScene = 0;
    gTimeOfDay = 0x34BC;
}
void StartSequenceScene(void) {
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
        gBrianSpawnX = scene->brianX;
        gBrianSpawnZ = scene->brianZ;
        gBrianSpawnYaw = scene->brianYaw;
        gArrivalFlags = WARP_ARRIVAL;
        gBrianStats.currHP = gBrianStats.maxHP;
        gPlayerState.animCue = 0;
        SpawnBrian();
    } else {
        gGameState |= GAME_STATE_HIDE_BRIAN;
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
        gCreditImageSlots[i].timer = 0;
    }
}
void StepSequenceScene(void) {
    StepCameraPath(gCameraPath, gCameraPathFrame++);
    if (gCameraPathFrames - gCameraPathFrame == 0x31 && !(gGameState & GAME_STATE_MAP_CHANGE)) {
        gGameState |= GAME_STATE_MAP_CHANGE;
        gTransitionStyle = TRANSITION_BLACK_SLOW;
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
void EndSequenceScene(void) {
    if (gNextScene < 0xE) {
        gGameState |= GAME_STATE_FADE_IN | GAME_STATE_LOAD_NEXT;
        return;
    }
    gGameMode = MODE_NONE;
    gGameState |= GAME_STATE_LOAD_NEXT | GAME_STATE_STOP_MUSIC | GAME_STATE_RESTART;
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
    BuildRotationMatrix(&mtx, -key.xrot * 0.017453292519943295, -key.yrot * 0.017453292519943295, key.zrot * 0.017453292519943295);
    TransformVecByCols(&mtx, 0.0f, 0.0f, -100.0f, &gCamera.focus_x, &gCamera.focus_y, &gCamera.focus_z);
    gCamera.focus_x += gCamera.cam_x;
    gCamera.focus_y += gCamera.cam_y;
    gCamera.focus_z = gCamera.cam_z - gCamera.focus_z;
}
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
        while (i != 0 && slot->timer != 0) {
            i--;
            slot++;
        }
        gNextCreditImage->palette[0] = 0;
        slot->timer = gNextCreditImage->duration;
        slot->image = gNextCreditImage;
        gNextCreditImage++;
    }
    gSPDisplayList(gDisplayListHead++, gCreditImageGfx);
    slot = gCreditImageSlots;
    i = 32;
    do {
        i--;
        left = slot->timer;
        if (left != 0) {
            img = slot->image;
            if (slot->timer < 16) {
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
            slot->timer--;
        }
        slot++;
    } while (i != 0);
    gCreditFrame++;
}

