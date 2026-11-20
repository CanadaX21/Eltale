#ifndef GAME_BOSS_H
#define GAME_BOSS_H

/* The functions of src/game/boss.c, declared once for every file that calls them. */

#include "common.h"

void ResetStoryFlags(void);
void LoadMapBoss(void);
void DrawBoss(Mtx* mtxList);
s32 IsNearBoss(void);
void SpawnBoss(s32 id, f32 x, f32 z, f32 yrot, EnemyAction* e);
u32 IsStoryFlagSet(u32 flag);
void SetStoryFlag(u16 flag);
void StartBossScene(s32 boss);
void OnBossDefeated(s32 boss);

#define LAST_BOSS  7  /* the last boss's row in gBossPlacements: its defeat leads to the ending (OnBossDefeated:
                         FIELD_MODE_LAST_BOSS). The story flags are the bosses beaten, one bit per placement row: LoadMapBoss
                         tests the row, StepBattleRound sets it (SetStoryFlag(gBattleBoss - 1); gBattleBoss is the row + 1) */

typedef struct {
    ActorTransform transform;
    ActorStats* stats;
} BossSpawnTransform;               /* 0x6C: the head of a MonsterBattleData, the boss's as StartBossScene copies it */

/* The state src/game/boss.c defines (.bss), in its storage order: the order of these declarations is the
 * unit's .bss layout (include/bss_order.h). */
extern CollisionPollingResult gBossFloorPoll;   /* the boss spawn's floor poll (SpawnBoss) */
extern s32 D_8007C0CC;   /* unreferenced */
extern BossSpawnTransform gBossSpawnTransform;   /* the boss's spawn transform */
extern u8 gStoryFlags[4];   /* bosses beaten, one bit each (SetStoryFlag) */
extern s32 gBossLoaded;   /* a boss is loaded */
extern s32 D_8007C144;   /* unreferenced */
extern EnemyAction gBoss;   /* the boss (an EnemyAction: battleData at 0x2C) */
extern s32 D_8007C278[2];   /* unreferenced */

#endif
