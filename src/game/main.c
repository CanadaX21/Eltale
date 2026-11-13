#include "common.h"
#include "game/main.h"
#include "game/audio.h"
#include "game/boss.h"
#include "game/camera.h"
#include "game/collision.h"
#include "game/controller.h"
#include "game/dma.h"
#include "game/dust.h"
#include "game/effects.h"
#include "game/encounter.h"
#include "game/ending.h"
#include "game/ending_text.h"
#include "game/enemy.h"
#include "game/hud.h"
#include "game/intro.h"
#include "game/lighting.h"
#include "game/map.h"
#include "game/map_objects.h"
#include "game/message.h"
#include "game/model.h"
#include "game/overlay.h"
#include "game/player.h"
#include "game/save_menu.h"
#include "game/screen_fade.h"
#include "game/shadow.h"
#include "game/signs.h"
#include "game/sound.h"
#include "game/spells.h"
#include "game/spirits.h"
#include "game/start_menu.h"
#include "game/text.h"
#include "game/title.h"
#include "sched/nnsched.h"
extern s8 gRespawnOnExit;
extern PendingWarp gPendingWarp;
extern u64 gspF3DEXDataStart[];
extern u8 gGfxFifoBuffer[];
extern u64 gRspDramStack[];           /* the RSP's DRAM stack */
extern u64 gRspYieldBuffer[];           /* its yield buffer */
extern s32 gNextMap;               /* next map */
/* the main thread (mainproc; Quest func_80000C50, Quest64-Decomp's mainproc) */
extern s32 gGameState;   /* game state flags */
extern OSMesgQueue gSiMesgQueue;       /* the SI queue */
extern s32 gCurrentMap;               /* current map */
extern s32 gNextSubmap;               /* next submap */
extern s32 gReturnDest;
extern s32 gExitDest;
extern u8 instrument_ctl_ROM_START[];                /* instrument bank, its table, the sound effect bank and table (ROM) */
extern u8 instrument_tbl_ROM_START[];
extern u8 sequence_bank_ROM_START[];
extern u8 sfx_ctl_ROM_START[];
extern u8 sfx_tbl_ROM_START[];
extern u8 pak_menu_module_ROM_START[];                /* the code loaded at 0x80100000 */
extern u8 pak_menu_module_ROM_END[];
extern u8 D_80100000[];
void func_801009EC(OSMesgQueue* siQueue, OSMesgQueue* retraceQueue);                /* in the code at 0x80100000 */

extern u8 instrument_ctl_ROM_END[];              /* the end of the instrument bank (its table starts at the same address) */
extern u8 sequence_bank_ROM_END[];
extern u8 sfx_ctl_ROM_END[];
extern f32 gBrianSpawnX;               /* Brian's spawn x */
extern f32 gBrianSpawnZ;
extern f32 gBrianSpawnYaw;
extern s32 gArrivalFlags;
extern u16 gSavedMap;               /* saved map */
extern u16 gSavedSubmap;               /* saved submap */
extern u16 gSavedEntrance;
extern s8 gMusicPlaying;                /* the music playing, -1 = none */
extern PlayerState gPlayerState;
extern u16 gLoadedSkyRecord;
/* ---- main.c's .bss (0x8007A170-0x8007A8C0), in address order; the three OSTime timers
   at its end are defined before their only user, FieldLoop. The 0x10 before it is nnsched.c's (nnsched's frame
   counter gRetraceCount). The D_ names marked unreferenced are space no code addresses (unknown variables).
   IDO 5.3 lays a variable out by its FIRST declaration, and those first declared among the file's first 32
   declarations (typedefs and variables, externs too; not prototypes) after all the others: this block sits
   past them, so it comes out in order. Declaring any of these variables above it moves them. ---- */
