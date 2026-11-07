#include "common.h"
/* The music and sound requests (InitializeSoundData to SetBGMVolume). The Start menu that followed them in this
   unit is its own file, start_menu.c: this file's .bss ends eight bytes before the menu's, as a .bss padded to sixteen
   does, and the menu's first function starts on a sixteen-byte boundary. */
extern u8 gSoundVolumes[];
/* ---- this file's .bss (0x8008FAF0-0x8008FB27), in address order; the object pads it to 0x8008FB30 ---- */
s8 gMusicPlaying;                 /* the music playing, -1 = none */
s8 gNextMusic;                    /* the music to play next (RequestMusic) */
u16 gMusicChangeFlags;            /* 1 change requested, 2 stop the current, 4 start the next */
u8 gSoundVolume;
u16 gMusicChangeDelay;
DelayedSound gDelayedSounds[16];  /* sounds waiting for their delay (PlaySound) */
void SetBGMVolume(void);
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
u8 ApplyMusicVolume(u32 BGMVolume);
void SetBGMVolume(void) {
    ApplyMusicVolume(gMusicVolumeLevels[gMusicVolumeSetting]);
}
