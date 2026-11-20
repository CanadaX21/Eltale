#ifndef GAME_SPELLS_H
#define GAME_SPELLS_H

/* The functions of src/game/spells.c, declared once for every file that calls them. */

#include "common.h"

/* spell update (UpdateSpells; names from vbhayden's Quest 64 scratches) */
typedef struct {
    u16 timer;          /* a view of ProjSlot (effects.c); 7 = finish */
    char unk2[6];
    u16 flags;          /* 0x08: 1 = active */
    char unkA[2];
    f32 x;              /* 0x0C */
    f32 y;
    f32 z;
    char unk18[0x10];
    f32 vx;             /* 0x28 */
    f32 vy;
    f32 vz;
} SpellFx;
typedef struct {
    u16 school;
    u16 index;
    u16 lifetime;       /* 0x04 */
    u16 trail_timer;    /* 0x06 */
    u16 delay;          /* 0x08 */
    u16 flags;          /* 0x0A: SPELLSLOT_ (below the struct) */
    f32 x;              /* 0x0C */
    f32 y;
    f32 z;
    f32 vx;             /* 0x18 */
    f32 vy;
    f32 vz;
    MonsterBattleData* caster;  /* 0x24 */
    MonsterBattleData* target;  /* 0x28 */
    SpellDef* def;              /* 0x2C */
    SpellAnimDef* anim;         /* 0x30 */
    SpellFx* fx;                /* 0x34 */
    SpellFx* hitFx;             /* 0x38 */
} SpellSlot;            /* gSpellSlots[16], 0x3C each */

/* SpellSlot.flags (SpawnSpell's flags argument, then the spell's own) */
#define SPELLSLOT_LANDED   0x01  /* it has landed: it stops moving, and collision kind 2 hits only now */
#define SPELLSLOT_HIT      0x02  /* it has hit once (a SPELL_PIERCES spell): it homes no more */
#define SPELLSLOT_SURE     0x04  /* sure to hit, its statuses sure (an item's spell) */
#define SPELLSLOT_BOOSTED  0x10  /* Brian with all four at 50: twice the extra shots, half again the lifetime */
/* GetSpellAvailability's result: what keeps the spell from being cast (0: nothing) */
#define SPELL_LOW_MP       0x1
#define SPELL_NOT_HERE     0x2   /* not castable in battle, or out of it (SpellDef.on_hit) */
#define SPELL_LOW_SPIRITS  0x4   /* the element's spirits below SpellDef.minSpirits */
/* PlayerState.path[]: a spell's directions, one per C button press (BrianSpellInput); the first press is the element
   (ELEMENT_FIRE .. ELEMENT_WIND) */
#define SPELL_DIR_UP     0
#define SPELL_DIR_LEFT   1
#define SPELL_DIR_DOWN   2
#define SPELL_DIR_RIGHT  3
typedef struct {
    u16 shots_left;
    u16 timer;
    u16 shot;
    u16 unk6;
    s32 spell;          /* 0x08 */
    u16 flags;
    u16 unkE;
    SpellDef* def;          /* 0x10: the spell (shot_gap: frames between shots) */
    MonsterBattleData* caster;  /* 0x14 */
} SpellQueue;           /* gSpellQueues[4], 0x18 each */

