# The platform boundary of Eltale Monsters

A technical reference derived from the decomp's build of the original Japanese ROM and the project's research. The behaviour it describes is read from the code, not observed running, and every identification carries its grade of evidence; "What this inventory covers, and what it cannot see" below states its limits. The tools that generate it are not part of this repository.

This is where the game's own code meets the N64's system software: which services of Nintendo's library (libultra) it uses, from where, what it assumes about the hardware as far as the evidence shows, and what this inventory cannot see. A port replaces the hardware and OS side with its own platform layer; the library code that is plain computation (matrices, the audio library, the C library) can run natively.

## Summary

- Game code scanned: 601 functions in 41 objects (main's C units, the title and intro overlays' C units, and the code blocks kept as assembly).
- The library linked into main: 329 functions (verified 284, probable 24, unresolved 21).
- The game calls 90 of them directly, at 375 call sites, in 15 services: intro overlay (C units) 3, main (C) 344, Controller Pak module (assembly) 28.
- Every call site was found twice: from the build's relocations and by scanning the ROM's own instruction words; the two agree (check 2 below).

## Services the game uses

| Service | Library functions the game calls | Call sites | Calling game functions | For a port |
|---|---|---|---|---|
| debug output | osSyncPrintf | 2 | Intro_Update [intro_overlay], RepairSaveSlot | empty stubs in the shipped library: calls to them do nothing |
| threads and messages | osCreateMesgQueue, osCreateThread, osRecvMesg, osSendMesg, osSetEventMesg, osStartThread | 64 | AfterCreditsLoop, AudioThread, CreditsLoop, EndingLoop, FieldLoop, InitAudio, InitControllers, IntroLoop, ReadAudioRom, ReadController, SaveModeLoop, StartAudioThread, TitleLoop, WaitForScheduler, boot, dma_write, func_8010055C_DA4ACC [pak_menu_module], func_801009EC_DA4F5C [pak_menu_module], idle, mainproc, nnScCreateScheduler, nnScEventBroadcast, nnScEventHandler, nnScExecuteAudio, nnScExecuteGraphics, nnScStartScheduler, nnScWaitTaskReady | the OS kernel: a native port provides threads and message queues itself |
| timers and clock | osGetTime | 2 | FieldLoop | CPU count register and the OS timer list: replaced by host timing |
| CPU, caches, TLB and interrupts | osInitialize, osInvalDCache, osInvalICache, osSetIntMask, osVirtualToPhysical, osWritebackDCache, osWritebackDCacheAll | 19 | AudioThread, BuildFrame, ReadAudioRom, __amDma, boot, dma_write, func_8010055C_DA4ACC [pak_menu_module], nnScAddClient, nnScExecuteAudio, nnScRemoveClient | N64 CPU state (cache, TLB, interrupts, exceptions): no counterpart on a host; cache calls become no-ops, addresses need no translation |
| video interface | osCreateViManager, osViBlack, osViGetCurrentFramebuffer, osViSetEvent, osViSetMode, osViSetSpecialFeatures, osViSwapBuffer | 15 | func_8010055C_DA4ACC [pak_menu_module], func_801009EC_DA4F5C [pak_menu_module], mainproc, nnScCreateScheduler, nnScEventHandler, nnScExecuteGraphics, nnScWaitTaskReady | the VI: frame buffer swap, video mode and retrace events; replaced by the host's presentation |
| RCP tasks (RSP and RDP) | osSpTaskLoad, osSpTaskStartGo, osSpTaskYield, osSpTaskYielded | 8 | nnScExecuteAudio, nnScExecuteGraphics | starts the RSP microcode tasks (graphics and audio); a port runs the display lists and audio command lists through its own renderer and mixer |
| cartridge DMA (PI) | osCreatePiManager, osPiStartDma | 4 | ReadAudioRom, __amDma, dma_write, idle | ROM reads by DMA: a port reads its asset data from files or memory instead |
| Controller Pak (saves) | osPfsAllocateFile, osPfsDeleteFile, osPfsFileState, osPfsFreeBlocks, osPfsInitPak, osPfsNumFiles, osPfsReadWriteFile, osPfsRepairId | 25 | AllocateSaveNote, DeleteSaveNote, OpenSaveMenu, ReadPakNote, RepairPak, ScanControllerPak, WritePakNote, func_80036124, func_801009EC_DA4F5C [pak_menu_module] | the Controller Pak file system the game saves to; a port keeps the save format or converts it |
| Rumble Pak | osMotorInit | 5 | Intro_Update [intro_overlay], OpenSaveMenu, RepairPak, ScanControllerPak, func_801009EC_DA4F5C [pak_menu_module] | the rumble accessory's detection and control |
| Transfer Pak | osGbpakInit | 6 | Intro_Update [intro_overlay], OpenSaveMenu, RepairPak, ScanControllerPak, func_80036184, func_801009EC_DA4F5C [pak_menu_module] | the Game Boy Transfer Pak's initialisation |
| controllers (SI) | osContGetQuery, osContGetReadData, osContInit, osContSetCh, osContStartQuery, osContStartReadData | 13 | InitControllers, ReadController, func_80036144, func_80036164, func_801009EC_DA4F5C [pak_menu_module] | controller polling through the serial interface and its PIF RAM; replaced by host input |
| audio output (AI) | osAiGetLength, osAiSetFrequency, osAiSetNextBuffer | 3 | AudioThread, InitAudio | the audio interface's DMA buffers and sample rate; replaced by the host's audio output |
| audio library | alAudioFrame, alBnkfNew, alCSPGetState, alCSPGetVol, alCSPNew, alCSPPlay, alCSPSetBank, alCSPSetChlFXMix, alCSPSetSeq, alCSPSetVol, alCSPStop, alCSeqNew, alHeapDBAlloc, alHeapInit, alInit, alLink, alSeqFileNew, alSndpAllocate, alSndpGetState, alSndpNew, alSndpPlay, alSndpSetFXMix, alSndpSetPan, alSndpSetPitch, alSndpSetSound, alSndpSetVol, alSndpStop, alUnlink | 71 | ApplyMusicVolume, AudioThread, GetMusicSlotState, GetSoundState, InitAudio, InitMusicPlayer, InitSoundEffects, LoadMusicSequences, LoadSeqFileHeader, StartSequence, StartSound, StepLoopingSound, StepMusicFadeIn, StepMusicFadeOut, StopMusicSlot, StopSound, SwitchMusic, __amDma, __amDmaNew, __clearAudioDMA | libultra's audio library runs on the CPU: sequence and sound players and the synthesizer, which builds command lists for the aspMain microcode; a port runs it natively with an audio-microcode replacement, or swaps the audio stack |
| graphics utilities | guLookAtReflectF, guMtxCatF, guMtxF2L, guMtxIdent, guMtxXFMF, guOrtho, guPerspectiveF, guPosition, guTranslate | 36 | BuildActorMtx, BuildLookAtMtx, BuildRotationMtx, DrawCompass, DrawDust, DrawGlows, DrawGroundRings, DrawHUD, DrawMapBillboards, DrawMapFrame, DrawMapProps, DrawModelPart, DrawScreenFade, DrawShadows, DrawSpellCross, DrawSpiritCounters, DrawStaffHitEffects, InitRoomCamera, MtxFromDirection, SpawnSparkleBurst, SpawnStaffHitEffect, SpiritGainWindow, UpdateCamera, UpdateLightDirections, UpdateSparkleBursts, WorldToScreen | matrix and view helpers (gu); portable C a port can compile natively (fixed-point matrix layout to keep) |
| math and C library | __ll_mul, __ull_div, cosf, sinf, sins, sqrtf | 102 | BrianBattleMovement, BrianState_Run, BrianState_Stand, BrianState_Walk, BuildActorMatrix, BuildLookAtMatrix, BuildRotationMatrix, BuildSRTMatrix, CameraAboveNpcs, CameraMode_Field, CameraMode_Fixed, CountBattleAgility, DamageBrian, DamageEnemy, DrawCompass, DrawDamageNumbers, DrawEnemyMarker, DrawGroundRings, DrawMapProps, DrawMenuDigits, DrawMenuNumber, DrawMenuNumberZeros, DrawObjectMarker, DrawSignMarker, DrawSpiritCounters, DrawSpiritMarker, DrawSpiritSparkle, DrawStaffHitEffects, EnemyMeleeAttack, FieldLoop, FindStaffTarget, IsNearBoss, ItemMenu, MatrixFromDirection, MoveEnemy, Normalize2D, PickEnemyAIOption, PushBrianOutOfEnemies, PushEnemyOut, PushNPCOutOfBrian, PushOutOfWallSegment, SelectEnemyAction, SortMapModels, SpawnBoss, SpellCollision0, SpellMovement0, SpellOrigin_Ahead, TryReadSign, UpdateBrian, UpdateCamera, UpdateCameraBob, UpdateNPCs, UpdateTalkMarker, rotateCoordinatesByAngle | portable C library code; a host's C library provides the same |

