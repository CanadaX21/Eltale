# Credits

Everyone whose work the Eltale Monsters decomp builds on, by the handle they use in the Quest 64 / Eltale community, with the account name where it differs. The people credited here didn't take part in this AI-assisted project: the credit is for the work it builds on, not an endorsement. Anyone credited who would rather not be can ask CanadaX21, and the name comes out.

The matching in this project was done by Claude (Anthropic's AI model) through decomp.wtf. CanadaX21 set the rules, made the calls on what counts as a match, and reviewed the work.

## Upstream code

- **Mallos31 (Eltale_Librarian)**: Mallos31/EM, the Eltale Monsters decomp this project started from (splits, build, first matches); Mallos31/Quest, the Quest 64 decomp whose matched C was ported: 121 functions came over byte-identical; and EltaleLibrarian/Quest64-Decomp (data names, migrated Mallos31-era C).
- **Rain (Rainchus)**: Rainchus/Quest64-Decomp (libultra matching, data and symbol work), Quest64-Recomp, and the Eltale recomp setup Rainchus/EltaleMonsters-Decomp.

## Names

- 40 function names come from the Quest decomps' names for byte-identical twins: nnScCreateScheduler, nnScStartScheduler, nnScGetAudioMQ, nnScEventHandler, nnScAddClient, nnScRemoveClient, nnScEventBroadcast, nnScExecuteAudio, nnScExecuteGraphics, nnScWaitTaskReady, getRandomNumber, calc_arctan_in_radians, rotateCoordinatesByAngle, dma_write, count_digits, Init_MISS_Icon, Setup_MISS_Icon, Draw_MISS_Icon, Inventory_Init, CheckIfInventoryFull, AddItemToInventory, CheckIfItemUsable, UseItem, NoEffectItem, CheckIfHealingNeeded, CheckIfTeleportPossible, CheckIfSpellItemUsable, UseHealingItem, UseWings, UseSpellItem, DrawItemBox, InitializeSoundData, UpdateBGMDelay, FindMapBGM, SetBGMVolume, string_copy, loadTextTextureAndPal, LoadOvl_TitleScreen, LoadOvl_Ending, osViSetEvent.
- Data names from Landmine36 (1): gBattleStone.
- Data names from Mallos31 (Eltale_Librarian) (34): __osException, __osCleanupThread, gRespawnLocationTbl, gZeroDigitText, __osCurrentHandle, __osThreadTail, __osActiveQueue, vi, __osTimerList, __osRcpImTable, gDayLength, gMISSData, gVisibleInvItemIDs, gItemLightRingSize, __osEventStateTab, piAccessBuf, __osCurrentTime, __osBaseCounter, __osViIntrCount, __osTimerCounter, gBossVAddr, gHudTexture, gStatusSymbolsPalette, gStatusSymbolsImage, gWingData, gItemData, gItemNamePtrs, gItemDescriptionPtrs, gBaseItemLightRingData, gInventoryPalette, aspMainTextStart, __osPfsPifRam, gGfxFifoBuffer, osTvType.
- Data names from Rain (Rainchus) (8): gRespawnLocationTbl, gZeroDigitText, __osActiveQueue, __osRcpImTable, gDayLength, gBossVAddr, aspMainTextStart, gGfxFifoBuffer.
- Data names from schooliedee (vbhayden) (2): gBattleIntroStep, gBattleIntroTimer.
- Data names from the Quest 64 decomps (Mallos31/Quest, Rainchus/Quest64-Decomp, EltaleLibrarian/Quest64-Decomp) (17): gGameState, gEnemies, gInventory, gLastAutoHitEnemy, gBattleState, gGroundSample, gSpellCameraTarget, gBattleCameraYawOffset, gRunFootstepTimer, gWallNavmeshActors, gWallNavmeshCamera, gCollisionRadius, gMonsterFileBuffer, gDefaultCamera, gRandomSeed, gProjAnims, gMapModelData.

## People, by handle

What each person's work gave the decomp: their decomp.me scratches, repositories and names. Their findings were used as leads and checked against Eltale's code.

### schooliedee (vbhayden)

- evidence (1): question #2 (pads): their Quest matches of DrawElementsPage's and UpdateStatusVisuals' twins carry the same unused locals (decomp.me onYlT; StqHE)
- functions whose notes credit them (33): IsInEncounterCircle; BrianState_Stand; UpdatePuffTimers; AllocProjectileSlot; CreateEncounter; UpdateEnvironmentObjects; StartScriptedBattle; StartSpellVisual; PollWorldCollision; CameraMode_Battle; SelectEnemyAction; SnapSpellCamera; CameraMode_Fixed; UpdateSpells; BrianStartCast; CommitEnemyAIOption; OnBossDefeated; TickDustEmitter; ResetSpells; CameraTurnToAngle; UpdateLavaPalette; UpdateDustEmitter; BrianBattleMovement; DrawElementsPage; SpawnBattleRing ...
- lead (2): LoadMap: the scratch's commented s4 fake confirms our wall (one more basic block) (decomp.me DeWpQ (Quest func_8000BD10)); LoadMap: the scratch's commented fake located the wall: placed must be live from entry (decomp.me DeWpQ (Quest func_8000BD10))
- match (1): LoadMap: the flat region table, the stepped block pointer and the reading that the counter must be live early, which the reused local satisfies (decomp.me DeWpQ (Quest func_8000BD10))
- name (1): Camera members (cam_x / cam_y / cam_z, focus_x / focus_y / focus_z): the member names (the camera struct's comments in the Quest 64 decomp.me scratches)
- names (1): Camera, EnemyAIOption and effect member names: the names kept and extended when three camera views became Camera and the enemy and effect records were named (Quest 64 scratches, as the header comments record)
- progress (1): LoadMap: the flat region table, the stepped block pointer, and the fake that showed what the last instruction needs (decomp.me DeWpQ (Quest func_8000BD10))

### Mallos31 (Eltale_Librarian)

- evidence (1): question #2 (pads): their Quest matches of DrawElementsPage's and UpdateStatusVisuals' twins carry the same unused locals (decomp.me onYlT; StqHE)
- functions whose notes credit them (9): ItemMenu; WorldToScreen; QueueSound; PushSoundQueue; DrawString; ResetSpells; UpdateStatusVisuals; DrawEnvironmentPage; LoadMapBoss
- identification (1): osContStartQuery / osContGetQuery (func_8003EEE0 / func_8003EF64): the comments at their placeholder declarations (the stubs call them without their argument, so no rename) (Mallos31/Quest symbol names (identical twins))
- lead (1): ItemMenu: read against our draft (ours ahead) (decomp.me hQUaF (Quest func_80021524))
- match (3): WorldToScreen: the starting forms that the target was read against; the match drops their flag (decomp.me cFdEN; TPP0T (Quest func_8002413C)); TitleScreen_UpdateMenu (title overlay): the structure: the button tests as one if-else chain over the menu states (our names and types) (Mallos31/Quest src/ovl_overlay1/DA68F0.c (Quest func_801006D8_DA6FC8)); TitleScreen_DrawFade (title overlay): the form: the switch with its default before the black kinds; the frame count unsigned (our names) (Mallos31/Quest src/ovl_overlay1/DA68F0.c (Quest func_80102F14_DA9804; its source credits Alex with the match))
- name (3): osTvType: the SDK name for 0x80000300 (the Quest decomps' data names (Quest osTvType)); rspbootTextStart: the RSP boot code's name (the Quest decomps' data names; EltaleMonsters-Decomp/symbol_addrs.txt); gspF3DEXTextStart / rspbootTextEnd: ours follows the ROM's microcode string (F3DEX 1.23); the boot code ends at the same address, so that name stays for the size (Quest gspFast3DTextStart at the same address)
- names (3): Audio SDK names (Quest twins): alAudioFrame, alSndpSetPitch / Vol / Pan agree; their alSeqpSetVol / n_alSeqpGetVol kept as the community spelling beside ours (alCSPSetVol / GetVol: this project's compact player family and its 2.0I headers) (Mallos31/Quest symbol names); gGfxFifoBuffer (0x8031C5A0): their gThreadOutStack recorded; ours kept: InitFrameTask sets it as the graphics task's output_buff (the RDP output FIFO, not a stack) (EltaleLibrarian/Quest64-Decomp, Rainchus/Quest64-Decomp (Quest 0x8031B2A0)); main's library functions: Quest 64 names as a second lineage: the second lineage for identities from Quest 64's twins (Mallos31/Quest (names on Quest 64's functions, through the Quest-Eltale function map))
- progress (2): WorldToScreen: the first visibility test reads the global, the second the near local (decomp.me cFdEN; TPP0T (Quest func_8002413C)); DrawString: the loop over *str, idx before the switch, the metrics pointer in the glyph branch (decomp.me fLEOq (Quest func_80030EA0))

### Rain (Rainchus)

- functions whose notes credit them (10): CommitEnemyAIOption; DrawModelPart; FindNoteExtension; ScanSaveList; SaveToPak; PushSoundQueue; SetupMapSpirits; BrianSpellInput; SpiritGainWindow; DrawScreenFade
- identification (2): Controller pak routines (0x8003D554, 0x8003D6B0, 0x8003ED60, 0x8003EDC0): osMotorInit, osGbpakInit, osPfsRepairId, osPfsNumFiles recorded as the next naming step (not applied yet: stubs call some of them without arguments, which the SDK prototypes refuse) (Rainchus/EltaleMonsters-Decomp symbol_addrs.txt); main's library functions: Quest 64 twins built from source: Quest 64's twins built from C, one basis of the identification; its file order and compile rules (Rainchus/Quest64-Decomp d964ce0 (a matching decompilation of Quest 64 USA))
- lead (1): SaveToPak: a near oracle: same 201 instructions and registers (decomp.me hrLAI (Quest func_8002D8BC))
- match (1): SaveToPak: the form: the cursor read at every use, the for with no step, the empty else (decomp.me hrLAI (Quest func_8002D8BC))
- name (2): rspbootTextStart: the RSP boot code's name (the Quest decomps' data names; EltaleMonsters-Decomp/symbol_addrs.txt); gspF3DEXTextStart / rspbootTextEnd: ours follows the ROM's microcode string (F3DEX 1.23); the boot code ends at the same address, so that name stays for the size (Quest gspFast3DTextStart at the same address)
- names (4): Audio SDK names (alAudioFrame, osAiGetLength, osAiSetNextBuffer, alSndpSetPan / Vol / Pitch / FXMix): the same names for the same seven functions: an independent confirmation of ours (the sndplayer event order reading); their n_alSeqpGetVol / alSeqpSetVol for 0x8003A620 / 0x80039E40 recorded (Rainchus/EltaleMonsters-Decomp symbol_addrs.txt (Eltale addresses)); gGfxFifoBuffer (0x8031C5A0): their gThreadOutStack recorded; ours kept: InitFrameTask sets it as the graphics task's output_buff (the RDP output FIFO, not a stack) (EltaleLibrarian/Quest64-Decomp, Rainchus/Quest64-Decomp (Quest 0x8031B2A0)); MODE_GAMEPLAY .. MODE_CREDITS: the game mode names; MODE_ENDING and MODE_AFTER_CREDITS are ours (Quest64-Decomp (gGameMode values)); main's library functions: candidate names: candidates checked against the code, never taken as proof (Rainchus/EltaleMonsters-Decomp symbol_addrs.txt)
- progress (2): BrianSpellInput: x-- vs x -= 1 compile differently: the cancel branch decrement; SpiritGainWindow: x-- vs x -= 1: the glow countdown

### LLONSIT (LLONSIT-glitch)

- functions whose notes credit them (2): WorldToScreen; SpellEffect_DrainMP
- match (2): SpellEffect_DrainMP: the brian pointer local that supplies the missing -g3 home (decomp.me vS0oh (Quest func_8001679C)); WorldToScreen: the starting forms that the target was read against; the match drops their flag (decomp.me cFdEN; TPP0T (Quest func_8002413C))
- progress (1): WorldToScreen: the first visibility test reads the global, the second the near local (decomp.me cFdEN; TPP0T (Quest func_8002413C))

### Landmine36

- name (1): MonsterBaseData stone (+0x14): the member name (with the battle tally in EnemyKnockback)

### zeldaret (the Ocarina of Time decompilation, zeldaret/oot)

- name (1): STACK_TOP / ARRAY_COUNT (common.h): the macro names and the idiom: a stack array sized at its definition, its top passed to the thread (zeldaret/oot (STACK_TOP and ARRAY_COUNT in its headers; its boot code passes STACK_TOP(stack) to osCreateThread))

## The community

Thanks also to the wider Quest 64 / Eltale community, whose years of shared research this builds on.

## Tools

- splat (Ethan Roseman / ethteck, MIT; github.com/ethteck/splat): the ROM splitter. asm-processor (Simon Lindholm, public domain): GLOBAL_ASM inside C. decomp-permuter (Simon Lindholm and contributors, MIT): the permutation search. m2c (Matt Kempster, GPL-3.0; github.com/matt-kempster/m2c): the machine drafts. objdiff (encounter; objdiff-cli 3.8.2 inside decomp.wtf): object diffing. decomp.me: the shared scratch site the community uses. (Licences checked against each project's repository and package metadata.)
- decomp.wtf (github.com/i12bp8/decomp.wtf, 0.10.6): the local matching service the matching was done in (judge, notes, drafts, the m2c seeds).
- IDO 5.3 (Silicon Graphics) and the Nintendo 64 SDK (libultra 2.0I headers): the original toolchain.
- ultralib (decompals and contributors; github.com/decompals/ultralib): the reconstruction of libultra's source, built locally per release with IDO 5.3 as the reference that identified most of the library's functions in main and the release it is closest to (docs/PLATFORM.md). Nothing from it is in this repository.
- The decompilation skills (github.com/lemiur/Game-Decompilation-Skill-For-Claude).
