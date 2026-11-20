#ifndef _VARIABLES_H_
#define _VARIABLES_H_

#include "structs.h"

/* The variables the game's C uses whose storage no C unit defines yet: assembly, binary pieces of the main
   image, link names (undefined_syms.jp.txt) and fixed addresses, each under a heading for where it lives.
   A variable a C unit defines is declared in that unit's header instead (docs/DECLARATIONS.md). */

/* Segmented addresses: display lists, vertices and matrices the code names by their RSP segment (1 and 2),
 * not by a RAM address. The segments' contents are assembly or binary. */
extern Gfx D_1000010[];   /* segment address 0x1000010 */
extern Gfx D_10000A0[];   /* Brian's render setup */
extern Gfx D_10000E8[];   /* segment address 0x10000E8 */
extern Vtx D_1000230[];   /* segment 1: the shadow quad */
extern Gfx D_1000270[];   /* segment 1: the shadows' render setup */
extern Gfx D_1000388[];   /* segment address 0x1000388 */
extern Mtx D_2000000[];   /* segment 2: this frame's matrices */
extern Mtx D_2000040;   /* segment 2: the HUD's orthographic projection */
extern Mtx D_2000080;   /* segment address 0x2000080 */
extern Light D_2008A40;   /* segment 2: FrameDynamic.lookat, its x and y halves */
extern Light D_2008A50;   /* segment address 0x2008A50 */

/* Fixed RAM addresses: buffers the game places by address (link names). */
extern u8 gOverlaySlot[];   /* the overlay slot (link name: title_overlay_VRAM): the title, intro and epilogue overlays and the NPC areas' held objects load there */
extern u8 D_80100000[];   /* 0x80100000 */
extern AnimEntry gMapObjectAnims[];   /* their animations */
extern u8 D_8010FE00[];   /* 0x8010FE00 */
extern u8 gEventBlob[];   /* 0x801C0000 */
extern u8 D_801E0000[];   /* 0x801E0000 */
extern u8 gMonsterFileBuffer[];   // the area's loaded file: the monster set's in a monster area (LoadMonsterSet), the NPC blob in an NPC area (LoadNPCArea) (Quest gMonsterFileBuffer)
extern u8 gShannonActorData[];   /* 0x802A0000 */
extern void* gBossVAddr;   /* 0x802A7BC0 */
extern u16 gDepthBuffer[];   /* the depth buffer */
extern u64 gRspDramStack[];   /* the RSP's DRAM stack */
extern u64 gRspYieldBuffer[];   /* its yield buffer */
extern GfxFrame gFrames[2];   /* the two frames' graphics tasks */
extern u8 gGfxFifoBuffer[];   /* 0x8031C5A0 */
extern FontStrip gFontSheet[];   /* 0x803245A0 */
extern u16 gTextPalettes[];   /* four palettes of 16 RGBA5551 colours */
extern u8 gAudioHeapMemory[];   /* the audio heap's memory (0x68000 bytes) */
extern u16 gInventoryPalette[];   /* 0x803AC460 */
extern u16 gFramebuffer0[320 * 240];   /* framebuffer 0 */
extern u16 gFramebuffer1[320 * 240];   /* framebuffer 1 */

/* Main's data not yet in C: tables in the binary pieces of main's image, named by link names
 * (`name = _<rom>_bin + offset`) or by the pieces' own labels. */
extern s32 gGameState;   /* game state flags: GAME_STATE_ (below) */

/* gGameState's bits, all of them here: the mode loops (src/game/main.c) and the field run on them
   (docs/GAME_FLOW.md); tools/save_format.py reads GAME_STATE_CLEARED here. */
#define GAME_STATE_ITEM_MENU      0x001  /* the item menu is open (R; ItemMenu closes it) */
#define GAME_STATE_MESSAGE        0x002  /* a message window holds play (OpenMessageWindow) */
#define GAME_STATE_SPELL_INPUT    0x004  /* a spell's directions are being entered (BrianSpellInput) */
#define GAME_STATE_SPIRIT_WINDOW  0x008  /* the spirit window: a spirit to give (SpiritGainWindow ends it) */
#define GAME_STATE_INN_CHOICE    0x010  /* an inn's Yes / No is up (StartNPCTalk, NPC_TALK_INN): BrianState_TalkEnd clears
                                           it and, on Yes, starts the rest and the save */
#define GAME_STATE_FADE_IN       0x020  /* fade the screen in once the mode loop has loaded its next place (FadeScreenIn,
                                           by gTransitionStyle) */
#define GAME_STATE_LOAD_NEXT     0x040  /* the mode loop loads its next place at its top: FieldLoop the map and submap
                                           gNextMap / gNextSubmap, EndingLoop its next scene, StaffCreditsLoop its scene */