## Calls by game function

Each game function that calls the library, with what it calls (segment and unit in brackets).

- boot [boot.c]: osInitialize, osCreateThread, osStartThread
- idle [boot.c]: osCreatePiManager, osCreateThread, osStartThread
- nnScCreateScheduler [nnsched.c]: osCreateMesgQueue x6, osViSetEvent, osSetEventMesg x3
- nnScStartScheduler [nnsched.c]: osCreateThread x3, osStartThread x3
- nnScEventHandler [nnsched.c]: osRecvMesg, osViBlack
- nnScAddClient [nnsched.c]: osSetIntMask x2
- nnScRemoveClient [nnsched.c]: osSetIntMask x2
- nnScEventBroadcast [nnsched.c]: osSendMesg
- nnScExecuteAudio [nnsched.c]: osRecvMesg x3, osWritebackDCacheAll, osSpTaskYield, osSpTaskYielded, osSpTaskLoad x2, osSpTaskStartGo x2, osSendMesg x3
- nnScExecuteGraphics [nnsched.c]: osRecvMesg x4, osSpTaskLoad, osSpTaskStartGo, osViBlack, osViSwapBuffer, osSendMesg
- nnScWaitTaskReady [nnsched.c]: osViGetCurrentFramebuffer x2, osRecvMesg
- PushOutOfWallSegment [collision.c]: sqrtf x6
- UpdateLightDirections [lighting.c]: guMtxXFMF
- DrawShadows [shadow.c]: guMtxF2L
- DrawSpiritSparkle [spirits.c]: sinf
- TryReadSign [signs.c]: sqrtf
- InitRoomCamera [camera.c]: guOrtho x2
- UpdateCamera [camera.c]: guPerspectiveF, guLookAtReflectF, guMtxF2L x2, guTranslate, sinf, cosf
- CameraMode_Field [camera.c]: sqrtf x2
- CameraMode_Fixed [camera.c]: sqrtf, cosf x2, sinf x2
- CameraAboveNpcs [camera.c]: sqrtf
- UpdateCameraBob [camera.c]: sinf
- SpellOrigin_Ahead [spells.c]: sqrtf
- SpellMovement0 [spells.c]: sqrtf x3
- SpellCollision0 [spells.c]: sqrtf x2
- mainproc [main.c]: osCreateViManager, osCreateMesgQueue x2, osViSetMode x2, osViBlack x2, osViSetSpecialFeatures
- BuildFrame [main.c]: osVirtualToPhysical x5, osWritebackDCache
- FieldLoop [main.c]: osGetTime x2, __ll_mul, __ull_div, osRecvMesg, osSendMesg
- EndingLoop [main.c]: osRecvMesg, osSendMesg
- AfterCreditsLoop [main.c]: osRecvMesg, osSendMesg
- SaveModeLoop [main.c]: osSendMesg
- TitleLoop [main.c]: osSendMesg
- IntroLoop [main.c]: osSendMesg
- CreditsLoop [main.c]: osSendMesg
- WaitForScheduler [main.c]: osRecvMesg
- DrawDamageNumbers [effects.c]: sins
- UpdateSparkleBursts [effects.c]: guMtxXFMF
- SpawnSparkleBurst [effects.c]: guMtxXFMF
- DrawStaffHitEffects [effects.c]: sqrtf, guMtxF2L
- SpawnStaffHitEffect [effects.c]: guMtxXFMF
- DrawGroundRings [effects.c]: sinf, sqrtf, guMtxF2L
- DrawGlows [effects.c]: guMtxF2L
- DrawDust [dust.c]: guMtxCatF, guMtxF2L
- DrawModelPart [model.c]: guMtxF2L x2
- DrawHUD [hud.c]: guTranslate
- DrawSpiritCounters [hud.c]: guTranslate x2, sins
- DrawCompass [hud.c]: guMtxF2L, sqrtf
- DrawSpellCross [hud.c]: guTranslate
- SpiritGainWindow [hud.c]: guTranslate
- UpdateTalkMarker [hud.c]: sins
- DrawSpiritMarker [hud.c]: sins
- DrawObjectMarker [hud.c]: sins
- DrawSignMarker [hud.c]: sins
- DrawEnemyMarker [hud.c]: sins
- ItemMenu [hud.c]: sins x2
- DrawScreenFade [screen_fade.c]: guMtxIdent
- Normalize2D [math.c]: sqrtf
- rotateCoordinatesByAngle [math.c]: sinf, cosf
- BuildActorMatrix [math.c]: sinf x3, cosf x3
- BuildActorMtx [math.c]: guMtxF2L
- BuildRotationMatrix [math.c]: sinf x3, cosf x3
- BuildRotationMtx [math.c]: guMtxF2L
- BuildLookAtMatrix [math.c]: sqrtf x2
- BuildLookAtMtx [math.c]: guMtxF2L
- MatrixFromDirection [math.c]: sqrtf x2
- MtxFromDirection [math.c]: guMtxF2L
- BuildSRTMatrix [math.c]: sinf x3, cosf x3
- WorldToScreen [math.c]: guMtxXFMF
- dma_write [dma.c]: osInvalICache, osInvalDCache, osPiStartDma, osRecvMesg
- ReadAudioRom [audio.c]: osInvalDCache, osPiStartDma, osRecvMesg
- InitAudio [audio.c]: alHeapInit, alHeapDBAlloc x7, osCreateMesgQueue x2, osAiSetFrequency, alInit
- __amDma [audio.c]: osVirtualToPhysical x2, alUnlink, alLink, osPiStartDma
- __amDmaNew [audio.c]: alLink, alHeapDBAlloc
- __clearAudioDMA [audio.c]: alUnlink, alLink
- AudioThread [audio.c]: osCreateMesgQueue x2, osRecvMesg x2, alCSPSetVol, osVirtualToPhysical, alAudioFrame, osSendMesg, osAiGetLength, osAiSetNextBuffer, alCSPGetVol x2
- LoadSeqFileHeader [audio.c]: alHeapDBAlloc x2
- StartAudioThread [audio.c]: osCreateThread, osStartThread
- InitMusicPlayer [audio.c]: alHeapDBAlloc x3, alCSPNew x2, alSeqFileNew, alBnkfNew, alCSPSetBank x2
- StartSequence [audio.c]: alCSeqNew, alCSPSetSeq, alCSPPlay, alCSPSetChlFXMix
- StopMusicSlot [audio.c]: alCSPStop
- GetMusicSlotState [audio.c]: alCSPGetState
- InitSoundEffects [audio.c]: alHeapDBAlloc x2, alSndpNew, alBnkfNew, alSndpAllocate
- StartSound [audio.c]: alSndpSetSound, alSndpGetState, alSndpPlay, alSndpSetFXMix
- StopSound [audio.c]: alSndpSetSound, alSndpStop
- GetSoundState [audio.c]: alSndpSetSound, alSndpGetState
- SwitchMusic [audio.c]: alCSPStop x2, alCSPSetVol x3
- StepLoopingSound [audio.c]: alSndpSetSound x5, alSndpSetPitch, alSndpSetVol x2, alSndpSetPan x2
- StepMusicFadeOut [audio.c]: alCSPStop, alCSPSetVol
- StepMusicFadeIn [audio.c]: alCSPSetVol
- LoadMusicSequences [audio.c]: alCSPStop x2
- ApplyMusicVolume [audio.c]: alCSPSetVol x2
- DrawMenuNumber [start_menu.c]: __ull_div
- DrawMenuNumberZeros [start_menu.c]: __ull_div
- DrawMenuDigits [start_menu.c]: __ull_div
- OpenSaveMenu [save_menu.c]: osPfsInitPak, osMotorInit, osGbpakInit
- RepairSaveSlot [save_menu.c]: osSyncPrintf
- InitControllers [controller.c]: osCreateMesgQueue, osSetEventMesg, osContInit, osContSetCh
- ReadController [controller.c]: osContSetCh, osContStartReadData, osRecvMesg, osContGetReadData
- ScanControllerPak [controller.c]: osPfsInitPak, osMotorInit, osGbpakInit, osPfsFileState, osPfsFreeBlocks
- AllocateSaveNote [controller.c]: osPfsAllocateFile
- DeleteSaveNote [controller.c]: osPfsDeleteFile
- ReadPakNote [controller.c]: osPfsReadWriteFile
- WritePakNote [controller.c]: osPfsFreeBlocks, osPfsReadWriteFile
- RepairPak [controller.c]: osMotorInit, osGbpakInit, osPfsInitPak x2, osPfsRepairId
- func_80036124 [controller.c]: osPfsNumFiles
- func_80036144 [controller.c]: osContStartQuery
- func_80036164 [controller.c]: osContGetQuery
- func_80036184 [controller.c]: osGbpakInit
- UpdateBrian [player.c]: sqrtf
- BrianState_Stand [player.c]: sqrtf
- BrianState_Walk [player.c]: sqrtf x2
- BrianState_Run [player.c]: sqrtf x2
- BrianBattleMovement [player.c]: sqrtf x2
- PushBrianOutOfEnemies [player.c]: sqrtf x3
- FindStaffTarget [player.c]: sqrtf
- DamageBrian [player.c]: sqrtf
- CountBattleAgility [player.c]: sqrtf
- UpdateNPCs [enemy.c]: sqrtf
- PushNPCOutOfBrian [enemy.c]: sqrtf x3
- SelectEnemyAction [enemy.c]: sqrtf
- EnemyMeleeAttack [enemy.c]: sqrtf
- PickEnemyAIOption [enemy.c]: sqrtf
- MoveEnemy [enemy.c]: sqrtf x2
- PushEnemyOut [enemy.c]: sqrtf x3
- DamageEnemy [enemy.c]: sqrtf
- IsNearBoss [boss.c]: sqrtf
- SpawnBoss [boss.c]: sqrtf
- DrawMapFrame [map.c]: guPosition x2
- DrawMapBillboards [map.c]: guPosition x2
- SortMapModels [map.c]: sqrtf, sins x3
- DrawMapProps [map.c]: sqrtf, guMtxF2L
- Intro_Update [intro.c]: osSyncPrintf, osMotorInit, osGbpakInit
- func_8010055C_DA4ACC [pak_menu_module]: osWritebackDCacheAll, osRecvMesg, osViBlack, osViSwapBuffer
- func_801009EC_DA4F5C [pak_menu_module]: osContStartReadData x2, osRecvMesg x3, osContGetReadData x2, osPfsInitPak x7, osMotorInit, osGbpakInit, osViBlack, osPfsNumFiles, osPfsFileState, osPfsFreeBlocks, osPfsDeleteFile, osPfsRepairId, osContStartQuery, osContGetQuery

