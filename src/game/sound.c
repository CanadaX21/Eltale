#include "common.h"
BSS_ORDER_PAD(0)
#include "game/sound.h"
#include "game/audio.h"
#include "game/start_menu.h"
#include "segment_symbols.h"
#include "monster_sets.h"
#include "maps.h"
#include "brian.h"
/* The music and sound requests (InitializeSoundData to SetBGMVolume), and after the code main's game tables (this
   file's .data). The Start menu that followed the sound code in this unit is its own file, start_menu.c: this file's
   .bss ends eight bytes before the menu's, as a .bss padded to sixteen does, and the menu's first function starts
   on a sixteen-byte boundary. */
/* ---- this file's .bss (0x8008FAF0-0x8008FB27), in address order; the object pads it to 0x8008FB30 ---- */
s8 gMusicPlaying;                 /* the music playing, -1 = none */
s8 gNextMusic;                    /* the music to play next (RequestMusic) */
u16 gMusicChangeFlags;            /* 1 change requested, 2 stop the current, 4 start the next */
u8 gSoundVolume;
u16 gMusicChangeDelay;
DelayedSound gDelayedSounds[16];  /* sounds waiting for their delay (PlaySound) */

void InitializeSoundData(void) {
    s32 i;
    DelayedSound* tmp;

    gMusicChangeFlags = 0;
    gMusicChangeDelay = 0;
    gNextMusic = -1;
    gMusicPlaying = gNextMusic;
    gSoundVolume = 0xFF;

    i = 0x10;
    tmp = gDelayedSounds;
    while(i > 0) {
        tmp->delay = 0;
        tmp++;
        i--;
    }
}
void UpdateSound(void)
{
  DelayedSound *slot;
  s32 i;
  
  if ((gMusicChangeFlags & 1) != 0)
  {
    if (gMusicChangeDelay == 0)
    {
      if (gMusicPlaying != (-1))
      {
        gMusicChangeFlags = gMusicChangeFlags | 2;
      }
      else
      {
        gMusicChangeFlags = gMusicChangeFlags | 4;
      }
      gMusicChangeFlags = gMusicChangeFlags & (~0x1);
    }
    else
    {
      gMusicChangeDelay--;
    }
  }
  if (gMusicChangeFlags & 2)
  {
    gMusicChangeFlags = gMusicChangeFlags & 0xFFFD;
    gMusicChangeFlags = gMusicChangeFlags | 4;
    StopMusicSlot(0);
  }
  else
    if ((gMusicChangeFlags & 4) && (!GetMusicSlotState(0)))
  {
    gMusicChangeFlags &= ~0x4;
    gMusicPlaying = gNextMusic;
    if (gMusicPlaying != -1)
    {
      LoadSequence(0, gMusicPlaying);
      SetBGMVolume();
      StartSequence(0);
    }
  }

  i = 16;
  slot = gDelayedSounds;
  for (i; i > 0; i--)
  {
    if (slot->delay != 0)
    {
      slot->delay--;
      if (!slot->delay)
      {
        PushSoundQueue(slot->id, slot->volume, 64);
      }
    }
    slot++;
  }
}
void RequestMusic(s8 song) {
    if (song != gMusicPlaying) {
        gNextMusic = song;
        gMusicChangeFlags |= 1;
    }
}
void UpdateBGMDelay(s8 song, u16 fade) {
    if (song != gMusicPlaying) {
        gNextMusic = song;
        gMusicChangeFlags |= 1;
        gMusicChangeDelay = fade;
    }
}
void FindMapBGM(s32 map, s32 room, u16 fade) {
    BgmEntry* entry = gMapBgmTable;
    s32 i;
    s8 song;

    for (i = 0x48; i != 0; i--, entry++) {
        if (map == entry->map && (room == entry->room || entry->room == -1)) {
            break;
        }
    }
    if (i != 0) {
        song = entry->song;
        if (song != gMusicPlaying) {
            gNextMusic = song;
            gMusicChangeFlags |= 1;
            gMusicChangeDelay = fade;
        }
    }
}
void PlaySound(u8 delay, s8 id, u8 level) {
    s32 i;
    s32 volume;
    DelayedSound* slot;

    volume = (u32)(gSoundVolumes[id] * level * gSoundVolume) >> 0x10;
    if (delay == 0) {
        PushSoundQueue(id, volume, 0x40);
        return;
    }
    
    i = 0xF;
    slot = gDelayedSounds;

    while (i != 0 && slot->delay != 0) {
        i--;
        slot++;
    }
    
    slot->delay = delay;
    slot->id = id;
    slot->volume = volume;
}
void PlaySoundNow(s8 id, u8 level) {
    u32 volume = (u32)(gSoundVolumes[id] * level * gSoundVolume) >> 0x10;

    if (volume != 0) {
        PushSoundQueue(id, volume, 0x40);
    }
}
void SetBGMVolume(void) {
    ApplyMusicVolume(gMusicVolumeLevels[gMusicVolumeSetting]);
}

/* ---- this file's .data ----
   It starts with this file's own sound tables. The tables after them are read by other units, in blocks that each
   start on a 16-byte boundary: Brian's (player.c), the enemies' and NPCs' (enemy.c), the maps' (map.c). The original may have
   kept those blocks in separate data-only objects linked between this file and start_menu.c; the bytes are the same either
   way. ROM offsets are the segments' own symbols, so the tables follow the assets if those move or change size;
   addresses inside files loaded at run time (above main's image) stay numbers. Texture, palette and text bytes are
   spans read from the ROM at build time (GLOBAL_ASM .incbin): the source holds their offsets, not their bytes. */

/* -- the sound tables: this file's own (FindMapBGM, SetBGMVolume, PlaySound) -- */
BgmEntry gMapBgmTable[0x48] = {      /* map, room (-1 = any room of the map), song */
    { 0, 2, 33, 0 }, { 0, 3, 33, 0 }, { 0, 4, 33, 0 }, { 11, 4, -1, 0 },
    { 11, 3, -1, 0 }, { 11, 5, -1, 0 }, { 11, 7, -1, 0 }, { 11, 6, -1, 0 },
    { 0, -1, 2, 0 }, { 1, -1, 25, 0 }, { 2, -1, 21, 0 }, { 3, -1, 21, 0 },
    { 4, -1, 34, 0 }, { 5, -1, 21, 0 }, { 6, -1, 37, 0 }, { 7, -1, 21, 0 },
    { 8, -1, 36, 0 }, { 9, -1, 22, 0 }, { 10, -1, 5, 0 }, { 11, -1, 39, 0 },
    { 12, -1, 26, 0 }, { 14, 13, 33, 0 }, { 15, 13, 33, 0 }, { 22, 0, 12, 0 },
    { 22, 1, 12, 0 }, { 22, 24, 33, 0 }, { 22, 25, 33, 0 }, { 23, 0, 7, 0 },
    { 23, 1, 7, 0 }, { 23, 2, 7, 0 }, { 23, 3, 7, 0 }, { 23, 4, 7, 0 },
    { 23, 5, 7, 0 }, { 23, 19, 7, 0 }, { 23, 20, 7, 0 }, { 23, 21, 7, 0 },
    { 25, 6, 33, 0 }, { 25, 7, 33, 0 }, { 13, -1, 15, 0 }, { 14, -1, 3, 0 },
    { 15, -1, 7, 0 }, { 16, -1, 7, 0 }, { 17, -1, 7, 0 }, { 18, -1, 7, 0 },
    { 19, -1, 7, 0 }, { 20, -1, 7, 0 }, { 21, -1, 8, 0 }, { 22, -1, 7, 0 },
    { 23, -1, 39, 0 }, { 24, -1, 5, 0 }, { 25, -1, 6, 0 }, { 26, 2, 9, 0 },
    { 26, 3, 11, 0 }, { 26, 4, 11, 0 }, { 30, 6, -1, 0 }, { 31, 1, 35, 0 },
    { 31, 2, 24, 0 }, { 32, 1, 12, 0 }, { 32, 2, 12, 0 }, { 33, 1, 12, 0 },
    { 33, 2, 12, 0 }, { 34, 1, 38, 0 }, { 26, -1, 23, 0 }, { 27, -1, 10, 0 },
    { 28, -1, 1, 0 }, { 29, -1, 18, 0 }, { 30, -1, 31, 0 }, { 31, -1, 12, 0 },
    { 32, -1, 24, 0 }, { 33, -1, 24, 0 }, { 34, -1, 33, 0 }, { 35, -1, 14, 0 },
};
u8 gMusicVolumeLevels[5] = { 0, 120, 180, 220, 255 };   /* the music volume per setting (SetBGMVolume) */
u8 gSoundVolumes[72] = {             /* each sound's volume, by sound id (PlaySound) */
    250, 250, 250, 250, 250, 200, 250, 250, 250, 250, 250, 250,
    250, 250, 250, 250, 250, 250, 250, 150, 250, 250, 250, 250,
    250, 255, 250, 255, 250, 250, 255, 250, 255, 180, 250, 250,
    250, 250, 250, 200, 250, 250, 250, 250, 250, 250, 250, 250,
    200, 250, 250, 250, 250, 180, 250, 250, 250, 200, 250, 200,
    200, 200, 200, 250, 250, 250, 250, 250, 250, 250, 0, 0,
};

