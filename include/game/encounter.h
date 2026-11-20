#ifndef GAME_ENCOUNTER_H
#define GAME_ENCOUNTER_H

/* The functions of src/game/encounter.c, declared once for every file that calls them. */

#include "common.h"
#include "map_format.h"

void InitEncounters(void);
void RollRandomEncounter(void);
void StartScriptedBattle(void);
void BattleStartCamera(void);
void StepBattleRound(void);
void EndBattleTurn(MonsterBattleData* actor);
EncounterZone* FindEncounterZone(f32 x, f32 z, EncounterZoneList* zones);
EncounterCircle* IsInEncounterCircle(f32 brian_x, f32 brian_z);
void CreateEncounter(f32 spawn_x, f32 spawn_z, f32 rotation_angle, u16* encounter_table);
void func_800200F0(void);
u8 RollItemDrop(void);   /* the item an enemy drops */
void func_8002018C(void);

/* gBattleState's bits */
#define BATTLE_ACTIVE         0x0001  /* a battle is on (RollRandomEncounter, StartScriptedBattle); StepBattleRound
                                         clears it at the battle's end */
#define BATTLE_ENEMY_TURN     0x0002  /* the turn is an enemy's, gTurnOrder[gTurnIndex] (clear: Brian's); EndBattleTurn
                                         toggles it */
#define BATTLE_TURN_START     0x0004  /* a turn waits to start: StepBattleRound sets up its movement ring and clears it */
#define BATTLE_WON            0x0008  /* every enemy is gone or down (StepBattleRound); UpdateBrian gives the rewards and
                                         clears it */
#define BATTLE_ESCAPED        0x0010  /* Brian left the battle circle (StepBattleRound) or cast an escape
                                         (SpellEffect_Escape) */
#define BATTLE_BOSS           0x0100  /* a boss battle (StartScriptedBattle) */
#define BATTLE_INTRO          0x0200  /* the opening camera runs (BattleStartCamera clears it) */
#define BATTLE_ENEMIES_DOWN   0x0400  /* EndBattleTurn found no enemy left above 0 HP to take the turn: StepBattleRound
                                         ends the battle as won */
#define BATTLE_NO_ENCOUNTERS  0x8000  /* RollRandomEncounter rolls nothing while it is set; InitEncounters keeps it,
                                         and both battle starts clear it (they assign the whole word). No code in the
                                         game sets it */

/* The state src/game/encounter.c defines (.bss), in its storage order: the order of these declarations is the
 * unit's .bss layout (include/bss_order.h). */
extern Coordinates2D gBattleCameraOffset;
extern s32 D_8008BE48;   /* unreferenced: the offset's third word (offsets of this kind -
                                            gSpellOffset, gEnemyStep, gHudVector - are OddVectors whose third
                                            word is a scale; no code reads this one) */
extern s16 gBattleIntroStep;   /* the battle intro camera's step: 0 looks at gBattleIntroEnemy first, 1 the
                                            view behind Brian */
extern s16 gBattleIntroTimer;   /* frames left in the intro step */
extern s16 gBattleIntroEnemy;   /* the enemy (gEnemies index) the intro looks at */
extern u16 gBattleState;   /* the BATTLE_ bits (above) */
extern u16 gTurnDelay;
extern s16 D_8008BE56;
extern u16 gTurnIndex;   /* whose turn: index into gTurnOrder */
extern u16 gTurnCount;   /* enemies in gTurnOrder */
extern f32 gBattleCenterX;   /* battle centre x (Quest D_8008C59C) */
extern f32 gBattleCenterZ;   /* battle centre z (Quest D_8008C5A0) */
extern f32 gTurnCenterX;   /* the turn's movement circle centre x */
extern f32 gTurnCenterZ;   /* and z */
extern f32 gBattleRadius;   /* battle arena radius */
extern u8 D_8008BE70[0x100];   /* unreferenced (256 bytes; Quest 64 has the same span after its radius) */
extern MonsterBattleData* gEnemyGroupHeads[8];   /* 8 fill the space to gEnemyGroupCount */
extern s32 gEnemyGroupCount;
extern u16 gBattleCircleRing;
extern u16 gTurnRing;
extern EncounterZoneList* gSubmapEncounterZones;
extern EncounterSet* gSubmapEncounterSet;
extern AnimEntry** gFieldMonsterAnims;   /* the field's gMonsterAnims, kept while a boss battle uses gBossAnimTables */
extern u8** gFieldEnemyNames;   /* the field's gEnemyNames, kept while a boss battle uses gBossNames */
extern s32 gBattleBoss;
extern f32 gEncounterWalkDistance;   /* distance walked toward the next encounter roll */
extern u16 gEncounterChance;   /* encounter chance out of 2000, +50 per 50 units walked */

#endif