u16 gGameMode;                         /* game mode (read with lhu by TitleLoop) */
u16 gFieldMode;
s32 gModeCountdown;                         /* the mode's countdown */
u16 gTransitionStyle;
u16 gFadeOutAt;
s32 gCurrentFrame;                         /* which frame is current */
s32 gMatrixHead;                         /* matrices used this frame */
Gfx* gDisplayListHead;                        /* the display list being written */
s32 gSkyFillR;                         /* the sky fill colour: red, then gSkyFillG / gSkyFillB */
s32 gSkyFillG;
u32 gSkyFillB;
s32 D_8007A194;                         /* unreferenced (three words: IDO would 8-align an array this size) */
s32 D_8007A198;                         /* unreferenced */
s32 D_8007A19C;                         /* unreferenced */
s16 gFogNear;                         /* fog near (lighting.c) */
s16 gFogFar;                         /* fog far */
s32 D_8007A1A4;                         /* unreferenced */
OSMesgQueue gDmaQueue;
OSMesg gDmaMesgBuf[1];
s32 gSpareLightAnim;                      /* a light animator allocated with the other two, never used */
s32 gMapLightAnim;                        /* the map and sky light animator (an index into gLightAnims) */
u8 D_8007A1CC;
u8 D_8007A1CD;
s32 gFrameCount;                         /* frames sent since the loop started */
s32 gLongestDisplayList;                         /* the longest display list so far */
u8* gSegment1Data;
NNSched gScheduler;                     /* the scheduler (0x680) */
OSMesgQueue* gGfxTaskQueue;                /* the scheduler's graphics task queue */
s32 D_8007A864;                         /* unreferenced */
NNScClient gMainSchedClient;                  /* the main thread's scheduler client */
OSMesgQueue gSchedDoneQueue;                 /* the scheduler's task-done queue */
OSMesg gSchedDoneMesgBuf[8];