#define GAME_STATE_MAP_CHANGE     0x080  /* a countdown runs (gModeCountdown). At its end FieldLoop leaves for the next
                                            place (a door, a warp, a defeat, the last boss, the restart) or, after the inn's
                                            rest, for the save pages; the ending's and the staff credits' loops load their next scene */
#define GAME_STATE_MAP_MUSIC     0x100  /* the next field start picks the map's music (FindMapBGM) */
#define GAME_STATE_STOP_MUSIC    0x200  /* the mode loop stops the music and waits for it as it ends (set by
                                           EndSequenceScene after the last scene) */
#define GAME_STATE_SAVE_MENU     0x400  /* a visit to the save menu: set by the inn's Yes (with GAME_STATE_MAP_CHANGE: the
                                           rest restores HP and MP, advances the time of day, sets the respawn place,
                                           then the save mode opens) and by the title's Continue and Delete; MenuLoop
                                           plays the save music while it is set */
#define GAME_STATE_RESPAWN        0x800  /* the next field start puts Brian at his save point (a defeat) */
#define GAME_STATE_UNSET_1000   0x1000  /* no code sets it: while set, Start opens no menu (FieldLoop) and Brian does not
                                           move (BrianFieldMovement) */
#define GAME_STATE_UNSET_2000   0x2000  /* no code sets it: while set, Brian does not move (BrianFieldMovement) */
#define GAME_STATE_UNSET_4000   0x4000  /* no code sets it: while set, the HUD is not drawn (BuildFrame) and Brian finds no
                                           one to talk to (FindTalkTarget) */
#define GAME_STATE_RESTART      0x8000  /* mainproc leaves its mode loop and sets a new game up again (InitNewGame, then
                                           the splash screen): EndSequenceScene after the last scene; a NPC_TALK_BOSS_FLAG
                                           NPC whose flag is LAST_BOSS would too (no record has that) */
#define GAME_STATE_HIDE_BRIAN  0x10000  /* Brian is not drawn (BuildFrame): the ending's scenes without him and the
                                           staff credits' scenes set it, SpawnBrian clears it */
#define GAME_STATE_CLEARED     0x20000  /* the game was finished: saved (SaveSlot.cleared); a load, the save list's scan
                                           and the end of the staff credits set it, InitNewGame keeps it */
