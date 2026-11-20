#include "common.h"
BSS_ORDER_PAD(0)
#include "game/ending_text.h"
#include "game/audio.h"
#include "game/camera.h"
#include "game/enemy.h"
#include "game/map.h"
#include "game/math.h"
#include "game/message.h"
#include "game/player.h"
#include "game/sound.h"
#include "game/collision.h"
#include "game/main.h"

/* ---- ending_text.c's .bss (0x8007A8C0-0x8007A8F0): the ending's timed text pages. Defined in their storage order, the order its header declares them in: IDO 5.3 lays the unit's
   .bss out by those declarations (include/bss_order.h). The D_ name marked unreferenced is space no code
   addresses (an unknown variable). ---- */
u8 D_8007A8C0[0x20];                     /* unreferenced */
s32 gTextPageTimer;                      /* frames until the next page */
s32 gTextPagesLeft;                      /* pages left */
TimedTextPage* gNextTextPage;            /* the next page */

void LoadEndingScene(s32 index) {
    EndingScene* entry = &gEndingScenes[index];
    MtxF mtx;

    gNextTextPage = entry->textPages;
    gTextPagesLeft = entry->textPageCount;
    gTextPageTimer = 60;
    gCurrentMap = -1;
    gNextMap = entry->map;
    gNextSubmap = entry->submap;
    ResetMessageWindows(0);
    gMessageWindowsOpen = 0;
    gPendingWarp.entrance = -1;
    LoadMap();
    gBrianSpawnX = entry->brianX;
    gBrianSpawnZ = entry->brianZ;
    gBrianSpawnYaw = entry->brianYaw;
    gArrivalFlags = WARP_ARRIVAL;
    gPlayerState.animCue = 0;
    gBrianStats.currHP = gBrianStats.maxHP;
    SpawnBrian();
    if (!(entry->flags & 1)) {
        gGameState |= GAME_STATE_HIDE_BRIAN;
    }
    gNpcCount = 0;
    LoadNPCArea(gSubmapHeader->area);
    SpawnNPCs(entry->npcs, entry->npcCount);
    gEnemyCount = 0;
    InitRoomCamera();
    gCamera.mode = 3;
    gCamera.cam_x = entry->cameraX;
    gCamera.cam_y = entry->cameraY;
    gCamera.cam_z = entry->cameraZ;
    BuildRotationMatrix(&mtx, -entry->cameraPitch * 0.017453292519943295, -entry->cameraYaw * 0.017453292519943295, 0.0f);
    TransformVecByCols(&mtx, 0.0f, 0.0f, -100.0f, &gCamera.focus_x, &gCamera.focus_y, &gCamera.focus_z);
    gCamera.focus_x += gCamera.cam_x;
    gCamera.focus_y += gCamera.cam_y;
    gCamera.focus_z = gCamera.cam_z - gCamera.focus_z;
    gViewConeHalf = entry->viewConeHalf;
    RequestMusic(entry->music);
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
            gGameState |= GAME_STATE_MAP_CHANGE;
            gModeCountdown = 0x31;
            gTransitionStyle = TRANSITION_BLACK_SLOW;
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
        music = &gEndingScenes[index].music;
        /* the previous entry's music: one entry back (the entry[-1].music form loads the two in the other order) */
        if (music[-(s32)(sizeof(EndingScene) / sizeof(s16))] != *music) {
            StartMusicFadeOut(0x40);
        }
    }
}
void CopyEndingSpeakerName(u8* text, u8* name) {
    s32 i;
    u8* dst;

    dst = gTextBuffer;
    for (i = MESSAGE_TEXT_SIZE; i != 0; i--) {
        *dst++ = *text++;
    }
    dst = gSpeakerName;
    for (i = 0x20; i != 0; i--) {
        *dst++ = *name++;
    }
}