## Data and callbacks crossing the boundary

- **RSP microcode (F3DEX 1.23 data)** (1): gspF3DEXDataStart in InitFrameTask
- **RSP microcode (F3DEX 1.23 text)** (1): gspF3DEXTextStart in InitFrameTask
- **RSP microcode (aspMain data)** (1): aspMainDataStart in AudioThread
- **RSP microcode (aspMain text)** (1): aspMainTextStart in AudioThread
- **RSP microcode (rspboot)** (3): rspbootTextStart in AudioThread, rspbootTextStart in InitFrameTask
- **RSP microcode (rspboot, its end)** (2): rspbootTextEnd in AudioThread, rspbootTextEnd in InitFrameTask
- **SDK data** (3): osTvType in mainproc, osViModeTable in mainproc
- **game function whose address is taken where alInit is called** (1): __amDmaNew in InitAudio
- **game function whose address is taken where osCreateThread is called** (6): AudioThread in StartAudioThread, idle in boot, mainproc in idle, nnScEventHandler in nnScStartScheduler, nnScExecuteAudio in nnScStartScheduler, nnScExecuteGraphics in nnScStartScheduler

## RSP microcode and fixed memory

From the project's ROM map, which owns these facts; [ROM_MAP.md](ROM_MAP.md) describes the blocks that hold them.