void ResetSpells(void);
s32 GetSpellFromPath(u16 presses, u8* path);
s32 GetSpellAvailability(u32 spell, u16 mp);
void SpawnSpell(MonsterBattleData* caster, u32 spell, s32 shot, u16 flags);   /* cast the spell */
void SpellOrigin_Feet(SpellSlot* spell, MonsterBattleData* monster);
void SpellOrigin_Middle(SpellSlot* spell, MonsterBattleData* monster);
void SpellOrigin_Above(SpellSlot* spell, MonsterBattleData* monster);
void SpellOrigin_FrontMiddle(SpellSlot* spell, MonsterBattleData* monster);
void SpellOrigin_FrontAbove(SpellSlot* spell, MonsterBattleData* monster);
void SpellOrigin_BehindAbove(SpellSlot* spell, MonsterBattleData* monster);
void SpellOrigin_TargetFeet(SpellSlot* spell, MonsterBattleData* monster);
void SpellOrigin_RandomAbove(SpellSlot* spell, MonsterBattleData* monster);
void SpellOrigin_Ahead(SpellSlot *spell, MonsterBattleData *monster);
void SpellOrigin_HighAhead(SpellSlot* spell, MonsterBattleData* monster);
void UpdateSpells(void);
s32 AnySpellActive(void);
void SpellMovement0(SpellSlot* spell);
void SpellMovement1(SpellSlot* spell);
MonsterBattleData* SpellCollision1(SpellSlot* spell, s32 slot);
s32 SpellCollision2(SpellSlot* spell);
void SpellEffect_Damage(SpellSlot* spell, MonsterBattleData* hit);
void SpellEffect_Heal(SpellSlot* spell, MonsterBattleData* hit);
void SpellEffect_DrainMP(SpellSlot* spell, MonsterBattleData* hit);
void SpellEffect_Escape(SpellSlot* spell, MonsterBattleData* hit);
void SpellEffect_Return(SpellSlot* spell, MonsterBattleData* hit);
void SpellEffect_StatusOnly(SpellSlot* spell, MonsterBattleData* hit);
void SpellEffect_Exit(SpellSlot* spell, MonsterBattleData* hit);
void SpellEffect_InstantKill(SpellSlot* spell, MonsterBattleData* hit);
void SpellEffect_Cure(SpellSlot* spell, MonsterBattleData* hit);
void SpellEffect_DamageClearStats(SpellSlot* spell, MonsterBattleData* hit);
void SpellTarget_Facing(SpellSlot* spell);
void SpellTarget_FirstEnemy(SpellSlot* spell);
void SpellTarget_Self(SpellSlot* spell);
void ApplyStatusSlot0(s32 level, MonsterBattleData* target);
void ApplyStatusSlot1(s32 level, MonsterBattleData* target);
void ApplyStatusSlot2(s32 level, MonsterBattleData* target);
void ApplyStatusSlot3(s32 level, MonsterBattleData* target);
void ApplyStatusSlot4(s32 level, MonsterBattleData* target);
void ApplyStatusSlot5(s32 level, MonsterBattleData* target);
void ApplyStatusSlot6(s32 level, MonsterBattleData* target);
void ApplyStatusSlot7(s32 level, MonsterBattleData* target);
void ApplyStatusSlot8(s32 level, MonsterBattleData* target);
void ApplyStatusSlot9(s32 level, MonsterBattleData* target);
void ApplyStatusSlot10(s32 level, MonsterBattleData* target);
void ApplyStatusSlot11(s32 level, MonsterBattleData* target);
u16 CalcSpellPower(SpellDef* def, MonsterBattleData* caster);
void* SpawnEffectObject(u16 colour, u16 flags, f32 x, f32 y, f32 z, f32 angle, EffectSpawnDef* model, OddVector* size, ActorTransform* actor);
void StartSpellVisual(SpellSlot* spell, MonsterBattleData* target);
void SpawnSpellTrail(SpellSlot* spell, MonsterBattleData* target);
void StartSpellHitVisual(SpellSlot* spell, MonsterBattleData* hit);
void StartSpellLandVisual(SpellSlot* spell, MonsterBattleData* target);
void CountDownStatuses(MonsterBattleData* actor);
s32 TestSpellHitMask(s32 slot, MonsterBattleData* hit);
void SetSpellHitMask(s32 slot, MonsterBattleData* hit);
void ClearHitMasks(void);
s32 RollSpellAccuracy(SpellSlot* spell, MonsterBattleData* hit);
s32 RollStatus(SpellSlot* spell, MonsterBattleData* caster, s32 effect);
void RollStatusDuration(SpellSlot* spell, MonsterBattleData* hit, s32 effect);

/* The state src/game/spells.c defines (.bss), in its storage order: the order of these declarations is the
 * unit's .bss layout (include/bss_order.h). */
extern s32 gSpellLightState;
extern s32 gSpellLightTimer;
extern SpellSlot gSpellSlots[16];   /* the spell slots */
extern SpellQueue gSpellQueues[4];   /* ResetSpells clears 4 */
extern OddVector gSpellOffset;   /* an offset the spells turn (x, y as a Coordinates2D) and an effect's
                                            size (x, y and scale: the spell visuals pass it to
                                            SpawnEffectObject) */
extern EnemyAction* gLastAutoHitEnemy;   /* last enemy hit by an auto-hit spell (Quest D_800871DC) */
extern CollisionPollingResult gGroundSample;   /* the spells' ground sample (Quest D_800871E0) */

typedef struct {
    f32 v[5];
} SpellTable5;          /* a small per-index table copied to the stack (struct copy) */

/* Storage the C does not define yet (assembly or binary), declared here for its type. */
extern void (*gSpellOriginHandlers[])(SpellSlot*, MonsterBattleData*);   /* assembly */
extern void (*gSpellTargetHandlers[])(SpellSlot*);   /* assembly */
extern void (*gSpellMovementHandlers[])(SpellSlot*);   /* assembly */
extern MonsterBattleData* (*gSpellCollisionHandlers[])(SpellSlot*, s32);   /* assembly */
extern void (*gSpellEffectHandlers[])(SpellSlot*, MonsterBattleData*);   /* assembly */
extern SpellTable5 gSpellSpawnRadii;   /* spawn radius by SpellDef.spawn_radius */
extern SpellTable5 gSpellHomingStrength;   /* movement 0: steering strength by SpellDef.homing */
extern SpellTable5 gSpellSpeeds;   /* movement 0: speed by SpellDef.speed */

#endif
