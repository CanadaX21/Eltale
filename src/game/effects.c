/*same as 191F0.c from Quest64*/
#include "common.h"
s32 count_digits(s32 arg0, u8* arg1, u16 arg2);

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

extern Dust gFadingPuffTemplate;              /* the fading puff template */
Dust* SpawnDust(Dust* dust);     /* spawn a dust puff from a template (dust.c); returns the new puff */
extern Dust gRunDustTemplate;              /* the dust puff template (player.c's running dust uses it too) */
void rotateCoordinatesByAngle(f32 angle, Coordinates2D* point);   /* rotate a 2D point */
void BuildRotationMatrix(MtxF *mtx, f32 xrot, f32 yrot, f32 zrot);   /* rotation matrix */
extern s32 gCurrentFrame;
extern Camera gCamera;   /* camera data (Quest D_80086DC0) */
extern Gfx gDigitRenderSetup[];             /* digit render setup */
f32 Atan2(f32 dx, f32 dz);
u32 getRandomNumber(u32 range);

/* projectile slots (0x58 each, 32 of them at gProjSlots; Quest UnkProjectileX34Struct at D_800875A8) */

typedef struct {
    u16 flags;              /* 1: 16 x 16 (else 32 x 32), 2: intensity (else CI with a gProjPalettes palette),
                               4 / 8: animated frames, 0x10: mirrored, 0x20: texgen (environment mapped), 0x40 */
    u16 frameMask;          /* 0x02: with flag 8 */
    u16 frameShift;         /* 0x04: the slot's timer >> this picks the frame */
    u16 palette;            /* 0x06: index into gProjPalettes */
    u8* texture;            /* 0x08: NULL = untextured */
    s16 s;                  /* 0x0C: texture scroll position, 10.2 */
    s16 t;
    s16 ds;                 /* 0x10: scroll per frame */
    s16 dt;
} ProjModelDef;         /* a projectile's material: gProjModelDefs[], 0x14 each (Quest D_800DA828); SetProjectileTexture */

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

extern AnimEntry gProjAnims[];       /* the projectile animations (Quest D_800EB9EC) */
extern Gfx* gProjGfx[];            /* the projectile display lists */
extern ProjModelDef gProjModelDefs[];

/* animated effect whose frame (unk0) picks a texture (gFramedPuffTextures, every 4th frame), a size and an alpha (UpdateFramedPuff) */
extern Gfx* gDisplayListHead;

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

/* the effect lists' reset (ResetEffects) and update (UpdateEffects) calls */
void ResetDustEmitters(void);
void ResetPuffTimers(void);
void ResetSparkleBursts(void);
void ResetStaffHitEffects(void);
void ResetGlows(void);
void ResetDamageNumbers(void);
void ResetProjectiles(void);
void ResetGroundRings(void);
void SpawnRunningDust(void);
void UpdatePuffTimers(void);
void UpdateSparkleBursts(void);
void DrawStaffHitEffects(mkst* dyn);
void DrawGlows(mkst* dyn);
void DrawGroundRings(mkst* dyn);
void DrawProjectiles(mkst* dyn);
void DrawDamageNumbers(void);

/* ---- effects.c's .bss (0x800863F0-0x80087E30), in address order. IDO 5.3 lays a variable
   out by its FIRST declaration, and those first declared among the file's first 32 declarations (typedefs and
   variables, externs too) after all later ones: this block sits past them, so it comes out in order. Declaring
   any of these variables above it moves them. The D_ names marked unreferenced are space no code addresses
   (unknown variables). ---- */
s32 gEnvTimersAdded;                         /* env timers added this frame (UpdateEnvironmentObjects; main.c resets it) */
u16 gProjFrameCounter;                         /* the projectiles' frame counter (DrawProjectiles: lhu / sh only) */
s32 gEnvTimersLast;                         /* and the count UpdatePuffTimers walks */
s32 D_800863FC;                         /* unreferenced */
DustEmitter gDustEmitters[8];
EnvTimer gEnvTimers1[8];                 /* the env object timers: two banks, swapped by gCurrentFrame */
EnvTimer gEnvTimers0[8];
DamageDigs gDamageNumbers[10];              /* damage numbers */
SparkleBurst gSparkleBursts[8];
ProjSlot gProjSlots[32];                /* projectile slots */
StaffHitSlot gStaffHitSlots[32];
GlowSlot gGlowSlots[4];
GroundRing gGroundRings[4];
u16 gLastProjPalette;                         /* the projectile palette loaded last (DrawProjectiles resets it to 0xFFFF) */
s32 D_80087DFC;                         /* unreferenced */
Coordinates2D gRotatedOffset;
s32 D_80087E08[2];                      /* unreferenced */
CollisionPollingResult gBattleRingFloorPoll;

void ResetEffects(void) {
    ResetDustEmitters();
    ResetPuffTimers();
    ResetSparkleBursts();
    ResetStaffHitEffects();
    ResetGlows();
    ResetDamageNumbers();
    ResetProjectiles();
    ResetGroundRings();
}
void UpdateEffects(mkst* dyn) {     /* draw the effects; arg0: this frame's matrix area */
    SpawnRunningDust();
    UpdatePuffTimers();
    UpdateSparkleBursts();
    DrawStaffHitEffects(dyn);
    DrawGlows(dyn);
    DrawGroundRings(dyn);
    DrawProjectiles(dyn);
    DrawDamageNumbers();
}
//#pragma GLOBAL_ASM("asm/nonmatchings/game/effects/ResetDustEmitters.s")
void ResetDustEmitters(void) {
    s32 i;
    DustEmitter* emitter;

    i = 8;
    emitter = gDustEmitters;
    do {
        emitter->timer = 0;
        i--;
        emitter++;
    } while (i != 0);
}