- rspboot: ROM 0x4C580-0x4C650, RAM 0x8004B980
- F3DEX 1.23 text: ROM 0x4C650-0x4DA80, RAM 0x8004BA50
- aspMain text: ROM 0x4DA80-0x4E8A0, RAM 0x8004CE80; its own end is not stated in code: the last nonzero word is at 0x4E894, zeros follow up to gGameState
- F3DEX 1.23 data: ROM 0x70E60-0x71660, RAM 0x80070260
- aspMain data: ROM 0x71660-0x71920, RAM 0x80070A60

Fixed RAM places the game loads ROM blocks to (block loads; the loader in brackets):

- RAM 0x80000400: ROM 0x1000-0x101000 (ipl3)
- RAM 0x80095AF0: ROM 0x71920-0x753F0 (InitGraphics@0x1CA4)
- RAM 0x80100000: ROM 0x753F0-0x851F0 (LoadMap@0xE46C)
- RAM 0x8010FE00: ROM 0x851F0-0xC8DE0 (LoadMap@0xE4F8)
- RAM 0x801E0000: ROM 0x3E1090-0x3F1BA0 (LoadMap@0xE58C)
- RAM 0x80206000: ROM 0x869620-0x86F6F0 (InitBrian@0x3F70)
- RAM 0x8020C0D0: ROM 0x86F6F0-0x871D10 (LoadNPCArea@0xA3A0)
- RAM 0x802A0000: ROM 0xA70350-0xA77F10 (LoadNPCArea@0xA3C0)
- RAM 0x803AC460: ROM 0xD384B0-0xD388B0 (Inventory_Init@0x25B38)
- RAM 0x80332D30: ROM 0xD3A8B0-0xD3A930 (loadTextTextureAndPal@0x35F20)
- RAM 0x803245A0: ROM 0xD3A930-0xD490C0 (loadTextTextureAndPal@0x35F38)
- RAM 0x800C0000: ROM 0xD490C0-0xD74AC0 (InitGraphics@0x1CD4)
- RAM 0x8039ADB0: ROM 0xD74AC0-0xD86170 (InitGraphics@0x1CF4)
- RAM 0x800814B0: ROM 0xD86170-0xD86770 (LoadMap@0xE484)
- RAM 0x80081AB0: ROM 0xD86770-0xD86970 (LoadMap@0xE49C)
- RAM 0x80100000: ROM 0xDA4570-0xDAB5F0 (mainproc@0x196C)
- RAM 0x8020C0D0: ROM 0xDAB5F0-0xDC62B0 (LoadOvl_TitleScreen@0x36DD0)
- RAM 0x8020C0D0: ROM 0xDC62B0-0xE22690 (LoadOvl_Splash_and_Intro@0x36E08)
- RAM 0x8020C0D0: ROM 0xE22690-0xE420D0 (LoadOvl_Ending@0x36E40)
- RAM 0x801C0000: ROM 0xE420D0-0xE43780 (EndingLoop@0x2CA8)
- RAM 0x801C0000: ROM 0xE43780-0xE5BF60 (AfterCreditsLoop@0x303C)

The two audio bank files (`.ctl`, ROM 0xE5BF60-0xE5ED70 and 0xECB7B0-0xECD470) have no fixed place: InitMusicPlayer and InitSoundEffects read them into buffers allocated from the audio heap at run time.

## Behaviour the evidence supports

