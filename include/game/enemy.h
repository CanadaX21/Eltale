#ifndef GAME_ENEMY_H
#define GAME_ENEMY_H

/* The functions of src/game/enemy.c, declared once for every file that calls them. */

#include "common.h"

void SetupNPCs(void);
void UpdateNPCs(void);
void DrawNPCs(Mtx* mtx);
void LoadNPCArea(s32 area);
void SpawnNPCs(NpcRecord* rec, s32 count);
void MoveActorWithCollision(f32 dx, f32 dz, ActorTransform* actor);   /* move an actor with collision */
void PushNPCOutOfBrian(f32* dx, f32* dz, ActorTransform* actor);
void StartNPCTalk(s32 index);
void ResumeNpcAfterTalk(s32 index);
void StartNPCText(Npc* npc, s32 textOffset);   /* a map object starts its text */
void StartEndingText(s32 index, u8* text);
void ResetEnemies(void);
void UpdateEnemies(void);
void DrawEnemies(Mtx* mtx);
void LoadMonsterSet(void);
void ClearEnemies(void);
void SpawnEnemy(s32 id, f32 x, f32 z, f32 yrot, MonsterBattleData* m, EnemyAction* e);
void SortTurnOrder(void);
void FlagTurnEnemy(void);
void EnemyHitReaction(MonsterBattleData* actor, EnemyAction* enemy, AnimEntry* anims);
void SelectEnemyAction(MonsterBattleData* actor, EnemyAction* enemy, AnimEntry* anims);
void EnemyKnockback(MonsterBattleData* actor, EnemyAction* enemy, AnimEntry* anims);
void EnemyFaceBrian(MonsterBattleData* actor, EnemyAction* enemy, AnimEntry* anims);
void EnemyMeleeAttack(MonsterBattleData* actor, EnemyAction* enemy, AnimEntry* anims);
void EnemyCastAttack(MonsterBattleData* actor, EnemyAction* enemy, AnimEntry* anims);
EnemyAIOption* PickEnemyAIOption(EnemyAction* enemy, MonsterBattleData* actor);
void CommitEnemyAIOption(EnemyAction* enemy, MonsterBattleData* actor, EnemyAIOption* option);
void TurnActorToward(MonsterBattleData* actor, f32 targetX, f32 targetZ);
void MoveEnemy(s32 leash, f32 dx, f32 dz, MonsterBattleData* m, EnemyAction* e);
void GetRootMotionOffset(MonsterBattleData* actor, Coordinates2D* offset);
void PushEnemyOut(f32* dx, f32* dz, MonsterBattleData* m);
s32 EnemyAgilityCheck(MonsterBattleData *actor);
u16 CalcEnemyMeleeDamage(EnemyAction* enemy);
u16 DamageEnemy(s32 index, u16 damage, u16 element, u16 flags, MonsterBattleData* attacker);   /* deal damage */
void ShowEnemyMiss(MonsterBattleData* actor);
s32 GetEnemyIndex(MonsterBattleData* battleData);
void SpawnEnemyLandingBurst(MonsterBattleData* actor);   /* the enemy's landing burst */
void StopBossEffectScript(void);
void StartBossEffectScript(s32 id, EnemyAction* enemy);
void UpdateBossEffectScript(void);   /* boss spell animation update (Quest func_8000B300) */

#endif