void FindFreeEffectSlot(ActorTransform* actor) {
    DustEmitter* emitter;
    s32 i;

    i = 7;
    emitter = gDustEmitters;
    while (i != 0 && emitter->timer != 0) {
        i--;
        emitter++;
    }
    emitter->timer = 4;
    emitter->actor = actor;
}
void SpawnRunningDust(void) {
    s32 i;
    DustEmitter* emitter;
    ActorTransform* actor;

    i = 8;
    emitter = gDustEmitters;
    do {
        if (emitter->timer != 0) {
            emitter->timer--;
            if (emitter->timer & 1) {
                actor = emitter->actor;
                gRunDustTemplate.x = actor->pos.x;
                gRunDustTemplate.y = actor->pos.y + 1.0f;
                gRunDustTemplate.z = actor->pos.z;
                gRotatedOffset.x = 0.0f;
                gRotatedOffset.y = 0.7f;
                rotateCoordinatesByAngle(actor->yrot, &gRotatedOffset);
                gRunDustTemplate.vx = gRotatedOffset.x;
                gRunDustTemplate.vy = 0.2f;
                gRunDustTemplate.vz = gRotatedOffset.y;
                SpawnDust(&gRunDustTemplate);
            }
        }
        i--;
        emitter++;
    } while (i != 0);
}
extern f32 gRunDustScales[];
extern u8 gRunDustAlphas[];
void UpdateRunDust(Dust* dust) {
    dust->x += dust->vx;
    dust->y += dust->vy;
    dust->z += dust->vz;
    dust->vx *= 0.8;
    dust->vy *= 0.9;
    dust->vz *= 0.8;
    dust->scaleX = dust->scaleY = gRunDustScales[dust->frames];
    gDPSetEnvColor(gDisplayListHead++, 0xFF, 0xFF, 0xFF, gRunDustAlphas[dust->frames]);
}
void ResetPuffTimers(void) {
    EnvTimer* recB;
    EnvTimer* recA;
    s32 n;

    gEnvTimersLast = 0;
    n = 8;
    recA = gEnvTimers1;
    recB = gEnvTimers0;

    for (; n != 0; recA++, recB++, n--) {
        recB->timer = 0;
        recA->timer = 0;
    }
}
void UpdateEnvironmentObjects(f32 x, f32 y, f32 z, ModelTransform* model) {
    f32 angle;
    s32 n;
    f32 dx;
    f32 dz;
    EnvTimer* rec;
    EnvTimer* prev;

    if (gCurrentFrame != 0) {
        rec = gEnvTimers1;
    } else {
        rec = gEnvTimers0;
    }

    if (gCurrentFrame != 0) {
        prev = gEnvTimers0;
    } else {
        prev = gEnvTimers1;
    }

    dx = x - gCamera.cam_x;
    dz = z - gCamera.cam_z;

    angle = Atan2(dx, dz) - gCamera.yaw + 3.141592653589793;

    if (angle < -3.141592653589793) {
        angle += 6.283185307179586;
    } else if (3.141592653589793 < angle) {
        angle -= 6.283185307179586;
    }

    if ((-1.5707963267948966 < angle) && (angle < 1.5707963267948966) && (gEnvTimersAdded < 8)) {
        n = gEnvTimersLast;
        rec += gEnvTimersAdded;

        while (n != 0 && model != prev->model) {
            n--;
            prev++;
        }

        if (n != 0) {
            rec->timer = prev->timer;
        } else {
            rec->timer = getRandomNumber(4) * 4 + 1;
        }

        rec->x = x;
        rec->y = y;
        rec->z = z;
        rec->size = model->model_scale;
        rec->model = model;

        gEnvTimersAdded++;
    }
}

void UpdatePuffTimers(void) {
    EnvTimer* timer;
    s32 n;
    Dust* dust = &gFadingPuffTemplate;

    gEnvTimersLast = gEnvTimersAdded;
    if (gCurrentFrame != 0) {
        timer = gEnvTimers1;
    } else {
        timer = gEnvTimers0;
    }
    n = gEnvTimersLast;
    while (n != 0) {
        if (timer->timer != 0) {
            timer->timer--;
            if (timer->timer == 0) {
                dust->x = timer->x;
                dust->y = timer->y;
                dust->z = timer->z;
                dust->scaleY = timer->size;
                dust->scaleX = dust->scaleY;
                SpawnDust(dust);
                timer->timer = 25;
            }
        }
        n--;
        timer++;
    }
}
extern s32 gFogColourR;
extern s32 gFogColourG;
extern s32 gFogColourB;
extern u8 gLevelUpPuffAlphas[];
void UpdateLevelUpPuff(Dust* dust) {
    s32 r;
    s32 g;
    s32 b;

    dust->x += dust->vx;
    dust->y += dust->vy;
    dust->z += dust->vz;
    dust->scaleX += 0.08;
    dust->scaleY += 0.08;
    r = gFogColourR + 0x40;
    g = gFogColourG + 0x40;
    b = gFogColourB + 0x40;
    if (r > 0xFF) {
        r = 0xFF;
    }
    if (g > 0xFF) {
        g = 0xFF;
    }
    if (b > 0xFF) {
        b = 0xFF;
    }
    gDPSetEnvColor(gDisplayListHead++, r, g, b, gLevelUpPuffAlphas[dust->frames]);
}
/* ---- effects.c's .data (0x8004EE20-0x8004F060, ROM 0x4FA20). IDO lays it out in definition order, so these five come
   first and the rest are function-local statics, each in the one function that uses it, in the order of the functions.
   The textures / display lists live in the effects block loaded at 0x800C0000. ---- */