- **Threads:** The game runs eight threads: idle (priority 1, created by boot), the game's main thread mainproc (5), the sample scheduler's event handler, audio and graphics threads (120, 110, 100) and the audio manager's AudioThread (50), and two that libultra starts for it: the PI manager (150, through osCreatePiManager) and the VI manager (254, through osCreateViManager(0xFE)). *Evidence:* src/game/boot.c boot and idle; src/game/main.c mainproc; src/sched/nnsched.c nnScStartScheduler; src/game/audio.c StartAudioThread (the priorities are the osCreateThread, osCreatePiManager and osCreateViManager arguments) *(read from the code)*
- **Scheduling:** Nintendo's sample scheduler (nnSc*, compiled into the game in unit nnsched.c) asks for a VI retrace message every field and for the RSP, RDP and pre-NMI events; its threads start the audio and graphics RSP tasks, and an audio task may yield the running graphics task and resume it. *Evidence:* src/sched/nnsched.c nnScCreateScheduler (nnScCreateScheduler(&gScheduler, 1) in src/game/main.c mainproc), nnScExecuteAudio, nnScExecuteGraphics *(read from the code)*
- **Frame rate:** Field play sends a new frame only after the previous frame's done message and at least two retraces (at most 30 frames a second on NTSC; the first frame waits for one): FieldLoop waits on the scheduler for the done message, then counts retraces, and takes each frame's duration in microseconds from osGetTime. *Evidence:* src/game/main.c FieldLoop (WaitForScheduler(2): NN_SC_DONE_MSG; the gRetraceCount loop; gFrameDeltaMicros) *(read from the code)*
- **Video mode:** mainproc starts the VI manager and sets NTSC LAN1 (320 by 240, 16-bit, anti-aliased, not interlaced) when osTvType is NTSC and MPAL LAN1 otherwise; nothing selects a PAL mode. The special features word 0x5A sets gamma off, gamma dithering off, divot on and the dither filter on. *Evidence:* src/game/main.c mainproc (osViModeTable[OS_VI_NTSC_LAN1] / [OS_VI_MPAL_LAN1], osViSetSpecialFeatures(0x5A)) *(read from the code)*
- **Presentation:** The scheduler's graphics thread shows each finished frame buffer with osViSwapBuffer. The screen is blanked (osViBlack) while mainproc sets the video mode, unblanked by the graphics thread after its first task, blanked again by the event handler on a pre-NMI (reset) message, and blanked by the Controller Pak module while it runs. *Evidence:* src/game/main.c mainproc; src/sched/nnsched.c nnScExecuteGraphics, nnScEventHandler; the Pak module's assembly *(read from the code)*
- **Graphics microcode:** Graphics tasks run the F3DEX 1.23 microcode: InitFrameTask points the task at rspboot and F3DEX's text and data, which the ROM map places in main. *Evidence:* src/game/main.c InitFrameTask; the microcode ranges above *(read from the code)*
- **Audio:** Audio is libultra's audio library: InitAudio asks the AI for 22050 Hz (keeping the rate it gets), 16 voices and the small-room reverb; music plays through compressed-sequence players and sound effects through a sound player. AudioThread builds one command list per video field (about 368 samples) for the aspMain microcode and queues the output with osAiSetNextBuffer. *Evidence:* src/game/audio.c InitAudio, InitMusicPlayer, InitSoundEffects, AudioThread (NUM_FIELDS 1) *(read from the code)*
- **Audio data:** The audio library reads its samples through the game: InitAudio registers __amDmaNew as the synthesizer's DMA routine, and __amDma and ReadAudioRom read the ROM with osPiStartDma (__amDma in fixed 0x500-byte chunks). *Evidence:* src/game/audio.c InitAudio, __amDma, ReadAudioRom *(read from the code)*
- **Cartridge reads:** Every other ROM read goes through dma_write, which invalidates the instruction and data caches over the destination and reads in chunks of at most 0x4000 bytes with osPiStartDma. *Evidence:* src/game/dma.c dma_write *(read from the code)*
- **Controllers:** InitControllers routes serial-interface events to its queue and initialises the controllers; ReadController polls them each time it runs (osContStartReadData, then osContGetReadData). *Evidence:* src/game/controller.c InitControllers, ReadController *(read from the code)*
- **Saves:** Saves live in the Controller Pak's file system (notes allocated, read, written and deleted through osPfs*). The game calls no EEPROM or flash routine, and no EPI access or cartridge-handle set-up (the route SRAM would take): its ROM reads go through osPiStartDma. *Evidence:* src/game/controller.c AllocateSaveNote, ReadPakNote, WritePakNote, DeleteSaveNote, ScanControllerPak; src/game/save_menu.c OpenSaveMenu *(read from the code)*
- **Accessories:** When osPfsInitPak reports a broken pak ID (PFS_ERR_ID_FATAL), OpenSaveMenu and ScanControllerPak probe for a Rumble Pak (osMotorInit) and then a Transfer Pak (osGbpakInit) and report PFS_ERR_NOPACK (no Controller Pak) if either answers; RepairPak returns PFS_ERR_CONTRFAIL in that case. The Controller Pak module and the intro probe too, with outcomes of their own (the module tries the Transfer Pak only after osMotorInit reports PFS_ERR_DEVICE; the intro does not call osPfsInitPak and keeps codes of its own). The game never drives the rumble motor. *Evidence:* src/game/save_menu.c OpenSaveMenu; src/game/controller.c ScanControllerPak, RepairPak; the Pak module's and the intro overlay's assembly *(read from the code)*
- **Debug output:** The game's debug messages (RepairSaveSlot's "Repair Info", one in the intro) go to an empty stub: nothing is printed in the shipped game. *Evidence:* src/game/save_menu.c RepairSaveSlot; the intro overlay's assembly; the library's two identical empty stubs *(read from the code)*
- **Memory:** The game stays inside the base 4 MB: its two 320 by 240 16-bit frame buffers sit at 0x803B5000 and 0x803DA800, the second ending exactly at 0x80400000; every fixed load the ROM map records ends below that (the highest, Inventory_Init's palette block, at 0x803AC860); no instruction in its code forms an address at or above 0x80400000; and it never asks for the memory size (no osGetMemSize call). *Evidence:* src/game/main.c (gFramebuffer0 and gFramebuffer1, the frame setup); the fixed loads above; the inventory's measures *(read from the code and measured)*

## What this inventory covers, and what it cannot see

The behaviours above are tied to the inventory: each lists the calls and references it requires, the calls it excludes and the measures it relies on, and the checks below test those; their other details (priorities, flag words, rates, sizes) are read from the code at the evidence given.

Covered: every direct call (jal) from the game's code (main's C units, the overlays' C units and the code blocks kept as assembly) into the library, checked against the ROM's own words; the library symbols the game's code and data take the address of; game functions whose address is taken where a callback-taking library function is called; the fixed load addresses and the microcode, from the ROM map.

Not covered, or only counted:

- **Indirect calls** (through a function pointer): 12 sites in 8 game functions; where they lead is not resolved here.
- **Callbacks the library makes into the game** (thread entries, the audio DMA routine, message-driven events) beyond the addresses listed above, and anything the game registers through a table of pointers.
- **Direct hardware or CPU access from the game's code**: 0 instructions forming an uncached or device address, or using coprocessor 0 or cache operations.
- **Addresses above the base 4 MB**: 0 instructions in the game's code form an address in 0x80400000-0x807FFFFF (where an Expansion Pak's memory would be); addresses passed in from data or computed at run time are not seen.
- **What the microcode does with the display lists and audio command lists** the game builds (the graphics commands themselves are not inventoried here).
- **Data-format assumptions** in the ROM's assets (big-endian words, 32-bit pointers and segment addresses inside loaded blocks): [ROM_MAP.md](ROM_MAP.md) describes the formats; not part of this inventory.
- **Timing**: frame rate, retrace pacing and DMA latency beyond what the calls and assumptions above state.
- **Inside the library**: how libultra itself drives the hardware; the inventory stops at the boundary.