void mainproc(s32 arg) {
    osCreateViManager(0xFE);
    osCreateMesgQueue(&gDmaQueue, gDmaMesgBuf, 1);
    InitAudio();
    InitMusicPlayer(instrument_ctl_ROM_START, instrument_ctl_ROM_END, sequence_bank_ROM_START, sequence_bank_ROM_END, instrument_tbl_ROM_START);
    InitSoundEffects(sfx_ctl_ROM_START, sfx_ctl_ROM_END, sfx_tbl_ROM_START);
    nnScCreateScheduler(&gScheduler, 1);
    InitControllers();
    if (osTvType == OS_TV_NTSC) {
        osViSetMode(&osViModeTable[OS_VI_NTSC_LAN1]);
    } else {
        osViSetMode(&osViModeTable[OS_VI_MPAL_LAN1]);
    }
    osViBlack(1);
    dma_write(pak_menu_module_ROM_START, D_80100000, pak_menu_module_ROM_END - pak_menu_module_ROM_START);
    loadTextTextureAndPal();
    func_801009EC(&gSiMesgQueue, &gScheduler.retraceMQ);
    osViBlack(1);
    osViSetSpecialFeatures(0x5A);
    nnScStartScheduler(&gScheduler);
    StartAudioThread(&gScheduler);
    osCreateMesgQueue(&gSchedDoneQueue, gSchedDoneMesgBuf, 8);
    nnScAddClient(&gScheduler, &gMainSchedClient, &gSchedDoneQueue);
    gGfxTaskQueue = nnScGetGfxMQ(&gScheduler);
    InitGraphics();

    while (1) {
        ResetStoryFlags();
        ResetMapObjectFlags();
        ResetSpiritsFound();
        InitBrian();
        Inventory_Init();
        ResetTimeOfDay();
        loadTextTextureAndPal();
        InitializeSoundData();
        InitNewGame();
        InitStartMenu();
        while (1) {
            gGameState &= ~3;
            if (gGameMode == 0) {
                gGameMode = MODE_GAMEPLAY;
            }
            switch (gGameMode) {
                case MODE_GAMEPLAY:
                    FieldLoop();
                    break;
                case MODE_ENDING:
                    EndingLoop();
                    break;
                case MODE_AFTER_CREDITS:
                    AfterCreditsLoop();
                    break;
                case MODE_PAUSED:
                    SaveModeLoop();
                    break;
                case MODE_TITLE:
                    TitleLoop();
                    break;
                case MODE_SPLASH:
                    IntroLoop();
                    break;
                case MODE_CREDITS:
                    CreditsLoop();
                    break;
            }
            if (gGameState & 0x8000) {
                break;
            }
        }
    }
}
extern u16 gPlayTime;
extern Camera gCamera;
extern u16 gItemBarSlide;
extern u16 gHudSlideState;
extern u16 gHudHpBarWidth;
extern u16 gHudMpBarWidth;
void InitNewGame(void) {
    gGameState &= 0x20000;
    gGameState |= 0x160;
    gGameMode = MODE_SPLASH;
    gCurrentMap = -1;
    gReturnDest = -1;
    gExitDest = -1;
    gLoadedSkyRecord = 0xFFFF;
    gCamera.unk84 = 0.0f;
    gPlayTime = 0;
    gNextMap = 0xD;
    gNextSubmap = 0x11;
    gPendingWarp.entrance = -1;
    gSavedMap = 0xD;
    gSavedSubmap = 0x11;
    gSavedEntrance = 0;
    gBrianSpawnX = -26.0f;
    gBrianSpawnZ = 0.0f;
    gBrianSpawnYaw = 1.5707964f;
    gArrivalFlags = 0x108;
    gPlayerState.animCue = 0;
    gHudSlideState = 1;
    gItemBarSlide = 0xC;
    gHudHpBarWidth = gHudMpBarWidth = 0;
    gFieldMode = 1;
    gTransitionStyle = 0;
    D_8007A1CC = 0;
    D_8007A1CD = 0;
}
extern u8 segment1_gfx_ROM_START[];
extern u8 segment1_gfx_ROM_END[];
extern u8 spell_bank_ROM_START[];
extern u8 cutscene_bank_ROM_START[];
extern u8 cutscene_bank_ROM_END[];
extern u8 gSegment1Gfx[];
extern u8 gEffectsBlock[];
extern u8 gNumberPalette[];
extern GfxFrame gFrames[2];       /* the two frames' graphics tasks */
extern u16 gFramebuffer0[320 * 240];    /* framebuffer 0 */
extern u16 gFramebuffer1[320 * 240];    /* framebuffer 1 */
void InitGraphics(void) {
    s32 i;
    u16* fb0;
    u16* fb1;

    dma_write(segment1_gfx_ROM_START, gSegment1Gfx, segment1_gfx_ROM_END - segment1_gfx_ROM_START);
    gSegment1Data = gSegment1Gfx;
    dma_write(spell_bank_ROM_START, gEffectsBlock, cutscene_bank_ROM_START - spell_bank_ROM_START);
    dma_write(cutscene_bank_ROM_START, gNumberPalette, cutscene_bank_ROM_END - cutscene_bank_ROM_START);
    gFrames[0].msg = 2;
    gFrames[0].framebuffer = gFramebuffer0;
    InitFrameTask(&gFrames[0]);
    gFrames[1].msg = 2;
    gFrames[1].framebuffer = gFramebuffer1;
    InitFrameTask(&gFrames[1]);
    gCurrentFrame = 0;
    i = 320 * 240;
    fb0 = gFramebuffer0;
    fb1 = gFramebuffer1;
    while (i != 0) {
        *fb0 = *fb1 = 0;
        i--;
        fb0++;
        fb1++;
    }
}
extern u64 gspF3DEXTextStart[];     /* the graphics microcode (its data says F3DEX 1.23): text at the address where the boot code ends */
void InitFrameTask(GfxFrame* frame) {
    frame->task.list.t.type = M_GFXTASK;
    frame->task.list.t.flags = 0;
    frame->task.list.t.ucode_boot = (u64*)rspbootTextStart;
    frame->task.list.t.ucode_boot_size = (uintptr_t) rspbootTextEnd - (uintptr_t) rspbootTextStart;
    frame->task.list.t.ucode = gspF3DEXTextStart;
    frame->task.list.t.ucode_data = gspF3DEXDataStart;
    frame->task.list.t.ucode_data_size = SP_UCODE_DATA_SIZE;
    frame->task.list.t.dram_stack = gRspDramStack;
    frame->task.list.t.dram_stack_size = SP_DRAM_STACK_SIZE8;
    frame->task.list.t.output_buff = (u64*) gGfxFifoBuffer;
    frame->task.list.t.output_buff_size = (u64*) &gGfxFifoBuffer[0x8000];
    frame->task.list.t.yield_data_ptr = gRspYieldBuffer;
    frame->task.list.t.yield_data_size = OS_YIELD_DATA_SIZE;
    frame->task.next = NULL;
    frame->task.flags = OS_SC_SWAPBUFFER;
    frame->task.msgQ = &gSchedDoneQueue;
    frame->task.msg = &frame->msg;
    frame->task.framebuffer = frame->framebuffer;
}
/* building a frame's display list (BuildFrame; Quest func_800011DC) */
extern s32 gDisplayListLength;               /* this frame's display list length */
extern s32 gActorLightAnim;
extern u16 gLightRecordFlags;               /* the light record's flags */
extern s8 D_8006CC80;
extern u16 gDepthBuffer[];             /* the depth buffer */
extern Gfx D_1000010[];
extern Mtx D_2000000[];
s32 func_8020C15C(void);                           /* the ending overlay */
void BuildFrame(GfxFrame* frame) {
    s32 len;                         /* also the ending overlay's result */
    FrameDynamic* dyn = &frame->dyn;
    s32 one = 1;                     /* a third local: the -g3 frame has its slot (Quest's twin has it too) */

    gDisplayListHead = frame->dyn.glist;
    gSPSegment(gDisplayListHead++, 0x00, 0x00000000);
    gSPSegment(gDisplayListHead++, 0x01, osVirtualToPhysical(gSegment1Data));
    gSPSegment(gDisplayListHead++, 0x02, osVirtualToPhysical(dyn));
    gSPDisplayList(gDisplayListHead++, D_1000010);
    gDPSetDepthImage(gDisplayListHead++, osVirtualToPhysical(gDepthBuffer));
    gDPPipeSync(gDisplayListHead++);
    gDPSetCycleType(gDisplayListHead++, G_CYC_FILL);
    gDPSetColorImage(gDisplayListHead++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 320, osVirtualToPhysical(gDepthBuffer));
    gDPSetFillColor(gDisplayListHead++, 0xFFFCFFFC);
    gDPFillRectangle(gDisplayListHead++, 8, 8, 311, 231);
    gDPPipeSync(gDisplayListHead++);
    gDPSetColorImage(gDisplayListHead++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 320, osVirtualToPhysical(frame->framebuffer));
    if (gGameMode == MODE_GAMEPLAY || gGameMode == MODE_ENDING || gGameMode == MODE_AFTER_CREDITS) {
        gDPSetFillColor(gDisplayListHead++, (GPACK_RGBA5551(gSkyFillR, gSkyFillG, gSkyFillB, 1) << 16) |
                                          GPACK_RGBA5551(gSkyFillR, gSkyFillG, gSkyFillB, 1));
        gDPFillRectangle(gDisplayListHead++, 8, 8, 311, 231);
    } else {
        gDPSetFillColor(gDisplayListHead++, (GPACK_RGBA5551(0, 0, 0, 1) << 16) | GPACK_RGBA5551(0, 0, 0, 1));
        gDPFillRectangle(gDisplayListHead++, 8, 8, 311, 231);
    }
    gDPPipeSync(gDisplayListHead++);
    gDPSetCycleType(gDisplayListHead++, G_CYC_2CYCLE);

    if (gGameMode == MODE_GAMEPLAY || gGameMode == MODE_ENDING || gGameMode == MODE_AFTER_CREDITS) {
        UpdateCamera(dyn);
        UpdateFrameLights();
        if (gLightRecordFlags & 4) {
            StepLightAnim(gMapLightAnim);
            DrawSky(dyn->mtx.list);
        }
        StepLightAnim(gMapLightAnim);
        DrawMapFrame(&dyn->mtx);
        if (!(gGameState & 0x10000)) {
            DrawBrian(dyn->mtx.list);
        }
        DrawNPCs(dyn->mtx.list);
        DrawMapObjects(dyn->mtx.list);
        DrawMapBillboards(&dyn->mtx);
        DrawEnemies(dyn->mtx.list);
        DrawBoss(dyn->mtx.list);
        UpdateSpiritSparkles();
        StepLightAnim(gActorLightAnim);
        DrawShadows(&dyn->mtx);
        DrawDust(&dyn->mtx);
        UpdateEffects(&dyn->mtx);
        DrawScreenFade(&dyn->mtx);
        if (gGameState & 2) {
            DrawDialogueBox();
        }
        if (!(gGameState & 0x4000) && (gFieldMode != 4)) {
            DrawHUD(&dyn->mtx);
        }
        if (gGameState & 1) {
            ItemMenu();
        }
        if (gGameMode == MODE_AFTER_CREDITS) {
            DrawCreditImages();
        }
    } else if (gGameMode == MODE_PAUSED) {
        StartMenu(&dyn->mtx);
    } else if (gGameMode == MODE_TITLE) {
        switch (TitleScreen_Update()) {
            case 0:
                switch (D_8007A1CC & 0xF0) {
                    case 0x10:
                        gGameMode = MODE_GAMEPLAY;
                        gGameState &= ~0x400;
                        break;
                    case 0x20:
                        gGameState |= 0x400;
                        EnterSaveMode(0);
                        break;
                    case 0x30:
                        gGameState |= 0x400;
                        EnterSaveMode(8);
                        break;
                    default:
                        gGameMode = MODE_GAMEPLAY;
                        break;
                }
                break;
            case 1:
                gGameMode = MODE_SPLASH;
                break;
        }
    } else if (gGameMode == MODE_SPLASH) {
        switch (Intro_Update()) {
            case 0:
                D_8006CC80 = 0;
                gGameMode = MODE_TITLE;
                break;
            case 1:
                D_8006CC80 = one;
                gGameMode = MODE_TITLE;
                break;
            default:
                D_8006CC80 = 0;
        }
    } else if (gGameMode == MODE_CREDITS) {
        len = func_8020C15C();
        DrawScreenFade(&dyn->mtx);
        if (len == 0) {
            gGameMode = MODE_AFTER_CREDITS;
        }
    }
    gDPFullSync(gDisplayListHead++);
    gSPEndDisplayList(gDisplayListHead++);
    osWritebackDCache(D_2000000, 0xDA70);

    len = gDisplayListHead - frame->dyn.glist;
    if (gLongestDisplayList < len) {
        gLongestDisplayList = len;
    }
    gDisplayListLength = len;

    frame->task.list.t.data_ptr = (u64*)dyn->glist;
    frame->task.list.t.data_size = (gDisplayListHead - dyn->glist) * sizeof(Gfx);
}
/* the mode loops (EndingLoop and on) */
extern s8 gControllerChannel;   /* the controller channel */
extern u16 gRetraceCount;               /* retraces since the last frame */
extern s32 gTimeOfDay;               /* time of day */
extern LightAnim gLightAnims[];       /* light records (lighting.c) */
extern u16 gScreenFadeFlags;   /* screen fade flags (screen_fade.c) */
extern s32 gEnvTimersAdded;
extern u8 event_blob_a_ROM_START[];                /* the ending's data, not code (mode 6; loaded at 0x801C0000): its text, NPC records, timed text pages and the 7 EndingScene records at D_801C14E4 */
extern u8 event_blob_b_ROM_START[];
extern u8 gEventBlob[];
extern u8 gSoundVolume;
/* the field / battle loop (FieldLoop) */
extern s32 gCurrentSubmap;               /* current submap */
extern u16 gSubmapFlags;               /* the submap's flags (bit 0: random battles and monsters here (FieldLoop, ResetEnemies; SetupNPCs only without it)) */
extern u16 gBattleState;               /* battle state */
extern u16 gButtonsHeld;               /* buttons (sound.c) */
extern u16 gButtonsPressed;
extern s32 gPendingSound;               /* a sound waiting to start, -1 = none */
extern s32 gMessageWindowsOpen;               /* message windows opened (message.c) */
u64 gLastFrameTime;                      /* osGetTime() when the last frame went out */
u64 gFrameTime;                      /* osGetTime() this frame */
u64 gFrameDeltaMicros;                      /* the microseconds in between */
void FieldLoop(void) {
    OSMesg msg;
    GfxFrame* frame;

    gFrameCount = 0;
    gModeCountdown = 0;
    msg = NULL;
    if (gGameState & 0x40) {
        if (gGameState & 0x800) {
            gCurrentMap = gSavedMap;
            gCurrentSubmap = gSavedSubmap;
            RespawnAtSavePoint();
        }
        ResetMessageWindows(0);
        gMessageWindowsOpen = 0;
        LoadMap();
        SpawnMapObjects();
        SetupMapSpirits();
        LoadRoomSigns();
        MapCursorToWorld();
        SpawnBrian();
        SetupNPCs();
        ResetEnemies();
        LoadMapBoss();
        InitRoomCamera();
        InitDayPalette();
        gMapLightAnim = AllocLightAnim();
        gSpareLightAnim = AllocLightAnim();
        gActorLightAnim = AllocLightAnim();
        gLightAnims[gActorLightAnim].mode = 2;
        InitEncounters();
        ResetSpells();
        ResetEffects();
        ResetDust();
        BuildColourRamp();
        ResetHUD();
        gGameState &= ~0x840;
    }
    if (gGameState & 0x400) {
        SetAnimation(&gPlayerState.brian.transform, 0, 1, 0, 1, 1);
    }
    if (gGameState & 0x20) {
        gGameState &= ~0x20;
        FadeScreenIn();
    }
    if (gGameState & 0x100) {
        FindMapBGM(gCurrentMap, gCurrentSubmap, 0);
        gGameState &= ~0x100;
    }
    gGameState &= ~0x480;
    gTransitionStyle = 0;
    gFadeOutAt = 0x12;
    gFieldMode = 0;
    while (gGameMode == MODE_GAMEPLAY) {
        ReadController(gControllerChannel);
        gMatrixHead = 0;
        gEnvTimersAdded = 0;
        ResetShadows();
        if (gSubmapFlags & 1) {
            RollRandomEncounter();
        }
        if (gBattleState & 1) {
            if (gBattleState & 0x200) {
                BattleStartCamera();
            } else {
                StepBattleRound();
                UpdateEnemies();
            }
        }
        UpdateBrian();
        UpdateNPCs();
        UpdateSpells();
        UpdateTimeOfDay();
        UpdateLavaPalette();
        UpdateDoorAnimations();
        BuildFrame(&gFrames[gCurrentFrame]);
        frame = &gFrames[gCurrentFrame];
        if (!(gGameState & 0x87)) {
            if (!(gPlayerState.brian.transform.stateFlags & ACTOR_BUSY) &&
                CheckDoorTriggers(gPlayerState.brian.transform.pos.x, gPlayerState.brian.transform.pos.z, gPlayerState.brian.transform.yrot) != 0) {
                gGameState |= 0x80;
                ClampCameraToTarget();
                StartBrianArrival();
                gTransitionStyle = 0;
            } else if (!(gGameState & 0x1000) && !(gPlayerState.brian.transform.stateFlags & ACTOR_BUSY) && (gButtonsPressed & START_BUTTON) &&
                       !(gBattleState & 1) && !(gButtonsHeld & Z_TRIG) && !(gGameState & 1)) {
                gGameMode = MODE_PAUSED;
                PlaySound(0, 1, 0xFF);
            }
        } else if ((gGameState & 0x80) && !(gGameState & 2)) {
            if (--gModeCountdown == 0) {
                if (gFieldMode == 4) {
                    gGameMode = MODE_ENDING;
                } else {
                    gGameMode = 0;
                }
                gGameState |= 0x20;
                if (!(gGameState & 0x400)) {
                    gGameState |= 0x40;
                }
                gNextMap = gPendingWarp.map;
                gNextSubmap = gPendingWarp.submap;
                if (gGameState & 0x400) {
                    gPlayerState.stats->currHP = gPlayerState.stats->maxHP;
                    gPlayerState.stats->currMP = gPlayerState.stats->maxMP;
                    gTimeOfDay += 9000;
                    SetTimeOfDay(gTimeOfDay);
                    gSavedMap = gCurrentMap;
                    gSavedSubmap = gCurrentSubmap;
                    gSavedEntrance = gPendingWarp.entrance;
                    gTransitionStyle = 2;
                }
                gGameState |= 0x100;
                StopLoopingSounds();
            } else if (gModeCountdown == gFadeOutAt) {
                FadeScreenOut();
            }
        }
        UpdateSound();
        if (gPendingSound != -1 && GetMusicSlotState(0) == 0) {
            LoadSequence(0, gPendingSound);
            StartSequence(0);
            gPendingSound = -1;
        }
        gFrameTime = osGetTime();
        gFrameDeltaMicros = OS_CYCLES_TO_USEC(gFrameTime - gLastFrameTime);
        if (gFrameCount == 0) {
            gFrameCount++;
            WaitForScheduler(1);
        } else {
            WaitForScheduler(2);
            while (gRetraceCount < 2) {
                osRecvMesg(&gSchedDoneQueue, &msg, OS_MESG_BLOCK);
            }
        }
        osSendMesg(gGfxTaskQueue, frame, OS_MESG_BLOCK);
        gRetraceCount = 0;
        gCurrentFrame ^= 1;
        gLastFrameTime = osGetTime();
    }
    if (gGameState & 0x200) {
        StopMusicSlot(0);
    }
    WaitForScheduler(2);
    if (gGameState & 0x200) {
        gGameState &= ~0x200;
        while (GetMusicSlotState(0) != 0) {
        }
    }
    if (gGameState & 0x400) {
        EnterSaveMode(4);
    }
}

