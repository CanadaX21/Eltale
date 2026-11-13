#ifndef GAME_ENCOUNTER_H
#define GAME_ENCOUNTER_H

/* The functions of src/game/encounter.c, declared once for every file that calls them. */

#include "common.h"

void InitEncounters(void);
void RollRandomEncounter(void);
void StartScriptedBattle(void);
void BattleStartCamera(void);
void StepBattleRound(void);
void EndBattleTurn(MonsterBattleData* actor);
EncounterZone* FindEncounterZone(f32 x, f32 z, EncounterZoneList* zones);
void CreateEncounter(f32 spawn_x, f32 spawn_z, f32 rotation_angle, u16* encounter_table);
void func_800200F0(void);
u8 RollItemDrop(void);   /* the item an enemy drops */
void func_8002018C(void);

#endif