## The library's functions

Each library function was identified with every basis found for it; the project's identification table, which records them all, is not part of this repository, and the probable and unresolved functions are listed below with their reasons. The grades:

- verified (284): libultra's own source compiles to it (decompals/ultralib built per release with the decomp's IDO 5.3), a verified caller calls or registers it under that name, Quest 64's twin is built from source by a matching decomp and a second lineage agrees, or the code was read against the source;
- probable (24): one weaker basis (a name for identical code that only use decides, a label, earlier naming);
- unresolved (21): no identity is established; each row keeps what is established (for 15 of them, the library source file their code comes from) and the candidates.

The decomp carries every verified name; probable and unresolved functions keep their placeholder or their earlier name.

**Which libultra.** No single release ultralib reconstructs compiles to the whole library. Only 2.0J-or-later source compiles to osCreateViManager, osAiSetFrequency, __osMotorAccess, __osMakeMotorData, osMotorInit, osPfsFreeBlocks, osPfsAllocateFile, __osPfsDeclearPage, osPfsDeleteFile, __osPfsReleasePages, osPfsRepairId, __osViSwapContext, __osPfsGetStatus, __osPfsRequestOneChannel, __osPfsSelectBank, __osContRamRead, __osSumcalc, __osCheckPackId, __osGetId, __osCheckId, __osPfsRWInode, osPfsChecker, __osSiRawStartDma, __osContAddressCrc, __osContDataCrc, __osContRamWrite; only 2.0I-or-earlier source to osSetEventMesg, osAiSetNextBuffer, osEPiRawStartDma, alSynStartVoiceParams, alSynSetFXMix, corrupted_init, osEPiRawWriteIo, osEPiRawReadIo. Compiled at -O1, -O2 and -O3 in different parts.

