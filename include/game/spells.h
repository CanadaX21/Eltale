#ifndef GAME_SPELLS_H
#define GAME_SPELLS_H

/* The functions of src/game/spells.c, declared once for every file that calls them. */

#include "common.h"

void ResetSpells(void);
s32 GetSpellFromPath(u16 presses, u8* path);
s32 GetSpellAvailability(u32 spell, u16 mp);
void SpawnSpell(MonsterBattleData* caster, u32 spell, s32 shot, u16 flags);   /* cast the spell */
void UpdateSpells(void);
s32 AnySpellActive(void);
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
void* SpawnEffectObject(u16 colour, u16 duration, f32 x, f32 y, f32 z, f32 angle, EffectSpawnDef* model, OddVector* size, ActorTransform* actor);
void CountDownStatuses(MonsterBattleData* actor);
s32 TestSpellHitMask(s32 slot, MonsterBattleData* hit);
void SetSpellHitMask(s32 slot, MonsterBattleData* hit);
void ClearHitMasks(void);

#endif