extern u8 gFadingPuffGfx[];              /* fading puff display list */
extern u8 gPuffGfx[];              /* puff display list */
extern u8 gPuffTexture[];              /* puff texture */
extern u8 gSparkleTexture[];              /* sparkle texture */
extern u8 gSparkleGfx[];              /* sparkle display list */
extern u8 D_800C3A30[];              /* sparkle frames (gFramedPuffTextures) */
extern u8 D_800C3C30[];
extern u8 D_800C3E30[];
extern u8 gBurstTexture[];              /* burst texture */
extern u8 gBurstGfx[];              /* burst display list */
extern u8 D_800CAE68[];              /* gProjPalettes's four projectile palettes (256 colours, 0x200 bytes each) */
extern u8 D_800CB068[];
extern u8 D_800CB268[];
extern u8 D_800CB468[];
Dust gRunDustTemplate = {                  /* the dust puff template (SpawnRunningDust; player.c's running dust) */
    0xC, 0, 0, UpdateRunDust, 2, 0x10, 0x10, 0, 0, { 0 }, gPuffTexture, gPuffGfx,
    0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0, { 0 },
};
f32 gRunDustScales[12] = { 3.25f, 3.24f, 3.23f, 3.22f, 3.21f, 3.2f, 3.0f, 2.8f, 2.6f, 2.4f, 2.2f, 2.0f };
u8 gRunDustAlphas[12] = { 0x10, 0x20, 0x40, 0x50, 0x60, 0x80, 0x90, 0xA0, 0xB0, 0xA0, 0x80, 0x40 };
Dust gFadingPuffTemplate = {                  /* the fading puff template (UpdatePuffTimers) */
    0x2D, 0, 0, UpdateLevelUpPuff, 2, 0x10, 0x10, 0, 0, { 0 }, gPuffTexture, gFadingPuffGfx,
    0.0f, 0.0f, 0.0f, 0.0f, 4.0f, 4.0f, 0.1f, 0.25f, 0.05f, 0, { 0 },
};
u8 gLevelUpPuffAlphas[48] = {
    0x08, 0x1C, 0x30, 0x44, 0x58, 0x6C, 0x80, 0x94, 0xA8, 0xBC, 0xD0, 0xD0, 0xD0, 0xD0, 0xD0, 0xD0,
    0xD0, 0xD0, 0xD0, 0xD0, 0xD0, 0xD0, 0xD0, 0xD0, 0xD0, 0xD0, 0xD0, 0xD0, 0xD0, 0xD0, 0xD0, 0xD0,
    0xD0, 0xC0, 0xB0, 0xA0, 0x90, 0x80, 0x70, 0x60, 0x50, 0x40, 0x30, 0x20, 0x10, 0x00, 0x00, 0x00,
};
void UpdateFramedPuff(Dust* dust);
void UpdateSparkle(Dust* dust);
void UpdateBurstSparkle(Dust* dust);
void SpawnSparkle(f32 x, f32 y, f32 z) {
    static Dust gSparkleTemplate = {                  /* the sparkle template (SpawnSparkle) */
        0xC, 0, 0, UpdateFramedPuff, 4, 0x20, 0x20, 0, 0, { 0 }, gSparkleTexture, gSparkleGfx,
        0.0f, 0.0f, 0.0f, 0.0f, 0.4f, 0.4f, 0.0f, 0.0f, 0.0f, 0, { 0 },
    };
    gSparkleTemplate.x = x;
    gSparkleTemplate.y = y;
    gSparkleTemplate.z = z;
    SpawnDust(&gSparkleTemplate);
}

void UpdateFramedPuff(Dust* dust) {
    static u8* gFramedPuffTextures[3] = { D_800C3E30, D_800C3C30, D_800C3A30 };
    static f32 gFramedPuffScales[12] = { 0.88f, 0.88f, 0.84f, 0.84f, 0.82f, 0.82f, 0.8f, 0.8f, 0.7f, 0.6f, 0.5f, 0.4f };
    static u8 gFramedPuffAlphas[12] = { 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x80, 0xA0, 0xC0, 0xC8, 0x80, 0x60 };
    u16 frame = dust->frames;

    if (!(frame & 3)) {
        dust->dustTexture = gFramedPuffTextures[frame >> 2];
    }
    dust->scaleX = dust->scaleY = gFramedPuffScales[frame];
    gDPSetEnvColor(gDisplayListHead++, 0xFF, 0xFF, 0xFF, gFramedPuffAlphas[dust->frames]);
}
void ResetDamageNumbers(void) {
    DamageDigs* num;
    s32 n;

    n = 10;
    num = gDamageNumbers;
    do {
        n--;
        num->damageDisplayTimer = 0;
        num++;
    } while (n != 0);
}
void SpawnDamageNumber(MonsterBattleData* target, s32 kind, s32 damage) {
    s32 i;
    s32 stacked;
    DamageDigs* num;

    i = 9;
    num = gDamageNumbers;
        while ((i != 0) && (num->damageDisplayTimer != 0)){
            i--;
            num++;
        }
    num->damageDisplayTimer = 0x2D;
    num->target = target;
    num->kind = kind;
    
    if (damage < 0) {
        damage = -damage;
    }

    num->numDigits = count_digits(damage, num->damageDigits, 1U);
    stacked = 0;
    
    for(i = 0; i < 10; i++) {
        if((gDamageNumbers[i].damageDisplayTimer != 0) && (target == gDamageNumbers[i].target)){
            stacked++;
        } 
    }
    num->stackIndex = stacked - 1;
}
s32 WorldToScreen(f32 x, f32 y, f32 z, s32* sx, s32* sy);   /* world to screen; 0 = off screen */
extern u16 gDigitSheet[];             /* the digit sheet: 128 x 10, RGBA16 */
s16 sins(u16 angle);        /* sins */
void DrawDamageNumbers(void) {
    static u8 gDigitColours[4][4] = {              /* digit colours */
        { 0xFF, 0xFF, 0xFF, 0xFF }, { 0xC8, 0xFF, 0xC8, 0xFF }, { 0xC8, 0xC8, 0xFF, 0xFF }, { 0xFF, 0x7F, 0x7F, 0xFF },
    };
    s32 n;
    MonsterBattleData* actor;
    s32 started;
    DamageDigs* d;
    s32 sx;
    s32 sy;
    f32 y;
    s32 dsdx;
    s32 dtdy;
    s32 hw;
    s32 hh;
    s32 bounce;
    u8* colour;
    s32 i;
    u8* digit;

    started = 0;
    n = 10;
    d = gDamageNumbers;
    do {
        if (d->damageDisplayTimer != 0) {
            actor = d->target;
            if (actor->transform.base.monster->monsterType == ON_GROUND) {
                y = actor->transform.pos.y + actor->transform.base.monster->height * actor->transform.scale;
            } else if (actor->transform.base.monster->monsterType == FLYING) {
                y = actor->stats->flyY;
            } else {
                y = actor->transform.pos.y + 600.0 * actor->transform.scale;
            }
            if (WorldToScreen(actor->transform.pos.x, y, actor->transform.pos.z, &sx, &sy) != 0) {
                if (started == 0) {
                    gSPDisplayList(gDisplayListHead++, gDigitRenderSetup);
                    gDPLoadTextureBlock(gDisplayListHead++, gDigitSheet, G_IM_FMT_RGBA, G_IM_SIZ_16b, 128, 10, 0,
                                        G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK,
                                        G_TX_NOLOD, G_TX_NOLOD);
                    started++;
                }
                if (d->kind & 0x8000) {
                    dsdx = 0x280;
                    dtdy = 0x280;
                    hw = 6;
                    hh = 8;
                } else {
                    dsdx = 0x400;
                    dtdy = 0x400;
                    hw = 4;
                    hh = 5;
                }
                if (d->damageDisplayTimer >= 30) {
                    bounce = sins((45 - d->damageDisplayTimer) << 12) >> 11;
                    if (bounce < 0) {
                        bounce = -bounce;
                    }
                    sy -= bounce;
                }
                sx -= d->stackIndex * hw;
                sy = sy - d->stackIndex * 5 - 8;
                colour = gDigitColours[d->kind & 0xF];
                gDPPipeSync(gDisplayListHead++);
                gDPSetEnvColor(gDisplayListHead++, colour[0], colour[1], colour[2], 0xFF);
                i = d->numDigits;
                digit = d->damageDigits;
                sx -= d->numDigits * 4;
                while (i != 0) {
                    i--;
                    if (*digit != ' ') {
                        gSPTextureRectangle(gDisplayListHead++, (sx - hw) << 2, (sy - hh) << 2, (sx + hw) << 2, (sy + hh) << 2,
                                            G_TX_RENDERTILE, *digit << 8, 0, dtdy, dsdx);
                        sx += hw * 2;
                    }
                    digit++;
                }
            }
            d->damageDisplayTimer--;
        }
        n--;
        d++;
    } while (n != 0);
}
void ResetSparkleBursts(void) {
    s32 n;
    EnvTimer* slot;

    slot = (EnvTimer*)gSparkleBursts;
    n = 8;
    do {
        n -= 4;
        slot[0].timer = 0;
        slot[1].timer = 0;
        slot[2].timer = 0;
        slot[3].timer = 0;
        slot += 4;
    } while (n != 0);
}
void QueueSparkleBurst(s32 kind, s32 frames, f32 x, f32 y, f32 z, f32 size) {
    SparkleBurst* slot = gSparkleBursts;
    s32 i;

    for (i = 7; i != 0 && slot->frames != 0; i--) {
        slot++;
    }
    slot->frames = frames;
    slot->kind = kind;
    slot->x = x;
    slot->y = y;
    slot->z = z;
    slot->size = size;
}