extern s32 gDisplayListLength;   /* this frame's display list length */
extern u8 gSaveChoiceYes[];   /* link name: _4E910_bin + 0x0 */
extern u8 gSaveChoiceNo[];   /* link name: _4E910_bin + 0x8 */
extern u16 gBossHpBonus[];   /* link name: _4E940_bin + 0x10 */
extern u16 gBossMpBonus[];   /* link name: _4E940_bin + 0x20 */
extern u16 gElementPowerOrder[];   /* per element: the opposite element (counts /16), then the other two (/8 each) */
extern u32 gAccuracyResults[];   /* link name: _4F940_bin + 0x64 */
extern u8 gStatusChances[];   /* each status slot's chance, out of 32 for Brian (28 with 0x8000) or 64 for enemies */
extern u32 gStatusDurationOther[];   /* each status slot's duration range on another target: rand(3) + 2 */
extern u32 gStatusDurationSelf[];   /* on the caster itself: rand(5) + 3 */
extern Gfx gDustDefaultGfx[];   /* link name: _4FC60_bin + 0x0 */
extern u8 gTexMaskSides[7];   /* texture side lengths 2, 4 .. 128: a side of up to gTexMaskSides[i] texels gets mask i + 1 */
extern Vec3f gBattleCameraMapAdjust[];   /* per map: battle camera height / distance adjustments */
extern f32 gBossBattleRadii[];   /* link name: _4FC60_bin + 0x23c */
extern u8 gUnlearnedSpellName[];   /* the placeholder for a spell not learned yet */
extern u8 gElementColours[];   /* link name: _4FC60_bin + 0x26c */
extern u16 gStatusIconBits[];   /* link name: _4FC60_bin + 0x27c */
extern u16 gStatusFxBits[];   /* link name: _4FC60_bin + 0x28c */
extern s32 gSpiritGlowBits[4];   /* link name: _4FC60_bin + 0x294 */
extern f32 gSpellCrossFlashPos[];   /* per element: an x, y pair, read flat */
extern f32 gSpiritGainFlashPos[];   /* per element: an x, y pair, read flat */
extern s32 gSpiritMaxBits[];   /* per element: a flag for the player flags (SpiritGainWindow) */
extern u8 gSpellLabelColours[];   /* label colour by availability */
extern u16 D_8004F4C0;   /* link name: _500C0_bin + 0x0 */
extern Gfx gScreenFadeGfx[];   /* link name: _500C0_bin + 0x50 */
extern Gfx gScreenFadeSetupGfx[];   /* link name: _50140_bin + 0x10 */
extern u8 gFadeRampBlack16[16][4];   /* screen fade settings (StartScreenFade) */
extern u8 D_8004F5B4[];   /* link name: _50140_bin + 0x74 */
extern u8 gFadeRampWhite16[16][4];   /* link name: _50140_bin + 0xa0 */
extern u8 gFadeRampBlack32[32][4];   /* link name: _50140_bin + 0xe0 */
extern u8 gFadeRampWhite32[32][4];   /* link name: _50140_bin + 0x180 */
extern u8 gCritFade[];   /* the critical hit cue */
extern s8 D_80061660[8];   /* the character delay for each speed code, by its low three bits */
extern s8 gMessageSpeed;   /* the message speed (the page shows 4 - it) */
extern Gfx gMenuRenderDL[];   /* link name: _622F0_bin + 0x80 */
extern u8 gSaveNoteName[];   /* the save note's name */
extern u16 gQuillPalette[];   /* link name: _622F0_bin + 0x160 */
extern u16 gMenuSheetPalette[];   /* the menu sheet's palette (256 RGBA16) */
extern u8 gMenuSheetPixels[];   /* the menu sprite sheet's pixels (CI8), in main's data after this file's */
extern u16 gBackgroundPalette[];   /* the background's palette */
extern u8 D_80067090[];   /* link name: _622F0_bin + 0x59a0 */
extern u8 D_80067898[];   /* link name: _622F0_bin + 0x61a8 */
extern u8 D_800680A0[];   /* link name: _622F0_bin + 0x69b0 */
extern u8 D_800688A8[];   /* link name: _622F0_bin + 0x71b8 */
extern u16 D_80068EB0[];   /* link name: _622F0_bin + 0x77c0 */
extern u8 D_800690B8[];   /* link name: _622F0_bin + 0x79c8 */
extern u8 D_800695A8[];   /* link name: _622F0_bin + 0x7eb8 */
extern u16 D_80069A98[];   /* link name: _622F0_bin + 0x83a8 */
extern u8 D_80069CA0[];   /* link name: _622F0_bin + 0x85b0 */
extern u8 D_8006A2C8[];   /* link name: _622F0_bin + 0x8bd8 */
extern u16 D_8006A8F0[];   /* link name: _622F0_bin + 0x9200 */
extern u8 D_8006AAF8[];   /* link name: _622F0_bin + 0x9408 */
extern u8 D_8006AFE8[];   /* link name: _622F0_bin + 0x98f8 */
extern u16 D_8006B4D8[];   /* link name: _622F0_bin + 0x9de8 */
extern u8 D_8006B6E0[];   /* link name: _622F0_bin + 0x9ff0 */
extern u8 D_8006BBD0[];   /* link name: _622F0_bin + 0xa4e0 */
extern s16 gYesNoHandPositions[][2];   /* the Yes / No hand positions: [0..1] one box, [2..3] the other */
extern s8 D_8006CC80;   /* main's: set as the splash and intro end; the screen clears it on its first frame */
extern Gfx gCreditImageGfx[];   /* render state for the images */
extern u64 gspF3DEXTextStart[];   /* the graphics microcode (its data says F3DEX 1.23): text at the address where the boot code ends */

