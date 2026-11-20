#include "common.h"
BSS_ORDER_PAD(0)
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
#include "game/staff_credits.h"
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
#include "game/pak_menu.h"
#include "game/player.h"
#include "game/save_menu.h"
#include "game/save_format.h"
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
#include "segment_symbols.h"
/* the main thread (mainproc; Quest func_80000C50, Quest64-Decomp's mainproc) */

/* ---- main.c's .bss (0x8007A170-0x8007A8C0), in address order; the three OSTime timers
   at its end are defined before their only user, FieldLoop. The 0x10 before it is nnsched.c's (nnsched's frame
   counter gRetraceCount). The D_ names marked unreferenced are space no code addresses (unknown variables).
   IDO 5.3 lays a unit's .bss out by its variables' block numbers, so a declaration added or removed before them, in
   a header too, can change their order; BSS_ORDER_PAD keeps the original order and `python3 tools/bss_order.py fix`
   recomputes it (docs/DECLARATIONS.md). ---- */
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
u8 gTitleChoice;
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
    ControllerPakMenu(&gSiMesgQueue, &gScheduler.retraceMQ);
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
            gGameState &= ~(GAME_STATE_ITEM_MENU | GAME_STATE_MESSAGE);
            if (gGameMode == MODE_NONE) {
                gGameMode = MODE_GAMEPLAY;
            }
            switch (gGameMode) {
                case MODE_GAMEPLAY:
                    FieldLoop();
                    break;
                case MODE_ENDING:
                    EndingLoop();
                    break;
                case MODE_STAFF_CREDITS:
                    StaffCreditsLoop();
                    break;
                case MODE_MENU:
                    MenuLoop();
                    break;
                case MODE_TITLE:
                    TitleLoop();
                    break;
                case MODE_SPLASH:
                    IntroLoop();
                    break;
                case MODE_EPILOGUE:
                    EpilogueLoop();
                    break;
            }
            if (gGameState & GAME_STATE_RESTART) {
                break;
            }
        }
    }
}
void InitNewGame(void) {
    gGameState &= GAME_STATE_CLEARED;
    gGameState |= GAME_STATE_FADE_IN | GAME_STATE_LOAD_NEXT | GAME_STATE_MAP_MUSIC;
    gGameMode = MODE_SPLASH;
    gCurrentMap = -1;
    gReturnDest = -1;
    gExitDest = -1;
    gLoadedSkyRecord = 0xFFFF;
    gCamera.arrivalYaw = 0.0f;
    gPlayTime = 0;
    gNextMap = 0xD;   /* a new game starts in map 13 (Melrode Monastery), submap 17 */
    gNextSubmap = 0x11;
    gPendingWarp.entrance = -1;
    gSavedMap = 0xD;
    gSavedSubmap = 0x11;
    gSavedEntrance = 0;
    gBrianSpawnX = -26.0f;
    gBrianSpawnZ = 0.0f;
    gBrianSpawnYaw = 1.5707964f;
    gArrivalFlags = WARP_ARRIVAL;
    gPlayerState.animCue = 0;
    gHudSlideState = 1;
    gItemBarSlide = 0xC;
    gHudHpBarWidth = gHudMpBarWidth = 0;
    gFieldMode = FIELD_MODE_NEW_GAME;
    gTransitionStyle = TRANSITION_BLACK;
    gTitleChoice = 0;
    D_8007A1CD = 0;
}
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
s32 func_8020C15C(void);                           /* the epilogue overlay's frame: 0 once it has ended */
void BuildFrame(GfxFrame* frame) {
    s32 len;                         /* also the epilogue overlay's result */
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
    if (gGameMode == MODE_GAMEPLAY || gGameMode == MODE_ENDING || gGameMode == MODE_STAFF_CREDITS) {
        gDPSetFillColor(gDisplayListHead++, (GPACK_RGBA5551(gSkyFillR, gSkyFillG, gSkyFillB, 1) << 16) |
                                          GPACK_RGBA5551(gSkyFillR, gSkyFillG, gSkyFillB, 1));
        gDPFillRectangle(gDisplayListHead++, 8, 8, 311, 231);
    } else {
        gDPSetFillColor(gDisplayListHead++, (GPACK_RGBA5551(0, 0, 0, 1) << 16) | GPACK_RGBA5551(0, 0, 0, 1));
        gDPFillRectangle(gDisplayListHead++, 8, 8, 311, 231);
    }
    gDPPipeSync(gDisplayListHead++);
    gDPSetCycleType(gDisplayListHead++, G_CYC_2CYCLE);

    if (gGameMode == MODE_GAMEPLAY || gGameMode == MODE_ENDING || gGameMode == MODE_STAFF_CREDITS) {
        UpdateCamera(dyn);
        UpdateFrameLights();
        if (gLightRecordFlags & 4) {
            StepLightAnim(gMapLightAnim);
            DrawSky(dyn->mtx.list);
        }
        StepLightAnim(gMapLightAnim);
        DrawMapFrame(&dyn->mtx);
        if (!(gGameState & GAME_STATE_HIDE_BRIAN)) {
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
        if (gGameState & GAME_STATE_MESSAGE) {
            DrawDialogueBox();
        }
        if (!(gGameState & GAME_STATE_UNSET_4000) && (gFieldMode != FIELD_MODE_LAST_BOSS)) {
            DrawHUD(&dyn->mtx);
        }
        if (gGameState & GAME_STATE_ITEM_MENU) {
            ItemMenu();
        }
        if (gGameMode == MODE_STAFF_CREDITS) {
            DrawCreditImages();
        }
    } else if (gGameMode == MODE_MENU) {
        StartMenu(&dyn->mtx);
    } else if (gGameMode == MODE_TITLE) {
        switch (TitleScreen_Update()) {
            case 0:
                switch (gTitleChoice & TITLE_CHOICE_MASK) {
                    case TITLE_CHOICE_NEW_GAME:
                        gGameMode = MODE_GAMEPLAY;
                        gGameState &= ~GAME_STATE_SAVE_MENU;
                        break;
                    case TITLE_CHOICE_CONTINUE:
                        gGameState |= GAME_STATE_SAVE_MENU;
                        EnterSaveMode(SAVE_MODE_LOAD);
                        break;
                    case TITLE_CHOICE_DELETE:
                        gGameState |= GAME_STATE_SAVE_MENU;
                        EnterSaveMode(SAVE_MODE_DELETE);
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
    } else if (gGameMode == MODE_EPILOGUE) {
        len = func_8020C15C();
        DrawScreenFade(&dyn->mtx);
        if (len == 0) {
            gGameMode = MODE_STAFF_CREDITS;
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
/* the field / battle loop (FieldLoop) */
u64 gLastFrameTime;                      /* osGetTime() when the last frame went out */
u64 gFrameTime;                      /* osGetTime() this frame */
u64 gFrameDeltaMicros;                      /* the microseconds in between */
void FieldLoop(void) {
    OSMesg msg;
    GfxFrame* frame;

    gFrameCount = 0;
    gModeCountdown = 0;
    msg = NULL;
    if (gGameState & GAME_STATE_LOAD_NEXT) {
        if (gGameState & GAME_STATE_RESPAWN) {
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
        gGameState &= ~(GAME_STATE_LOAD_NEXT | GAME_STATE_RESPAWN);
    }
    if (gGameState & GAME_STATE_SAVE_MENU) {
        SetAnimation(&gPlayerState.brian.transform, BRIAN_ANIM_STAND, 1, BRIAN_ANIM_STAND, 1, 1);
    }
    if (gGameState & GAME_STATE_FADE_IN) {
        gGameState &= ~GAME_STATE_FADE_IN;
        FadeScreenIn();
    }
    if (gGameState & GAME_STATE_MAP_MUSIC) {
        FindMapBGM(gCurrentMap, gCurrentSubmap, 0);
        gGameState &= ~GAME_STATE_MAP_MUSIC;
    }
    gGameState &= ~(GAME_STATE_SAVE_MENU | GAME_STATE_MAP_CHANGE);
    gTransitionStyle = TRANSITION_BLACK;
    gFadeOutAt = 0x12;
    gFieldMode = FIELD_MODE_NORMAL;
    while (gGameMode == MODE_GAMEPLAY) {
        ReadController(gControllerChannel);
        gMatrixHead = 0;
        gEnvTimersAdded = 0;
        ResetShadows();
        if (gSubmapFlags & SUBMAP_ENCOUNTERS) {
            RollRandomEncounter();
        }
        if (gBattleState & BATTLE_ACTIVE) {
            if (gBattleState & BATTLE_INTRO) {
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
        if (!(gGameState & (GAME_STATE_ITEM_MENU | GAME_STATE_MESSAGE | GAME_STATE_SPELL_INPUT | GAME_STATE_MAP_CHANGE))) {
            if (!(gPlayerState.brian.transform.stateFlags & ACTOR_BUSY) &&
                CheckDoorTriggers(gPlayerState.brian.transform.pos.x, gPlayerState.brian.transform.pos.z, gPlayerState.brian.transform.yrot) != 0) {
                gGameState |= GAME_STATE_MAP_CHANGE;
                ClampCameraToTarget();
                StartBrianDeparture();
                gTransitionStyle = TRANSITION_BLACK;
            } else if (!(gGameState & GAME_STATE_UNSET_1000) && !(gPlayerState.brian.transform.stateFlags & ACTOR_BUSY) && (gButtonsPressed & START_BUTTON) &&
                       !(gBattleState & BATTLE_ACTIVE) && !(gButtonsHeld & Z_TRIG) && !(gGameState & GAME_STATE_ITEM_MENU)) {
                gGameMode = MODE_MENU;
                PlaySound(0, 1, 0xFF);
            }
        } else if ((gGameState & GAME_STATE_MAP_CHANGE) && !(gGameState & GAME_STATE_MESSAGE)) {
            if (--gModeCountdown == 0) {
                if (gFieldMode == FIELD_MODE_LAST_BOSS) {
                    gGameMode = MODE_ENDING;
                } else {
                    gGameMode = MODE_NONE;
                }
                gGameState |= GAME_STATE_FADE_IN;
                if (!(gGameState & GAME_STATE_SAVE_MENU)) {
                    gGameState |= GAME_STATE_LOAD_NEXT;
                }
                gNextMap = gPendingWarp.map;
                gNextSubmap = gPendingWarp.submap;
                if (gGameState & GAME_STATE_SAVE_MENU) {        /* the inn's rest */
                    gPlayerState.stats->currHP = gPlayerState.stats->maxHP;
                    gPlayerState.stats->currMP = gPlayerState.stats->maxMP;
                    gTimeOfDay += 9000;
                    SetTimeOfDay(gTimeOfDay);
                    gSavedMap = gCurrentMap;
                    gSavedSubmap = gCurrentSubmap;
                    gSavedEntrance = gPendingWarp.entrance;
                    gTransitionStyle = TRANSITION_BLACK_SLOW;
                }
                gGameState |= GAME_STATE_MAP_MUSIC;
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
    if (gGameState & GAME_STATE_STOP_MUSIC) {
        StopMusicSlot(0);
    }
    WaitForScheduler(2);
    if (gGameState & GAME_STATE_STOP_MUSIC) {
        gGameState &= ~GAME_STATE_STOP_MUSIC;
        while (GetMusicSlotState(0) != 0) {
        }
    }
    if (gGameState & GAME_STATE_SAVE_MENU) {
        EnterSaveMode(SAVE_MODE_SAVE);
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
    gTransitionStyle = TRANSITION_WHITE_SLOW;
    gTimeOfDay = 0x41EB;
    while (gGameMode == MODE_ENDING) {
        if (gGameState & GAME_STATE_LOAD_NEXT) {
            gFrameCount = 0;
            LoadEndingScene(scene++);
            InitDayPalette();
            gMapLightAnim = AllocLightAnim();
            gSpareLightAnim = AllocLightAnim();
            gActorLightAnim = AllocLightAnim();
            gLightAnims[gActorLightAnim].mode = 2;
            BuildColourRamp();
            gScreenFadeFlags |= FADE_SETUP;
            if (gGameState & GAME_STATE_FADE_IN) {
                gGameState &= ~GAME_STATE_FADE_IN;
                FadeScreenIn();
            }
            gGameState &= ~(GAME_STATE_LOAD_NEXT | GAME_STATE_MAP_CHANGE | GAME_STATE_MAP_MUSIC);
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
        if (gGameState & GAME_STATE_MAP_CHANGE) {
            if (--gModeCountdown == 0) {
                StopLoopingSounds();
                if (scene >= 7) {
                    gGameMode = MODE_EPILOGUE;
                    gGameState |= GAME_STATE_FADE_IN | GAME_STATE_LOAD_NEXT | GAME_STATE_MAP_MUSIC;
                } else {
                    gGameState |= GAME_STATE_FADE_IN | GAME_STATE_LOAD_NEXT | GAME_STATE_MAP_MUSIC;
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
        if (gGameState & GAME_STATE_LOAD_NEXT) {
            WaitForScheduler(2);
            gFrameCount = 0;
        }
    }
    if (gGameState & GAME_STATE_STOP_MUSIC) {
        StopMusicSlot(0);
    }
    if (gGameState & GAME_STATE_STOP_MUSIC) {
        gGameState &= ~GAME_STATE_STOP_MUSIC;
        while (GetMusicSlotState(0) != 0) {
        }
    }
}
void StaffCreditsLoop(void) {
    GfxFrame* frame;
    OSMesg msg;
#ifdef MATCHING_HACK_IDO
    s32 pad;                         /* unused: the -g3 stack word below msg */
#endif

    msg = NULL;
    dma_write(event_blob_b_ROM_START, gEventBlob, instrument_ctl_ROM_START - event_blob_b_ROM_START);
    gModeCountdown = 0;
    gTransitionStyle = TRANSITION_BLACK_SLOW;
    gSoundVolume = 0;
    ResetSceneSequence();
    while (gGameMode == MODE_STAFF_CREDITS) {
        if (gGameState & GAME_STATE_LOAD_NEXT) {
            gFrameCount = 0;
            StartSequenceScene();
            InitDayPalette();
            gMapLightAnim = AllocLightAnim();
            gSpareLightAnim = AllocLightAnim();
            gActorLightAnim = AllocLightAnim();
            gLightAnims[gActorLightAnim].mode = 2;
            BuildColourRamp();
            gScreenFadeFlags |= FADE_SETUP;
            if (gGameState & GAME_STATE_FADE_IN) {
                gGameState &= ~GAME_STATE_FADE_IN;
                FadeScreenIn();
            }
            gGameState &= ~(GAME_STATE_LOAD_NEXT | GAME_STATE_MAP_CHANGE);
        }
        ReadController(gControllerChannel);
        gMatrixHead = 0;
        gEnvTimersAdded = 0;
        ResetShadows();
        StepSequenceScene();
        UpdateTimeOfDay();
        UpdateLavaPalette();
        BuildFrame(&gFrames[gCurrentFrame]);
        frame = &gFrames[gCurrentFrame];
        if (gGameState & GAME_STATE_MAP_CHANGE) {
            if (--gModeCountdown == 0) {
                StopLoopingSounds();
                EndSequenceScene();
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
        if (gGameState & GAME_STATE_LOAD_NEXT) {
            WaitForScheduler(2);
            gFrameCount = 0;
        }
    }
    if (gGameState & GAME_STATE_STOP_MUSIC) {
        StopMusicSlot(0);
    }
    if (gGameState & GAME_STATE_STOP_MUSIC) {
        gGameState &= ~GAME_STATE_STOP_MUSIC;
        while (GetMusicSlotState(0) != 0) {
        }
    }
    gGameState |= GAME_STATE_CLEARED;
}
void func_8020C0D0(void);            /* the loaded overlay's entry */
void MenuLoop(void) {
    GfxFrame* frame;
    s16 fieldMode = FIELD_MODE_LOADED;                  /* the field mode a respawn sets; read only there */

    gFrameCount = 0;
    gRespawnOnExit = 0;
    if (gGameState & GAME_STATE_SAVE_MENU) {
        RequestMusic(0x13);
    }
    while (gGameMode == MODE_MENU) {
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
    if (gGameState & GAME_STATE_SAVE_MENU) {
        RequestMusic(-1);
        while (gMusicPlaying != -1) {
            UpdateSound();
        }
    }
    if (gRespawnOnExit != 0) {
        RespawnAtSavePoint();
        gFieldMode = fieldMode;
        gGameState &= ~GAME_STATE_SAVE_MENU;
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
void EpilogueLoop(void) {
    GfxFrame* frame;

    gFrameCount = 0;
    LoadOvl_Epilogue();
    func_8020C0D0();
    BuildColourRamp();
    gScreenFadeFlags |= FADE_SETUP;
    UpdateBGMDelay(0x14, 0x3C);
    while (gGameMode == MODE_EPILOGUE) {
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
void FadeScreenIn(void) {
    switch (gTransitionStyle) {
        case TRANSITION_BLACK:
            StartScreenFade(0x10, FADE_BRIGHTEN, gFadeRampBlack16[15]);
            break;
        case TRANSITION_WHITE:
            StartScreenFade(0x10, FADE_BRIGHTEN, gFadeRampWhite16[15]);
            break;
        case TRANSITION_BLACK_SLOW:
            StartScreenFade(0x20, FADE_BRIGHTEN, gFadeRampBlack32[31]);
            break;
        case TRANSITION_WHITE_SLOW:
            StartScreenFade(0x20, FADE_BRIGHTEN, gFadeRampWhite32[31]);
            break;
        case TRANSITION_GREY_SLOW:
            StartScreenFade(0x100, FADE_BRIGHTEN, gFadeRamp[255]);
            break;
    }
}
void FadeScreenOut(void) {
    switch (gTransitionStyle) {
        case TRANSITION_BLACK:
            StartScreenFade(0xF, FADE_HOLD, gFadeRampBlack16);
            break;
        case TRANSITION_WHITE:
            StartScreenFade(0xF, FADE_HOLD, gFadeRampWhite16);
            break;
        case TRANSITION_BLACK_SLOW:
            StartScreenFade(0x1F, FADE_HOLD, gFadeRampBlack32);
            break;
        case TRANSITION_WHITE_SLOW:
            StartScreenFade(0x1F, FADE_HOLD, gFadeRampWhite32);
            break;
        case TRANSITION_GREY_SLOW:
            StartScreenFade(0xFF, FADE_HOLD, gFadeRamp);
            break;
    }
}