void UpdateSparkleBursts(void) {
    static Dust gBurstTemplate = {                  /* a burst template (UpdateSparkleBursts) */
        0x14, 0, 0, UpdateSparkle, 2, 0x10, 0x10, 0, 0, { 0 }, gBurstTexture, gBurstGfx,
        0.0f, 0.0f, 0.0f, 0.0f, 1.3f, 1.3f, 0.0f, 0.0f, 0.0f, 0, { 0 },
    };
    f32 a;
    f32 vx;
    f32 vy;
    f32 vz;
    f32 b;
    s32 i;
    MtxF mtx;
    SparkleBurst* slot;
    Dust dust;

    dust = gBurstTemplate;
    slot = gSparkleBursts;
    i = 8;
    do {
        if (slot->frames != 0) {
            if (slot->frames & 1) {
                a = (f32)getRandomNumber(32) * 3.141592653589793 / 16;
                b = (f32)getRandomNumber(32) * 3.141592653589793 / 16;
                BuildRotationMatrix(&mtx, a, b, 0.0f);
                guMtxXFMF(mtx.mf, 0.0f, 0.0f, slot->size, &vx, &vy, &vz);
                dust.x = slot->x + vx;
                dust.y = slot->y + vy;
                dust.z = slot->z + vz;
                dust.vx = vx * -0.03;
                dust.vy = vy * -0.03;
                dust.vz = vz * -0.03;
                dust.colour = slot->kind;
                SpawnDust(&dust);
            }
            slot->frames--;
        }
        i--;
        slot++;
    } while (i != 0);
}
extern u8 gParticleColours[][4];     /* particle colours, RGBA */
void UpdateSparkle(Dust* dust) {
    u8* colour;

    dust->x += dust->vx;
    dust->y += dust->vy;
    dust->z += dust->vz;
    dust->vx *= 0.9;
    dust->vy *= 0.9;
    dust->vz *= 0.9;
    dust->roll += 0.39269908169872414;
    if (dust->frames < 10) {
        dust->scaleX = dust->scaleY = 1.0f;
    }
    colour = gParticleColours[dust->colour];
    if (dust->frames & 1) {
        gDPSetEnvColor(gDisplayListHead++, colour[0], colour[1], colour[2], 0xDC);
    } else {
        gDPSetEnvColor(gDisplayListHead++, colour[0], colour[1], colour[2], 0x20);
    }
}
void SpawnSparkleBurst(s32 colour, f32 x, f32 y, f32 z, f32 size) {
    static Dust gBurstDustTemplate = {                  /* the burst's dust template (SpawnSparkleBurst) */
        0x14, 0, 0, UpdateBurstSparkle, 2, 0x10, 0x10, 0, 0, { 0 }, gBurstTexture, gBurstGfx,
        0.0f, 0.0f, 0.0f, 0.0f, 1.3f, 1.3f, 0.0f, 0.0f, 0.0f, 0, { 0 },
    };
    s32 i;
    f32 vx;
    f32 vy;
    f32 vz;
    MtxF mtx;
    Dust dust;

    dust = gBurstDustTemplate;
    i = 15;
    do {
        vx = (f32)getRandomNumber(32) * 3.141592653589793 / 16;
        vy = (f32)getRandomNumber(32) * 3.141592653589793 / 16;
        BuildRotationMatrix(&mtx, vx, vy, 0.0f);
        guMtxXFMF(mtx.mf, 0.0f, 0.0f, size, &vx, &vy, &vz);
        dust.x = x;
        dust.y = y;
        dust.z = z;
        dust.vx = vx;
        dust.vy = vy;
        dust.vz = vz;
        dust.colour = colour;
        SpawnDust(&dust);
        i--;
    } while (i != 0);
}
void UpdateBurstSparkle(Dust* dust) {
    u8* colour;

    dust->x += dust->vx;
    dust->y += dust->vy;
    dust->z += dust->vz;
    dust->vx *= 0.8;
    dust->vy *= 0.8;
    dust->vz *= 0.8;
    dust->roll += 0.39269908169872414;
    if (dust->frames < 10) {
        dust->scaleX = dust->scaleY = 1.0f;
    }
    colour = gParticleColours[dust->colour];
    if (dust->frames & 1) {
        gDPSetEnvColor(gDisplayListHead++, colour[0], colour[1], colour[2], 0xDC);
    } else {
        gDPSetEnvColor(gDisplayListHead++, colour[0], colour[1], colour[2], 0x20);
    }
}
void ResetStaffHitEffects(void) {
    s32 i;

    for (i = 0; i < 32; i++) {
        gStaffHitSlots[i].timer = 0;
    }
}
void MatrixFromDirection(MtxF* mf, f32 x, f32 y, f32 z);
void ScaleMatrix(MtxF* mf, f32 x, f32 y, f32 z);
f32 sqrtf(f32 x);
extern Mtx D_2000000[];
extern Gfx gStaffHitSetupGfx[];
extern Gfx gStaffHitGfx[];
extern s32 gMatrixHead;
void DrawStaffHitEffects(mkst* dyn) {
    s32 t;
    s32 drawn;
    StaffHitSlot* slot;
    s32 i;
    u32 alpha;
    f32 dx;
    f32 dy;
    f32 dz;
    MtxF mtx;

    drawn = 0;
    slot = gStaffHitSlots;
    i = 32;
    do {
        if (slot->timer != 0) {
            slot->x += slot->vx;
            slot->y += slot->vy;
            slot->z += slot->vz;
            if (gMatrixHead < 0x1E6) {
                if (drawn == 0) {
                    gSPDisplayList(gDisplayListHead++, gStaffHitSetupGfx);
                    drawn++;
                }
                t = slot->timer;
                if (t & 1) {
                    alpha = 250;
                } else {
                    alpha = 150;
                }
                if (t < 8) {
                    alpha = (t * alpha) >> 3;
                }
                MatrixFromDirection(&mtx, slot->vx, slot->vy, slot->vz);
                dx = slot->x - slot->startX;
                dy = slot->y - slot->startY;
                dz = slot->z - slot->startZ;
                ScaleMatrix(&mtx, 0.35f, sqrtf(dx * dx + dy * dy + dz * dz) * 0.1, 0.35f);
                mtx.mf[3][0] = slot->x;
                mtx.mf[3][1] = slot->y;
                mtx.mf[3][2] = slot->z;
                guMtxF2L(&mtx, &dyn->drawMtx[gMatrixHead]);
                gSPMatrix(gDisplayListHead++, D_2000000 + 3 + gMatrixHead++, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
                gDPSetEnvColor(gDisplayListHead++, 0xFF, 0xFF, 0xFF, alpha);
                gSPDisplayList(gDisplayListHead++, gStaffHitGfx);
            }
            slot->timer--;
            slot->vx *= 0.6;
            slot->vy *= 0.6;
            slot->vz *= 0.6;
        }
        i--;
        slot++;
    } while (i != 0);
}

void SpawnStaffHitEffect(f32 x, f32 y, f32 z) {
    StaffHitSlot* slot;
    s32 free;
    s32 n;
    f32 a;
    f32 b;
    f32 size;
    MtxF mtx;

    slot = gStaffHitSlots;
    free = 31;
    n = 16;
    do {
        while (free != 0 && slot->timer != 0) {
            free--;
            slot++;
        }
        slot->timer = getRandomNumber(8) + 10;
        slot->x = x;
        slot->startX = x;
        slot->y = y;
        slot->startY = y;
        slot->startZ = z;
        slot->z = z;
        a = getRandomNumber(20) * 3.141592653589793 / 10.0;
        b = getRandomNumber(20) * 3.141592653589793 / 10.0;
        size = (f32)getRandomNumber(10) / 3.0 + 3.0;
        BuildRotationMatrix(&mtx, a, b, 0.0f);
        guMtxXFMF(mtx.mf, 0.0f, 0.0f, size, &slot->vx, &slot->vy, &slot->vz);
        n--;
    } while (n != 0 && free != 0);
}
void ResetGroundRings(void) {
    GroundRing* ring;
    s32 i;

    ring = gGroundRings;
    for (i = 0; i < 4; i++) {
        ring->phase = 0;
        ring++;
    }
}
extern PlayerState gPlayerState;
void PollWorldCollision(f32 sample_x, f32 sample_y, f32 sample_z, u16 arg3_flags, CollisionPollingResult* result);
s32 SpawnBattleRing(u16 type, f32 x, f32 y, f32 z, f32 radius) {
    static u16 sRingDurations[4] = { 0x14, 0xC, 0xC, 0 };   /* per ring type */
    f32 angle;
    f32 dy;
    s32 i;
    s32 slot;
    Vec3f* point;
    Vec3f* normal;
    GroundRing* ring;

    ring = gGroundRings;
    for (slot = 0; slot < 3; slot++) {
        if (ring->phase == 0) {
            break;
        }
        ring++;
    }
    ring->type = type;
    ring->phase = 1;
    ring->time = 0;
    ring->duration = sRingDurations[type];
    ring->x = x;
    ring->y = y;
    ring->z = z;
    point = ring->points;
    normal = ring->normals;
    i = 16;
    angle = 0.0f;
    while (i != 0) {
        gRotatedOffset.x = 0.0f;
        gRotatedOffset.y = radius;
        rotateCoordinatesByAngle(angle, &gRotatedOffset);
        point->x = gRotatedOffset.x + x;
        point->z = gRotatedOffset.y + z;
        PollWorldCollision(point->x, gPlayerState.brian.transform.pos.y, point->z, 0xA00, &gBattleRingFloorPoll);
        if (gBattleRingFloorPoll.flags & 0x8000) {
            point->y = y;
            normal->x = 0.0f;
            normal->y = 1.0f;
            normal->z = 0.0f;
        } else {
            dy = gBattleRingFloorPoll.y - y;
            if (dy < -40.0 || dy > 40.0) {
                point->y = y;
                normal->x = 0.0f;
                normal->y = 1.0f;
                normal->z = 0.0f;
            } else {
                point->y = gBattleRingFloorPoll.y;
                normal->x = gBattleRingFloorPoll.sn_x;
                normal->y = gBattleRingFloorPoll.sn_y;
                normal->z = gBattleRingFloorPoll.sn_z;
            }
        }
        angle += 0.39269908169872414;
        point++;
        normal++;
        i--;
    }
    return slot;
}
void StartGroundRing(s32 index, f32 x, f32 y, f32 z) {
    GroundRing* slot = &gGroundRings[index];

    slot->phase = 3;
    slot->time = 0;
    slot->duration = 0xA;
    slot->x = x;
    slot->y = y;
    slot->z = z;
}

f32 sinf(f32 x);            /* sinf */
f32 sqrtf(f32 x);            /* sqrtf */
extern Gfx gRingRenderSetup[];             /* ring render setup */
extern u8 gRingTexture[];              /* ring texture, I8 8 x 8 */
extern Gfx gRingSegmentGfx[];             /* one ring segment */
void MatrixFromDirection(MtxF* mtx, f32 x, f32 y, f32 z);
void ScaleMatrix(MtxF* m, f32 x, f32 y, f32 z);
void DrawGroundRings(mkst* dyn) {
    static u8 gRingColours[3][4] = {              /* ring colours */
        { 0xC8, 0xC8, 0x00, 0x00 }, { 0xC8, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0xC8, 0x00 },
    };
    static u8 gRingFlickerAlphas[2] = { 0xFF, 0x96 };   /* the ring flicker alphas */
    s32 n;
    GroundRing* ring;
    s32 started;
    f32 ax;
    f32 ay;
    f32 az;
    f32 bx;
    f32 by;
    f32 bz;
    f32 dx;
    f32 dy;
    f32 dz;
    f32 len;
    f32 scale;
    Vec3f* p;
    s32 i;
    u8 alpha;
    u16 t;
    u8* colour;
    MtxF mtx;

    started = 0;
    ring = gGroundRings;
    n = 4;
    do {
        if (ring->phase != 0) {
            if (started == 0) {
                gSPDisplayList(gDisplayListHead++, gRingRenderSetup);
                gDPLoadTextureBlock(gDisplayListHead++, gRingTexture, G_IM_FMT_I, G_IM_SIZ_8b, 8, 8, 0,
                                    G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, 3, G_TX_NOMASK,
                                    G_TX_NOLOD, G_TX_NOLOD);
                started++;
            }
            t = ++ring->time;
            alpha = gRingFlickerAlphas[t & 1];
            switch (ring->phase) {
                case 1:
                    if (t < 4) {
                        alpha = (alpha * t) >> 2;
                    }
                    if (ring->duration < t) {
                        ring->phase += 1;
                    } else {
                        scale = sinf((t * 3.141592653589793) / (ring->duration * 2));
                    }
                    break;
                case 3:
                    alpha = alpha * (ring->duration - t) / ring->duration;
                    if (t >= ring->duration) {
                        ring->phase = 0;
                    }
                    break;
            }
            colour = gRingColours[ring->type];
            gDPSetEnvColor(gDisplayListHead++, colour[0], colour[1], colour[2], alpha);
            p = ring->points;
            i = 16;
            do {
                ax = p->x;
                ay = p->y + 5.0f;
                az = p->z;
                p++;
                i--;
                if (i == 0) {
                    p = ring->points;
                }
                bx = p->x;
                by = p->y + 5.0f;
                bz = p->z;
                if (ring->phase == 1) {
                    ax = (ax - ring->x) * scale + ring->x;
                    ay = (ay - ring->y) * scale + ring->y;
                    az = (az - ring->z) * scale + ring->z;
                    bx = (bx - ring->x) * scale + ring->x;
                    by = (by - ring->y) * scale + ring->y;
                    bz = (bz - ring->z) * scale + ring->z;
                }
                dx = bx - ax;
                dy = by - ay;
                dz = bz - az;
                len = sqrtf(dx * dx + dy * dy + dz * dz);
                if (len != 0.0f) {
                    MatrixFromDirection(&mtx, dx, dy, dz);
                    ScaleMatrix(&mtx, 2.0f, len, 1.5f);
                    mtx.mf[3][0] = ax;
                    mtx.mf[3][1] = ay;
                    mtx.mf[3][2] = az;
                    guMtxF2L(mtx.mf, &dyn->drawMtx[gMatrixHead]);
                    gSPMatrix(gDisplayListHead++, &D_2000000[gMatrixHead] + 3, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
                    gMatrixHead++;
                    gSPDisplayList(gDisplayListHead++, gRingSegmentGfx);
                }
            } while (i != 0);
        }
        ring++;
        n--;
    } while (n != 0);
}
void ResetProjectiles(void) {
    ProjSlot* slot;
    s32 n;

    n = 0x20;
    slot = gProjSlots;
    do {
        n--;
        slot->timer = 0;
        slot++;
    } while (n != 0);
}
ProjSlot* AllocProjectileSlot(s32 anim, s32 model, s32 texture, s32 colour, s32 flags, f32 x, f32 y, f32 z, f32 angle, f32 scaleX, f32 scaleY, f32 scaleZ) {
    s32 n;
    ProjSlot* slot;

    n = 0x1F;
    slot = gProjSlots;
    while (n != 0 && slot->timer) {
        n--;
        slot++;
    }

    slot->x = x;
    slot->y = y;
    slot->z = z;
    slot->angle = angle;
    slot->scaleX = scaleX;
    slot->scaleY = scaleY;
    slot->scaleZ = scaleZ;
    slot->vx = 0.0f;
    slot->vy = 0.0f;
    slot->vz = 1.0f;
    slot->anim = gProjAnims[anim];
    slot->anim.dlists = &gProjGfx[model];
    slot->timer = slot->period = slot->anim.frames;
    slot->age = 0;
    slot->colour = colour;
    slot->flags = flags;
    slot->frame = 0;
    slot->tex = gProjModelDefs[texture];
    return slot;
}
extern s32 gPartPositionCount;
void BuildActorMatrix(MtxF* mf, f32 xrot, f32 yrot, f32 zrot, f32 scale, f32 x, f32 y, f32 z);
void DrawModelPart(MtxF* mf, Vec3f scale, s32 flags, s32 part, s32 frame, s32 frames, AnimEntry* anim,
                   ModelPart* parts, Mtx* mtxBuf);   /* draw one part and its children (model.c) */
extern Gfx gProjRenderSetup[];                /* the projectiles' render setup */
void DrawProjectileModel(ProjSlot* slot, Mtx* mtxBuf);
void SetProjectileTexture(ProjSlot* slot);
void DrawProjectiles(mkst* dyn) {
    ProjSlot* slot;
    s32 i;
    u32 alpha;
    u8* colour;
    u16* frame;

    frame = &gProjFrameCounter;
    gSPDisplayList(gDisplayListHead++, gProjRenderSetup);
    gLastProjPalette = 0xFFFF;
    i = 32;
    slot = gProjSlots;
    do {
        if (slot->timer != 0) {
            SetProjectileTexture(slot);
            if (slot->flags & 2) {
                if (*frame & 1) {
                    alpha = 240;
                } else {
                    alpha = 150;
                }
            } else {
                alpha = 250;
            }
            if (!(slot->flags & 1) && slot->timer < 8) {
                alpha = (slot->timer * alpha) >> 3;
            } else if (slot->flags & 0x80) {
                if (slot->age < 8) {
                    alpha = (slot->age * alpha) >> 3;
                }
            }
            colour = gParticleColours[slot->colour & 0xFF];
            gDPSetEnvColor(gDisplayListHead++, colour[0], colour[1], colour[2], alpha);
            if (slot->flags & 8) {
                gSPSetGeometryMode(gDisplayListHead++, G_CULL_BACK);
            } else {
                gSPClearGeometryMode(gDisplayListHead++, G_CULL_BACK);
            }
            DrawProjectileModel(slot, dyn->list);
            slot->frame++;
            slot->timer--;
            if (slot->timer == 0 && (slot->flags & 1)) {
                slot->timer = slot->period;
            }
            slot->age++;
        }
        i--;
        slot++;
    } while (i != 0);
    gDPPipeSync(gDisplayListHead++);
    gDPSetTextureLUT(gDisplayListHead++, G_TT_NONE);
    (*frame)++;
}
void DrawProjectileModel(ProjSlot* slot, Mtx* mtxBuf) {
    u32 i;
    ModelPart* parts;
    s32 frame;
    MtxF mtx;
    Vec3f scale;

    gPartPositionCount = 0;
    if (slot->flags & 4) {
        MatrixFromDirection(&mtx, slot->vx, slot->vy, slot->vz);
        mtx.mf[3][0] = slot->x;
        mtx.mf[3][1] = slot->y;
        mtx.mf[3][2] = slot->z;
        ScaleMatrix(&mtx, slot->scaleX * 0.01, slot->scaleY * 0.01, slot->scaleZ * 0.01);
    } else {
        BuildActorMatrix(&mtx, 0.0f, slot->angle, 0.0f, 0.01f, slot->x, slot->y, slot->z);
        ScaleMatrix(&mtx, slot->scaleX, slot->scaleY, slot->scaleZ);
    }
    scale.x = slot->scaleX * 0.01;
    scale.y = slot->scaleY * 0.01;
    scale.z = slot->scaleZ * 0.01;
    parts = slot->anim.parts;
    frame = slot->frame;
    if (gMatrixHead < 486) {
        for (i = 0; i < slot->anim.count; i++) {
            if (parts[i].parent == 0) {
                DrawModelPart(&mtx, scale, 0, i, frame, 0, &slot->anim, parts, mtxBuf);
            }
        }
    }
}

void SetProjectileTexture(ProjSlot* slot) {
    static u8* gProjPalettes[4] = { D_800CAE68, D_800CB068, D_800CB268, D_800CB468 };
    u32 size;
    s32 mask;
    ProjModelDef* tex;
    s32 geom;
    u8* addr;
    s32 cm;

    geom = 0;
    tex = &slot->tex;
    if (tex->texture != NULL) {
        if (slot->flags & 0x20) {
            geom = G_LIGHTING;
        }
        tex->s += tex->ds;
        tex->t += tex->dt;
        addr = tex->texture;
        if (tex->flags & 1) {
            size = 16;
        } else {
            size = 32;
        }
        if (tex->flags & 1) {
            mask = 4;
        } else {
            mask = 5;
        }
        if (tex->flags & 4) {
            addr += (slot->age >> tex->frameShift) * size * size;
        } else if (tex->flags & 8) {
            addr += ((slot->age >> tex->frameShift) & tex->frameMask) * size * size;
        }
        if (tex->flags & 0x10) {
            cm = G_TX_MIRROR;
        } else {
            cm = G_TX_NOMIRROR;
        }
        if (tex->flags & 2) {
            gDPLoadTextureBlock(gDisplayListHead++, addr, G_IM_FMT_I, G_IM_SIZ_8b, size, size, 0, cm, cm, mask, mask,
                                G_TX_NOLOD, G_TX_NOLOD);
            gDPSetTextureLUT(gDisplayListHead++, G_TT_NONE);
            if (tex->flags & 0x40) {
                if (slot->flags & 0x20) {
                    gDPSetCombineLERP(gDisplayListHead++, 1, TEXEL0, ENVIRONMENT, TEXEL0, TEXEL0, 0, ENVIRONMENT, 0,
                                      COMBINED, 0, SHADE, 0, COMBINED, 0, SHADE, 0);
                } else {
                    gDPSetCombineLERP(gDisplayListHead++, 1, TEXEL0, ENVIRONMENT, TEXEL0, TEXEL0, 0, ENVIRONMENT, 0,
                                      0, 0, 0, COMBINED, COMBINED, 0, SHADE, 0);
                }
            } else if (slot->flags & 0x20) {
                gDPSetCombineLERP(gDisplayListHead++, 1, TEXEL0, ENVIRONMENT, TEXEL0, 0, 0, 0, ENVIRONMENT,
                                  COMBINED, 0, SHADE, 0, COMBINED, 0, SHADE, 0);
            } else {
                gDPSetCombineLERP(gDisplayListHead++, 1, TEXEL0, ENVIRONMENT, TEXEL0, 0, 0, 0, ENVIRONMENT,
                                  0, 0, 0, COMBINED, COMBINED, 0, SHADE, 0);
            }
        } else {
            gDPLoadTextureBlock(gDisplayListHead++, addr, G_IM_FMT_CI, G_IM_SIZ_8b, size, size, 0, cm, cm, mask, mask,
                                G_TX_NOLOD, G_TX_NOLOD);
            gDPSetTextureLUT(gDisplayListHead++, G_TT_RGBA16);
            if (slot->flags & 0x20) {
                gDPSetCombineLERP(gDisplayListHead++, TEXEL0, 0, ENVIRONMENT, 0, TEXEL0, 0, SHADE, 0,
                                  COMBINED, 0, SHADE, 0, COMBINED, 0, ENVIRONMENT, 0);
            } else {
                gDPSetCombineLERP(gDisplayListHead++, TEXEL0, 0, ENVIRONMENT, 0, TEXEL0, 0, SHADE, 0,
                                  0, 0, 0, COMBINED, COMBINED, 0, ENVIRONMENT, 0);
            }
            if (gLastProjPalette != tex->palette) {
                gLastProjPalette = tex->palette;
                gDPLoadTLUT_pal256(gDisplayListHead++, gProjPalettes[gLastProjPalette]);
            }
        }
        gDPSetTileSize(gDisplayListHead++, G_TX_RENDERTILE, tex->s, tex->t, tex->s + size, tex->t + size);
        if (tex->flags & 0x20) {
            gSPTexture(gDisplayListHead++, size << 6, size << 6, 0, G_TX_RENDERTILE, G_ON);
            geom |= G_LIGHTING | G_TEXTURE_GEN;
        } else {
            gSPTexture(gDisplayListHead++, 0x8000, 0x8000, 0, G_TX_RENDERTILE, G_ON);
        }
    } else {
        gDPPipeSync(gDisplayListHead++);
        gDPSetCombineLERP(gDisplayListHead++, 1, SHADE, ENVIRONMENT, SHADE, SHADE, 0, ENVIRONMENT, 0,
                          0, 0, 0, COMBINED, 0, 0, 0, COMBINED);
        gSPTexture(gDisplayListHead++, 0x8000, 0x8000, 0, G_TX_RENDERTILE, G_OFF);
    }
    gSPClearGeometryMode(gDisplayListHead++, G_LIGHTING | G_TEXTURE_GEN);
    if (geom != 0) {
        gSPSetGeometryMode(gDisplayListHead++, geom);
    }
}
extern Vec3f gBrianFloorPos;             /* Brian's point on the floor (his shadow, the glows) */
extern Gfx gGlowRenderSetup[];             /* glow render setup */
extern Gfx gGlowGfx[];             /* the glow */
void BuildLookAtMatrix(MtxF *mtx, f32 x, f32 y, f32 z, f32 atX, f32 atY, f32 atZ);
void ResetGlows(void) {
    s32 i;

    for (i = 0; i < 4; i++) {
        gGlowSlots[i].timer = 0;
    }
}

void DrawGlows(mkst* dyn) {
    static u8 gGlowColours[4][4] = {              /* the glows' colours */
        { 0xFF, 0x00, 0x00, 0x00 }, { 0xFF, 0xFF, 0x00, 0x00 }, { 0x00, 0x00, 0xFF, 0x00 }, { 0x00, 0xFF, 0x00, 0x00 },
    };
    f32 x;
    f32 y;
    f32 z;
    s32 i;
    s32 started;
    GlowSlot* glow;
    u32 alpha;
    MtxF mtx;
    u8 (*c)[4];

    started = 0;
    for (glow = gGlowSlots, i = 0; i != 4; i++, glow++) {
        if (glow->timer != 0) {
            if (gMatrixHead < 486) {
                if (started == 0) {
                    gSPDisplayList(gDisplayListHead++, gGlowRenderSetup);
                    started++;
                }
                alpha = 200 - getRandomNumber(32);
                if (glow->timer < 8) {
                    alpha = (glow->timer * alpha) >> 3;
                } else if (glow->timer >= 22) {
                    alpha = ((25 - glow->timer) * alpha) >> 2;
                }
                x = gBrianFloorPos.x;
                y = gPlayerState.brian.transform.scale * 70.0 + gBrianFloorPos.y;
                z = gBrianFloorPos.z;
                BuildLookAtMatrix(&mtx, x, y, z, gCamera.cam_x, gCamera.cam_y, gCamera.cam_z);
                ScaleMatrix(&mtx, gPlayerState.brian.transform.scale, gPlayerState.brian.transform.scale, gPlayerState.brian.transform.scale);
                guMtxF2L(&mtx, &dyn->drawMtx[gMatrixHead]);
                gSPMatrix(gDisplayListHead++, D_2000000 + 3 + gMatrixHead++, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
                c = gGlowColours + i;
                gDPSetEnvColor(gDisplayListHead++, (*c)[0], (*c)[1], (*c)[2], alpha);
                gSPDisplayList(gDisplayListHead++, gGlowGfx);
            }
            glow->timer--;
        }
    }
}

void StartLevelUpEffect(s32 stat) {
    gGlowSlots[stat].timer = 0x19;
}