void EndingLoop(void) {
    s32 scene;
    GfxFrame* frame;
    OSMesg msg;

    scene = 0;
    msg = NULL;
    dma_write(event_blob_a_ROM_START, gEventBlob, event_blob_b_ROM_START - event_blob_a_ROM_START);
    gModeCountdown = 0;
    gTransitionStyle = 3;
    gTimeOfDay = 0x41EB;
    while (gGameMode == MODE_ENDING) {
        if (gGameState & 0x40) {
            gFrameCount = 0;
            LoadEndingScene(scene++);
            InitDayPalette();
            gMapLightAnim = AllocLightAnim();
            gSpareLightAnim = AllocLightAnim();
            gActorLightAnim = AllocLightAnim();
            gLightAnims[gActorLightAnim].mode = 2;
            BuildColourRamp();
            gScreenFadeFlags |= 8;
            if (gGameState & 0x20) {
                gGameState &= ~0x20;
                FadeScreenIn();
            }
            gGameState &= ~0x1C0;
        }
        ReadController(gControllerChannel);
        gMatrixHead = 0;
        gEnvTimersAdded = 0;
        ResetShadows();
        if (StepEndingText() != 0) {
            FadeSceneMusic(scene);
        }
        UpdateTimeOfDay();
        UpdateLavaPalette();
        BuildFrame(&gFrames[gCurrentFrame]);
        frame = &gFrames[gCurrentFrame];
        if (gGameState & 0x80) {
            if (--gModeCountdown == 0) {
                StopLoopingSounds();
                if (scene >= 7) {
                    gGameMode = MODE_CREDITS;
                    gGameState |= 0x160;
                } else {
                    gGameState |= 0x160;
                }
            } else if (gModeCountdown == gFadeOutAt) {
                FadeScreenOut();
            }
        }
        UpdateSound();
        if (gFrameCount == 0) {
            gFrameCount++;
            WaitForScheduler(1);
        } else {
            WaitForScheduler(2);
            while (gRetraceCount < 2) {
                osRecvMesg(&gSchedDoneQueue, &msg, OS_MESG_BLOCK);
            }
        }
        osSendMesg(gGfxTaskQueue, frame, OS_MESG_BLOCK);
        gRetraceCount = 0;
        gCurrentFrame ^= 1;
        if (gGameState & 0x40) {
            WaitForScheduler(2);
            gFrameCount = 0;
        }
    }
    if (gGameState & 0x200) {
        StopMusicSlot(0);
    }
    if (gGameState & 0x200) {
        gGameState &= ~0x200;
        while (GetMusicSlotState(0) != 0) {
        }
    }
}
void AfterCreditsLoop(void) {
    GfxFrame* frame;
    OSMesg msg;
#ifdef MATCHING_HACK_IDO
    s32 pad;                         /* unused: the -g3 stack word below msg */
#endif

    msg = NULL;
    dma_write(event_blob_b_ROM_START, gEventBlob, instrument_ctl_ROM_START - event_blob_b_ROM_START);
    gModeCountdown = 0;
    gTransitionStyle = 2;
    gSoundVolume = 0;
    ResetEndingScenes();
    while (gGameMode == MODE_AFTER_CREDITS) {
        if (gGameState & 0x40) {
            gFrameCount = 0;
            StartScene();
            InitDayPalette();
            gMapLightAnim = AllocLightAnim();
            gSpareLightAnim = AllocLightAnim();
            gActorLightAnim = AllocLightAnim();
            gLightAnims[gActorLightAnim].mode = 2;
            BuildColourRamp();
            gScreenFadeFlags |= 8;
            if (gGameState & 0x20) {
                gGameState &= ~0x20;
                FadeScreenIn();
            }
            gGameState &= ~0xC0;
        }
        ReadController(gControllerChannel);
        gMatrixHead = 0;
        gEnvTimersAdded = 0;
        ResetShadows();
        StepEndingScene();
        UpdateTimeOfDay();
        UpdateLavaPalette();
        BuildFrame(&gFrames[gCurrentFrame]);
        frame = &gFrames[gCurrentFrame];
        if (gGameState & 0x80) {
            if (--gModeCountdown == 0) {
                StopLoopingSounds();
                EndEndingScene();
            } else if (gModeCountdown == gFadeOutAt) {
                FadeScreenOut();
            }
        }
        UpdateSound();
        if (gFrameCount == 0) {
            gFrameCount++;
            WaitForScheduler(1);
        } else {
            WaitForScheduler(2);
            while (gRetraceCount < 2) {
                osRecvMesg(&gSchedDoneQueue, &msg, OS_MESG_BLOCK);
            }
        }
        osSendMesg(gGfxTaskQueue, frame, OS_MESG_BLOCK);
        gRetraceCount = 0;
        gCurrentFrame ^= 1;
        if (gGameState & 0x40) {
            WaitForScheduler(2);
            gFrameCount = 0;
        }
    }
    if (gGameState & 0x200) {
        StopMusicSlot(0);
    }
    if (gGameState & 0x200) {
        gGameState &= ~0x200;
        while (GetMusicSlotState(0) != 0) {
        }
    }
    gGameState |= 0x20000;
}
void func_8020C0D0(void);            /* the loaded overlay's entry */
void SaveModeLoop(void) {
    GfxFrame* frame;
    s16 fieldMode = 2;                  /* the field mode a respawn sets; read only there */

    gFrameCount = 0;
    gRespawnOnExit = 0;
    if (gGameState & 0x400) {
        RequestMusic(0x13);
    }
    while (gGameMode == MODE_PAUSED) {
        ReadController(gControllerChannel);
        gMatrixHead = 0;
        BuildFrame(&gFrames[gCurrentFrame]);
        frame = &gFrames[gCurrentFrame];
        UpdateSound();
        if (gFrameCount == 0) {
            gFrameCount++;
            WaitForScheduler(1);
        } else {
            WaitForScheduler(2);
        }
        osSendMesg(gGfxTaskQueue, frame, OS_MESG_BLOCK);
        gCurrentFrame ^= 1;
    }
    if (gGameState & 0x400) {
        RequestMusic(-1);
        while (gMusicPlaying != -1) {
            UpdateSound();
        }
    }
    if (gRespawnOnExit != 0) {
        RespawnAtSavePoint();
        gFieldMode = fieldMode;
        gGameState &= ~0x400;
    }
    SetBGMVolume();
    WaitForScheduler(2);
}