/* The cutscene bank (resident from boot, 0x8039ADB0): its records the C uses, still binary, named by address. */
extern u8 gNumberPalette[];   /* 0x8039ADB0 */
extern u8 gHudTexture[];   /* 0x8039AFB0 */
extern u16 gDigitSheet[];   /* the digit sheet: 128 x 10, RGBA16 */
extern u8 gInfoWindowPalette[];   /* 0x8039EC90 */
extern u8 gStatusSymbolsPalette[];   /* 0x803A4118 */
extern HudImage gHudImage;   /* the HUD image (compass, HP/MP frame) */
extern HudImage gInfoWindowImage;   /* 0x803A8718 */
extern HudImage gStatusSymbolsImage;   /* 0x803A8728 */
extern HudImage gHudFontImage;   /* 0x803A8748 */
extern u16* gHudFontPalettes[];   /* 0x803A8778 */
extern Gfx gHudRenderSetup[];   /* HUD render setup */
extern Gfx gSpiritFlashGfx[];   /* the flash */
extern Gfx gSpiritGlowGfx[];   /* the glow */
extern Gfx gTalkMarkerIcon[];   /* the talk marker icon (64 x 29; UpdateTalkMarker, DrawSignMarker) */
extern Gfx gObjectMarkerIcon[];   /* the icon (16 x 30) */
extern Gfx gSpiritMarkerIcon[];   /* the icon (24 x 30) */
extern Gfx gEnemyMarkerIcon[];   /* the enemy marker icon (24 x 42) */
extern EffectSpawnDef gStatusFxModels[];   /* 0x803AA690 */
extern StatusFxDef gStatusFxDefs[];   /* 0x803AA6C4 */
extern Gfx gCompassDialGfx[];   /* compass dial display list */
extern Gfx D_803AA7C0[];   /* 0x803AA7C0 */
extern Gfx D_803AA828[];   /* 0x803AA828 */
extern Gfx gItemBoxGfx[];   /* 0x803AA8A8 */
extern Gfx D_803AA940[];   /* 0x803AA940 */
extern Gfx D_803AA998[];   /* 0x803AA998 */
extern Gfx D_803AA9F0[];   /* 0x803AA9F0 */
extern WingDest gWingData[];   /* 0x803AAA40: the Wings' destinations, by the item's itemArg1 (UseWings) */
extern ItemData gItemData[];   /* 0x803AAAA0 */
extern u8* gItemNamePtrs[];   /* 0x803AB288 */
extern u8* gItemDescriptionPtrs[];   /* 0x803AB2F0 */
extern EffectSpawnDef gItemEffectModels[];   /* 0x803AB358 */
extern BaseItemLightRingData gBaseItemLightRingData;   /* 0x803AB36C */
extern f32 gBossTalkRadii[];   /* per boss placement, how near Brian must be for its scene; gBossLoaded is the placement + 1 */
extern CameraValues gBossCameraSetups[][2];   /* per boss: two camera setups */
extern MonsterBaseData gBossBaseData[];   /* the bosses' base data */
extern MonsterBaseData D_803AB9A8;   /* Mammon's base data (boss 7) */
extern MonsterBaseData gMammonBaseData;   /* Mammon's base data again (= D_803AB9A8): the second test names it so */
extern u8 gLastBossDefeatText[];   /* 0x803AC328 */
extern u8* gBossNames[];   /* the bosses' names, by monsterID */
extern u8* gBossSceneNames[];   /* boss scene names (0x20 bytes each) */
extern u8* gBossSceneTexts[];   /* boss scene texts (MESSAGE_TEXT_SIZE bytes each) */

/* Other link names: a segment's address under the name the code uses. */
extern u8 D_D37700[];   /* link name: dialogue_bank_ROM_START + (gDialogueSigns - dialogue_bank_VRAM): the signs' strings (LoadSignText) */
extern u8 gSegment1Gfx[];   /* link name: ALIGN (main_BSS_END, 0x10 */
extern u8 gEffectsBlock[];   /* link name: spell_bank_VRAM */
extern u16 gLavaPalette[];   /* a palette map 0x1D loads, rotated here */
extern u64 gspF3DEXDataStart[];   /* link name: main_VRAM + (main_tail_ROM_START - main_ROM_START)) + 0x910 */

/* Storage defined by assembly. */
extern void (*gEnemyActionHandlers[])(MonsterBattleData*, EnemyAction*, AnimEntry*);   // enemy action handlers (Quest D_8004C290)
extern s32 gElementMasks[];   /* element bits per element */
extern void (*gApplyStatusHandlers[])(u8, MonsterBattleData*);   /* assembly: asm/data/4F890.data.s */
extern u16 gHitMaskBits[];   /* assembly: asm/data/4F890.data.s */
extern s32 (*gItemCheckHandlers[])(ActorTransform*, ItemData*);   /* item use handlers (Quest D_8004D480) */
extern void (*gItemUseHandlers[])(s32, ItemData*, u8, s32);   /* assembly: asm/data/500A0.data.s */
extern char* gSaveMessageTexts[];   /* their texts */
extern char* gSaveModePromptTexts[];   /* the prompts of the save mode (after the inn's rest); [0..3], the Repair, Reconnect, Yes and No labels, are drawn from it in every mode */
extern char* gTitleModePromptTexts[];   /* the prompts of the load and delete modes (the title's Continue and Delete) */
extern u8* D_8006CC28[];   /* assembly: asm/data/6D820.data.s */
extern f64 D_8006F570;   /* assembly: asm/data/70170.rodata.s */
extern f64 D_8006F578;   /* assembly: asm/data/70170.rodata.s */
extern f64 D_8006F580;   /* assembly: asm/data/70170.rodata.s */
extern f64 D_8006F588;   /* assembly: asm/data/70170.rodata.s */
extern f64 D_8006F590;   /* assembly: asm/data/70170.rodata.s */
extern f64 D_8006F598;   /* assembly: asm/data/70170.rodata.s */

#endif
