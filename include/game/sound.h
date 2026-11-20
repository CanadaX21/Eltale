#ifndef GAME_SOUND_H
#define GAME_SOUND_H

/* The functions of src/game/sound.c, declared once for every file that calls them. */

#include "common.h"

void InitializeSoundData(void);
void UpdateSound(void);   /* sound update (sound.c) */
void RequestMusic(s8 song);   /* music request (sound.c) */
void UpdateBGMDelay(s8 song, u16 fade);
void FindMapBGM(s32 map, s32 room, u16 fade);
void PlaySound(u8 delay, s8 id, u8 level);   /* sound.c */
void PlaySoundNow(s8 id, u8 level);
void SetBGMVolume(void);

/* background music by place: 0x48 entries (FindMapBGM); room -1 = any room of the map */
typedef struct {
    s16 map;
    s16 room;
    s8 song;
    s8 pad;
} BgmEntry;

/* The state src/game/sound.c defines (.bss), in its storage order: the order of these declarations is the
 * unit's .bss layout (include/bss_order.h). */
extern s8 gMusicPlaying;   /* the music playing, -1 = none */
extern s8 gNextMusic;   /* the music to play next (RequestMusic) */
extern u16 gMusicChangeFlags;   /* 1 change requested, 2 stop the current, 4 start the next */
extern u8 gSoundVolume;
extern u16 gMusicChangeDelay;
extern DelayedSound gDelayedSounds[16];   /* sounds waiting for their delay (PlaySound) */

/* The data src/game/sound.c defines. */
extern BgmEntry gMapBgmTable[0x48];
extern u8 gMusicVolumeLevels[5];   /* the music volume per setting (SetBGMVolume) */
extern u8 gSoundVolumes[72];
extern AnimEntry* gBrianAnims[1];   /* Brian's animation table per model */
extern BrianData1 gNewGameStats;   /* his stats at a new game */
extern u32 gSpiritExpTable[100];   /* exp needed per level */
extern u16 gXPTable[70];   /* XP needed per stat level */
extern RomRange gBrianAnimRom[30];   /* Brian's animations in ROM */
extern RespawnLocation gRespawnLocationTbl[17];
extern MonsterSet gMonsterSets[6];
extern BossFxEntry* gBossEffectScripts[20];   /* the boss effect scripts, by a boss action's low tag byte */
extern SpellAnimStage gEnemySpellEffects[22];   /* the enemies' effect stages: the boss scripts count them flat; stages 18-21 with the 8 bytes after them are the landing burst's SpellAnimDef */
extern RomRange gNpcBlobs[7];   /* each area's NPC blob in ROM: its NPCs' models and names (LoadNPCArea loads it into gMonsterFileBuffer) */
extern sMapData gMapTable[36];   /* every map's header */
extern u8* gRegionBlocks[18];   /* extra ROM blocks, 3 words each: start, end, RAM (sMapData.regionBlock; Quest D_800558A0) */
extern s8 gTileOffsets0[2];   /* tile neighbourhoods: (dx, dz) pairs, one table per tile mode */
extern s8 gTileOffsets1[314];
extern s8 gTileOffsets2[178];
extern s8 gTileOffsets3[18];
extern s8 gTileOffsets4[42];
extern ModelPart gDoorAnimTable;   /* door kinds 2 and 3 */
extern ModelPart D_80057D24;   /* unreferenced: a part of the same kind */
extern ModelPart gDoorAnimAlt;   /* door kinds 2 and 3 when the door's flags' bit 0 is set */
extern ModelPart gDoorAnim4;   /* kind 4 */
extern ModelPart gMapModelPaths[4];   /* the animated models' animations */
extern MapPropGfx gFlameModelLists[2];   /* the flame kinds' setup and model, [kind - 0x11] */
extern u8** gFlameFrameTables[2];   /* their 8 texture frames, [kind - 0x11] (DrawFlames) */
extern Gfx gMapDustGfx[16];   /* their display list */
extern Gfx gDustGfx2[10];   /* its display list */
extern SkyRecord gSkyRecords[8];
extern u8 gPropAlpha[36];   /* per map: the props' alpha */

/* Labels of the unit's own assembly (its data still written as assembly in the .c) that its C uses. */
extern u8 gOpeningSpeechText[];   /* the opening speech (SetupNPCs): src/game/sound.c's own assembly */
extern u8 gFlameTextures0[];   /* src/game/sound.c's own assembly */
extern u8 gFlameTextures1[];   /* src/game/sound.c's own assembly */
extern u8 gMapDustPalette[];   /* src/game/sound.c's own assembly */
extern u8 gMapDustTextures[];   /* src/game/sound.c's own assembly */
extern u8 gDustTexture2[];   /* src/game/sound.c's own assembly */

#endif