void TitleLoop(void) {
    GfxFrame* frame;

    gFrameCount = 0;
    LoadOvl_TitleScreen();
    func_8020C0D0();
    while (gGameMode == MODE_TITLE) {
        ReadController(gControllerChannel);
        gMatrixHead = 0;
        BuildFrame(&gFrames[gCurrentFrame]);
        frame = &gFrames[gCurrentFrame];
        if (gFrameCount == 0) {
            gFrameCount++;
            WaitForScheduler(1);
        } else {
            WaitForScheduler(2);
        }
        osSendMesg(gGfxTaskQueue, frame, OS_MESG_BLOCK);
        gCurrentFrame ^= 1;
    }
    WaitForScheduler(2);
}
void IntroLoop(void) {
    GfxFrame* frame;

    gFrameCount = 0;
    LoadOvl_Splash_and_Intro();
    func_8020C0D0();
    while (gGameMode == MODE_SPLASH) {
        ReadController(gControllerChannel);
        gMatrixHead = 0;
        UpdateSound();
        BuildFrame(&gFrames[gCurrentFrame]);
        frame = &gFrames[gCurrentFrame];
        if (gFrameCount == 0) {
            gFrameCount++;
            WaitForScheduler(1);
        } else {
            WaitForScheduler(2);
        }
        osSendMesg(gGfxTaskQueue, frame, OS_MESG_BLOCK);
        gCurrentFrame ^= 1;
    }
    RequestMusic(-1);
    UpdateSound();
    WaitForScheduler(2);
}
void CreditsLoop(void) {
    GfxFrame* frame;

    gFrameCount = 0;
    LoadOvl_Ending();
    func_8020C0D0();
    BuildColourRamp();
    gScreenFadeFlags |= 8;
    UpdateBGMDelay(0x14, 0x3C);
    while (gGameMode == MODE_CREDITS) {
        ReadController(gControllerChannel);
        gMatrixHead = 0;
        UpdateSound();
        BuildFrame(&gFrames[gCurrentFrame]);
        frame = &gFrames[gCurrentFrame];
        if (gFrameCount == 0) {
            gFrameCount++;
            WaitForScheduler(1);
        } else {
            WaitForScheduler(2);
        }
        osSendMesg(gGfxTaskQueue, frame, OS_MESG_BLOCK);
        gCurrentFrame ^= 1;
    }
    WaitForScheduler(2);
}
void WaitForScheduler(s16 type) {
    s16* msg = NULL;

    do {
        osRecvMesg(&gSchedDoneQueue, (OSMesg*)&msg, OS_MESG_BLOCK);
    } while (*msg != type);
}
extern u8 gFadeRamp[256][4];
extern u8 gFadeRampBlack16[16][4];
extern u8 gFadeRampWhite16[16][4];
extern u8 gFadeRampBlack32[32][4];
extern u8 gFadeRampWhite32[32][4];
void FadeScreenIn(void) {
    switch (gTransitionStyle) {
        case 0:
            StartScreenFade(0x10, 4, gFadeRampBlack16[15]);
            break;
        case 1:
            StartScreenFade(0x10, 4, gFadeRampWhite16[15]);
            break;
        case 2:
            StartScreenFade(0x20, 4, gFadeRampBlack32[31]);
            break;
        case 3:
            StartScreenFade(0x20, 4, gFadeRampWhite32[31]);
            break;
        case 4:
            StartScreenFade(0x100, 4, gFadeRamp[255]);
            break;
    }
}
void FadeScreenOut(void) {
    switch (gTransitionStyle) {
        case 0:
            StartScreenFade(0xF, 2, gFadeRampBlack16);
            break;
        case 1:
            StartScreenFade(0xF, 2, gFadeRampWhite16);
            break;
        case 2:
            StartScreenFade(0x1F, 2, gFadeRampBlack32);
            break;
        case 3:
            StartScreenFade(0x1F, 2, gFadeRampWhite32);
            break;
        case 4:
            StartScreenFade(0xFF, 2, gFadeRamp);
            break;
    }
}
