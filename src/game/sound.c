#include "common.h"
#include "game/sound.h"
#include "game/audio.h"
/* The music and sound requests (InitializeSoundData to SetBGMVolume), and after the code main's game tables (this
   file's .data). The Start menu that followed the sound code in this unit is its own file, start_menu.c: this file's
   .bss ends eight bytes before the menu's, as a .bss padded to sixteen does, and the menu's first function starts
   on a sixteen-byte boundary. */
extern u8 gSoundVolumes[];
/* ---- this file's .bss (0x8008FAF0-0x8008FB27), in address order; the object pads it to 0x8008FB30 ---- */
s8 gMusicPlaying;                 /* the music playing, -1 = none */
s8 gNextMusic;                    /* the music to play next (RequestMusic) */
u16 gMusicChangeFlags;            /* 1 change requested, 2 stop the current, 4 start the next */
u8 gSoundVolume;
u16 gMusicChangeDelay;
DelayedSound gDelayedSounds[16];  /* sounds waiting for their delay (PlaySound) */
/* background music by place: 0x48 entries (FindMapBGM); room -1 = any room of the map */
typedef struct {
    s16 map;
    s16 room;
    s8 song;
    s8 pad;
} BgmEntry;
extern BgmEntry gMapBgmTable[];

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
extern u8 gMusicVolumeLevels[];
extern s8 gMusicVolumeSetting;
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
extern u8 brian_anim_00_ROM_START[], brian_anim_00_ROM_END[];
extern u8 brian_anim_01_ROM_START[], brian_anim_01_ROM_END[];
extern u8 brian_anim_02_ROM_START[], brian_anim_02_ROM_END[];
extern u8 brian_anim_03_ROM_START[], brian_anim_03_ROM_END[];
extern u8 brian_anim_04_ROM_START[], brian_anim_04_ROM_END[];
extern u8 brian_anim_05_ROM_START[], brian_anim_05_ROM_END[];
extern u8 brian_anim_06_ROM_START[], brian_anim_06_ROM_END[];
extern u8 brian_anim_07_ROM_START[], brian_anim_07_ROM_END[];
extern u8 brian_anim_08_ROM_START[], brian_anim_08_ROM_END[];
extern u8 brian_anim_09_ROM_START[], brian_anim_09_ROM_END[];
extern u8 brian_anim_10_ROM_START[], brian_anim_10_ROM_END[];
extern u8 brian_anim_11_ROM_START[], brian_anim_11_ROM_END[];
extern u8 brian_anim_12_ROM_START[], brian_anim_12_ROM_END[];
extern u8 brian_anim_13_ROM_START[], brian_anim_13_ROM_END[];
extern u8 brian_anim_14_ROM_START[], brian_anim_14_ROM_END[];
extern u8 brian_anim_15_ROM_START[], brian_anim_15_ROM_END[];
extern u8 brian_anim_16_ROM_START[], brian_anim_16_ROM_END[];
extern u8 brian_anim_17_ROM_START[], brian_anim_17_ROM_END[];
extern u8 brian_anim_18_ROM_START[], brian_anim_18_ROM_END[];
extern u8 brian_anim_19_ROM_START[], brian_anim_19_ROM_END[];
extern u8 brian_anim_20_ROM_START[], brian_anim_20_ROM_END[];
extern u8 brian_anim_21_ROM_START[], brian_anim_21_ROM_END[];
extern u8 brian_anim_22_ROM_START[], brian_anim_22_ROM_END[];
extern u8 brian_anim_23_ROM_START[], brian_anim_23_ROM_END[];
extern u8 brian_anim_24_ROM_START[], brian_anim_24_ROM_END[];
extern u8 brian_anim_25_ROM_START[], brian_anim_25_ROM_END[];
extern u8 brian_anim_26_ROM_START[], brian_anim_26_ROM_END[];
extern u8 brian_anim_27_ROM_START[], brian_anim_27_ROM_END[];
extern u8 brian_anim_28_ROM_START[], brian_anim_28_ROM_END[];
extern u8 brian_anim_29_ROM_START[], brian_anim_29_ROM_END[];
extern u8 monsters_0_ROM_START[], monsters_0_ROM_END[];
extern u8 monsters_1_ROM_START[], monsters_1_ROM_END[];
extern u8 monsters_2_ROM_START[], monsters_2_ROM_END[];
extern u8 monsters_3_ROM_START[], monsters_3_ROM_END[];
extern u8 monsters_4_ROM_START[], monsters_4_ROM_END[];
extern u8 monsters_5_ROM_START[], monsters_5_ROM_END[];
extern u8 npc_blob_0_ROM_START[], npc_blob_0_ROM_END[];
extern u8 npc_blob_1_ROM_START[], npc_blob_1_ROM_END[];
extern u8 npc_blob_2_ROM_START[], npc_blob_2_ROM_END[];
extern u8 npc_blob_3_ROM_START[], npc_blob_3_ROM_END[];
extern u8 npc_blob_4_ROM_START[], npc_blob_4_ROM_END[];
extern u8 npc_blob_5_ROM_START[], npc_blob_5_ROM_END[];
extern u8 npc_blob_6_ROM_START[], npc_blob_6_ROM_END[];

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
AnimEntry* gBrianAnims[1] = {        /* his animation table: at 0x64 in his model as InitBrian loads it (to 0x80206000) */
    (AnimEntry*)0x80206064,
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
MonsterSet gMonsterSets[6] = {       /* each monster area's file in ROM; its animation table, base stats and names as LoadMonsterSet loads the file to gMonsterFileBuffer (0x8020E6F0) */
    { monsters_0_ROM_START, monsters_0_ROM_END, (AnimEntry**)0x80270890, (MonsterBaseData*)0x802705F0, (u8**)0x80270934 },
    { monsters_1_ROM_START, monsters_1_ROM_END, (AnimEntry**)0x80295990, (MonsterBaseData*)0x80295648, (u8**)0x80295A6C },
    { monsters_2_ROM_START, monsters_2_ROM_END, (AnimEntry**)0x80269010, (MonsterBaseData*)0x80268DA8, (u8**)0x8026909C },
    { monsters_3_ROM_START, monsters_3_ROM_END, (AnimEntry**)0x80281FD0, (MonsterBaseData*)0x80281CF8, (u8**)0x80282088 },
    { monsters_4_ROM_START, monsters_4_ROM_END, (AnimEntry**)0x80278C10, (MonsterBaseData*)0x80278970, (u8**)0x80278CB0 },
    { monsters_5_ROM_START, monsters_5_ROM_END, (AnimEntry**)0x80236A38, (MonsterBaseData*)0x80236958, (u8**)0x80236A70 },
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
/* the enemies' effect stages, 16 bytes each: model (in gEffectModels), colour, duration, unk6, size x, size y. The boss
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
/* the ending's NPC text (SetupNPCs hands it to StartEndingText), then zero bytes to 0x80056BB0: read from the ROM at build time */
GLOBAL_ASM(
.section .data
glabel gEndingNpcText
.incbin "baserom.jp.z64", 0x574B8, 0x2F8
)
extern u8 map_00_ROM_START[], map_00_ROM_END[];
extern u8 map_01_ROM_START[], map_01_ROM_END[];
extern u8 map_02_ROM_START[], map_02_ROM_END[];
extern u8 map_03_ROM_START[], map_03_ROM_END[];
extern u8 map_04_ROM_START[], map_04_ROM_END[];
extern u8 map_05_ROM_START[], map_05_ROM_END[];
extern u8 map_06_ROM_START[], map_06_ROM_END[];
extern u8 map_07_ROM_START[], map_07_ROM_END[];
extern u8 map_08_ROM_START[], map_08_ROM_END[];
extern u8 map_09_ROM_START[], map_09_ROM_END[];
extern u8 map_10_ROM_START[], map_10_ROM_END[];
extern u8 map_11_ROM_START[], map_11_ROM_END[];
extern u8 map_12_ROM_START[], map_12_ROM_END[];
extern u8 map_13_ROM_START[], map_13_ROM_END[];
extern u8 map_14_ROM_START[], map_14_ROM_END[];
extern u8 map_15_ROM_START[], map_15_ROM_END[];
extern u8 map_16_ROM_START[], map_16_ROM_END[];
extern u8 map_17_ROM_START[], map_17_ROM_END[];
extern u8 map_18_ROM_START[], map_18_ROM_END[];
extern u8 map_19_ROM_START[], map_19_ROM_END[];
extern u8 map_20_ROM_START[], map_20_ROM_END[];
extern u8 map_21_ROM_START[], map_21_ROM_END[];
extern u8 map_22_ROM_START[], map_22_ROM_END[];
extern u8 map_23_ROM_START[], map_23_ROM_END[];
extern u8 map_24_ROM_START[], map_24_ROM_END[];
extern u8 map_25_ROM_START[], map_25_ROM_END[];
extern u8 map_26_ROM_START[], map_26_ROM_END[];
extern u8 map_27_ROM_START[], map_27_ROM_END[];
extern u8 map_28_ROM_START[], map_28_ROM_END[];
extern u8 map_29_ROM_START[], map_29_ROM_END[];
extern u8 map_30_ROM_START[], map_30_ROM_END[];
extern u8 map_31_ROM_START[], map_31_ROM_END[];
extern u8 map_32_ROM_START[], map_32_ROM_END[];
extern u8 map_33_ROM_START[], map_33_ROM_END[];
extern u8 map_34_ROM_START[], map_34_ROM_END[];
extern u8 map_35_ROM_START[], map_35_ROM_END[];
extern u8 region_0_ROM_START[], region_0_ROM_END[];
extern u8 region_1_ROM_START[], region_1_ROM_END[];
extern u8 region_2_ROM_START[], region_2_ROM_END[];
extern u8 region_3_ROM_START[], region_3_ROM_END[];
extern u8 region_4_ROM_START[], region_4_ROM_END[];
extern u8 region_5_ROM_START[], region_5_ROM_END[];
extern u8 sky_0_ROM_START[], sky_0_ROM_END[];
extern u8 sky_1_ROM_START[], sky_1_ROM_END[];
extern u8 sky_2_ROM_START[], sky_2_ROM_END[];
extern u8 sky_3_ROM_START[], sky_3_ROM_END[];
extern u8 sky_4_ROM_START[], sky_4_ROM_END[];
extern u8 sky_5_ROM_START[], sky_5_ROM_END[];
extern u8 sky_6_ROM_START[], sky_6_ROM_END[];
extern u8 sky_7_ROM_START[], sky_7_ROM_END[];


/* -- the maps' tables (read by map.c) -- */
extern u8 gFlameTextures0[], gFlameTextures1[], gMapDustPalette[], gMapDustTextures[], gDustTexture2[];
sMapData gMapTable[36] = {   /* flags, ROM block, load address, then pointers into the map as loaded, encounters, unk34, NPC list, region block, unk40 */
    /*  0 */ { 0x81, { 0, 0 }, map_00_ROM_START, map_00_ROM_END, (void*)0x8010FE00,
             (ModelGeometry*)0x8014F640, (MapModelData*)0x8014FB80, (Submap*)0x8014FF78, (MapLight*)0x8015084C,
             (MapPropGfx*)0x80150290, (void*)0x80150518, (NavMeshData*)0x80154810, (NavMeshData*)0x801547BC,
             0x0, 0x0, (sMapList*)0x80154A90, 0, { 0, 0, 0, 5 } },
    /*  1 */ { 0x1, { 0, 0 }, map_01_ROM_START, map_01_ROM_END, (void*)0x8010FE00,
             (ModelGeometry*)0x80140430, (MapModelData*)0x80140790, (Submap*)0x80140A28, (MapLight*)0x80140F3C,
             (MapPropGfx*)0x80140C20, (void*)0x80140D10, (NavMeshData*)0x80141CA0, (NavMeshData*)0x80141C8C,
             0x0, 0x0, (sMapList*)0x80141D44, 1, { 0, 0, 0, 1 } },
    /*  2 */ { 0x4, { 0, 0 }, map_02_ROM_START, map_02_ROM_END, (void*)0x8010FE00,
             (ModelGeometry*)0x80144464, (MapModelData*)0x80144710, (Submap*)0x80144858, (MapLight*)0x80144CFC,
             (MapPropGfx*)0x80144A50, (void*)0x80144AD0, (NavMeshData*)0x80145D70, (NavMeshData*)0x80145D54,
             0x80146128, 0x80146130, NULL, 1, { 0, 0, 0, 1 } },
    /*  3 */ { 0x4, { 0, 0 }, map_03_ROM_START, map_03_ROM_END, (void*)0x8010FE00,
             (ModelGeometry*)0x8013C564, (MapModelData*)0x8013C870, (Submap*)0x8013C9F0, (MapLight*)0x8013CEEC,
             (MapPropGfx*)0x8013CC00, (void*)0x8013CCB8, (NavMeshData*)0x8013E190, (NavMeshData*)0x8013E168,
             0x8013E460, 0x8013E468, (sMapList*)0x8013E4DC, 1, { 0, 0, 0, 2 } },
    /*  4 */ { 0x1, { 0, 0 }, map_04_ROM_START, map_04_ROM_END, (void*)0x8010FE00,
             (ModelGeometry*)0x80149C98, (MapModelData*)0x8014A020, (Submap*)0x8014A3F8, (MapLight*)0x8014AAFC,
             (MapPropGfx*)0x8014A6B0, (void*)0x8014A8C8, (NavMeshData*)0x8014C1C0, (NavMeshData*)0x8014C190,
             0x0, 0x0, (sMapList*)0x8014C2A4, 2, { 0, 0, 0, 3 } },
    /*  5 */ { 0x4, { 0, 0 }, map_05_ROM_START, map_05_ROM_END, (void*)0x8010FE00,
             (ModelGeometry*)0x80157D98, (MapModelData*)0x801582C0, (Submap*)0x80158518, (MapLight*)0x80158A1C,
             (MapPropGfx*)0x80158730, (void*)0x801587E8, (NavMeshData*)0x8015A9C0, (NavMeshData*)0x8015A99C,
             0x8015AD2C, 0x8015AD34, (sMapList*)0x8015ADAC, 2, { 0, 0, 0, 2 } },
    /*  6 */ { 0x1, { 0, 0 }, map_06_ROM_START, map_06_ROM_END, (void*)0x8010FE00,
             (ModelGeometry*)0x80133438, (MapModelData*)0x80133720, (Submap*)0x80133948, (MapLight*)0x80133DEC,
             (MapPropGfx*)0x80133B40, (void*)0x80133BC0, (NavMeshData*)0x801348B0, (NavMeshData*)0x80134898,
             0x0, 0x0, (sMapList*)0x80134974, 2, { 0, 0, 0, 1 } },
    /*  7 */ { 0x4, { 0, 0 }, map_07_ROM_START, map_07_ROM_END, (void*)0x8010FE00,
             (ModelGeometry*)0x8013F1FC, (MapModelData*)0x8013F500, (Submap*)0x8013F6B8, (MapLight*)0x8013FBBC,
             (MapPropGfx*)0x8013F8D0, (void*)0x8013F988, (NavMeshData*)0x80141550, (NavMeshData*)0x80141528,
             0x801419CC, 0x801419D4, (sMapList*)0x80141A4C, 3, { 0, 0, 0, 2 } },
    /*  8 */ { 0x1, { 0, 0 }, map_08_ROM_START, map_08_ROM_END, (void*)0x8010FE00,
             (ModelGeometry*)0x8015B89C, (MapModelData*)0x8015BD20, (Submap*)0x8015C090, (MapLight*)0x8015C5FC,
             (MapPropGfx*)0x8015C270, (void*)0x8015C3D0, (NavMeshData*)0x8015DB50, (NavMeshData*)0x8015DB28,
             0x0, 0x0, (sMapList*)0x8015DC8C, 3, { 0, 0, 0, 2 } },
    /*  9 */ { 0x4, { 0, 0 }, map_09_ROM_START, map_09_ROM_END, (void*)0x8010FE00,
             (ModelGeometry*)0x8015A824, (MapModelData*)0x8015AE10, (Submap*)0x8015B230, (MapLight*)0x8015B7AC,
             (MapPropGfx*)0x8015B4F0, (void*)0x8015B580, (NavMeshData*)0x8015E870, (NavMeshData*)0x8015E860,
             0x8015F1A8, 0x8015F1B0, NULL, 4, { 0, 0, 0, 1 } },
    /* 10 */ { 0x4, { 0, 0 }, map_10_ROM_START, map_10_ROM_END, (void*)0x8010FE00,
             (ModelGeometry*)0x801343D8, (MapModelData*)0x801345C0, (Submap*)0x80134738, (MapLight*)0x80134CEC,
             (MapPropGfx*)0x80134920, (void*)0x80134AB8, (NavMeshData*)0x80135DB0, (NavMeshData*)0x80135D90,
             0x0, 0x0, NULL, 4, { 0, 0, 0, 2 } },
    /* 11 */ { 0x4, { 0, 0 }, map_11_ROM_START, map_11_ROM_END, (void*)0x8010FE00,
             (ModelGeometry*)0x80169190, (MapModelData*)0x80169610, (Submap*)0x80169CE8, (MapLight*)0x8016A510,
             NULL, (void*)0x8016A018, (NavMeshData*)0x8016D6D8, (NavMeshData*)0x8016D57C,
             0x8016D9C4, 0x8016D9DC, (sMapList*)0x8016DA7C, 4, { 0, 0, 0, 8 } },
    /* 12 */ { 0x4, { 0, 0 }, map_12_ROM_START, map_12_ROM_END, (void*)0x8010FE00,
             (ModelGeometry*)0x80125DA4, (MapModelData*)0x80126010, (Submap*)0x801261D4, (MapLight*)0x801267BC,
             (MapPropGfx*)0x801264C0, (void*)0x80126588, (NavMeshData*)0x80127800, (NavMeshData*)0x801277CC,
             0x8012796C, 0x80127974, (sMapList*)0x801279EC, 5, { 0, 0, 0, 3 } },
    /* 13 */ { 0x2, { 0, 0 }, map_13_ROM_START, map_13_ROM_END, (void*)0x801539F0,
             (ModelGeometry*)0x80194B14, (MapModelData*)0x80195480, (Submap*)0x80195E98, (MapLight*)0x801967F8,
             (MapPropGfx*)0x80196108, (void*)0x80196510, (NavMeshData*)0x80198F84, (NavMeshData*)0x80198980,
             0x0, 0x0, (sMapList*)0x80199330, 0, { 0, 0, 0, 18 } },
    /* 14 */ { 0x2, { 0, 0 }, map_14_ROM_START, map_14_ROM_END, (void*)0x801539F0,
             (ModelGeometry*)0x8019DFA0, (MapModelData*)0x8019EC80, (Submap*)0x8019F7E0, (MapLight*)0x8019FFF0,
             NULL, (void*)0x8019FD88, (NavMeshData*)0x801A3900, (NavMeshData*)0x801A3590,
             0x0, 0x0, (sMapList*)0x801A3D7C, 1, { 0, 0, 0, 16 } },
    /* 15 */ { 0x2, { 0, 0 }, map_15_ROM_START, map_15_ROM_END, (void*)0x801539F0,
             (ModelGeometry*)0x80166974, (MapModelData*)0x80167120, (Submap*)0x80167738, (MapLight*)0x80167EF0,
             (MapPropGfx*)0x80167948, (void*)0x80167C98, (NavMeshData*)0x801696A0, (NavMeshData*)0x80169530,
             0x0, 0x0, (sMapList*)0x80169980, 0, { 0, 0, 0, 14 } },
    /* 16 */ { 0x2, { 0, 0 }, map_16_ROM_START, map_16_ROM_END, (void*)0x801539F0,
             (ModelGeometry*)0x8017DA08, (MapModelData*)0x8017E510, (Submap*)0x8017EF80, (MapLight*)0x8017FB58,
             (MapPropGfx*)0x8017F300, (void*)0x8017F850, (NavMeshData*)0x8018302C, (NavMeshData*)0x80182D34,
             0x0, 0x0, (sMapList*)0x8018348C, 1, { 0, 0, 0, 22 } },
    /* 17 */ { 0x2, { 0, 0 }, map_17_ROM_START, map_17_ROM_END, (void*)0x801539F0,
             (ModelGeometry*)0x80168D4C, (MapModelData*)0x801691F0, (Submap*)0x80169680, (MapLight*)0x80169BD0,
             (MapPropGfx*)0x801697F8, (void*)0x80169AA0, (NavMeshData*)0x8016A978, (NavMeshData*)0x8016A75C,
             0x0, 0x0, (sMapList*)0x8016ABA4, 2, { 0, 0, 0, 8 } },
    /* 18 */ { 0x2, { 0, 0 }, map_18_ROM_START, map_18_ROM_END, (void*)0x801539F0,
             (ModelGeometry*)0x801621D4, (MapModelData*)0x80162940, (Submap*)0x80162C10, (MapLight*)0x801630B0,
             NULL, (void*)0x80162E90, (NavMeshData*)0x80164180, (NavMeshData*)0x801640FC,
             0x0, 0x0, (sMapList*)0x801643B8, 2, { 0, 0, 0, 8 } },
    /* 19 */ { 0x2, { 0, 0 }, map_19_ROM_START, map_19_ROM_END, (void*)0x801539F0,
             (ModelGeometry*)0x8015DE28, (MapModelData*)0x8015E010, (Submap*)0x8015E1A0, (MapLight*)0x8015E468,
             NULL, (void*)0x8015E2E0, (NavMeshData*)0x8015EA70, (NavMeshData*)0x8015EA28,
             0x0, 0x0, NULL, 2, { 0, 0, 0, 4 } },
    /* 20 */ { 0x2, { 0, 0 }, map_20_ROM_START, map_20_ROM_END, (void*)0x801539F0,
             (ModelGeometry*)0x8016B158, (MapModelData*)0x8016B920, (Submap*)0x8016BEE8, (MapLight*)0x8016C500,
             (MapPropGfx*)0x8016C0C8, (void*)0x8016C3A8, (NavMeshData*)0x8016DE70, (NavMeshData*)0x8016DD1C,
             0x0, 0x0, (sMapList*)0x8016E0BC, 2, { 0, 0, 0, 12 } },
    /* 21 */ { 0x2, { 0, 0 }, map_21_ROM_START, map_21_ROM_END, (void*)0x801539F0,
             (ModelGeometry*)0x801A2518, (MapModelData*)0x801A2CE0, (Submap*)0x801A34B8, (MapLight*)0x801A3B08,
             NULL, (void*)0x801A3938, (NavMeshData*)0x801A6A68, (NavMeshData*)0x801A6644,
             0x0, 0x0, (sMapList*)0x801A6E68, 3, { 0, 0, 0, 13 } },
    /* 22 */ { 0x2, { 0, 0 }, map_22_ROM_START, map_22_ROM_END, (void*)0x801539F0,
             (ModelGeometry*)0x8017DA18, (MapModelData*)0x8017E500, (Submap*)0x8017F0A8, (MapLight*)0x8017FDF8,
             (MapPropGfx*)0x8017F3D8, (void*)0x8017F9D8, (NavMeshData*)0x80182FCC, (NavMeshData*)0x80182D28,
             0x0, 0x0, (sMapList*)0x80183594, 3, { 0, 0, 0, 26 } },
    /* 23 */ { 0x2, { 0, 0 }, map_23_ROM_START, map_23_ROM_END, (void*)0x801539F0,
             (ModelGeometry*)0x80186F78, (MapModelData*)0x801879E0, (Submap*)0x801886D8, (MapLight*)0x80189368,
             (MapPropGfx*)0x80188A08, (void*)0x80189040, (NavMeshData*)0x8018C910, (NavMeshData*)0x8018C644,
             0x0, 0x0, (sMapList*)0x8018D010, 4, { 0, 0, 0, 26 } },
    /* 24 */ { 0x2, { 0, 0 }, map_24_ROM_START, map_24_ROM_END, (void*)0x801539F0,
             (ModelGeometry*)0x80164FB0, (MapModelData*)0x80165070, (Submap*)0x80165250, (MapLight*)0x80165538,
             NULL, (void*)0x801653B8, (NavMeshData*)0x80166464, (NavMeshData*)0x801661A0,
             0x0, 0x0, (sMapList*)0x8016669C, 4, { 0, 0, 0, 3 } },
    /* 25 */ { 0x2, { 0, 0 }, map_25_ROM_START, map_25_ROM_END, (void*)0x801539F0,
             (ModelGeometry*)0x8016EEE0, (MapModelData*)0x8016F360, (Submap*)0x8016F860, (MapLight*)0x80170018,
             (MapPropGfx*)0x8016FAC0, (void*)0x8016FD70, (NavMeshData*)0x801716D0, (NavMeshData*)0x8017162C,
             0x0, 0x0, (sMapList*)0x801718D0, 5, { 0, 0, 0, 10 } },
    /* 26 */ { 0x88, { 0, 0 }, map_26_ROM_START, map_26_ROM_END, (void*)0x8010FE00,
             (ModelGeometry*)0x801649D8, (MapModelData*)0x80165460, (Submap*)0x80165A34, (MapLight*)0x80165EB8,
             NULL, (void*)0x80165C38, (NavMeshData*)0x8016D480, (NavMeshData*)0x8016D430,
             0x8016DAC0, 0x8016DAC8, (sMapList*)0x8016DB98, 0, { 0, 0, 0, 5 } },
    /* 27 */ { 0x88, { 0, 0 }, map_27_ROM_START, map_27_ROM_END, (void*)0x8010FE00,
             (ModelGeometry*)0x80168D08, (MapModelData*)0x8016A590, (Submap*)0x8016A668, (MapLight*)0x8016A778,
             NULL, (void*)0x8016A6F0, (NavMeshData*)0x8016E920, (NavMeshData*)0x8016E910,
             0x8016EFD8, 0x8016EFE0, NULL, 0, { 0, 0, 0, 1 } },
    /* 28 */ { 0x88, { 0, 0 }, map_28_ROM_START, map_28_ROM_END, (void*)0x8010FE00,
             (ModelGeometry*)0x80163770, (MapModelData*)0x80164C90, (Submap*)0x80164D30, (MapLight*)0x80164EE8,
             NULL, (void*)0x80164E68, (NavMeshData*)0x80169C70, (NavMeshData*)0x80169C5C,
             0x8016A0B4, 0x8016A0BC, NULL, 0, { 0, 0, 0, 1 } },
    /* 29 */ { 0x88, { 0, 0 }, map_29_ROM_START, map_29_ROM_END, (void*)0x8010FE00,
             (ModelGeometry*)0x8012FC8C, (MapModelData*)0x80130610, (Submap*)0x801306B0, (MapLight*)0x80130868,
             NULL, (void*)0x801307E8, (NavMeshData*)0x80133440, (NavMeshData*)0x80133428,
             0x80133ACC, 0x80133AD4, NULL, 0, { 0, 0, 0, 1 } },
    /* 30 */ { 0x8, { 0, 0 }, map_30_ROM_START, map_30_ROM_END, (void*)0x8010FE00,
             (ModelGeometry*)0x8014CFFC, (MapModelData*)0x8014D4A0, (Submap*)0x8014DDF4, (MapLight*)0x8014E6C0,
             NULL, (void*)0x8014E370, (NavMeshData*)0x80152534, (NavMeshData*)0x80151E6C,
             0x80152A8C, 0x80152AC4, (sMapList*)0x80152BC4, 0, { 0, 0, 0, 16 } },
    /* 31 */ { 0x8, { 0, 0 }, map_31_ROM_START, map_31_ROM_END, (void*)0x8010FE00,
             (ModelGeometry*)0x801476DC, (MapModelData*)0x80147D20, (Submap*)0x80147F00, (MapLight*)0x8014838C,
             NULL, (void*)0x80148068, (NavMeshData*)0x8014BF70, (NavMeshData*)0x8014BF38,
             0x8014C3F8, 0x8014C400, NULL, 0, { 0, 0, 0, 3 } },
    /* 32 */ { 0x8, { 0, 0 }, map_32_ROM_START, map_32_ROM_END, (void*)0x8010FE00,
             (ModelGeometry*)0x8013B150, (MapModelData*)0x8013B5F0, (Submap*)0x8013B7D0, (MapLight*)0x8013BBB4,
             NULL, (void*)0x8013B900, (NavMeshData*)0x8013D580, (NavMeshData*)0x8013D550,
             0x8013D9A4, 0x8013D9AC, (sMapList*)0x8013DA3C, 0, { 0, 0, 0, 3 } },
    /* 33 */ { 0x8, { 0, 0 }, map_33_ROM_START, map_33_ROM_END, (void*)0x8010FE00,
             (ModelGeometry*)0x80140DC4, (MapModelData*)0x80141190, (Submap*)0x80141370, (MapLight*)0x80141714,
             NULL, (void*)0x80141468, (NavMeshData*)0x80143300, (NavMeshData*)0x801432C4,
             0x8014365C, 0x80143664, NULL, 0, { 0, 0, 0, 3 } },
    /* 34 */ { 0x8, { 0, 0 }, map_34_ROM_START, map_34_ROM_END, (void*)0x8010FE00,
             (ModelGeometry*)0x80139374, (MapModelData*)0x80139680, (Submap*)0x80139ACC, (MapLight*)0x8013A1B0,
             NULL, (void*)0x80139E80, (NavMeshData*)0x8013D910, (NavMeshData*)0x8013D854,
             0x8013DCE4, 0x8013DD3C, (sMapList*)0x8013DE48, 0, { 0, 0, 0, 11 } },
    /* 35 */ { 0x88, { 0, 0 }, map_35_ROM_START, map_35_ROM_END, (void*)0x8010FE00,
             (ModelGeometry*)0x801269B4, (MapModelData*)0x80127A80, (Submap*)0x80127AE8, (MapLight*)0x80127C28,
             NULL, (void*)0x80127BA8, (NavMeshData*)0x801281D0, (NavMeshData*)0x801281C0,
             0x0, 0x0, NULL, 0, { 0, 0, 0, 1 } },
};
u8* gRegionBlocks[18] = {     /* the extra region blocks, 3 words each: ROM start, end, load address (LoadMap, sMapData.regionBlock) */
    region_0_ROM_START, region_0_ROM_END, (u8*)0x801F8000,
    region_1_ROM_START, region_1_ROM_END, (u8*)0x801F8000,
    region_2_ROM_START, region_2_ROM_END, (u8*)0x801F8000,
    region_3_ROM_START, region_3_ROM_END, (u8*)0x801F8000,
    region_4_ROM_START, region_4_ROM_END, (u8*)0x801F8000,
    region_5_ROM_START, region_5_ROM_END, (u8*)0x801F8000,
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
ModelPart gDoorAnimAlt = { sDoorKeysAlt, 11, 100, { 0.0f, 0.0f, 0.0f }, 0, 0, 0, 0, NULL, 0, { 0, 0 } };   /* kinds 2 and 3 when the door's flag 1 is set */
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