/* -- Brian's tables (read by player.c) -- */
AnimEntry* gBrianAnims[1] = {        /* his animation table by model (one model): in his model as InitBrian loads it */
    gBrianModelAnims,
};
BrianData1 gNewGameStats = {         /* his stats at a new game (InitBrian copies them to gBrianStats) */
    0, 0,                          /* unk0, unk2 */
    50, 50, 15, 15, 5, 4,          /* HP, max HP, MP, max MP, agility, defence */
    0, 500,                          /* exp, stone */
    50.0f, 160.0f, 0.07f,         /* radius, height, model scale */
    { 1, 1, 1, 1 },              /* spirits: fire, earth, water, wind */
    0, 0, 0, 0,                  /* HP, MP, agility, defence XP */
    0, 0, 0, 0, 0, 0, 0, 0,  /* HP, MP, agility, defence, spirit levels; unk35-37 */
};
u32 gSpiritExpTable[100] = {         /* the EXP a spirit level needs */
    8, 10, 13, 17, 22, 28, 35, 44, 55, 69,
    86, 106, 130, 159, 193, 234, 282, 338, 404, 481,
    571, 674, 793, 929, 1085, 1263, 1464, 1691, 1947, 2235,
    2557, 2916, 3315, 3757, 4246, 4785, 5377, 6025, 6733, 7505,
    8344, 9253, 10237, 11297, 12438, 13663, 14975, 16377, 17871, 19462,
    21151, 22940, 24832, 26829, 28933, 31145, 33466, 35898, 38442, 41098,
    43866, 46747, 49740, 52846, 56063, 59390, 62826, 66371, 70022, 73778,
    77636, 81594, 85650, 89801, 94045, 98378, 102798, 107300, 111883, 116541,
    121272, 126072, 130938, 135865, 140849, 145887, 150975, 156109, 161284, 166497,
    171745, 177022, 182326, 187652, 192996, 198356, 203726, 209104, 214486, 219868,
};
u16 gXPTable[70] = {                 /* the XP a stat level needs */
    10, 11, 11, 11, 12, 12, 12, 13, 13, 13,
    13, 14, 14, 14, 15, 15, 16, 16, 16, 17,
    17, 18, 18, 18, 19, 19, 20, 20, 21, 21,
    22, 22, 23, 23, 24, 25, 25, 26, 26, 27,
    28, 28, 29, 30, 31, 31, 32, 33, 34, 35,
    35, 36, 37, 38, 39, 40, 41, 42, 43, 44,
    45, 46, 48, 49, 50, 50, 50, 50, 50, 50,
};
RomRange gBrianAnimRom[30] = {       /* his animations in ROM, by animation (LoadBrianAnimation) */
    { brian_anim_00_ROM_START, brian_anim_00_ROM_END },
    { brian_anim_01_ROM_START, brian_anim_01_ROM_END },
    { brian_anim_02_ROM_START, brian_anim_02_ROM_END },
    { brian_anim_03_ROM_START, brian_anim_03_ROM_END },
    { brian_anim_04_ROM_START, brian_anim_04_ROM_END },
    { brian_anim_05_ROM_START, brian_anim_05_ROM_END },
    { brian_anim_06_ROM_START, brian_anim_06_ROM_END },
    { brian_anim_07_ROM_START, brian_anim_07_ROM_END },
    { brian_anim_08_ROM_START, brian_anim_08_ROM_END },
    { brian_anim_09_ROM_START, brian_anim_09_ROM_END },
    { brian_anim_10_ROM_START, brian_anim_10_ROM_END },
    { brian_anim_11_ROM_START, brian_anim_11_ROM_END },
    { brian_anim_12_ROM_START, brian_anim_12_ROM_END },
    { brian_anim_13_ROM_START, brian_anim_13_ROM_END },
    { brian_anim_14_ROM_START, brian_anim_14_ROM_END },
    { brian_anim_15_ROM_START, brian_anim_15_ROM_END },
    { brian_anim_16_ROM_START, brian_anim_16_ROM_END },
    { brian_anim_17_ROM_START, brian_anim_17_ROM_END },
    { brian_anim_18_ROM_START, brian_anim_18_ROM_END },
    { brian_anim_19_ROM_START, brian_anim_19_ROM_END },
    { brian_anim_20_ROM_START, brian_anim_20_ROM_END },
    { brian_anim_21_ROM_START, brian_anim_21_ROM_END },
    { brian_anim_22_ROM_START, brian_anim_22_ROM_END },
    { brian_anim_23_ROM_START, brian_anim_23_ROM_END },
    { brian_anim_24_ROM_START, brian_anim_24_ROM_END },
    { brian_anim_25_ROM_START, brian_anim_25_ROM_END },
    { brian_anim_26_ROM_START, brian_anim_26_ROM_END },
    { brian_anim_27_ROM_START, brian_anim_27_ROM_END },
    { brian_anim_28_ROM_START, brian_anim_28_ROM_END },
    { brian_anim_29_ROM_START, brian_anim_29_ROM_END },
};
RespawnLocation gRespawnLocationTbl[17] = {   /* where Brian wakes up in a map with a save point (RespawnAtSavePoint): map, submap, x, z, yaw */
    { 15, 5, -2.36f, 1.23f, 1.5793f },
    { 16, 7, -2.36f, 1.23f, 1.5793f },
    { 16, 15, -2.36f, 1.23f, 1.5793f },
    { 18, 6, 13.31f, 16.56f, 2.43f },
    { 17, 0, -7.9f, -40.5f, -1.51f },
    { 18, 7, -14.4f, 18.41f, 2.46f },
    { 20, 5, -2.36f, 1.23f, 1.5793f },
    { 22, 22, 9.93f, 17.45f, 2.2467f },
    { 22, 14, -2.36f, 1.23f, 1.5793f },
    { 23, 0, -2.36f, 1.23f, 1.5793f },
    { 23, 20, -3.65f, -8.81f, 2.4174f },
    { 23, 22, 2.45f, 14.41f, 2.4171f },
    { 23, 8, -2.36f, 1.23f, 1.5793f },
    { 26, 3, 14.15f, 0.67f, 1.97f },
    { 25, 3, 2.96f, 0.12f, -0.9817f },
    { 30, 15, 1.4f, -89.5f, -0.2801f },
    { 15, 13, 11.0f, 15.98f, 2.2914f },
};
/* Zero bytes no code reads. Brian's tables end at 0x80056198 and the enemies' start on the next 16-byte boundary, as a
   separate object's .data would; kept as a definition because this file holds both blocks. */
static u32 sZeroAfterBrianTables[2] = { 0, 0 };

