#ifndef GAME_AUDIO_H
#define GAME_AUDIO_H

/* The functions of src/game/audio.c, declared once for every file that calls them. */

#include "common.h"

/* The audio DMA buffers, as in the N64 SDK's audiomgr.c (AMDMABuffer / AMDMAState). */
typedef struct {
    ALLink node;            /* 0x00 */
    s32 startAddr;          /* 0x08 (signed here: __amDma compares it with slt) */
    u32 lastFrame;          /* 0x0C: the audio frame it was last used in */
    char* ptr;              /* 0x10 */
} AMDMABuffer;
typedef struct {
    u8 initialized;
    AMDMABuffer* firstUsed; /* 0x04 */
    AMDMABuffer* firstFree; /* 0x08 */
} AMDMAState;
typedef struct {
    ALSeqpConfig config;    /* 0x000 */
    ALCSPlayer player;      /* 0x01C */
    ALCSeq seq;             /* 0x098: what the player plays, alCSeqNew'd from seqBuffer (StartSequence) */
    u8* seqBuffer;          /* 0x190: the sequence data (0x8000 bytes, InitMusicPlayer) */
    s32 seqId;              /* 0x194: the sequence loaded into it (LoadSequence) */
    char unk198[4];
} MusicSlot;                /* 0x19C */

void ReadAudioRom(u32 rom, void* dst, s32 size);
void InitAudio(void);
s32 __amDma(s32 addr, s32 len, void* state);
ALDMAproc __amDmaNew(AMDMAState** state);
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

/* The state src/game/audio.c defines (.bss), in its storage order: the order of these declarations is the
 * unit's .bss layout (include/bss_order.h). */
extern OSThread gAudioThread;   /* the audio thread (StartAudioThread) */
extern u64 gAudioStack[1024];   /* the audio thread's stack (8 KB: 0x400 x 8 fill the space) */
extern ALHeap gAudioHeap;
extern Acmd* gAcmdLists[2];   /* the two ACMD lists (0x8000 bytes each) */
extern void* D_8008F088[2];   /* (0x40 bytes each) */
extern s16* gAudioBuffers[3];   /* the three audio output buffers (0x8000 bytes each) */
extern OSMesgQueue audDMAMessageQ;   /* audDMAMessageQ */
extern OSMesgQueue gAudioDmaQueue;
extern OSMesg audDMAMessageBuf;
extern OSMesg gAudioDmaMesgBuf;
extern OSIoMesg audDMAIOMesgBuf[32];   /* audDMAIOMesgBuf (32 x 0x18 fill the space) */
extern ALGlobals gAudioGlobals;
extern ALSynConfig gSynConfig;
extern ALSndPlayer gSndPlayer;
extern ALSndpConfig gSndpConfig;
extern ALInstrument* gSfxInstrument;   /* its instrument 0: one sound per effect */
extern ALBank* gSfxBank;
extern s16* gSoundIds;
extern s32 gSoundIdsAllocated;   /* sound ids allocated */
extern AMDMAState dmaState;   /* dmaState */
extern AMDMABuffer dmaBuffs[32];   /* dmaBuffs */
extern s32 gAcmdLen;   /* alAudioFrame's command count */
extern u32* gSoundScriptPos;
extern u32* gSoundScriptLoop;
extern s32 gLoopingSoundSlot;   /* the slot being updated (StepLoopingSound reads it) */
extern u32 gSlotLastVolume[5];   /* last volume per slot */
extern u32* gSlotScriptPos0;
extern u32* gSlotScriptPos1;
extern u32* gSlotScriptPos2;
extern u32* gSlotScriptPos3;
extern u32* gSlotScriptPos4;
extern u32* gSlotScriptLoop0;
extern u32* gSlotScriptLoop1;
extern u32* gSlotScriptLoop2;
extern u32* gSlotScriptLoop3;
extern u32* gSlotScriptLoop4;
extern MusicSlot gMusicSlots[2];   /* the two music slots */

/* The data src/game/audio.c defines. */
extern f32 gSoundPitchTable[73];
extern s32 D_80055924;   /* unreferenced */
extern ALBankFile* gMusicBankFile;   /* the music bank file (InitMusicPlayer loads it) */
extern ALBankFile* gSfxBankFile;   /* the sound effect bank file */
extern ALSeqFile* gSeqFile;   /* the sequence file (InitMusicPlayer) */
extern s32 nextDMA;   /* nextDMA, as in the SDK's audiomgr.c */
extern u32 audFrameCt;   /* audFrameCt, as in the SDK's audiomgr.c */
extern u32 gSoundQueue[8];   /* the sound queue: sound per entry, 0xFF = empty */
extern s32 gSoundQueueVolumeScale[8];   /* the sound queue: volume scale per entry */
extern s32 gSoundQueuePan[8];   /* the sound queue: pan per entry, 0x40 = centre */
extern s32 gSoundQueueRead;   /* the sound queue: the entry ProcessSoundQueue takes next */
extern s32 gSoundQueueWrite;   /* the sound queue: the entry PushSoundQueue fills next */
extern MusicVolume gMusicVolume;   /* the music volume */
extern s32 gActiveMusicSlot;   /* the music slot in use (0 or 1) */
extern s32 gMusicRequest;   /* the music request waiting to start, 0xFF = none */
extern s32 gLoadedSequence;   /* the sequence playing in slot 0, 0xFF = none */
extern s32 gMusicSlotVolumes[2];   /* each music slot's volume */
extern u32 gMusicFadeOutStep;   /* volume lost per frame while the music fades out, 0 = none */
extern s32 gMusicFadeInStep;   /* volume gained per frame while the music fades in, 0 = none */
extern s32 gLoopingSounds[5];   /* looping sound per slot, 0xFF = none */
extern s32 gLoopingSoundFrames[5];   /* frames left in the current step, per slot */
extern u32 gSlotVolumeScale[5];   /* volume scale per slot, 0 = none */
extern s32 gSlotPan[5];   /* default pan per slot, 0x40 = centre */
extern u32* gSoundScripts[0x47];

#endif
