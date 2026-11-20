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
void StartNPCText(Npc* npc, s32 textOffset);   /* an NPC starts its text */
void StartNPCTextFromRAM(s32 index, u8* text);
void ResetEnemies(void);
void UpdateEnemies(void);
void DrawEnemies(Mtx* mtx);
void LoadMonsterSet(void);
void ClearEnemies(void);
void SpawnEnemy(s32 id, f32 x, f32 z, f32 yrot, MonsterBattleData* m, EnemyAction* e);
void SortTurnOrder(void);
void FlagTurnEnemy(void);
void EnemyIdle(MonsterBattleData* actor, EnemyAction* enemy, AnimEntry* anims);
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

/* The state src/game/enemy.c defines (.bss), in its storage order: the order of these declarations is the
 * unit's .bss layout (include/bss_order.h). */
extern RomRange* gAreaNpcBlob;   /* the current area's NPC blob (its gNpcBlobs entry) */
extern u16 gWalkingNpcCount;   /* NPCs walking at once, at most 3 (see StartNPCTalk's BUG) */
extern Coordinates2D gNpcStep;   /* a walking NPC's step */
extern s32 D_8007AB80[2];   /* unreferenced */
extern CollisionPollingResult gActorMovePoll;
extern s32 gNpcCount;   /* NPCs loaded (gNpcs) */
extern s32 gNpcsDrawn;   /* NPCs drawn this frame (within 300 units of Brian) */
extern s32 D_8007ABAC;   /* unreferenced */
extern ActorTransform* gNpcsDrawnList[16];   /* their transforms */
extern Npc gNpcs[16];   /* the submap's NPCs, 0x84 each */
extern u8 gTextBuffer[MESSAGE_TEXT_SIZE];   /* one message's text: enemy.c, ending_text.c and boss.c fill it */
extern u8 gSpeakerName[0x20];   /* a 0x20-byte buffer */
extern s32 gEnemyCount;   /* enemies in battle */
extern EnemyAction gEnemies[6];   /* the enemies in battle, 0x130 each (Quest D_8007C998) */
extern EnemyAction* gCardEnemy;   /* the enemy on the info card: the one whose turn started last (EnemyIdle) */
extern s32 gEnemiesDrawn;   /* enemies drawn this frame */
extern MonsterBattleData* gDrawnEnemies[6];   /* the enemies drawn this frame */
extern AnimEntry** gMonsterAnims;
extern u8** gEnemyNames;   /* enemy names */
extern u16 gTurnOrder[6];   /* the enemies in turn order */
extern MonsterSet* gCurrentMonsterSet;
extern BossEffectScript gBossEffectScript;   /* a boss's effect script player */
extern Coordinates2D gEnemyStep;
extern f32 D_8007C078;   /* gEnemyStep's scale where SpawnEnemyLandingBurst uses it as an OddVector */
extern s32 D_8007C07C;   /* unreferenced */
extern CollisionPollingResult gEnemyFloorPoll;   /* an enemy's spawn floor poll */
extern s32 D_8007C09C;   /* unreferenced */
extern s32 D_8007C0A0[4];   /* unreferenced */

#endif
