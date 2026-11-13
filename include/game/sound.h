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

#endif
