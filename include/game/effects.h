#ifndef GAME_EFFECTS_H
#define GAME_EFFECTS_H

/* The functions of src/game/effects.c, declared once for every file that calls them. */

#include "common.h"
#include "proj_material.h"

typedef struct {
    s32 timer;                      /* counts down; a puff on odd frames */
    ActorTransform* actor;              /* the puff starts at its feet */
} DustEmitter;                      /* a running-dust emitter */
/* env object timers walked by UpdatePuffTimers (0x18 each; Quest D_80087250/D_80087310) */
typedef struct {
    s32 timer;
    f32 x;
    f32 y;
    f32 z;
    f32 size;
    ModelTransform* model;
} EnvTimer;
typedef struct {
    u16 timer;              /* frames left */
    u16 period;             /* 0x02: its start, to loop (flags 1) */
    u16 age;                /* 0x04: frames so far */
    u16 colour;             /* 0x06: into gParticleColours */
    u16 flags;              /* 0x08: 1 = loops, 2 = flickers, 4 = faces its velocity, 8 = culls backfaces, 0x20 = lit, 0x80 = fades in */
    u16 unkA;
    f32 x;
    f32 y;
    f32 z;
    f32 angle;
    f32 scaleX;
    f32 scaleY;
    f32 scaleZ;
    f32 vx;
    f32 vy;
    f32 vz;
    s32 frame;              /* 0x34: the animation's frame */
    AnimEntry anim;         /* 0x38: from gProjAnims */
    ProjModelDef tex;       /* 0x44 */
} ProjSlot;                 /* 0x58: gProjSlots[32]; spells.c's SpellFx and the header's StatusFxObject are views of it */
/* gSparkleBursts seen as its 8 slots of 0x18 (ResetSparkleBursts clears them, QueueSparkleBurst fills the first free one) */
typedef struct {
    s32 frames;
    s32 kind;
    f32 x;
    f32 y;
    f32 z;
    f32 size;
} SparkleBurst;
/* 32 slots of 0x28 ending at gGlowSlots (ResetStaffHitEffects clears them); gGlowSlots: 4-byte entries (StartLevelUpEffect) */
typedef struct {
    u16 timer;               /* frames left, 0 = free */
    char unk2[2];
    f32 x;               /* start position (SpawnStaffHitEffect) */
    f32 y;
    f32 z;
    f32 vx;              /* velocity */
    f32 vy;
    f32 vz;
    f32 startX;              /* position */
    f32 startY;
    f32 startZ;
} StaffHitSlot;
typedef struct {
    u16 timer;
    u16 unk2;
} GlowSlot;
/* 0x194-byte effect slots right after gGlowSlots[4] (StartGroundRing starts one) */
typedef struct {
    u16 phase;
    u16 time;
    u16 duration;
    u16 type;
    f32 x;
    f32 y;
    f32 z;
    Vec3f points[16];               /* 0x14: a ground ring (SpawnBattleRing samples 16 points on a circle) */
    Vec3f normals[16];              /* 0xD4: the floor normal under each point */
} GroundRing;

void ResetEffects(void);
void UpdateEffects(mkst* dyn);
void ResetDustEmitters(void);
void StartDustEmitter(ActorTransform* actor);
void SpawnRunningDust(void);
void UpdateRunDust(Dust* dust);
void ResetPuffTimers(void);
void UpdateEnvironmentObjects(f32 x, f32 y, f32 z, ModelTransform* model);
void UpdatePuffTimers(void);
void UpdateLevelUpPuff(Dust* dust);
void SpawnSparkle(f32 x, f32 y, f32 z);
void UpdateFramedPuff(Dust* dust);
void ResetDamageNumbers(void);
void SpawnDamageNumber(MonsterBattleData* target, s32 kind, s32 damage);   /* damage / heal digits */
void DrawDamageNumbers(void);
void ResetSparkleBursts(void);
void QueueSparkleBurst(s32 kind, s32 frames, f32 x, f32 y, f32 z, f32 size);
void UpdateSparkleBursts(void);
void UpdateSparkle(Dust* dust);
void SpawnSparkleBurst(s32 colour, f32 x, f32 y, f32 z, f32 size);
void UpdateBurstSparkle(Dust* dust);
void ResetStaffHitEffects(void);
void DrawStaffHitEffects(mkst* dyn);
void SpawnStaffHitEffect(f32 x, f32 y, f32 z);   /* staff hit effect */
void ResetGroundRings(void);
s32 SpawnBattleRing(u16 type, f32 x, f32 y, f32 z, f32 radius);
void StartGroundRing(s32 index, f32 x, f32 y, f32 z);
void DrawGroundRings(mkst* dyn);
void ResetProjectiles(void);
ProjSlot* AllocProjectileSlot(s32 anim, s32 model, s32 texture, s32 colour, s32 flags, f32 x, f32 y, f32 z, f32 angle, f32 scaleX, f32 scaleY, f32 scaleZ);   /* the slot it took, or none */
void DrawProjectiles(mkst* dyn);
void DrawProjectileModel(ProjSlot* slot, Mtx* mtxBuf);
void SetProjectileTexture(ProjSlot* slot);
void ResetGlows(void);
void DrawGlows(mkst* dyn);
void StartLevelUpEffect(s32 stat);   /* level-up effect */

/* The state src/game/effects.c defines (.bss), in its storage order: the order of these declarations is the
 * unit's .bss layout (include/bss_order.h). */
extern s32 gEnvTimersAdded;   /* env timers added this frame (UpdateEnvironmentObjects; main.c resets it) */
extern u16 gProjFrameCounter;   /* the projectiles' frame counter (DrawProjectiles: lhu / sh only) */
extern s32 gEnvTimersLast;   /* and the count UpdatePuffTimers walks */
extern s32 D_800863FC;   /* unreferenced */
extern DustEmitter gDustEmitters[8];
extern EnvTimer gEnvTimers1[8];   /* the env object timers: two banks, swapped by gCurrentFrame */
extern EnvTimer gEnvTimers0[8];
extern DamageDigs gDamageNumbers[10];   /* damage numbers */
extern SparkleBurst gSparkleBursts[8];
extern ProjSlot gProjSlots[32];   /* projectile slots */
extern StaffHitSlot gStaffHitSlots[32];
extern GlowSlot gGlowSlots[4];
extern GroundRing gGroundRings[4];
extern u16 gLastProjPalette;   /* the projectile palette loaded last (DrawProjectiles resets it to 0xFFFF) */
extern s32 D_80087DFC;   /* unreferenced */
extern Coordinates2D gRotatedOffset;
extern s32 D_80087E08[2];   /* unreferenced */
extern CollisionPollingResult gBattleRingFloorPoll;

/* The data src/game/effects.c defines. */
extern Dust gRunDustTemplate;   /* the dust puff template (player.c's running dust uses it too) */
extern f32 gRunDustScales[12];
extern u8 gRunDustAlphas[12];
extern Dust gFadingPuffTemplate;   /* the fading puff template */
extern u8 gLevelUpPuffAlphas[48];

#endif