Probable and unresolved functions, each with the reason its row records (where a decision settled it, the decision's). The letters: U, ultralib compiles to it; R, a verified caller names it; Q, Quest 64's twin; E, evidence recorded earlier; M, a recorded decision.

- 0x80036AC0 osInitialize: probable. E: named from the Quest decomp's name of its modified twin, checked against the code; no code basis found here. Outside names, unconfirmed: __osInitialize_common.
- 0x80037020 osCreatePiManager: probable. The decomp's earlier name; no code basis found here. Outside names, unconfirmed: osCreatePiManager.
- 0x8003719C InitPiHandles: probable. The decomp's earlier name; no code basis found here.
- 0x800376C0 osWritebackDCacheAll: probable. Q: masked-identical to Quest 0x80034350, named by the Quest decomp's symbol files only. Outside names, unconfirmed: osWritebackDCacheAll.
- 0x80039950 osInvalICache: probable. Q: masked-identical to Quest 0x800363E0, named by the Quest decomp's symbol files only. Outside names, unconfirmed: osInvalICache.
- 0x80039E40 alCSPSetVol: probable. M: the code is identical to libultra's alSeqpSetVol (Quest's twin is built from seqpsetvol.c): the CSP and Seqp setters compile alike. The decomp's name comes from its use: the game passes it the compressed-sequence player that alCSPNew sets up. Candidates from the code: alCSPSetVol, alSeqpSetVol.
- 0x80039E80 func_80039E80: unresolved. M: an empty function (it only returns) at the start of ultralib's synthesizer.o built at -O3, an object that matches the ROM there as a whole; the outside candidates (osAckRamromRead, osAckRamromWrite, __osInitialize_autodetect) are debug stubs of other objects and are ruled out; probably what -O3 leaves of an inlined static.
- 0x80039FC0 func_80039FC0: unresolved. M: an empty function 320 bytes into ultralib's synthesizer.o built at -O3 (the object matches the ROM as a whole); the outside candidates are ruled out as at 0x80039E80; probably what -O3 leaves of an inlined static.
- 0x8003A620 alCSPGetVol: probable. M: the code is identical to n_alSeqpGetVol (Quest's twin is built from n_alseqpgetvol.c); the decomp's name comes from its use with the compressed-sequence player. Candidates from the code: alCSPGetVol, alSeqpGetVol, n_alSeqpGetVol.
- 0x8003A6A0 func_8003A6A0: unresolved. Provenance established: the code equals, word for word outside the relocated fields, code at an offset no symbol marks in ultralib's compiled audio/csplayer.c (builds H,H-O3,I,I-O3,J-O3,K-O3,L-O3): it comes from that file, probably as a static function (IDO gives those no symbol); identity not established: no basis names it, and no compiled function that carries a symbol matches it in any build (those with the static keyword removed included).
- 0x8003A6DC func_8003A6DC: unresolved. Provenance established: the code equals, word for word outside the relocated fields, code at an offset no symbol marks in ultralib's compiled audio/csplayer.c (builds H,H-O3,I,I-O3,J-O3,K-O3,L-O3): it comes from that file, probably as a static function (IDO gives those no symbol); identity not established: no basis names it, and no compiled function that carries a symbol matches it in any build (those with the static keyword removed included).
- 0x8003A8AC func_8003A8AC: unresolved. Provenance established: the code equals, word for word outside the relocated fields, code at an offset no symbol marks in ultralib's compiled audio/csplayer.c (builds H,H-O3,I,I-O3,J-O3,K-O3,L-O3): it comes from that file, probably as a static function (IDO gives those no symbol); identity not established: no basis names it, and no compiled function that carries a symbol matches it in any build (those with the static keyword removed included).
- 0x8003B860 func_8003B860: unresolved. M: two empty functions under one label in the decomp (bnkf.o's first 16 bytes at -O3: two returns); the object matches the ROM as a whole; probably what -O3 leaves of two inlined statics.
- 0x8003B870 func_8003B870: unresolved. Provenance established: the code equals, word for word outside the relocated fields, code at an offset no symbol marks in ultralib's compiled audio/bnkf.c (builds H,H-O3,I,I-O3,J-O3,K-O3,L-O3): it comes from that file, probably as a static function (IDO gives those no symbol); identity not established: no basis names it, and no compiled function that carries a symbol matches it in any build (those with the static keyword removed included).
- 0x8003BAB0 alCSPSetBank: probable. M: the code is identical to alSeqpSetBank (Quest's twin is built from seqpsetbank.c); the decomp's name comes from its use with the compressed-sequence player. Candidates from the code: alCSPSetBank, alSeqpSetBank.
- 0x8003BC30 func_8003BC30: unresolved. Provenance established: the code equals, word for word outside the relocated fields, code at an offset no symbol marks in ultralib's compiled audio/cseq.c (builds H,H-O3,I,I-O3,J-O3,K-O3,L-O3): it comes from that file, probably as a static function (IDO gives those no symbol); identity not established: no basis names it, and no compiled function that carries a symbol matches it in any build (those with the static keyword removed included). Outside names, unconfirmed: __getTrackByte.
- 0x8003BCF4 func_8003BCF4: unresolved. Provenance established: the code equals, word for word outside the relocated fields, code at an offset no symbol marks in ultralib's compiled audio/cseq.c (builds H,H-O3,I,I-O3,J-O3,K-O3,L-O3): it comes from that file, probably as a static function (IDO gives those no symbol); identity not established: no basis names it, and no compiled function that carries a symbol matches it in any build (those with the static keyword removed included). Outside names, unconfirmed: __readVarLen.
- 0x8003C5B0 alCSPSetSeq: probable. U: identical code compiles from alCSPSetSeq,alSeqpSetSeq (ultralib audio/cspsetseq.c,audio/seqpsetseq.c); the decomp's name, from its use. Candidates from the code: alCSPSetSeq, alSeqpSetSeq. Outside names, unconfirmed: alSeqpSetBank.
- 0x8003C5F0 alCSPPlay: probable. U: identical code compiles from alCSPPlay,alSeqpPlay (ultralib audio/cspplay.c,audio/seqpplay.c); the decomp's name, from its use. Candidates from the code: alCSPPlay, alSeqpPlay.
- 0x8003C620 alCSPSetChlFXMix: probable. U: identical code compiles from alCSPSetChlFXMix,alSeqpSetChlFXMix (ultralib audio/cspsetfxmix.c,audio/seqpsetfxmix.c); the decomp's name, from its use. Candidates from the code: alCSPSetChlFXMix, alSeqpSetChlFXMix.
- 0x8003C680 alCSPStop: probable. U: identical code compiles from alCSPStop,alSeqpStop (ultralib audio/cspstop.c,audio/seqpstop.c); the decomp's name, from its use. Candidates from the code: alCSPStop, alSeqpStop. Outside names, unconfirmed: alSeqpPlay.
- 0x8003C6B0 alCSPGetState: probable. M: the code is identical to alSeqpGetState (Quest's twin is built from seqpgetstate.c); the decomp's name comes from its use with the compressed-sequence player. Candidates from the code: alCSPGetState, alSeqpGetState. Outside names, unconfirmed: n_alSeqpGetState.
- 0x8003C6C0 func_8003C6C0: unresolved. Provenance established: the code equals, word for word outside the relocated fields, code at an offset no symbol marks in ultralib's compiled audio/sndplayer.c (builds H,H-O3,I,I-O3,J-O3,K-O3,L-O3): it comes from that file, probably as a static function (IDO gives those no symbol); identity not established: no basis names it, and no compiled function that carries a symbol matches it in any build (those with the static keyword removed included). Outside names, unconfirmed: _removeEvents.
- 0x8003D0F0 osPfsInitPak: probable. E: named from the Quest decomp's name of its new twin, checked against the code; no code basis found here. Outside names, unconfirmed: osPfsInitPak.
- 0x8003D8D0 func_8003D8D0: probable. M: the first of the two identical empty stubs: rmonPrintf by syncprintf.o's -O3 order (see 0x8003D8EC); nothing in the game calls it. Candidates from the code: osSyncPrintf, rmonPrintf.
- 0x8003D8EC osSyncPrintf: probable. M: one of two identical empty stubs (0x8003D8D0, 0x8003D8EC): code alone cannot tell them apart. libultra's syncprintf.c defines osSyncPrintf then rmonPrintf; built at -O3, as this library's C library is (memcpy, strlen and strchr, ldiv and lldiv sit in -O3 order, and _Printf compiles only at -O3), the object holds rmonPrintf first and osSyncPrintf second (checked by an independent build): this one is osSyncPrintf, the function the game calls with its debug format strings. Candidates from the code: osSyncPrintf, rmonPrintf.
- 0x8003DEA0 osPfsFileState: probable. E: named from the Quest decomp's name of its new twin, checked against the code; no code basis found here. Outside names, unconfirmed: osPfsFileState.
- 0x8003E9E4 osPfsReadWriteFile: probable. The decomp's earlier name; no code basis found here. Outside names, unconfirmed: osPfsReadWriteFile.
- 0x8003EF90 __osSetSR: probable. Q: masked-identical to Quest 0x8003C990, named by the Quest decomp's symbol files only. Outside names, unconfirmed: __osSetSR.
- 0x8003EFA0 __osGetSR: probable. Q: masked-identical to Quest 0x8003C9A0, named by the Quest decomp's symbol files only. Outside names, unconfirmed: __osGetSR.
- 0x8003EFB0 __osSetFpcCsr: probable. Q: masked-identical to Quest 0x8003C9B0, named by the Quest decomp's symbol files only. Outside names, unconfirmed: __osSetFpcCsr.
- 0x8003F060 __osExceptionPreamble: probable. Q: masked-identical to Quest 0x8003CA60, named by the Quest decomp's symbol files only. Outside names, unconfirmed: __osExceptionPreamble.
- 0x8003F070 __osException: probable. The decomp's earlier name; no code basis found here. Outside names, unconfirmed: __osException.
- 0x8003F594 func_8003F594: unresolved. No code basis; outside candidates only. Outside names, unconfirmed: send_mesg.
- 0x8003F960 osUnmapTLBAll: probable. The decomp's earlier name; no code basis found here.
- 0x80043E44 func_80043E44: unresolved. Provenance established: the code equals, word for word outside the relocated fields, code at an offset no symbol marks in ultralib's compiled audio/env.c (builds H,H-O3,I,I-O3,J-O3,K-O3,L-O3): it comes from that file, probably as a static function (IDO gives those no symbol); identity not established: no basis names it, and no compiled function that carries a symbol matches it in any build (those with the static keyword removed included). Outside names, unconfirmed: _getRate.
- 0x80044014 func_80044014: unresolved. Provenance established: the code equals, word for word outside the relocated fields, code at an offset no symbol marks in ultralib's compiled audio/env.c (builds H,H-O3,I,I-O3,J-O3,K-O3,L-O3): it comes from that file, probably as a static function (IDO gives those no symbol); identity not established: no basis names it, and no compiled function that carries a symbol matches it in any build (those with the static keyword removed included).
- 0x800442AC func_800442AC: unresolved. Provenance established: the code equals, word for word outside the relocated fields, code at an offset no symbol marks in ultralib's compiled audio/env.c (builds H,H-O3,I,I-O3,J-O3,K-O3,L-O3): it comes from that file, probably as a static function (IDO gives those no symbol); identity not established: no basis names it, and no compiled function that carries a symbol matches it in any build (those with the static keyword removed included). Outside names, unconfirmed: _getVol.
- 0x80044C38 func_80044C38: unresolved. M: an empty function 392 bytes into ultralib's seqplayer.o built at -O3 (the object matches the ROM as a whole); the outside candidates are ruled out as at 0x80039E80; probably what -O3 leaves of an inlined static.
- 0x8004500C func_8004500C: unresolved. Provenance established: the code equals, word for word outside the relocated fields, code at an offset no symbol marks in ultralib's compiled audio/seqplayer.c (builds H,H-O3,I,I-O3,J-O3,K-O3,L-O3): it comes from that file, probably as a static function (IDO gives those no symbol); identity not established: no basis names it, and no compiled function that carries a symbol matches it in any build (those with the static keyword removed included). Outside names, unconfirmed: __handleMetaMsg.
- 0x800452AC func_800452AC: unresolved. Provenance established: the code equals, word for word outside the relocated fields, code at an offset no symbol marks in ultralib's compiled audio/seqplayer.c (builds H,H-O3,I,I-O3,J-O3,K-O3,L-O3): it comes from that file, probably as a static function (IDO gives those no symbol); identity not established: no basis names it, and no compiled function that carries a symbol matches it in any build (those with the static keyword removed included). Outside names, unconfirmed: __handleMIDIMsg.
- 0x8004ADA0 func_8004ADA0: unresolved. M: an empty function 64 bytes into ultralib's seq.o built at -O3 (the object matches the ROM as a whole); the outside candidates are ruled out as at 0x80039E80; probably what -O3 leaves of an inlined static.
- 0x8004ADA8 func_8004ADA8: unresolved. Provenance established: the code equals, word for word outside the relocated fields, code at an offset no symbol marks in ultralib's compiled audio/seq.c (builds H,H-O3,I,I-O3,J-O3,K-O3,L-O3): it comes from that file, probably as a static function (IDO gives those no symbol); identity not established: no basis names it, and no compiled function that carries a symbol matches it in any build (those with the static keyword removed included). Outside names, unconfirmed: readVarLen.
- 0x8004B228 func_8004B228: unresolved. Provenance established: the code equals, word for word outside the relocated fields, code at an offset no symbol marks in ultralib's compiled audio/seq.c (builds H,H-O3,I,I-O3,J-O3,K-O3,L-O3): it comes from that file, probably as a static function (IDO gives those no symbol); identity not established: no basis names it, and no compiled function that carries a symbol matches it in any build (those with the static keyword removed included). Outside names, unconfirmed: read32.
- 0x8004B278 func_8004B278: unresolved. Provenance established: the code equals, word for word outside the relocated fields, code at an offset no symbol marks in ultralib's compiled audio/seq.c (builds H,H-O3,I,I-O3,J-O3,K-O3,L-O3): it comes from that file, probably as a static function (IDO gives those no symbol); identity not established: no basis names it, and no compiled function that carries a symbol matches it in any build (those with the static keyword removed included). Outside names, unconfirmed: read16.

## Checks

- OK identification: each of the 329 SDK functions identified and graded
- OK verified names in the decomp: every verified identity is the decomp's name
- OK calls into the SDK: 375 call sites from the relocations, 375 jal words in the ROM's game code, the same sites and targets
- OK jumps into the SDK: 0 plain jumps from the game's code into the SDK
- OK services: 90 SDK functions called, each in a service
- OK assumptions: 14 assumptions, every required call and reference found, every excluded call absent
