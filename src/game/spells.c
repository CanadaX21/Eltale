#include "common.h"
u32 getRandomNumber(u32 range);
extern u16 gHitMaskBits[];
extern u16 gBattleState;
extern s32 gEnemyCount;
extern u16 gMapFlags;   /* map kind flags (Quest D_80084EE0) */
extern s32 gReturnDest;   /* Return destination map/submap/entry (Quest D_80084EF8..) */
extern s32 gReturnRoom;
extern s32 gReturnEntrance;
extern s32 gCurrentMap;   /* current map (Quest D_80084EE4) */
extern s32 gExitDest;   /* Exit destination map/submap/entry (Quest D_80084F04..) */
extern s32 gExitRoom;
extern s32 gExitEntrance;
extern EnemyAction gEnemies[];   /* enemy table, 6 x 0x130 (Quest D_8007C998) */
extern EnemyAction* gLastAutoHitEnemy;    /* last enemy hit by an auto-hit spell (Quest D_800871DC) */
s32 TestSpellHitMask(s32 slot, MonsterBattleData* hit);
extern CollisionPollingResult gGroundSample;   /* the spells' ground sample (Quest D_800871E0) */
extern f64 D_8006F308;     /* 0.3: projectile fall acceleration (Quest D_800714A8) */
void PollWorldCollision(f32 sample_x, f32 sample_y, f32 sample_z, u16 arg3_flags, CollisionPollingResult* result);
void StartCameraShake(s32 kind);
void* AllocProjectileSlot(s32 anim, s32 model, s32 texture, s32 colour, s32 flags, f32 x, f32 y, f32 z, f32 angle,
                    f32 scaleX, f32 scaleY, f32 scaleZ);