/* -- the enemies' and NPCs' tables (read by enemy.c) -- */
/* the typed sets' tables, each defined by its own set's segment (the six sets load at one RAM range) */
MonsterSet gMonsterSets[6] = {       /* each monster area's file in ROM; its table of animation tables, base records and name table in the file as LoadMonsterSet loads it to gMonsterFileBuffer (0x8020E6F0) */
    { monsters_0_ROM_START, monsters_0_ROM_END, gMonSet0AnimTables, gMonSet0BaseData, gMonSet0NameTable },
    { monsters_1_ROM_START, monsters_1_ROM_END, gMonSet1AnimTables, gMonSet1BaseData, gMonSet1NameTable },
    { monsters_2_ROM_START, monsters_2_ROM_END, gMonSet2AnimTables, gMonSet2BaseData, gMonSet2NameTable },
    { monsters_3_ROM_START, monsters_3_ROM_END, gMonSet3AnimTables, gMonSet3BaseData, gMonSet3NameTable },
    { monsters_4_ROM_START, monsters_4_ROM_END, gMonSet4AnimTables, gMonSet4BaseData, gMonSet4NameTable },
    { monsters_5_ROM_START, monsters_5_ROM_END, gMonSet5AnimTables, gMonSet5BaseData, gMonSet5NameTable },
};
/* the boss effect scripts, in gBossEffectScripts' order: frame, stage (in gEnemySpellEffects), flags (1 = last entry), x, y, z, angle */
static BossFxEntry sBossFxScript00[6] = {
    { 25, 0, 0x0, { 0, 0 }, 0.0f, 190.0f, 200.0f, 0.0f },
    { 26, 0, 0x8, { 0, 0 }, 0.0f, 0.0f, -2.0f, 0.0f },
    { 29, 1, 0x0, { 0, 0 }, 0.0f, 190.0f, 200.0f, 0.0f },
    { 33, 1, 0x0, { 0, 0 }, 0.0f, 190.0f, 200.0f, 0.0f },
    { 37, 1, 0x0, { 0, 0 }, 0.0f, 190.0f, 200.0f, 0.0f },
    { 41, 1, 0x1, { 0, 0 }, 0.0f, 190.0f, 200.0f, 0.0f },
};
static BossFxEntry sBossFxScript01[1] = {
    { 32, 2, 0x5, { 0, 0 }, 0.0f, 0.0f, 200.0f, 0.0f },
};
static BossFxEntry sBossFxScript02[5] = {
    { 30, 16, 0x0, { 0, 0 }, 0.0f, 110.0f, 40.0f, 0.0f },
    { 34, 16, 0x0, { 0, 0 }, 0.0f, 110.0f, 40.0f, 0.0f },
    { 38, 16, 0x0, { 0, 0 }, 0.0f, 110.0f, 40.0f, 0.0f },
    { 42, 16, 0x0, { 0, 0 }, 0.0f, 110.0f, 40.0f, 0.0f },
    { 46, 16, 0x1, { 0, 0 }, 0.0f, 110.0f, 40.0f, 0.0f },
};
static BossFxEntry sBossFxScript03[1] = {
    { 25, 17, 0x1, { 0, 0 }, 0.0f, 200.0f, 0.0f, 0.0f },
};
static BossFxEntry sBossFxScript05[6] = {
    { 1, 4, 0x0, { 0, 0 }, 0.0f, 130.0f, 0.0f, 0.0f },
    { 2, 5, 0x0, { 0, 0 }, 0.0f, 0.0f, 0.0f, 0.0f },
    { 8, 5, 0x0, { 0, 0 }, 0.0f, 0.0f, 0.0f, 0.0f },
    { 14, 5, 0x0, { 0, 0 }, 0.0f, 0.0f, 0.0f, 0.0f },
    { 20, 5, 0x0, { 0, 0 }, 0.0f, 0.0f, 0.0f, 0.0f },
    { 26, 5, 0x1, { 0, 0 }, 0.0f, 0.0f, 0.0f, 0.0f },
};
static BossFxEntry sBossFxScript07[1] = {
    { 23, 14, 0x1, { 0, 0 }, 0.0f, 0.0f, 0.0f, 0.0f },
};
static BossFxEntry sBossFxScript08[5] = {
    { 18, 6, 0x0, { 0, 0 }, 0.0f, 160.0f, 160.0f, 0.0f },
    { 19, 0, 0x8, { 0, 0 }, 0.0f, 0.0f, -1.0f, 0.0f },
    { 20, 7, 0x0, { 0, 0 }, 0.0f, 160.0f, 160.0f, 0.0f },
    { 26, 7, 0x0, { 0, 0 }, 0.0f, 160.0f, 160.0f, 0.0f },
    { 32, 7, 0x1, { 0, 0 }, 0.0f, 160.0f, 160.0f, 0.0f },
};
static BossFxEntry sBossFxScript10[6] = {
    { 25, 8, 0x0, { 0, 0 }, 0.0f, 0.0f, 0.0f, 0.0f },
    { 32, 9, 0x0, { 0, 0 }, 0.0f, 0.0f, 0.0f, 0.0f },
    { 38, 9, 0x0, { 0, 0 }, 0.0f, 0.0f, 0.0f, 0.0f },
    { 44, 9, 0x0, { 0, 0 }, 0.0f, 0.0f, 0.0f, 0.0f },
    { 50, 9, 0x0, { 0, 0 }, 0.0f, 0.0f, 0.0f, 0.0f },
    { 56, 9, 0x1, { 0, 0 }, 0.0f, 0.0f, 0.0f, 0.0f },
};
static BossFxEntry sBossFxScript11[4] = {
    { 0, 10, 0x0, { 0, 0 }, 0.0f, 0.0f, 0.0f, 0.0f },
    { 4, 10, 0x0, { 0, 0 }, 0.0f, 0.0f, 0.0f, 0.0f },
    { 8, 10, 0x0, { 0, 0 }, 0.0f, 0.0f, 0.0f, 0.0f },
    { 31, 15, 0x3, { 0, 0 }, 0.0f, 0.0f, 0.0f, 0.0f },
};
static BossFxEntry sBossFxScript12[4] = {
    { 1, 11, 0x0, { 0, 0 }, 0.0f, 0.0f, 0.0f, 0.0f },
    { 9, 11, 0x0, { 0, 0 }, 0.0f, 0.0f, 0.0f, 0.7853982f },
    { 17, 11, 0x0, { 0, 0 }, 0.0f, 0.0f, 0.0f, 15.707963f },
    { 25, 11, 0x1, { 0, 0 }, 0.0f, 0.0f, 0.0f, 3.1415927f },
};
static BossFxEntry sBossFxScript13[5] = {
    { 35, 12, 0x0, { 0, 0 }, 0.0f, 150.0f, 150.0f, 0.0f },
    { 36, 0, 0x8, { 0, 0 }, 0.0f, 0.0f, -2.0f, 0.0f },
    { 41, 13, 0x0, { 0, 0 }, 0.0f, 150.0f, 150.0f, 0.0f },
    { 47, 13, 0x0, { 0, 0 }, 0.0f, 150.0f, 150.0f, 0.0f },
    { 53, 13, 0x1, { 0, 0 }, 0.0f, 150.0f, 150.0f, 0.0f },
};
static BossFxEntry sBossFxScript14[2] = {
    { 50, 18, 0x0, { 0, 0 }, 0.0f, 800.0f, 0.0f, 0.0f },
    { 51, 19, 0x1, { 0, 0 }, 0.0f, 800.0f, 0.0f, 0.0f },
};
static BossFxEntry sBossFxScript16[4] = {
    { 38, 21, 0x0, { 0, 0 }, 0.0f, 380.0f, 350.0f, 0.0f },
    { 42, 21, 0x0, { 0, 0 }, 0.0f, 380.0f, 350.0f, 0.0f },
    { 46, 21, 0x0, { 0, 0 }, 0.0f, 380.0f, 350.0f, 0.0f },
    { 50, 21, 0x1, { 0, 0 }, 0.0f, 380.0f, 350.0f, 0.0f },
};
BossFxEntry* gBossEffectScripts[20] = {  /* by a boss action's low tag byte (StartBossEffectScript) */
    sBossFxScript00, sBossFxScript01, sBossFxScript02, sBossFxScript03,
    NULL, sBossFxScript05, NULL, sBossFxScript07,
    sBossFxScript08, NULL, sBossFxScript10, sBossFxScript11,
    sBossFxScript12, sBossFxScript13, sBossFxScript14, NULL,
    sBossFxScript16, NULL, NULL, NULL,
};
/* the enemies' effect stages, 16 bytes each: model (in gEffectModels), colour, flags (SpawnEffectObject's), unk6, size x, size y. The boss
   effect scripts count them flat (BossFxEntry.stage); stages 18-21 and the 8 sound bytes after them are read together
   as one SpellAnimDef, the landing burst (SpawnEnemyLandingBurst). */
SpellAnimStage gEnemySpellEffects[22] = {
    { 70, 0x7, 10, 0, 1.1f, 1.1f },   /* 0 */
    { 71, 0x37, 2, 0, 1.0f, 1.0f },   /* 1 */
    { 72, 0x7, 2, 0, 11.0f, 11.0f },   /* 2 */
    { 73, 0x7, 0, 0, 4.0f, 4.0f },   /* 3 */
    { 79, 0x7, 130, 0, 4.0f, 4.0f },   /* 4 */
    { 80, 0x7, 2, 0, 6.0f, 6.0f },   /* 5 */
    { 85, 0x6, 2, 0, 0.6f, 0.6f },   /* 6 */
    { 86, 0x32, 2, 0, 1.2f, 1.2f },   /* 7 */
    { 88, 0x7, 2, 0, 2.0f, 2.0f },   /* 8 */
    { 89, 0x7, 0, 0, 2.0f, 2.0f },   /* 9 */
    { 91, 0x7, 2, 0, 1.0f, 1.0f },   /* 10 */
    { 93, 0x2C, 2, 0, 1.0f, 1.0f },   /* 11 */
    { 94, 0x4, 10, 0, 2.0f, 2.0f },   /* 12 */
    { 95, 0x38, 2, 0, 1.0f, 1.0f },   /* 13 */
    { 84, 0x7, 2, 0, 1.0f, 1.0f },   /* 14 */
    { 90, 0x7, 0, 0, 1.0f, 1.0f },   /* 15 */
    { 75, 0x2A, 2, 0, 1.0f, 1.0f },   /* 16 */
    { 76, 0x36, 0, 0, 6.0f, 6.0f },   /* 17 */
    { 96, 0x4, 128, 0, 1.0f, 1.0f },   /* 18 */
    { 97, 0x35, 128, 0, 1.0f, 1.0f },   /* 19 */
    { 66, 0x2, 0, 0, 1.0f, 1.0f },   /* 20 */
    { 71, 0x37, 2, 0, 0.5f, 0.5f },   /* 21 */
};
static s8 sLandingBurstSounds[8] = {  /* the landing burst's sounds and delays: its SpellAnimDef's bytes 0x40-0x47, read through gEnemySpellEffects[18] */
    0, 0, 0, 0, 0, 0, 0, 0,
};
RomRange gNpcBlobs[7] = {            /* each NPC area's blob in ROM: its NPCs' models and names (LoadNPCArea loads it to gMonsterFileBuffer) */
    { npc_blob_0_ROM_START, npc_blob_0_ROM_END },
    { npc_blob_1_ROM_START, npc_blob_1_ROM_END },
    { npc_blob_2_ROM_START, npc_blob_2_ROM_END },
    { npc_blob_3_ROM_START, npc_blob_3_ROM_END },
    { npc_blob_4_ROM_START, npc_blob_4_ROM_END },
    { npc_blob_5_ROM_START, npc_blob_5_ROM_END },
    { npc_blob_6_ROM_START, npc_blob_6_ROM_END },
};
/* the opening speech: SetupNPCs hands it to StartNPCTextFromRAM on a new game's first field start, the submap's
   first NPC speaking; then zero bytes to 0x80056BB0. Read from the ROM at build time */
