#ifndef GAME_AUDIO_H
#define GAME_AUDIO_H

/* The functions of src/game/audio.c, declared once for every file that calls them. */

#include "common.h"

void ReadAudioRom(u32 rom, void* dst, s32 size);
void InitAudio(void);
s32 __amDma(s32 addr, s32 len, void* state);
void __clearAudioDMA(void);
void AudioThread(NNSched* sc);   /* the audio thread (StartAudioThread starts it) */
void LoadSeqFileHeader(u8* seqFile);
void StartAudioThread(void* sched);
void InitMusicPlayer(u8* ctlStart, u8* ctlEnd, u8* seqStart, u8* seqEnd, u8* tblStart);
void LoadSequence(s32 slot, s32 seq);
s32 StartSequence(s32 slot);
void StopMusicSlot(s32 slot);
s32 GetMusicSlotState(s32 slot);
void InitSoundEffects(u8* bankStart, u8* bankEnd, u8* table);
void StartSound(s32 id);
void StopSound(s32 id);
void GetSoundState(s32 id);
void UpdateSoundPlayer(void);
void SelectSoundScript(s32 index);
s32 ChangeMusicIfIdle(void);
void QueueSound(s32 sound);
void PushSoundQueue(s32 sound, s32 volumeScale, s32 pan);
void ProcessSoundQueue(void);
void StepLoopingSound(void);
void StepMusicFadeOut(void);
void StepMusicFadeIn(void);
s32 LoadMusicSequences(s32 seq);
s32 StartMusicFadeOut(u32 frames);   /* start the music fade-out (src/game/audio.c) */
void StopLoopingSounds(void);
void StopLoopingSoundSlot4(void);
u8 ApplyMusicVolume(u32 BGMVolume);

#endif
