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

#endif