GLOBAL_ASM(
.section .data
glabel gOpeningSpeechText
.incbin "baserom.jp.z64", 0x574B8, 0x2F8
)


/* -- the maps' tables (read by map.c) -- */
/* map 15, built from typed sources: its load address (the segment's link address) and the records gMapTable names */
#include "map_format.h"
/* maps 12 and 32, built from typed sources (the encounter tables are s32 fields in sMapData: their names cast) */
/* maps 4, 8, 9, 18, 19, 20, 26, 31, 33, 35, built from typed sources */
/* maps 23, 25, built from typed sources */
/* maps 10, 13, 14, 16, 24, built from typed sources */
/* maps 17, 21, 22, built from typed sources */
/* maps 0, 1, 3, 5, 6, 7, 30, built from typed sources */
/* maps 2, 11, 27, 28, 29, built from typed sources */
/* maps 34, built from typed sources */
sMapData gMapTable[36] = {   /* flags, ROM block, load address, then pointers into the map as loaded, encounters, encounterSets, NPC list, region block, unk40 */
    /*  0 */ { MAP_TOWN | MAP_FOG_SKY, { 0, 0 }, map_00_ROM_START, map_00_ROM_END, map_00_VRAM,   /* Melrode: typed sources */
             gMap00Geometry, gMap00ModelSets, gMap00Submaps, gMap00Lights,
             gMap00PropGfx, gMap00Cameras, gMap00CameraNavmesh, gMap00ActorNavmesh,
             0x0, 0x0, gMap00NpcList, 0, { 0, 0, 0, 5 } },
    /*  1 */ { MAP_TOWN, { 0, 0 }, map_01_ROM_START, map_01_ROM_END, map_01_VRAM,   /* Dondoran: typed sources */
             gMap01Geometry, gMap01ModelSets, gMap01Submaps, gMap01Lights,
             gMap01PropGfx, gMap01Cameras, gMap01CameraNavmesh, gMap01ActorNavmesh,
             0x0, 0x0, gMap01NpcList, 1, { 0, 0, 0, 1 } },
    /*  2 */ { MAP_FIELD, { 0, 0 }, map_02_ROM_START, map_02_ROM_END, map_02_VRAM,   /* Holy Plains: typed sources */
             gMap02Geometry, gMap02ModelSets, gMap02Submaps, gMap02Lights,
             gMap02PropGfx, gMap02Cameras, gMap02CameraNavmesh, gMap02ActorNavmesh,
             (s32)gMap02EncounterZones, gMap02EncounterSets, NULL, 1, { 0, 0, 0, 1 } },
    /*  3 */ { MAP_FIELD, { 0, 0 }, map_03_ROM_START, map_03_ROM_END, map_03_VRAM,   /* Dondoran Flats: typed sources */
             gMap03Geometry, gMap03ModelSets, gMap03Submaps, gMap03Lights,
             gMap03PropGfx, gMap03Cameras, gMap03CameraNavmesh, gMap03ActorNavmesh,
             (s32)gMap03EncounterZones, gMap03EncounterSets, gMap03NpcList, 1, { 0, 0, 0, 2 } },
    /*  4 */ { MAP_TOWN, { 0, 0 }, map_04_ROM_START, map_04_ROM_END, map_04_VRAM,   /* Larapool: typed sources */
             gMap04Geometry, gMap04ModelSets, gMap04Submaps, gMap04Lights,
             gMap04PropGfx, gMap04Cameras, gMap04CameraNavmesh, gMap04ActorNavmesh,
             0x0, 0x0, gMap04NpcList, 2, { 0, 0, 0, 3 } },
    /*  5 */ { MAP_FIELD, { 0, 0 }, map_05_ROM_START, map_05_ROM_END, map_05_VRAM,   /* West Carmagh: typed sources */
             gMap05Geometry, gMap05ModelSets, gMap05Submaps, gMap05Lights,
             gMap05PropGfx, gMap05Cameras, gMap05CameraNavmesh, gMap05ActorNavmesh,
             (s32)gMap05EncounterZones, gMap05EncounterSets, gMap05NpcList, 2, { 0, 0, 0, 2 } },
    /*  6 */ { MAP_TOWN, { 0, 0 }, map_06_ROM_START, map_06_ROM_END, map_06_VRAM,   /* Normoon: typed sources */
             gMap06Geometry, gMap06ModelSets, gMap06Submaps, gMap06Lights,
             gMap06PropGfx, gMap06Cameras, gMap06CameraNavmesh, gMap06ActorNavmesh,
             0x0, 0x0, gMap06NpcList, 2, { 0, 0, 0, 1 } },
    /*  7 */ { MAP_FIELD, { 0, 0 }, map_07_ROM_START, map_07_ROM_END, map_07_VRAM,   /* East Limelin: typed sources */
             gMap07Geometry, gMap07ModelSets, gMap07Submaps, gMap07Lights,
             gMap07PropGfx, gMap07Cameras, gMap07CameraNavmesh, gMap07ActorNavmesh,
             (s32)gMap07EncounterZones, gMap07EncounterSets, gMap07NpcList, 3, { 0, 0, 0, 2 } },
    /*  8 */ { MAP_TOWN, { 0, 0 }, map_08_ROM_START, map_08_ROM_END, map_08_VRAM,   /* Limelin: typed sources */
             gMap08Geometry, gMap08ModelSets, gMap08Submaps, gMap08Lights,
             gMap08PropGfx, gMap08Cameras, gMap08CameraNavmesh, gMap08ActorNavmesh,
             0x0, 0x0, gMap08NpcList, 3, { 0, 0, 0, 2 } },
    /*  9 */ { MAP_FIELD, { 0, 0 }, map_09_ROM_START, map_09_ROM_END, map_09_VRAM,   /* Dindom Dries: typed sources */
             gMap09Geometry, gMap09ModelSets, gMap09Submaps, gMap09Lights,
             gMap09PropGfx, gMap09Cameras, gMap09CameraNavmesh, gMap09ActorNavmesh,
             (s32)gMap09EncounterZones, gMap09EncounterSets, NULL, 4, { 0, 0, 0, 1 } },
    /* 10 */ { MAP_FIELD, { 0, 0 }, map_10_ROM_START, map_10_ROM_END, map_10_VRAM,   /* Shamwood: typed sources */
             gMap10Geometry, gMap10ModelSets, gMap10Submaps, gMap10Lights,
             gMap10PropGfx, gMap10Cameras, gMap10CameraNavmesh, gMap10ActorNavmesh,
             0x0, 0x0, NULL, 4, { 0, 0, 0, 2 } },
    /* 11 */ { MAP_FIELD, { 0, 0 }, map_11_ROM_START, map_11_ROM_END, map_11_VRAM,   /* Baragoon Moor: typed sources */
             gMap11Geometry, gMap11ModelSets, gMap11Submaps, gMap11Lights,
             NULL, gMap11Cameras, gMap11CameraNavmesh, gMap11ActorNavmesh,
             (s32)gMap11EncounterZones, gMap11EncounterSets, gMap11NpcList, 4, { 0, 0, 0, 8 } },
    /* 12 */ { MAP_FIELD, { 0, 0 }, map_12_ROM_START, map_12_ROM_END, map_12_VRAM,   /* Isle of Skye: typed sources */
             gMap12Geometry, gMap12ModelSets, gMap12Submaps, gMap12Lights,
             gMap12PropGfx, gMap12Cameras, gMap12CameraNavmesh, gMap12ActorNavmesh,
             (s32)gMap12EncounterZones, gMap12EncounterSets, gMap12NpcList, 5, { 0, 0, 0, 3 } },
    /* 13 */ { MAP_INTERIOR, { 0, 0 }, map_13_ROM_START, map_13_ROM_END, map_13_VRAM,   /* Melrode Monastery: typed sources */
             gMap13Geometry, gMap13ModelSets, gMap13Submaps, gMap13Lights,
             gMap13PropGfx, gMap13Cameras, gMap13CameraNavmesh, gMap13ActorNavmesh,
             0x0, 0x0, gMap13NpcList, 0, { 0, 0, 0, 18 } },
    /* 14 */ { MAP_INTERIOR, { 0, 0 }, map_14_ROM_START, map_14_ROM_END, map_14_VRAM,   /* Dondoran Castle: typed sources */
             gMap14Geometry, gMap14ModelSets, gMap14Submaps, gMap14Lights,
             NULL, gMap14Cameras, gMap14CameraNavmesh, gMap14ActorNavmesh,
             0x0, 0x0, gMap14NpcList, 1, { 0, 0, 0, 16 } },
    /* 15 */ { MAP_INTERIOR, { 0, 0 }, map_15_ROM_START, map_15_ROM_END, map_15_VRAM,   /* Melrode buildings: typed sources */
             gMap15Geometry, gMap15ModelSets, gMap15Submaps, gMap15Lights,
             gMap15PropGfx, gMap15Cameras, gMap15CameraNavmesh, gMap15ActorNavmesh,
             0x0, 0x0, gMap15NpcList, 0, { 0, 0, 0, 14 } },
    /* 16 */ { MAP_INTERIOR, { 0, 0 }, map_16_ROM_START, map_16_ROM_END, map_16_VRAM,   /* Dondoran buildings: typed sources */
             gMap16Geometry, gMap16ModelSets, gMap16Submaps, gMap16Lights,
             gMap16PropGfx, gMap16Cameras, gMap16CameraNavmesh, gMap16ActorNavmesh,
             0x0, 0x0, gMap16NpcList, 1, { 0, 0, 0, 22 } },
    /* 17 */ { MAP_INTERIOR, { 0, 0 }, map_17_ROM_START, map_17_ROM_END, map_17_VRAM,   /* Larapool inn: typed sources */
             gMap17Geometry, gMap17ModelSets, gMap17Submaps, gMap17Lights,
             gMap17PropGfx, gMap17Cameras, gMap17CameraNavmesh, gMap17ActorNavmesh,
             0x0, 0x0, gMap17NpcList, 2, { 0, 0, 0, 8 } },
    /* 18 */ { MAP_INTERIOR, { 0, 0 }, map_18_ROM_START, map_18_ROM_END, map_18_VRAM,   /* West Carmagh buildings: typed sources */
             gMap18Geometry, gMap18ModelSets, gMap18Submaps, gMap18Lights,
             NULL, gMap18Cameras, gMap18CameraNavmesh, gMap18ActorNavmesh,
             0x0, 0x0, gMap18NpcList, 2, { 0, 0, 0, 8 } },
    /* 19 */ { MAP_INTERIOR, { 0, 0 }, map_19_ROM_START, map_19_ROM_END, map_19_VRAM,   /* rooms off Normoon (every exit leads to map 6): typed sources */
             gMap19Geometry, gMap19ModelSets, gMap19Submaps, gMap19Lights,
             NULL, gMap19Cameras, gMap19CameraNavmesh, gMap19ActorNavmesh,
             0x0, 0x0, NULL, 2, { 0, 0, 0, 4 } },
    /* 20 */ { MAP_INTERIOR, { 0, 0 }, map_20_ROM_START, map_20_ROM_END, map_20_VRAM,   /* Normoon buildings: typed sources */
             gMap20Geometry, gMap20ModelSets, gMap20Submaps, gMap20Lights,
             gMap20PropGfx, gMap20Cameras, gMap20CameraNavmesh, gMap20ActorNavmesh,
             0x0, 0x0, gMap20NpcList, 2, { 0, 0, 0, 12 } },
    /* 21 */ { MAP_INTERIOR, { 0, 0 }, map_21_ROM_START, map_21_ROM_END, map_21_VRAM,   /* Limelin Castle: typed sources */
             gMap21Geometry, gMap21ModelSets, gMap21Submaps, gMap21Lights,
             NULL, gMap21Cameras, gMap21CameraNavmesh, gMap21ActorNavmesh,
             0x0, 0x0, gMap21NpcList, 3, { 0, 0, 0, 13 } },
    /* 22 */ { MAP_INTERIOR, { 0, 0 }, map_22_ROM_START, map_22_ROM_END, map_22_VRAM,   /* Limelin buildings: typed sources */
             gMap22Geometry, gMap22ModelSets, gMap22Submaps, gMap22Lights,
             gMap22PropGfx, gMap22Cameras, gMap22CameraNavmesh, gMap22ActorNavmesh,
             0x0, 0x0, gMap22NpcList, 3, { 0, 0, 0, 26 } },
    /* 23 */ { MAP_INTERIOR, { 0, 0 }, map_23_ROM_START, map_23_ROM_END, map_23_VRAM,   /* Highland buildings: typed sources */
             gMap23Geometry, gMap23ModelSets, gMap23Submaps, gMap23Lights,
             gMap23PropGfx, gMap23Cameras, gMap23CameraNavmesh, gMap23ActorNavmesh,
             0x0, 0x0, gMap23NpcList, 4, { 0, 0, 0, 26 } },
    /* 24 */ { MAP_INTERIOR, { 0, 0 }, map_24_ROM_START, map_24_ROM_END, map_24_VRAM,   /* Shamwood (Lavaar): typed sources */
             gMap24Geometry, gMap24ModelSets, gMap24Submaps, gMap24Lights,
             NULL, gMap24Cameras, gMap24CameraNavmesh, gMap24ActorNavmesh,
             0x0, 0x0, gMap24NpcList, 4, { 0, 0, 0, 3 } },
    /* 25 */ { MAP_INTERIOR, { 0, 0 }, map_25_ROM_START, map_25_ROM_END, map_25_VRAM,   /* ship and cabin rooms: the Pirate Ship at Skye (0-1, save point), rooms off Skye (2-4), the ferry cabins (5, 8, 9), two rooms off the World of Mammon (6-7): typed sources */
             gMap25Geometry, gMap25ModelSets, gMap25Submaps, gMap25Lights,
             gMap25PropGfx, gMap25Cameras, gMap25CameraNavmesh, gMap25ActorNavmesh,
             0x0, 0x0, gMap25NpcList, 5, { 0, 0, 0, 10 } },
    /* 26 */ { MAP_DUNGEON | MAP_FOG_SKY, { 0, 0 }, map_26_ROM_START, map_26_ROM_END, map_26_VRAM,   /* Blue Cave (room 2 Crystal Valley, 3-4 Epona's Cottage: typed sources */
             gMap26Geometry, gMap26ModelSets, gMap26Submaps, gMap26Lights,
             NULL, gMap26Cameras, gMap26CameraNavmesh, gMap26ActorNavmesh,
             (s32)gMap26EncounterZones, gMap26EncounterSets, gMap26NpcList, 0, { 0, 0, 0, 5 } },
    /* 27 */ { MAP_DUNGEON | MAP_FOG_SKY, { 0, 0 }, map_27_ROM_START, map_27_ROM_END, map_27_VRAM,   /* Cull Hazard: typed sources */
             gMap27Geometry, gMap27ModelSets, gMap27Submaps, gMap27Lights,
             NULL, gMap27Cameras, gMap27CameraNavmesh, gMap27ActorNavmesh,
             (s32)gMap27EncounterZones, gMap27EncounterSets, NULL, 0, { 0, 0, 0, 1 } },
    /* 28 */ { MAP_DUNGEON | MAP_FOG_SKY, { 0, 0 }, map_28_ROM_START, map_28_ROM_END, map_28_VRAM,   /* Baragoon Tunnel (Shilf): typed sources */
             gMap28Geometry, gMap28ModelSets, gMap28Submaps, gMap28Lights,
             NULL, gMap28Cameras, gMap28CameraNavmesh, gMap28ActorNavmesh,
             (s32)gMap28EncounterZones, gMap28EncounterSets, NULL, 0, { 0, 0, 0, 1 } },
    /* 29 */ { MAP_DUNGEON | MAP_FOG_SKY, { 0, 0 }, map_29_ROM_START, map_29_ROM_END, map_29_VRAM,   /* Boil Hole (Fargo): typed sources */
             gMap29Geometry, gMap29ModelSets, gMap29Submaps, gMap29Lights,
             NULL, gMap29Cameras, gMap29CameraNavmesh, gMap29ActorNavmesh,
             (s32)gMap29EncounterZones, gMap29EncounterSets, NULL, 0, { 0, 0, 0, 1 } },
    /* 30 */ { MAP_DUNGEON, { 0, 0 }, map_30_ROM_START, map_30_ROM_END, map_30_VRAM,   /* Brannoch Castle: typed sources */
             gMap30Geometry, gMap30ModelSets, gMap30Submaps, gMap30Lights,
             NULL, gMap30Cameras, gMap30CameraNavmesh, gMap30ActorNavmesh,
             (s32)gMap30EncounterZones, gMap30EncounterSets, gMap30NpcList, 0, { 0, 0, 0, 16 } },
    /* 31 */ { MAP_DUNGEON, { 0, 0 }, map_31_ROM_START, map_31_ROM_END, map_31_VRAM,   /* Connor Forest: typed sources */
             gMap31Geometry, gMap31ModelSets, gMap31Submaps, gMap31Lights,
             NULL, gMap31Cameras, gMap31CameraNavmesh, gMap31ActorNavmesh,
             (s32)gMap31EncounterZones, gMap31EncounterSets, NULL, 0, { 0, 0, 0, 3 } },
    /* 32 */ { MAP_DUNGEON, { 0, 0 }, map_32_ROM_START, map_32_ROM_END, map_32_VRAM,   /* Glencoe Forest: typed sources */
             gMap32Geometry, gMap32ModelSets, gMap32Submaps, gMap32Lights,
             NULL, gMap32Cameras, gMap32CameraNavmesh, gMap32ActorNavmesh,
             (s32)gMap32EncounterZones, gMap32EncounterSets, gMap32NpcList, 0, { 0, 0, 0, 3 } },
    /* 33 */ { MAP_DUNGEON, { 0, 0 }, map_33_ROM_START, map_33_ROM_END, map_33_VRAM,   /* Windward Forest: typed sources */
             gMap33Geometry, gMap33ModelSets, gMap33Submaps, gMap33Lights,
             NULL, gMap33Cameras, gMap33CameraNavmesh, gMap33ActorNavmesh,
             (s32)gMap33EncounterZones, gMap33EncounterSets, NULL, 0, { 0, 0, 0, 3 } },
    /* 34 */ { MAP_DUNGEON, { 0, 0 }, map_34_ROM_START, map_34_ROM_END, map_34_VRAM,   /* World of Mammon: typed sources */
             gMap34Geometry, gMap34ModelSets, gMap34Submaps, gMap34Lights,
             NULL, gMap34Cameras, gMap34CameraNavmesh, gMap34ActorNavmesh,
             (s32)gMap34EncounterZones, gMap34EncounterSets, gMap34NpcList, 0, { 0, 0, 0, 11 } },
    /* 35 */ { MAP_DUNGEON | MAP_FOG_SKY, { 0, 0 }, map_35_ROM_START, map_35_ROM_END, map_35_VRAM,   /* Nepty's lair (underwater: typed sources */
             gMap35Geometry, gMap35ModelSets, gMap35Submaps, gMap35Lights,
             NULL, gMap35Cameras, gMap35CameraNavmesh, gMap35ActorNavmesh,
             0x0, 0x0, NULL, 0, { 0, 0, 0, 1 } },
};
u8* gRegionBlocks[18] = {     /* the extra region blocks, 3 words each: ROM start, end, load address (LoadMap, sMapData.regionBlock) */
    region_0_ROM_START, region_0_ROM_END, region_0_VRAM,   /* region_0 is a code segment linked where it loads */
    region_1_ROM_START, region_1_ROM_END, region_1_VRAM,   /* region_1 is a code segment linked where it loads */
    region_2_ROM_START, region_2_ROM_END, region_2_VRAM,   /* region_2 is a code segment linked where it loads */
    region_3_ROM_START, region_3_ROM_END, region_3_VRAM,   /* region_3 is a code segment linked where it loads */
    region_4_ROM_START, region_4_ROM_END, region_4_VRAM,   /* region_4 is a code segment linked where it loads */
    region_5_ROM_START, region_5_ROM_END, region_5_VRAM,   /* region_5 is a code segment linked where it loads */
};
/* the tile neighbourhoods, (dx, dz) pairs, one table per tile mode: DrawMapFrame visits the camera's tile plus each */
s8 gTileOffsets0[2] = {
    0, 0,
};
s8 gTileOffsets1[314] = {
    -1, -7, 0, -7, 1, -7, -3, -6, -2, -6, -1, -6, 0, -6, 1, -6, 2, -6, 2, -6,
    -4, -5, -3, -5, -2, -5, -1, -5, 0, -5, 1, -5, 2, -5, 3, -5, 4, -5, -5, -4,
    -4, -4, -3, -4, -2, -4, -1, -4, 0, -4, 1, -4, 2, -4, 3, -4, 4, -4, 5, -4,
    -6, -3, -5, -3, -4, -3, -3, -3, -2, -3, -1, -3, 0, -3, 1, -3, 2, -3, 3, -3,
    4, -3, 5, -3, 6, -3, -6, -2, -5, -2, -4, -2, -3, -2, -2, -2, -1, -2, 0, -2,
    1, -2, 2, -2, 3, -2, 4, -2, 5, -2, 6, -2, -7, -1, -6, -1, -5, -1, -4, -1,
    -3, -1, -2, -1, -1, -1, 0, -1, 1, -1, 2, -1, 3, -1, 4, -1, 5, -1, 6, -1,
    7, -1, -7, 0, -6, 0, -5, 0, -4, 0, -3, 0, -2, 0, -1, 0, 0, 0, 1, 0,
    2, 0, 3, 0, 4, 0, 5, 0, 6, 0, 7, 0, -7, 1, -6, 1, -5, 1, -4, 1,
    -3, 1, -2, 1, -1, 1, 0, 1, 1, 1, 2, 1, 3, 1, 4, 1, 5, 1, 6, 1,
    7, 1, -6, 2, -5, 2, -4, 2, -3, 2, -2, 2, -1, 2, 0, 2, 1, 2, 2, 2,
    3, 2, 4, 2, 5, 2, 6, 2, -6, 3, -5, 3, -4, 3, -3, 3, -2, 3, -1, 3,
    0, 3, 1, 3, 2, 3, 3, 3, 4, 3, 5, 3, 6, 3, -5, 4, -4, 4, -3, 4,
    -2, 4, -1, 4, 0, 4, 1, 4, 2, 4, 3, 4, 4, 4, 5, 4, -4, 5, -3, 5,
    -2, 5, -1, 5, 0, 5, 1, 5, 2, 5, 3, 5, 4, 5, -3, 6, -2, 6, -1, 6,
    0, 6, 1, 6, 2, 6, 3, 6, -1, 7, 0, 7, 1, 7,
};
s8 gTileOffsets2[178] = {
    -1, -5, 0, -5, 1, -5, -3, -4, -2, -4, -1, -4, 0, -4, 1, -4, 2, -4, 3, -4,
    -4, -3, -3, -3, -2, -3, -1, -3, 0, -3, 1, -3, 2, -3, 3, -3, 4, -3, -4, -2,
    -3, -2, -2, -2, -1, -2, 0, -2, 1, -2, 2, -2, 3, -2, 4, -2, -5, -1, -4, -1,
    -3, -1, -2, -1, -1, -1, 0, -1, 1, -1, 2, -1, 3, -1, 4, -1, 5, -1, -5, 0,
    -4, 0, -3, 0, -2, 0, -1, 0, 0, 0, 1, 0, 2, 0, 3, 0, 4, 0, 5, 0,
    -5, 1, -4, 1, -3, 1, -2, 1, -1, 1, 0, 1, 1, 1, 2, 1, 3, 1, 4, 1,
    5, 1, -4, 2, -3, 2, -2, 2, -1, 2, 0, 2, 1, 2, 2, 2, 3, 2, 4, 2,
    -4, 3, -3, 3, -2, 3, -1, 3, 0, 3, 1, 3, 2, 3, 3, 3, 4, 3, -3, 4,
    -2, 4, -1, 4, 0, 4, 1, 4, 2, 4, 3, 4, -1, 5, 0, 5, 1, 5,
};
s8 gTileOffsets3[18] = {
    -1, -1, 0, -1, 1, -1, -1, 0, 0, 0, 1, 0, -1, 1, 0, 1, 1, 1,
};
s8 gTileOffsets4[42] = {
    -1, -2, 0, -2, -1, -2, -2, -1, -1, -1, 0, -1, 1, -1, 2, -1, -2, 0, -1, 0,
    0, 0, 1, 0, 2, 0, -2, 1, -1, 1, 0, 1, 1, 1, 2, 1, -1, 2, 0, 2,
    1, 2,
};
/* the doors' animations (StartDoorAnimation): each part's keys, then the part */
static ModelKeyFrame sDoorKeys[11] = {   /* x, y, z, rx, ry, rz, sx, sy, sz, tension, continuity, bias, linear, frame */
    { 0.0f, 128.5f, 0.0f, 0.0f, 0.0f, 0.0f, 14.2857f, 14.2857f, 14.2857f, 0.0f, 0.0f, 0.0f, 0, 0 },
    { 0.0f, 128.5f, 0.0f, 0.0f, 0.0f, 0.0f, 14.2857f, 14.2857f, 14.2857f, 0.0f, 0.0f, 0.0f, 0, 13 },
    { 0.0f, 128.5f, 0.0f, 0.0f, 0.0f, 0.0f, 14.2857f, 14.2857f, 14.2857f, 0.0f, 0.0f, 0.0f, 0, 22 },
    { 0.0f, 128.5f, 0.0f, -8.9999f, 0.0f, 0.0f, 14.2857f, 14.2857f, 14.2857f, 0.0f, 0.0f, 0.0f, 0, 30 },
    { 0.0f, 128.5f, 0.0f, -30.0f, 0.0f, 0.0f, 14.2857f, 14.2857f, 14.2857f, 0.0f, 0.0f, 0.0f, 0, 37 },
    { 0.0f, 128.5f, 0.0f, -45.0f, 0.0f, 0.0f, 14.2857f, 14.2857f, 14.2857f, 0.0f, 0.0f, 0.0f, 0, 43 },
    { 0.0f, 128.5f, 0.0f, -59.7f, 0.0f, 0.0f, 14.2857f, 14.2857f, 14.2857f, 0.0f, 0.0f, 0.0f, 0, 50 },
    { 0.0f, 128.5f, 0.0f, -70.0f, 0.0f, 0.0f, 14.2857f, 14.2857f, 14.2857f, 0.0f, 0.0f, 0.0f, 0, 56 },
    { 0.0f, 128.5f, 0.0f, -65.9999f, 0.0f, 0.0f, 14.2857f, 14.2857f, 14.2857f, 0.0f, 0.0f, 0.0f, 0, 63 },
    { 0.0f, 128.5f, 0.0f, -45.0f, 0.0f, 0.0f, 14.2857f, 14.2857f, 14.2857f, 0.0f, 0.0f, 0.0f, 0, 69 },
    { 0.0f, 128.5f, 0.0f, 0.0f, 0.0f, 0.0f, 14.2857f, 14.2857f, 14.2857f, 0.0f, 0.0f, 0.0f, 0, 81 },
};
ModelPart gDoorAnimTable = { sDoorKeys, 11, 81, { 0.0f, 0.0f, 0.0f }, 0, 0, 0, 0, NULL, 0, { 0, 0 } };   /* door kinds 2 and 3 */
static ModelKeyFrame sDoorKeysUnused[15] = {   /* x, y, z, rx, ry, rz, sx, sy, sz, tension, continuity, bias, linear, frame */
    { 0.0f, 128.5f, 0.0f, 0.0f, 0.0f, 0.0f, 14.2857f, 14.2857f, 14.2857f, 0.0f, 0.0f, 0.0f, 0, 0 },
    { 0.0f, 128.5f, 0.0f, 0.0f, 0.0f, 0.0f, 14.2857f, 14.2857f, 14.2857f, 0.0f, 0.0f, 0.0f, 0, 7 },
    { 0.0f, 128.5f, 0.0f, 0.0f, 0.0f, 0.0f, 14.2857f, 14.2857f, 14.2857f, 0.0f, 0.0f, 0.0f, 0, 13 },
    { 0.0f, 128.5f, 0.0f, 0.0f, 0.0f, 0.0f, 14.2857f, 14.2857f, 14.2857f, 0.0f, 0.0f, 0.0f, 0, 21 },
    { 0.0f, 128.5f, 0.0f, -28.0f, 0.0f, 0.0f, 14.2857f, 14.2857f, 14.2857f, 0.0f, 0.0f, 0.0f, 0, 30 },
    { 0.0f, 128.5f, 0.0f, -38.0f, 0.0f, 0.0f, 14.2857f, 14.2857f, 14.2857f, 0.0f, 0.0f, 0.0f, 0, 35 },
    { 0.0f, 128.5f, 0.0f, -49.5f, 0.0f, 0.0f, 14.2857f, 14.2857f, 14.2857f, 0.0f, 0.0f, 0.0f, 0, 44 },
    { 0.0f, 128.5f, 0.0f, -70.0f, 0.0f, 0.0f, 14.2857f, 14.2857f, 14.2857f, 0.0f, 0.0f, 0.0f, 0, 52 },
    { 0.0f, 128.5f, 0.0f, -73.0f, 0.0f, 0.0f, 14.2857f, 14.2857f, 14.2857f, 0.0f, 0.0f, 0.0f, 0, 58 },
    { 0.0f, 128.5f, 0.0f, -69.0f, 0.0f, 0.0f, 14.2857f, 14.2857f, 14.2857f, 0.0f, 0.0f, 0.0f, 0, 64 },
    { 0.0f, 128.5f, 0.0f, -65.9999f, 0.0f, 0.0f, 14.2857f, 14.2857f, 14.2857f, 0.0f, 0.0f, 0.0f, 0, 71 },
    { 0.0f, 128.5f, 0.0f, -54.0f, 0.0f, 0.0f, 14.2857f, 14.2857f, 14.2857f, 0.0f, 0.0f, 0.0f, 0, 78 },
    { 0.0f, 128.5f, 0.0f, -26.0f, 0.0f, 0.0f, 14.2857f, 14.2857f, 14.2857f, 0.0f, 0.0f, 0.0f, 0, 84 },
    { 0.0f, 128.5f, 0.0f, -2.0f, 0.0f, 0.0f, 14.2857f, 14.2857f, 14.2857f, 0.0f, 0.0f, 0.0f, 0, 91 },
    { 0.0f, 128.5f, 0.0f, 0.0f, 0.0f, 0.0f, 14.2857f, 14.2857f, 14.2857f, 0.0f, 0.0f, 0.0f, 0, 110 },
};
ModelPart D_80057D24 = { sDoorKeysUnused, 15, 110, { 0.0f, 0.0f, 0.0f }, 0, 0, 0, 0, NULL, 0, { 0, 0 } };   /* unreferenced: a part of the same kind */
static ModelKeyFrame sDoorKeysAlt[11] = {   /* x, y, z, rx, ry, rz, sx, sy, sz, tension, continuity, bias, linear, frame */
    { 0.0f, 128.5f, 0.0f, 0.0f, 0.0f, 0.0f, 14.2857f, 14.2857f, 14.2857f, 0.0f, 0.0f, 0.0f, 0, 0 },
    { 0.0f, 128.5f, 0.0f, 0.0f, 0.0f, 0.0f, 14.2857f, 14.2857f, 14.2857f, 0.0f, 0.0f, 1.0f, 0, 21 },
    { 0.0f, 128.5f, 0.0f, -28.0f, 0.0f, 0.0f, 14.2857f, 14.2857f, 14.2857f, 0.0f, 0.0f, 0.0f, 0, 30 },
    { 0.0f, 128.5f, 0.0f, -38.0f, 0.0f, 0.0f, 14.2857f, 14.2857f, 14.2857f, 0.0f, 0.0f, 0.0f, 0, 35 },
    { 0.0f, 128.5f, 0.0f, -49.5f, 0.0f, 0.0f, 14.2857f, 14.2857f, 14.2857f, 0.0f, 0.0f, 0.0f, 0, 44 },
    { 0.0f, 128.5f, 0.0f, -70.0f, 0.0f, 0.0f, 14.2857f, 14.2857f, 14.2857f, 0.0f, 0.0f, 0.0f, 0, 52 },
    { 0.0f, 128.5f, 0.0f, -73.0f, 0.0f, 0.0f, 14.2857f, 14.2857f, 14.2857f, 0.0f, 0.0f, 0.0f, 0, 58 },
    { 0.0f, 128.5f, 0.0f, -69.0f, 0.0f, 0.0f, 14.2857f, 14.2857f, 14.2857f, 0.0f, 0.0f, 0.0f, 0, 64 },
    { 0.0f, 128.5f, 0.0f, -66.0f, 0.0f, 0.0f, 14.2857f, 14.2857f, 14.2857f, 0.0f, 0.0f, 0.0f, 0, 71 },
    { 0.0f, 128.5f, 0.0f, -54.0f, 0.0f, 0.0f, 14.2857f, 14.2857f, 14.2857f, 1.0f, 1.0f, 0.0f, 0, 78 },
    { 0.0f, 128.5f, 0.0f, 0.0f, 0.0f, 0.0f, 14.2857f, 14.2857f, 14.2857f, 0.0f, 0.0f, 0.0f, 0, 100 },
};
ModelPart gDoorAnimAlt = { sDoorKeysAlt, 11, 100, { 0.0f, 0.0f, 0.0f }, 0, 0, 0, 0, NULL, 0, { 0, 0 } };   /* door kinds 2 and 3 when the door's flags' bit 0 is set */
static ModelKeyFrame sDoorKeys4[2] = {   /* x, y, z, rx, ry, rz, sx, sy, sz, tension, continuity, bias, linear, frame */
    { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0, 0 },
    { 12.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0, 30 },
};
ModelPart gDoorAnim4 = { sDoorKeys4, 2, 30, { 0.0f, 0.0f, 0.0f }, 0, 0, 0, 0, NULL, 0, { 0, 0 } };   /* kind 4 */
static ModelKeyFrame sMapModelPathKeys0[3] = {   /* x, y, z, rx, ry, rz, sx, sy, sz, tension, continuity, bias, linear, frame */
    { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0, 0 },
    { 0.0f, 0.0f, 10.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0, 30 },
    { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0, 60 },
};
static ModelKeyFrame sMapModelPathKeys1[4] = {   /* x, y, z, rx, ry, rz, sx, sy, sz, tension, continuity, bias, linear, frame */
    { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0, 0 },
    { 0.0f, 10.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0, 15 },
    { 0.0f, -10.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0, 45 },
    { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0, 60 },
};
static ModelKeyFrame sMapModelPathKeys2[5] = {   /* x, y, z, rx, ry, rz, sx, sy, sz, tension, continuity, bias, linear, frame */
    { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0, 0 },
    { -5.0f, 0.0f, -5.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0, 20 },
    { 0.0f, 0.0f, -10.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0, 40 },
    { 5.0f, 0.0f, -5.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0, 60 },
    { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0, 80 },
};
static ModelKeyFrame sMapModelPathKeys3[5] = {   /* x, y, z, rx, ry, rz, sx, sy, sz, tension, continuity, bias, linear, frame */
    { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0, 0 },
    { -5.0f, 5.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0, 20 },
    { 0.0f, 10.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0, 40 },
    { 5.0f, 5.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0, 60 },
    { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0, 80 },
};
ModelPart gMapModelPaths[4] = {   /* the animated models' paths (DrawMapFrame) */
    { sMapModelPathKeys0, 3, 60, { 0.0f, 0.0f, 0.0f }, 0, 0, 0, 0, NULL, 0, { 0, 0 } },
    { sMapModelPathKeys1, 4, 60, { 0.0f, 0.0f, 0.0f }, 0, 0, 0, 0, NULL, 0, { 0, 0 } },
    { sMapModelPathKeys2, 5, 80, { 0.0f, 0.0f, 0.0f }, 0, 0, 0, 0, NULL, 0, { 0, 0 } },
    { sMapModelPathKeys3, 5, 80, { 0.0f, 0.0f, 0.0f }, 0, 0, 0, 0, NULL, 0, { 0, 0 } },
};
/* the flames' art: per kind a 256-colour palette, then its 8 frames at 0x400 + 0x600 n; read from the ROM at build time */
GLOBAL_ASM(
.section .data
glabel gFlameTextures0
.incbin "baserom.jp.z64", 0x5901C, 0x3000
glabel gFlameTextures1
.incbin "baserom.jp.z64", 0x5C01C, 0x3000
)
static Vtx sFlameQuad0[4] = {
    { { { 8, 23, 0 }, 0, { 960, 0 }, { 0, 0, 0, 0 } } },
    { { { -8, 23, 0 }, 0, { 0, 0 }, { 0, 0, 0, 0 } } },
    { { { 8, 0, 0 }, 0, { 960, 1984 }, { 0, 0, 0, 0 } } },
    { { { -8, 0, 0 }, 0, { 0, 1984 }, { 0, 0, 0, 0 } } },
};
static Vtx sFlameQuad1[4] = {
    { { { 8, 32, 0 }, 0, { 960, 0 }, { 0, 0, 0, 0 } } },
    { { { -8, 32, 0 }, 0, { 0, 0 }, { 0, 0, 0, 0 } } },
    { { { 8, 0, 0 }, 0, { 960, 1984 }, { 0, 0, 0, 0 } } },
    { { { -8, 0, 0 }, 0, { 0, 1984 }, { 0, 0, 0, 0 } } },
};
static Gfx sFlameSetup0[13] = {   /* flame kind 0x11: its palette */
    gsDPPipeSync(),
    gsSPClearGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_SHADING_SMOOTH),
    gsSPSetGeometryMode(G_ZBUFFER | G_SHADE),
    gsDPSetTextureLUT(G_TT_RGBA16),
    gsDPSetCombineLERP(0, 0, 0, TEXEL0, TEXEL0, 0, ENVIRONMENT, 0, 0, 0, 0, COMBINED, 0, 0, 0, COMBINED),
    gsSPTexture(0x8000, 0x8000, 0, G_TX_RENDERTILE, G_ON),
    gsDPLoadTLUT_pal256(gFlameTextures0),
    gsSPEndDisplayList(),
};
static Gfx sFlameSetup1[13] = {   /* flame kind 0x12 */
    gsDPPipeSync(),
    gsSPClearGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_SHADING_SMOOTH),
    gsSPSetGeometryMode(G_ZBUFFER | G_SHADE),
    gsDPSetTextureLUT(G_TT_RGBA16),
    gsDPSetCombineLERP(0, 0, 0, TEXEL0, TEXEL0, 0, ENVIRONMENT, 0, 0, 0, 0, COMBINED, 0, 0, 0, COMBINED),
    gsSPTexture(0x8000, 0x8000, 0, G_TX_RENDERTILE, G_ON),
    gsDPLoadTLUT_pal256(gFlameTextures1),
    gsSPEndDisplayList(),
};
static Gfx sFlameModel0[3] = {   /* its quad */
    gsSPVertex(sFlameQuad0, 4, 0),
    gsSP2Triangles(0, 1, 2, 0, 1, 3, 2, 0),
    gsSPEndDisplayList(),
};
static Gfx sFlameModel1[3] = {   /* its quad */
    gsSPVertex(sFlameQuad1, 4, 0),
    gsSP2Triangles(0, 1, 2, 0, 1, 3, 2, 0),
    gsSPEndDisplayList(),
};
MapPropGfx gFlameModelLists[2] = { { sFlameSetup0, sFlameModel0 }, { sFlameSetup1, sFlameModel1 } };   /* the flame kinds' setup and model, [kind - 0x11] */
static u8* sFlameFrames0[8] = {
    gFlameTextures0 + 0x400, gFlameTextures0 + 0xA00, gFlameTextures0 + 0x1000, gFlameTextures0 + 0x1600,
    gFlameTextures0 + 0x1C00, gFlameTextures0 + 0x2200, gFlameTextures0 + 0x2800, gFlameTextures0 + 0x2E00,
};
static u8* sFlameFrames1[8] = {
    gFlameTextures1 + 0x400, gFlameTextures1 + 0xA00, gFlameTextures1 + 0x1000, gFlameTextures1 + 0x1600,
    gFlameTextures1 + 0x1C00, gFlameTextures1 + 0x2200, gFlameTextures1 + 0x2800, gFlameTextures1 + 0x2E00,
};
u8** gFlameFrameTables[2] = { sFlameFrames0, sFlameFrames1 };   /* their 8 texture frames, [kind - 0x11] (DrawFlames) */
/* the map dust's art: its 256-colour palette area (0x400), then its 8 textures of 0x400 (UpdateDustEmitter picks one per object); read from the ROM at build time */
GLOBAL_ASM(
.section .data
glabel gMapDustPalette
.incbin "baserom.jp.z64", 0x5F1F8, 0x400
glabel gMapDustTextures
.incbin "baserom.jp.z64", 0x5F5F8, 0x2000
)
static Vtx sMapDustQuad[4] = {
    { { { 1, 1, 1 }, 0, { 1984, 0 }, { 0, 0, 0, 0 } } },
    { { { -1, 1, 1 }, 0, { 0, 0 }, { 0, 0, 0, 0 } } },
    { { { 1, -1, 1 }, 0, { 1984, 1984 }, { 0, 0, 0, 0 } } },
    { { { -1, -1, 1 }, 0, { 0, 1984 }, { 0, 0, 0, 0 } } },
};
Gfx gMapDustGfx[16] = {   /* the map dust's display list (gMapDustTemplate) */
    gsDPPipeSync(),
    gsDPSetCycleType(G_CYC_2CYCLE),
    gsDPSetRenderMode(G_RM_PASS, G_RM_ZB_XLU_SURF2),
    gsSPClearGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_SHADING_SMOOTH),
    gsSPSetGeometryMode(G_ZBUFFER | G_SHADE),
    gsDPSetCombineLERP(0, 0, 0, TEXEL0, TEXEL0, 0, ENVIRONMENT, 0, 0, 0, 0, COMBINED, 0, 0, 0, COMBINED),
    gsSPTexture(0x8000, 0x8000, 0, G_TX_RENDERTILE, G_ON),
    gsDPLoadTLUT_pal256(gMapDustPalette),
    gsSPVertex(sMapDustQuad, 4, 0),
    gsSP2Triangles(0, 1, 2, 0, 1, 3, 2, 0),
    gsSPEndDisplayList(),
};
/* the second dust's texture (gDustEmitterTemplate); read from the ROM at build time */
GLOBAL_ASM(
.section .data
glabel gDustTexture2
.incbin "baserom.jp.z64", 0x616B8, 0x100
)
static Vtx sDustQuad2[4] = {
    { { { 1, 2, 1 }, 0, { 448, 0 }, { 0, 0, 0, 0 } } },
    { { { -1, 2, 1 }, 0, { 0, 0 }, { 0, 0, 0, 0 } } },
    { { { 1, -2, 1 }, 0, { 448, 960 }, { 0, 0, 0, 0 } } },
    { { { -1, -2, 1 }, 0, { 0, 960 }, { 0, 0, 0, 0 } } },
};
Gfx gDustGfx2[10] = {   /* its display list */
    gsDPPipeSync(),
    gsDPSetCycleType(G_CYC_2CYCLE),
    gsDPSetRenderMode(G_RM_PASS, G_RM_ZB_XLU_SURF2),
    gsSPClearGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_SHADING_SMOOTH),
    gsSPSetGeometryMode(G_ZBUFFER | G_SHADE),
    gsDPSetCombineLERP(0, 0, 0, TEXEL0, TEXEL0, 0, ENVIRONMENT, 0, 0, 0, 0, COMBINED, 0, 0, 0, COMBINED),
    gsSPTexture(0x8000, 0x8000, 0, G_TX_RENDERTILE, G_ON),
    gsSPVertex(sDustQuad2, 4, 0),
    gsSP2Triangles(0, 1, 2, 0, 1, 3, 2, 0),
    gsSPEndDisplayList(),
};
SkyRecord gSkyRecords[8] = {   /* the skies: the ROM block LoadMap reads (0x800 palettes, then 0x4000 texture), the display list in segment 1 that DrawSky runs */
    { sky_0_ROM_START, (Gfx*)0x01001300 },
    { sky_1_ROM_START, (Gfx*)0x01001300 },
    { sky_2_ROM_START, (Gfx*)0x01002410 },
    { sky_3_ROM_START, (Gfx*)0x01003630 },
    { sky_4_ROM_START, (Gfx*)0x01003630 },
    { sky_5_ROM_START, (Gfx*)0x01003630 },
    { sky_6_ROM_START, (Gfx*)0x01003630 },
    { sky_7_ROM_START, (Gfx*)0x01003630 },
};
u8 gPropAlpha[36] = {   /* per map: the props' alpha (DrawMapProps) */
    200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200,
    200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200,
    200, 200, 120, 120, 120, 120, 120, 150, 150, 150, 120, 120,
};
