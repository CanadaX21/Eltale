#ifndef GAME_EFFECTS_H
#define GAME_EFFECTS_H

/* The functions of src/game/effects.c, declared once for every file that calls them. */

#include "common.h"

void ResetEffects(void);
void UpdateEffects(mkst* dyn);
void ResetDustEmitters(void);
void FindFreeEffectSlot(ActorTransform* actor);
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
void DrawProjectiles(mkst* dyn);
void ResetGlows(void);
void DrawGlows(mkst* dyn);
void StartLevelUpEffect(s32 stat);   /* level-up effect */

#endif