extern Submap* gSubmapHeader;
extern PendingWarp gPendingWarp;
extern s32 gArrivalFlags;
extern PlayerState gPlayerState;
extern s32 gGameState;
extern s32 gModeCountdown;
void Setup_MISS_Icon(MonsterBattleData* actor);
MonsterBattleData* SpellCollision0();
float sqrtf(float value);
extern Coordinates2D gSpellOffset;
void rotateCoordinatesByAngle(f32 angle, Coordinates2D* coordinates);
typedef struct {
    char unk0[0x28];
    f32 vx;
    f32 vy;
    f32 vz;
} SpellEffectSlot;  /* unused (SpellFx is the whole object); kept for the declaration count */  /* returned by SpawnEffectObject (AllocProjectileSlot's 0x58 slot) */
typedef struct {
    u16 kind;       /* 1 / 2: which burst StartSpellVisual starts (on the caster, at spawn) */
    u16 arg;
    u16 land_kind;  /* 0x04: on the target when the object lands (StartSpellLandVisual) */
    u16 land_arg;
    u16 hit_kind;   /* 0x08: on the target when a hit resolves (StartSpellHitVisual) */
    u16 hit_arg;
} SpellBurstDef;    /* gSpellBurstDefs[school][index], 0xC each: the per-slot effect entry */
extern EffectSpawnDef gEffectModels[];
extern SpellBurstDef* gSpellBurstDefs[];
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
    char unk0[0x10];
    s16 trail_model;    /* 0x10: second animation stage, < 0 = none */
    u16 trail_rate;     /* 0x12: high byte = frames between trail pieces */
} SpellAnimStages;
typedef struct {
    u16 school;
    u16 index;
    u16 lifetime;       /* 0x04 */
    u16 trail_timer;    /* 0x06 */
    u16 delay;          /* 0x08 */
    u16 flags;          /* 0x0A: 2 = has hit */
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
void StartSpellLandVisual(SpellSlot* spell, MonsterBattleData* target);
extern SpellSlot gSpellSlots[16];   /* the spell slots */
typedef struct {
    f32 v[5];
} SpellTable5;          /* a small per-index table copied to the stack (struct copy) */
extern SpellTable5 gSpellHomingStrength;  /* movement 0: steering strength by SpellDef.homing */
extern SpellTable5 gSpellSpeeds;  /* movement 0: speed by SpellDef.speed */
typedef struct {
    u16 shots_left;
    u16 timer;
    u16 shot;
    u16 unk6;
    s32 spell;          /* 0x08 */
    u16 unkC;
    u16 unkE;
    SpellDef* def;          /* 0x10: the spell (shot_gap: frames between shots) */
    MonsterBattleData* caster;  /* 0x14 */
} SpellQueue;           /* gSpellQueues[4], 0x18 each */
extern SpellQueue gSpellQueues[];
extern void (*gSpellMovementHandlers[])(SpellSlot*);
extern MonsterBattleData* (*gSpellCollisionHandlers[])(SpellSlot*, s32);
extern void (*gSpellEffectHandlers[])(SpellSlot*, MonsterBattleData*);
extern void (*gApplyStatusHandlers[])(u8, MonsterBattleData*);
extern s32 gSpellLightState;
extern s32 gSpellLightTimer;
extern s32 gMapLightAnim;
extern s32 gSpellEndLights[];
void SpawnSpell(MonsterBattleData* caster, u32 spell, s32 shot, u16 flags);
void SetSpellHitMask(s32 slot, MonsterBattleData* hit);
s32 RollSpellAccuracy(SpellSlot* spell, MonsterBattleData* hit);
s32 RollStatus(SpellSlot* spell, MonsterBattleData* caster, s32 effect);
void RollStatusDuration(SpellSlot* spell, MonsterBattleData* hit, s32 effect);
void SpawnSpellTrail(SpellSlot* spell, MonsterBattleData* target);
void StartSpellHitVisual(SpellSlot* spell, MonsterBattleData* hit);
void ShowEnemyMiss(MonsterBattleData* actor);
void StartBrianDodge();   /* RULE-EXCEPTION R25: unprototyped; this caller passes the caster, the definition in player.c takes nothing */
void SetLightAnimLights(s32 n, s32 mode, s32 count, s32* keys);
f32 Atan2(f32 dx, f32 dz);
void QueueSparkleBurst(s32 kind, s32 frames, f32 x, f32 y, f32 z, f32 size);
void SpawnSparkleBurst(s32 colour, f32 x, f32 y, f32 z, f32 size);
void PlaySound(u8 delay, s8 id, u8 level);

void ResetSpells(void) {
    s32 i;

    for (i = 0; i < 16; i++) {
        gSpellSlots[i].lifetime = 0;
    }
    for (i = 0; i < 4; i++) {
        gSpellQueues[i].shots_left = 0;
    }
    gSpellLightState = 0;
}

extern SpellDef* gSpellDefs[];   /* each element's first spell record */
s32 GetSpellFromPath(u16 presses, u8* path) {
    SpellDef* def;
    u8 element;
    u8 tmp;
    u32 i;
    u32 j;
    u8 sorted[3];

    element = path[0];
    for (i = 0; i < presses; i++) {
        sorted[i] = path[i];
    }
    if (presses >= 3) {
        if (sorted[2] < sorted[1]) {
            tmp = sorted[2];
            sorted[2] = sorted[1];
            sorted[1] = tmp;
        }
    }
    def = gSpellDefs[element];
    for (i = 15; i != 0; i--, def++) {
        if (presses == def->presses) {
            for (j = 0; j < def->presses; j++) {
                if (sorted[j] != def->path[j]) {
                    break;
                }
            }
            if (j == def->presses) {
                break;
            }
        }
    }
    if (i == 0) {
        i = 15;
    }
    return (element << 8) - i + 15;
}
extern u16 gSpellMpCosts[];        /* MP cost by press count, [1..3] */
s32 GetSpellAvailability(u32 spell, u16 mp) {
    s32 element;
    SpellDef* def;
    s32 result;

    result = 0;
    element = (spell >> 8) & 0xF;
    spell &= 0xFF;                  /* RULE-EXCEPTION R30: spell becomes the index, as in DrawSpellLabel; a separate index local loses the match */
    def = &gSpellDefs[element][spell];
    if (gPlayerState.stats->spirits[element] < def->minSpirits) {
        result = 4;
    }
    if (!(((gBattleState & 1) ? 1 : 2) & def->on_hit)) {
        result |= 2;
    }
    if (mp < gSpellMpCosts[def->presses]) {
        result |= 1;
    }
    return result;
}

extern SpellTable5 gSpellSpawnRadii;  /* spawn radius by SpellDef.spawn_radius */
extern Vec3f gSpellSpawnOffsets[];      /* spawn offsets, 8 entries, by (spawn_pattern + shot) & 7 */
extern SpellAnimDef* gSpellAnimDefs[];
extern s32 gSpellCastLights[];
extern void (*gSpellTargetHandlers[])(SpellSlot*);
extern void (*gSpellOriginHandlers[])(SpellSlot*, MonsterBattleData*);
void StartSpellVisual(SpellSlot* spell, MonsterBattleData* target);
void ClearHitMaskBit();   /* RULE-EXCEPTION R25: unprototyped; this caller passes four arguments (a0-a3), the definition takes one (four parameters add -g3 arg stores) */
void SpawnSpell(MonsterBattleData* caster, u32 spell, s32 shot, u16 flags) {
    SpellQueue* queue;
    SpellSlot* slot;
    s32 i;
    s32 element;
    SpellDef* def;
    s32 index;
    SpellTable5 radius;
    SpellAnimDef* anim;

    radius = gSpellSpawnRadii;
    if (shot == 0) {
        SetLightAnimLights(gMapLightAnim, 2, 8, gSpellCastLights);
        gSpellLightState |= 1;
        gSpellLightTimer = 8;
    }
    element = (spell >> 8) & 0xF;
    index = spell & 0xFF;
    def = &gSpellDefs[element][index];
    anim = &gSpellAnimDefs[element][index];
    if ((shot == 0) && (def->shots >= 2)) {
        queue = gSpellQueues;
        i = 3;
        while (i != 0 && queue->shots_left != 0) {
            i--;
            queue++;
        }
        queue->shots_left = def->shots - 1;
        if (flags & 0x10) {
            queue->shots_left *= 2;
        }
        queue->timer = def->shot_gap + 1;
        queue->shot = 0;
        queue->spell = spell;
        queue->unkC = flags;
        queue->def = def;
        queue->caster = caster;
    }
    slot = gSpellSlots;
    i = 15;
    while (i != 0 && slot->lifetime != 0) {
        i--;
        slot++;
    }
    slot->school = element;
    slot->index = index;
    slot->caster = caster;
    slot->target = NULL;
    slot->def = def;
    slot->anim = anim;
    slot->lifetime = def->lifetime;
    slot->delay = def->delay;
    slot->trail_timer = 100;
    slot->flags = flags;
    ClearHitMaskBit(15 - i, index, def, element);
    switch (def->projectile_type) {
        case 0:
            gSpellOffset.x = gSpellSpawnOffsets[(def->spawn_pattern + shot) & 7].x * radius.v[def->spawn_radius];
            gSpellOffset.y = gSpellSpawnOffsets[(def->spawn_pattern + shot) & 7].z * radius.v[def->spawn_radius];
            rotateCoordinatesByAngle(caster->transform.yrot, &gSpellOffset);
            slot->vx = gSpellOffset.x;
            slot->vz = gSpellOffset.y;
            if (def->gravity & 2) {
                slot->vy = 0.0f;
            } else {
                slot->vy = gSpellSpawnOffsets[(def->spawn_pattern + shot) & 7].y * radius.v[def->spawn_radius];
            }
            if (flags & 0x10) {
                slot->lifetime += slot->lifetime >> 1;
            }
            break;
        case 1:
        case 2:
            slot->vz = 0.0f;
            slot->vy = 0.0f;
            slot->vx = 0.0f;
            break;
    }
    gSpellTargetHandlers[def->target_type](slot);
    if (slot->target != NULL) {
        gSpellOriginHandlers[def->originType](slot, caster);
        if (def->groundMode == 2) {
            PollWorldCollision(slot->x, slot->y, slot->z, 0xA00, &gGroundSample);
            slot->y = gGroundSample.y;
        }
        StartSpellVisual(slot, slot->caster);
        return;
    }
    slot->lifetime = 0;
}
void SpellOrigin_Feet(SpellSlot* spell, MonsterBattleData* monster) {

    spell->x = monster->transform.pos.x;
    spell->z = monster->transform.pos.z;
    if (monster->transform.base.monster->monsterType == FLYING) {
        spell->y = monster->stats->flyY - (monster->transform.base.monster->radius * monster->transform.scale);
        return;
    }
    spell->y = monster->transform.pos.y;
}
void SpellOrigin_Middle(SpellSlot* spell, MonsterBattleData* monster) {
    
    spell->x = monster->transform.pos.x;
    spell->z = monster->transform.pos.z;
    if (monster->transform.base.monster->monsterType == FLYING) {
        spell->y = monster->stats->flyY;
        return;
    }
    spell->y = monster->transform.pos.y + monster->transform.base.monster->height * 0.5 * monster->transform.scale;
}
void SpellOrigin_Above(SpellSlot* spell, MonsterBattleData* monster) {

    spell->x = monster->transform.pos.x;
    spell->z = monster->transform.pos.z;
    if (monster->transform.base.monster->monsterType == FLYING) {
        spell->y = (monster->stats->flyY + (monster->transform.base.monster->radius * monster->transform.scale) + spell->def->radius);
        return;
    }
    spell->y = (monster->transform.pos.y + (2.0 * monster->transform.base.monster->height * monster->transform.scale) + spell->def->radius);
}
void SpellOrigin_FrontMiddle(SpellSlot* spell, MonsterBattleData* monster) {

    gSpellOffset.x = 0.0f;
    gSpellOffset.y = (monster->transform.base.monster->radius * monster->transform.scale) + spell->def->radius;
    rotateCoordinatesByAngle(monster->transform.yrot, &gSpellOffset);
    spell->x = monster->transform.pos.x + gSpellOffset.x;
    spell->z = monster->transform.pos.z + gSpellOffset.y;
    if (monster->transform.base.monster->monsterType == FLYING) {
        spell->y =  monster->stats->flyY;
        return;
    }
    spell->y =  ( monster->transform.pos.y + ( monster->transform.base.monster->height * 0.5 *  monster->transform.scale));
}
void SpellOrigin_FrontAbove(SpellSlot* spell, MonsterBattleData* monster) {

    gSpellOffset.x = 0.0f;
    gSpellOffset.y = (monster->transform.base.monster->radius * monster->transform.scale) + spell->def->radius;
    rotateCoordinatesByAngle(monster->transform.yrot, &gSpellOffset);
    spell->x = monster->transform.pos.x + gSpellOffset.x;
    spell->z = monster->transform.pos.z + gSpellOffset.y;
    if (monster->transform.base.monster->monsterType == FLYING) {
        spell->y = monster->stats->flyY + (monster->transform.base.monster->radius * monster->transform.scale) + spell->def->radius;
        return;
    }
    spell->y =  ( monster->transform.pos.y+ (2.0 *  monster->transform.base.monster->height *  monster->transform.scale) +  spell->def->radius);
}
void SpellOrigin_BehindAbove(SpellSlot* spell, MonsterBattleData* monster) {

    gSpellOffset.x = 0.0f;
    gSpellOffset.y = (-monster->transform.base.monster->radius * monster->transform.scale) + spell->def->radius;
    rotateCoordinatesByAngle(monster->transform.yrot, &gSpellOffset);
    spell->x = monster->transform.pos.x + gSpellOffset.x;
    spell->z = monster->transform.pos.z + gSpellOffset.y;
    if (monster->transform.base.monster->monsterType == FLYING) {
        spell->y = monster->stats->flyY + (monster->transform.base.monster->radius * monster->transform.scale) + spell->def->radius;
        return;
    }
    spell->y =  (monster->transform.pos.y + (2.0 *  monster->transform.base.monster->height *  monster->transform.scale) +  spell->def->radius);
}
void SpellOrigin_TargetFeet(SpellSlot* spell, MonsterBattleData* monster) {

    if (spell->target != NULL) {
        monster = spell->target;
    }
    spell->x = monster->transform.pos.x;
    spell->z = monster->transform.pos.z;
    if (monster->transform.base.monster->monsterType == FLYING) {
        spell->y = monster->stats->flyY - (monster->transform.base.monster->radius * monster->transform.scale);
        return;
    }
    if (monster->transform.base.monster->monsterType == MAMMON) {
        spell->y = monster->transform.pos.y + (100.0f * monster->transform.scale);
        return;
    }
    spell->y = monster->transform.pos.y;
}
void SpellOrigin_RandomAbove(SpellSlot* spell, MonsterBattleData* monster) {

    gSpellOffset.x = 0.0f;
    gSpellOffset.y = (f32) getRandomNumber(20) + 20.0;
    rotateCoordinatesByAngle((f32) getRandomNumber(16) * M_PI / 8 - M_PI, &gSpellOffset);
    spell->x = monster->transform.pos.x + gSpellOffset.x;
    spell->z = monster->transform.pos.z + gSpellOffset.y;
    if (monster->transform.base.monster->monsterType == FLYING) {
        spell->y = monster->stats->flyY + (monster->transform.base.monster->radius * monster->transform.scale) + spell->def->radius;
        return;
    }
    spell->y = monster->transform.pos.y + (2.0 * monster->transform.base.monster->height * monster->transform.scale) + spell->def->radius;
}
void SpellOrigin_Ahead(SpellSlot *spell, MonsterBattleData *monster)
{
  f32 d;
  f32 dx;
  f32 dist;
    
  if (monster->transform.actorFlags & 1)
  {
    d = spell->target->transform.pos.x - monster->transform.pos.x;
    dx = d;
    d = spell->target->transform.pos.z - monster->transform.pos.z;
    dist = sqrtf((dx * dx) + (d * d));
  }
  else
  {
    dist = monster->transform.base.monster->radius * 10.0 * monster->transform.scale;
  }
  gSpellOffset.y = dist;
  gSpellOffset.x = dist * 1.5;
  rotateCoordinatesByAngle(monster->transform.yrot, &gSpellOffset);
  spell->x = monster->transform.pos.x + gSpellOffset.x;
  spell->z = monster->transform.pos.z + gSpellOffset.y;
  if (monster->transform.base.monster->monsterType == FLYING)
  {
    spell->y = monster->stats->flyY - (monster->transform.base.monster->radius * monster->transform.scale);
    return;
  }
  spell->y = monster->transform.pos.y;
}
void SpellOrigin_HighAhead(SpellSlot* spell, MonsterBattleData* monster) {
    gSpellOffset.x = 0.0f;
    gSpellOffset.y = 380.0f;
    rotateCoordinatesByAngle(monster->transform.yrot, &gSpellOffset);
    spell->x = monster->transform.pos.x + (gSpellOffset.x * monster->transform.scale);
    spell->y = monster->transform.pos.y + (380.0f * monster->transform.scale);
    spell->z = monster->transform.pos.z + (gSpellOffset.y * monster->transform.scale);
}
void UpdateSpells(void) {
    SpellQueue* context;
    SpellSlot* spell_obj;
    s32 active;
    s32 j;
    s32 effects_applied;
    s32 effects_attempted;
    MonsterBattleData* caster;
    MonsterBattleData* actor_hit;
    SpellAnimDef* spell_anims;
    s32 i;

    context = gSpellQueues;
    for (i = 4; i != 0; i--, context++) {
        if (context->shots_left == 0) {
            continue;
        }
        context->timer--;
        if (!context->timer) {
            context->shots_left--;
            context->shot++;
            SpawnSpell(context->caster, context->spell, context->shot, context->unkC);
            context->timer = (u16)(context->def->shot_gap + 1);
        }
    }

    spell_obj = gSpellSlots;
    active = 0;
    for (i = 0; i < 16; i++, spell_obj++) {
        if (spell_obj->lifetime != 0) {
            active++;
            if (spell_obj->delay != 0) {
                spell_obj->delay--;
            } else {
                spell_obj->lifetime--;

                gSpellMovementHandlers[spell_obj->def->projectile_type](spell_obj);

                actor_hit = gSpellCollisionHandlers[spell_obj->def->auto_hit](spell_obj, i);
                if (actor_hit != NULL) {
                    SetSpellHitMask(i, actor_hit);
                    if (!(actor_hit->stats->statusFlags & 0x100) || (spell_obj->def->target_type == 2)) {
                        if (RollSpellAccuracy(spell_obj, actor_hit) != 0) {
                            caster = spell_obj->caster;
                            if (actor_hit == caster) {
                                Setup_MISS_Icon(caster);
                            } else if (actor_hit->transform.actorFlags & 1) {
                                ShowEnemyMiss(actor_hit);
                            } else {
                                StartBrianDodge(caster);
                            }
                        } else {
                            gSpellEffectHandlers[spell_obj->def->spell_type](spell_obj, actor_hit);
                            effects_applied = 0;
                            effects_attempted = 0;
                            for (j = 0; j < 12; j++) {
                                if (spell_obj->def->effects[j] != 0) {
                                    effects_attempted++;
                                    if (RollStatus(spell_obj, spell_obj->caster, j) != 0) {
                                        RollStatusDuration(spell_obj, actor_hit, j);
                                        gApplyStatusHandlers[j](spell_obj->def->effects[j], actor_hit);
                                        effects_applied++;
                                    }
                                }
                            }

                            if ((spell_obj->def->spell_type == 5) && (effects_attempted != 0) && (effects_applied == 0)) {
                                Setup_MISS_Icon(actor_hit);
                            }
                        }
                    }
                    StartSpellHitVisual(spell_obj, actor_hit);
                    if (!(spell_obj->def->on_hit & 4)) {
                        spell_obj->lifetime = 0;
                        if ((spell_obj->fx != NULL) && (spell_obj->fx->flags & 1)) {
                            spell_obj->fx->timer = 7;
                            spell_obj->fx->flags &= ~1;
                        }
                    } else {
                        spell_obj->flags |= 2;
                    }
                }
            }

            if (spell_obj->fx != NULL) {
                spell_obj->fx->x = spell_obj->x;
                spell_obj->fx->y = spell_obj->y;
                spell_obj->fx->z = spell_obj->z;
                spell_obj->fx->vx = spell_obj->vx;
                spell_obj->fx->vy = spell_obj->vy;
                spell_obj->fx->vz = spell_obj->vz;

                if (spell_obj->lifetime == 0) {
                    if (spell_obj->fx->flags & 1) {
                        spell_obj->fx->timer = 7;
                        spell_obj->fx->flags &= ~1;
                    }
                }
            }
            spell_anims = spell_obj->anim;
            if (spell_anims->trail_model >= 0) {
                j = spell_anims->trail_rate;
                if ((u32)spell_obj->trail_timer >= j >> 8) {
                    SpawnSpellTrail(spell_obj, spell_obj->target);
                    spell_obj->trail_timer = 0;
                } else {
                    spell_obj->trail_timer++;
                }
            }
        }
    }
    if ((gSpellLightState & 1) && (active == 0)) {
        gSpellLightState = 2;
        gSpellLightTimer = 0x1E;
        return;
    }
    if (gSpellLightState & 2) {
        if (--gSpellLightTimer == 0) {
            gSpellLightState = 0;
            SetLightAnimLights(gMapLightAnim, 2, 8, gSpellEndLights);
        }
    }
}
s32 AnySpellActive(void) {
    SpellSlot* slot;
    s32 active;

    active = 16;
    slot = gSpellSlots;
    while (active != 0 && slot->lifetime == 0) {
        active--;
        slot++;
    }
    return active;
}
void SpellMovement0(SpellSlot* spell) {
    f32 len;
    f32 dx;
    f32 dy;
    f32 dz;
    f32 dist;
    f32 vx;
    f32 vy;
    f32 vz;
    f32 s;
    f32 k;
    MonsterBattleData* target;
    SpellTable5 steer;
    SpellTable5 speed;

    steer = gSpellHomingStrength;
    speed = gSpellSpeeds;
    target = spell->target;
    if (!(spell->flags & 1)) {
        if (target != NULL) {
            if ((spell->flags & 2) || (target->transform.modelID == -1)) {
                dz = 0.0f;
                dy = 0.0f;
                dx = 0.0f;
            } else {
                dx = target->transform.pos.x - spell->x;
                if (target->transform.base.monster->monsterType == FLYING) {
                    dy = target->stats->flyY - spell->y;
                } else {
                    dy = (target->transform.pos.y + (target->transform.base.monster->height * target->transform.scale) * 0.5) - spell->y;
                }
                dz = target->transform.pos.z - spell->z;
                dist = sqrtf((dx * dx) + (dy * dy) + (dz * dz));
                k = steer.v[spell->def->homing];
                dx = (k * dx) / dist;
                dy = (k * dy) / dist;
                dz = (k * dz) / dist;
            }
            vx = spell->vx;
            vy = spell->vy;
            vz = spell->vz;
            len = sqrtf((vx * vx) + (vy * vy) + (vz * vz));
            if (len != 0.0f) {
                vx = vx / (f64) len;
                vy = vy / (f64) len;
                vz = vz / (f64) len;
                dx += vx;
                dy += vy;
                dz += vz;
            }
            dist = sqrtf((dx * dx) + (dy * dy) + (dz * dz));
            s = speed.v[spell->def->speed];
            dx = (s * dx) / dist;
            dy = (s * dy) / dist;
            dz = (s * dz) / dist;
            spell->vx = (spell->vx * 0.9) + dx;
            spell->vy = (spell->vy * 0.9) + dy;
            spell->vz = (spell->vz * 0.9) + dz;
        }
        if (spell->def->gravity & 1) {
            spell->vy = spell->vy - 0.3;
        }
        spell->x += spell->vx;
        spell->y += spell->vy;
        spell->z += spell->vz;
        if (spell->def->groundMode != 0) {
            PollWorldCollision(spell->x, spell->y, spell->z, 0xA00, &gGroundSample);
            switch (spell->def->groundMode) {
                case 1:
                    if (spell->y < gGroundSample.y) {
                        spell->flags |= 1;
                        spell->lifetime = 8;
                        spell->vz = 0.0f;
                        spell->vy = 0.0f;
                        spell->vx = 0.0f;
                        if (spell->fx != NULL) {
                            spell->fx->timer = 0xF;
                            spell->fx->flags &= ~1;
                        }
                        StartCameraShake(1);
                        if (spell->anim->land_model >= 0) {
                            StartSpellLandVisual(spell, spell->target);
                        }
                    }
                    break;
                case 2:
                    spell->y = gGroundSample.y;
                    break;
            }
        }
    }
}
void SpellMovement1(SpellSlot* spell) {
    MonsterBattleData* proj_target;
    SpellFx* fx;

    if (!(spell->flags & 1)) {
        if (spell->def->gravity & 1) {
            spell->vy = spell->vy - 0.3;
        }

        spell->y = spell->y + spell->vy;

        if (spell->def->groundMode == 1) {
            PollWorldCollision(spell->x, spell->y, spell->z, 0xA00, &gGroundSample);

            if (spell->y < gGroundSample.y) {
                fx = spell->fx;

                spell->y = gGroundSample.y;
                spell->flags |= 1;
                spell->lifetime = 8;

                spell->vz = 0.0f;
                spell->vy = 0.0f;
                spell->vx = 0.0f;

                if (fx != NULL) {
                    fx->timer = 0xF;
                    fx = spell->fx;
                    fx->flags &= ~1;
                }

                StartCameraShake(1);
                StartSpellLandVisual(spell, spell->target);
            }
        }
        if (spell->def->target_type == 0) {
            proj_target = spell->target;
            if (!(proj_target->transform.stateFlags & ACTOR_BUSY)) {
                spell->x = proj_target->transform.pos.x;
                spell->z = proj_target->transform.pos.z;
            }
        }
    }
}
MonsterBattleData* SpellCollision0(spell, arg1)
    SpellSlot* spell;
    s32 arg1;
{
    MonsterBattleData* enemy;
    EnemyAction* iter;
    s32 count;
    f32 dx;
    f32 dy;
    MonsterBattleData* hit;
    f32 dz;

    if (spell->target != NULL) {
        if (!(((u16) spell->target->transform.actorFlags) & 1)) {
            hit = &gPlayerState.brian;
            if (TestSpellHitMask(arg1, spell->target) != 0) {
                hit = NULL;
            } else {
                dx = hit->transform.pos.x - spell->x;
                dz = hit->transform.pos.z - spell->z;
                dy = ((hit->transform.pos.y + (hit->transform.base.player->height * hit->transform.scale) * 0.5) - spell->y) * 0.5;
                if ((spell->def->radius + (hit->transform.base.player->radius * hit->transform.scale)) < sqrtf((dx * dx) + (dy * dy) + (dz * dz))) {
                    hit = NULL;
                }
            }
        } else {
            count = gEnemyCount;
            iter = gEnemies;
            while (count != 0) {
                enemy = &iter->battleData;
                if (enemy->transform.modelID != -1) {
                    hit = enemy;
                    if (TestSpellHitMask(arg1, enemy) == 0) {
                        dx = enemy->transform.pos.x - spell->x;
                        dz = enemy->transform.pos.z - spell->z;
                        switch (enemy->transform.base.monster->monsterType) {
                            default:
                                dy = 0.0f;
                                break;
                            case ON_GROUND:
                                dy = ((enemy->transform.pos.y + (enemy->transform.base.monster->height * enemy->transform.scale) * 0.5) - spell->y) * 0.5;
                                break;
                            case FLYING:
                                dy = (enemy->stats->flyY - spell->y) * 0.5;
                                break;
                        }
                        if (sqrtf((dx * dx) + (dy * dy) + (dz * dz)) < (spell->def->radius + (enemy->transform.base.monster->radius * enemy->transform.scale))) {
                            break;
                        }
                    }
                    count--;
                }
                iter++;
            }
            if (count == 0) {
                hit = NULL;
            } else {
                gLastAutoHitEnemy = iter;
            }
        }
    } else {
        hit = NULL;
    }
    return hit;
}

MonsterBattleData* SpellCollision1(SpellSlot* spell, s32 slot) {
    MonsterBattleData* enemy_transform;
    MonsterBattleData* spell_target;
    EnemyAction* iter_enemy;
    MonsterBattleData* enemy_hit;
    s32 enemies_to_check;

    spell_target = spell->target;

    if (!(spell->target->transform.actorFlags & 1)) {
        enemy_hit = &gPlayerState.brian;
        if (TestSpellHitMask(slot, &gPlayerState.brian) != 0) {
            enemy_hit = NULL;
        }
    } else {
        enemy_hit = spell->target;
        if ((enemy_hit == NULL) || (TestSpellHitMask(slot, enemy_hit) != 0)) {
            enemies_to_check = gEnemyCount;
            enemy_hit = NULL;
            iter_enemy = &gEnemies[0];
            while (enemies_to_check != 0) {
                enemy_transform = &iter_enemy->battleData;
                if (enemy_transform->transform.modelID != -1) {
                    if (TestSpellHitMask(slot, enemy_transform) == 0) {
                        enemy_hit = enemy_transform;
                        gLastAutoHitEnemy = iter_enemy;
                    }
                    enemies_to_check--;
                }
                iter_enemy++;
            }
        }
    }
    return enemy_hit;
}
s32 SpellCollision2(SpellSlot* spell) {
    s32 hit;

    hit = 0;
    if ((spell->flags & 1) != 0) {
        hit = SpellCollision0();
    }
    return hit;
}
extern u8 gCritFade[];
u16 CalcSpellPower(SpellDef* def, MonsterBattleData* caster);
void StartScreenFade(s32 frames, u16 flags, void* ramp);
void SpellEffect_Damage(SpellSlot* spell, MonsterBattleData* hit) {
    u16 power;
    u16 crit;

    crit = 0;
    power = CalcSpellPower(spell->def, spell->caster);
    if (spell->caster->stats->statusFlags & 1) {
        power >>= 1;
    }
    if (hit->transform.actorFlags & 1) {
        if (gPlayerState.flags & 0x1000) {
            if (getRandomNumber((gPlayerState.flags & 0x10000) ? 4 : 0x20) == 0) {
                power += (power + 1) >> 1;
                StartScreenFade(4, 0, gCritFade);
                crit = 0x4000;
            }
        }
        if (DamageEnemy(GetEnemyIndex(hit), power & 0xFFFF, spell->def->path[0], (u16) (spell->def->on_hit | crit), spell->caster) != 0) {
            gPlayerState.stats->MPXP++;
        }
    } else {
        if (getRandomNumber(0x20) == 0) {
            power += (power + 1) >> 1;
            StartScreenFade(4, 0, gCritFade);
            crit = 0x4000;
        }
        DamageBrian(power & 0xFFFF, (u16) (spell->def->on_hit | crit), spell->caster);
    }
}
void SpellEffect_Heal(SpellSlot* spell, MonsterBattleData* hit) {
    s32 index;
    u16 heal;

    if (hit->transform.actorFlags & 1) {
        index = GetEnemyIndex(hit);
        gLastAutoHitEnemy = &gEnemies[index];
        heal = (hit->transform.base.monster->maxHP * spell->def->power) / 100;
        heal += getRandomNumber(heal >> 2);
        gLastAutoHitEnemy->currHP += heal;
        if (hit->transform.base.monster->maxHP < gLastAutoHitEnemy->currHP) {
            gLastAutoHitEnemy->currHP = hit->transform.base.monster->maxHP;
        }
    } else {
        heal = (gPlayerState.stats->spirits[spell->def->path[0]] >> 1) + (hit->transform.base.monster->maxHP * spell->def->power) / 100;
        heal += getRandomNumber(heal >> 2);
        if (hit->transform.base.monster->currHP != hit->transform.base.monster->maxHP) {
            hit->transform.base.monster->currHP += heal;
            if (hit->transform.base.monster->maxHP < hit->transform.base.monster->currHP) {
                hit->transform.base.monster->currHP = hit->transform.base.monster->maxHP;
            }
            gPlayerState.stats->MPXP++;
        }
    }
    SpawnDamageNumber(hit, 1, heal);
}
void SpellEffect_DrainMP(SpellSlot* spell, MonsterBattleData* hit) {
    MonsterBattleData* brian = &gPlayerState.brian;
    u16 amount;

    if (hit->transform.actorFlags & 1) {
        gLastAutoHitEnemy = &gEnemies[GetEnemyIndex(hit)];
        amount = (gLastAutoHitEnemy->currMP * 11 + 63) >> 6;
        gLastAutoHitEnemy->currMP -= amount;
        if (gPlayerState.brian.transform.base.player->currMP != gPlayerState.brian.transform.base.player->maxMP) {
            brian->transform.base.player->currMP += amount;
            if (brian->transform.base.player->maxMP < brian->transform.base.player->currMP) {
                brian->transform.base.player->currMP = brian->transform.base.player->maxMP;
            }
            gPlayerState.stats->MPXP++;
        }
    } else {
        amount = (hit->transform.base.monster->currMP * 11 + 63) >> 6;
        hit->transform.base.monster->currMP -= amount;
        gLastAutoHitEnemy = &gEnemies[GetEnemyIndex(hit)];
        gLastAutoHitEnemy->currMP += amount;
        if (spell->caster->transform.base.monster->maxMP < gLastAutoHitEnemy->currMP) {
            gLastAutoHitEnemy->currMP = spell->caster->transform.base.monster->maxMP;
        }
    }
    SpawnDamageNumber(hit, 1, amount);
}

void SpellEffect_Escape(SpellSlot* spell, MonsterBattleData* hit) {
    if (hit->transform.actorFlags & 1) {
        hit->transform.modelID = -1;
        gEnemyCount--;
        return;
    }
    if (!(gBattleState & 0x100)) {
        gBattleState |= 0x10;
        gPlayerState.stats->MPXP++;
        return;
    }
    Setup_MISS_Icon(hit);
}
void SpellEffect_Return(SpellSlot* spell, MonsterBattleData* hit) {
    if ((gMapFlags == 4) && (gReturnDest != -1) && !(gSubmapHeader->flags & 4)) {
        gPendingWarp.map = gReturnDest;
        gPendingWarp.submap = gReturnRoom;
        gPendingWarp.entrance = gReturnEntrance;
        gArrivalFlags = 0x108;
        gPlayerState.animCue = 0;
        gPlayerState.stats->MPXP++;
        gGameState |= 0x80;
        gModeCountdown = 0x1E;
    }
}
void SpellEffect_StatusOnly(SpellSlot* spell, MonsterBattleData* hit) {
    s32 enemy_index;

    if (spell->def->target_type == 2) {
        return;
    }
    if (hit->transform.actorFlags & 1) {
        enemy_index = GetEnemyIndex(hit);
        DamageEnemy(enemy_index, 0, 0, 4, spell->caster);
        gPlayerState.stats->MPXP++;
    } else {
        DamageBrian(0, 4, spell->caster);
    }
}
void SpellEffect_Exit(SpellSlot* spell, MonsterBattleData* hit) {
    if ((gMapFlags & 8) && (gExitDest != -1) && !(gSubmapHeader->flags & 8) && (gCurrentMap != 0x1E) && (gCurrentMap != 0x22)) {
        gPendingWarp.map = gExitDest;
        gPendingWarp.submap = gExitRoom;
        gPendingWarp.entrance = gExitEntrance;
        gArrivalFlags = 0x108;
        gPlayerState.animCue = 0;
        gPlayerState.stats->MPXP++;
        gGameState |= 0x80;
        gModeCountdown = 0x1E;
    }
}
void SpellEffect_InstantKill(SpellSlot* spell, MonsterBattleData* hit) {
    if (hit->transform.actorFlags & 1) {
        if (!(gBattleState & 0x100)) {
            ClearStatusEffects(hit);
            hit->transform.modelID = -1;
            hit->transform.stateFlags &= ~ACTOR_BUSY;
            gEnemyCount--;
            gPlayerState.stats->MPXP++;
            return;
        }
        Setup_MISS_Icon(hit);
        return;
    }
    DamageBrian((hit->transform.base.monster->currHP * 4) & 0xFFFF, 4, spell->caster, hit);
}
void SpellEffect_Cure(SpellSlot* spell, MonsterBattleData* hit) {
    ActorStats* data;

    data = hit->stats;
    if (data->statusFlags != 0) {
        data->statusFlags = 0;
        if (hit->transform.actorFlags & 1) {
            GetEnemyIndex(hit);
        }
        hit->stats->atkScale = 0x10;
        hit->stats->def = hit->transform.base.monster->def;
        hit->stats->agi = hit->transform.base.monster->agi;
        hit->stats->moveScale = 1.0f;

        if (!(hit->transform.actorFlags & 1)) {
            gPlayerState.stats->MPXP++;
        }
    }
}
void SpellEffect_DamageClearStats(SpellSlot* spell, MonsterBattleData* hit) {
    SpellEffect_Damage(spell, hit);
    SpellEffect_Cure(spell, hit);
}
void SpellTarget_Facing(SpellSlot* spell) {
    MonsterBattleData* caster;
    EnemyAction* enemy;
    s32 count;
    f32 facing;
    f32 best;
    f32 angle;

    caster = spell->caster;
    if (caster->transform.actorFlags & 1) {
        spell->target = &gPlayerState.brian;
        return;
    }
    if ((spell->def->on_hit & 8) && ((spell->vx != 0.0f) || (spell->vz != 0.0f))) {
        facing = Atan2(spell->vx, spell->vz);
    } else {
        facing = caster->transform.yrot;
    }
    caster = spell->caster;
    count = gEnemyCount;
    best = 9.424778f;
    enemy = gEnemies;
    while (count != 0) {
        if (enemy->battleData.transform.modelID != -1) {
            gSpellOffset.x = enemy->battleData.transform.pos.x - caster->transform.pos.x;
            gSpellOffset.y = enemy->battleData.transform.pos.z - caster->transform.pos.z;
            rotateCoordinatesByAngle(facing, &gSpellOffset);
            angle = Atan2(gSpellOffset.x, gSpellOffset.y);
            if (angle > M_PI) {
                angle -= 2 * M_PI;
            } else if (angle < -M_PI) {
                angle += 2 * M_PI;
            }
            if (angle < 0.0f) {
                angle = -angle;
            }
            if (angle < best) {
                best = angle;
                spell->target = &enemy->battleData;
            }
            count--;
        }
        enemy++;
    }
}
void SpellTarget_FirstEnemy(SpellSlot* spell) {
    EnemyAction* enemy;
    s32 i;

    if (spell->caster->transform.actorFlags & 1) {
        spell->target = &gPlayerState.brian;
        return;
    }
    enemy = gEnemies;
    i = 6;
    while (i != 0 && enemy->battleData.transform.modelID == -1) {
        i--;
        enemy++;
    }
    if (i != 0) {
        spell->target = &enemy->battleData;
        return;
    }
    spell->target = NULL;
}
void SpellTarget_Self(SpellSlot* spell) {
    spell->target = spell->caster;
}
void ApplyStatusSlot0(s32 level, MonsterBattleData* target) {
    ActorStats* data;

    if (!(gBattleState & 0x100)) {
        data = target->stats;
        data->statusFlags |= 1;
        target->transform.scale = target->transform.base.monster->modelScale * 0.5;
        UpdateStatusVisuals(target, 1, 4, -1);
        return;
    }
    Setup_MISS_Icon(target);
}
void ApplyStatusSlot1(s32 level, MonsterBattleData* target) {
    ActorStats* data;

    data = target->stats;
    data->statusFlags |= 2;
    UpdateStatusVisuals(target, 2, 2, -1);
}
void ApplyStatusSlot2(s32 level, MonsterBattleData* target) {
    ActorStats* data;

    data = target->stats;
    data->statusFlags |= 4;
    switch (level) {
    case 1:
        target->stats->atkScale = 0x18;
        break;
    case 2:
        target->stats->atkScale = 0x20;
        break;
    }
    UpdateStatusVisuals(target, 4, 3, -1);
}
void ApplyStatusSlot3(s32 level, MonsterBattleData* target) {
    ActorStats* data;

    if (!(gBattleState & 0x100)) {
        data = target->stats;
        data->statusFlags |= 8;
        target->transform.stateFlags |= ACTOR_BUSY;
        UpdateStatusVisuals(target, 8, (u16) (level - 1), -1);
        return;
    }
    Setup_MISS_Icon(target);
}
void ApplyStatusSlot4(s32 level, MonsterBattleData* target) {
    target->stats->statusFlags |= 0x10;

    switch (level) {
    case 1:
        target->stats->moveScale = 0.5f;
        break;
    case 2:
        target->stats->moveScale = 2.0f;
        break;
    }

    UpdateStatusVisuals(target, 0x10, (u16) (target->stats->moveScale > 1.0 ? 8 : 9), -1);
}
void ApplyStatusSlot5(s32 level, MonsterBattleData* target) {
    target->stats->statusFlags |= 0x20;

    switch (level) {
    case 1:
        target->stats->agi = target->transform.base.monster->agi * 2;
        UpdateStatusVisuals(target, 0x20, 7, -1);
        return;
    case 2:
        target->stats->agi = target->transform.base.monster->agi * 4;
        UpdateStatusVisuals(target, 0x20, 7, -1);
        return;
    case 3:
        target->stats->agi = target->transform.base.monster->agi >> 1;
        UpdateStatusVisuals(target, 0x20, 0xC, -1);
        return;
    }
}
void ApplyStatusSlot6(s32 level, MonsterBattleData* target) {
    ActorStats* data;  

    if (!(gBattleState & 0x100)) {
        data = target->stats;
        data->statusFlags |= 0x40;
        UpdateStatusVisuals(target, 0x40, 0xB, -1);
        return;
    }
    Setup_MISS_Icon(target);
}
void ApplyStatusSlot7(s32 level, MonsterBattleData* target) {
    ActorStats* data;

    data = target->stats;
    data->statusFlags |= 0x80;
    UpdateStatusVisuals(target, 0x80, 0xA, -1);
}
void ApplyStatusSlot8(s32 level, MonsterBattleData* target) {
    ActorStats* data;

    data = target->stats;
    data->statusFlags |= 0x100;
    UpdateStatusVisuals(target, 0x100, 4, -1);
}
void ApplyStatusSlot9(s32 level, MonsterBattleData* target) {
    ActorStats* data;

    data = target->stats;
    data->statusFlags |= 0x200;
    UpdateStatusVisuals(target, 0x200, 3, -1);
}
void ApplyStatusSlot10(s32 level, MonsterBattleData* target) {
    target->stats->statusFlags |= 0x400;
    switch (level) {
    case 1:
        target->stats->def = target->transform.base.monster->def >> 1;
        break;
    case 2:
        target->stats->def = target->transform.base.monster->def - (target->transform.base.monster->def >> 2);
        break;
    case 3:
        target->stats->def = target->transform.base.monster->def + (target->transform.base.monster->def >> 1);
        break;
    case 4:
        target->stats->def = target->transform.base.monster->def * 2;
        break;
    }

    UpdateStatusVisuals(target, 0x400, (u16) (target->transform.base.monster->def < target->stats->def ? 5 : 6), -1);
}
void ApplyStatusSlot11(s32 level, MonsterBattleData* target) {
    ActorStats* data;

    data = target->stats;
    data->statusFlags |= 0x800;
}
extern u16 gElementPowerOrder[];      /* per element: the opposite element (counts /16), then the other two (/8 each) */
extern BrianData1 gBrianStats;  /* Brian's stats */
u16 CalcSpellPower(SpellDef* def, MonsterBattleData* caster) {
    u16 level;
    u16* w;
    u8 el;
    s32 i;
    f32 power;
    f32 step;

    if (caster->transform.actorFlags & 1) {
        level = gEnemies[GetEnemyIndex(caster)].monBaseData->atk;
    } else {
        w = &gElementPowerOrder[def->path[0] * 2];
        el = def->path[0];
        level = gBrianStats.spirits[el];
        level += gBrianStats.spirits[*w++] >> 4;
        level += gBrianStats.spirits[*w++] >> 3;
        level += gBrianStats.spirits[*w] >> 3;
    }
    power = (u32) def->power * 0.02;
    step = power * 0.2;
    for (i = 0; i != level; i++) {
        power += step;
        step *= 1.03;
    }
    return (u32) power;
}

void* SpawnEffectObject(u16 colour, u16 duration, f32 x, f32 y, f32 z, f32 angle, EffectSpawnDef* model, OddVector* size, ActorTransform* actor) {
    f32 scaled_y;
    f32 scaled_x;

    /* Quest's scratch calls +0x18 "hitboxWidth"; MonsterBaseData names it radius */
    switch (model->sizeModeX) {
    case 0:
        scaled_x = (actor->base.monster->radius * actor->scale) * size->x;
        break;
    case 1:
        scaled_x = (actor->base.monster->height * actor->scale) * size->x;
        break;
    case 3:
        scaled_x = size->scale * size->x;
        break;
    default:
        scaled_x = size->x;
    }

    switch (model->sizeModeY) {
    case 0:
        scaled_y = (actor->base.monster->radius * actor->scale) * size->y;
        break;
    case 1:
        scaled_y = (actor->base.monster->height * actor->scale) * size->y;
        break;
    case 3:
        scaled_y = size->scale * size->y;
        break;
    default:
        scaled_y = size->y;
    }

    return AllocProjectileSlot(model->anim, model->gfx, model->texture, colour & 0xFF, duration,
                         x, y, z, angle, scaled_x, scaled_y, scaled_x);
}
void StartSpellVisual(SpellSlot* spell, MonsterBattleData* target) {
    MonsterBaseData* actor_base;
    SpellBurstDef* burst;
    SpellDef* def;
    f32 angle;
    SpellAnimDef* anim;
    EffectSpawnDef* model;
    f32 size;
    f32 topY;

    anim = spell->anim;
    def = spell->def;

    if (anim->model >= 0) {
        model = &gEffectModels[anim->model];
        ((OddVector*)&gSpellOffset)->x = anim->size_x;
        ((OddVector*)&gSpellOffset)->y = anim->size_y;
        ((OddVector*)&gSpellOffset)->scale = def->radius;

        if ((spell->vx != 0.0f) || (spell->vz != 0.0f)) {
            angle = Atan2(spell->vx, spell->vz);
        } else {
            angle = target->transform.yrot;
        }

        spell->fx = SpawnEffectObject(anim->color, anim->duration, spell->x, spell->y, spell->z, angle,
                                    model, (OddVector*)&gSpellOffset, &target->transform);

        spell->fx->vx = spell->vx;
        spell->fx->vy = spell->vy;
        spell->fx->vz = spell->vz;
    } else {
        spell->fx = NULL;
    }

    burst = gSpellBurstDefs[spell->school] + spell->index;

    if (burst->kind != 0) {
        actor_base = target->transform.base.monster;
        size = 2.0 * actor_base->radius * target->transform.scale;
        topY = target->transform.pos.y + actor_base->height * target->transform.scale;
        switch (burst->kind) {
        case 1:
            QueueSparkleBurst(burst->arg, 0x1E, target->transform.pos.x, topY, target->transform.pos.z, size * 1.2);
            break;
        case 2:
            SpawnSparkleBurst(burst->arg, target->transform.pos.x, topY, target->transform.pos.z, size * 0.2);
            break;
        }
    }

    if (anim->sound >= 0) {
        PlaySound(anim->delay, anim->sound, 0xFF);
    }
}
void SpawnSpellTrail(SpellSlot* spell, MonsterBattleData* target) {
    EffectSpawnDef* model;
    SpellDef* def;
    SpellAnimDef* anim;
    SpellFx* fx;
    f32 angle;

    anim = spell->anim;
    def = spell->def;
    model = &gEffectModels[anim->trail_model];
    ((OddVector*)&gSpellOffset)->x = anim->trail_size_x;
    ((OddVector*)&gSpellOffset)->y = anim->trail_size_y;
    ((OddVector*)&gSpellOffset)->scale = def->radius;
    if (anim->trail_flags & 0x10) {
        angle = (u32)spell->lifetime * 3.141592653589793 / 10.0;
    } else {
        angle = target->transform.yrot;
    }
    fx = SpawnEffectObject(anim->trail_rate, anim->trail_flags, spell->x, spell->y, spell->z, angle, model,
                       (OddVector*)&gSpellOffset, &spell->caster->transform);
    fx->vx = spell->vx;
    fx->vy = spell->vy;
    fx->vz = spell->vz;
    if (anim->trail_sound >= 0) {
        PlaySound(anim->trail_delay, anim->trail_sound, 0xFF);
    }
}
void StartSpellHitVisual(SpellSlot* spell, MonsterBattleData* hit) {
    MonsterBaseData* actor_base;
    SpellBurstDef* burst;
    SpellDef* def;
    SpellAnimDef* anim;
    EffectSpawnDef* entry;
    u16 flags;
    f32 y;
    f32 size;

    anim = spell->anim;
    def = spell->def;

    if (anim->hit_model >= 0) {
        entry = &gEffectModels[anim->hit_model];
        ((OddVector*) &gSpellOffset)->x = anim->hit_size_x;
        ((OddVector*) &gSpellOffset)->y = anim->hit_size_y;
        ((OddVector*) &gSpellOffset)->scale = def->radius;
        flags = anim->hit_flags;
        if (anim->hit_flags & 0x40) {
            hit = spell->caster;
        }
        if (hit->transform.base.monster->monsterType == FLYING) {
            if (flags & 0x100) {
                y = hit->stats->flyY;
            } else {
                y = hit->transform.pos.y;
            }
        } else if (flags & 0x100) {
            y = hit->transform.pos.y + hit->transform.base.monster->height * 0.5 * hit->transform.scale;
        } else {
            y = hit->transform.pos.y;
        }
        spell->hitFx = SpawnEffectObject(anim->hit_color, anim->hit_flags, hit->transform.pos.x, y, hit->transform.pos.z, hit->transform.yrot,
                                           entry, (OddVector*) &gSpellOffset, &hit->transform);
        spell->hitFx->vx = spell->vx;
        spell->hitFx->vy = spell->vy;
        spell->hitFx->vz = spell->vz;
    } else {
        spell->hitFx = NULL;
    }

    burst = gSpellBurstDefs[spell->school] + spell->index;

    if (burst->hit_kind != 0) {
        actor_base = hit->transform.base.monster;
        size = 2.0 * actor_base->radius * actor_base->modelScale;
        y = hit->transform.pos.y + actor_base->height * actor_base->modelScale;
        switch (burst->hit_kind) {
        case 1:
            QueueSparkleBurst(burst->hit_arg, 0x1E, hit->transform.pos.x, y, hit->transform.pos.z, size * 1.2);
            break;
        case 2:
            SpawnSparkleBurst(burst->hit_arg, hit->transform.pos.x, y, hit->transform.pos.z, size * 0.2);
            break;
        }
    }

    if (anim->hit_sound >= 0) {
        PlaySound(anim->hit_delay, anim->hit_sound, 0xFF);
    } else if (spell->caster != spell->target) {
        PlaySound(0, 0x18, 0xFF);
    }
}
void StartSpellLandVisual(SpellSlot* spell, MonsterBattleData* target) {
    MonsterBaseData* actor_base;
    SpellBurstDef* burst;
    SpellDef* def;
    SpellAnimDef* anim;
    SpellFx* fx;
    EffectSpawnDef* entry;
    f32 size;
    f32 y;

    anim = spell->anim;
    def = spell->def;

    if (anim->land_model >= 0) {
        entry = &gEffectModels[anim->land_model];
        ((OddVector*) &gSpellOffset)->x = anim->land_size_x;
        ((OddVector*) &gSpellOffset)->y = anim->land_size_y;
        ((OddVector*) &gSpellOffset)->scale = def->radius;
        fx = SpawnEffectObject(anim->land_color, anim->land_duration, spell->x, spell->y, spell->z,
                           target->transform.yrot, entry, (OddVector*) &gSpellOffset,
                           &spell->caster->transform);
        fx->vx = spell->vx;
        fx->vy = spell->vy;
        fx->vz = spell->vz;
    }

    burst = gSpellBurstDefs[spell->school] + spell->index;

    if (burst->land_kind != 0) {
        actor_base = target->transform.base.monster;
        size = 2.0 * actor_base->radius * target->transform.scale;
        y = target->transform.pos.y + actor_base->height * target->transform.scale;
        switch (burst->land_kind) {
        case 1:
            QueueSparkleBurst(burst->land_arg, 0x1E, target->transform.pos.x, y,
                          target->transform.pos.z, size * 1.2);
            break;
        case 2:
            SpawnSparkleBurst(burst->land_arg, target->transform.pos.x, y, target->transform.pos.z,
                          size * 0.2);
            break;
        }
    }

    if (anim->land_sound >= 0) {
        PlaySound(anim->land_delay, anim->land_sound, 0xFF);
    }
}
void CountDownStatuses(MonsterBattleData* actor) {
    ActorStats* status;
    s32 bit;
    s32 i;

    status = actor->stats;
    bit = 1;
    i = 0;
    do {
        if (status->statusFlags & bit) {
            if (--status->statusTimers[i] == 0) {
                status->statusFlags ^= bit;
                if (bit & 1) {
                    actor->transform.scale = actor->transform.base.monster->modelScale;
                } else if (bit & 4) {
                    actor->stats->atkScale = 0x10;
                } else if (bit & 0x10) {
                    actor->stats->moveScale = 1.0f;
                } else if (bit & 0x20) {
                    actor->stats->agi = actor->transform.base.monster->agi;
                } else if (bit & 0x400) {
                    actor->stats->def = actor->transform.base.monster->def;
                } else if (bit & 8) {
                    actor->transform.stateFlags &= ~ACTOR_BUSY;
                }
            }
        }
        i++;
        bit <<= 1;
    } while (i != 12);
}
s32 TestSpellHitMask(s32 slot, MonsterBattleData* hit) {
    return gHitMaskBits[slot] & hit->stats->hitMask;
}
void SetSpellHitMask(s32 slot, MonsterBattleData* hit) {
    hit->stats->hitMask |= gHitMaskBits[slot];
}
void ClearHitMasks(void) {
    s32 i;

    gPlayerState.brian.statsBuf.hitMask = 0;
    for (i = 0; i < 6; i++) {
        gEnemies[i].battleData.statsBuf.hitMask = 0;
    }
}
void ClearHitMaskBit(s32 slot) {
    s32 mask = ~gHitMaskBits[slot];
    s32 i;

    gPlayerState.brian.statsBuf.hitMask &= mask;
    for (i = 0; i < 6; i++) {
        gEnemies[i].battleData.statsBuf.hitMask &= mask;
    }
}
extern u32 gAccuracyResults[];
s32 RollStaffHit(s32 enemy, MonsterBattleData* caster);
s32 EnemyAgilityCheck(MonsterBattleData* attacker); /* the enemy-side agility check (Quest func_8000ABF4) */
s32 RollSpellAccuracy(SpellSlot* spell, MonsterBattleData* hit) {
    s32 result;
    MonsterBattleData* caster;

    if (spell->flags & 4) {
        result = 2;
    } else {
        result = getRandomNumber(100) < spell->def->hit_rate;
        if (result != 0) {
            caster = spell->caster;
            if (hit != caster) {
                if (hit->transform.actorFlags & 1) {
                    result = RollStaffHit(GetEnemyIndex(hit), spell->caster);
                } else {
                    result = EnemyAgilityCheck(caster);
                }
            }
            result++;
        }
    }
    return gAccuracyResults[result];
}
extern u8 gStatusChances[];  /* each status slot's chance, out of 32 for Brian (28 with 0x8000) or 64 for enemies */
s32 RollStatus(SpellSlot* spell, MonsterBattleData* caster, s32 effect) {
    s32 result;
    SpellDef* def;

    if (spell->flags & 4) {
        result = 1;
    } else if (def = spell->def, def->target_type == 2) {
        result = 1;
    } else if (caster->transform.actorFlags & 1) {
        result = getRandomNumber(0x40) < gStatusChances[effect];
    } else {
        result = getRandomNumber((gPlayerState.flags & 0x8000) ? 0x1C : 0x20) < gStatusChances[effect];
    }
    return result;
}
extern u32 gStatusDurationOther[];  /* each status slot's duration range on another target: rand(3) + 2 */
extern u32 gStatusDurationSelf[];  /* on the caster itself: rand(5) + 3 */
void RollStatusDuration(SpellSlot* spell, MonsterBattleData* hit, s32 effect) {
    if (hit != spell->caster) {
        hit->stats->statusTimers[effect] = getRandomNumber(gStatusDurationOther[effect]) + 2;
        return;
    }
    hit->stats->statusTimers[effect] = getRandomNumber(gStatusDurationSelf[effect]) + 3;
}
