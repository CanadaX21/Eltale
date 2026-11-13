#ifndef GAME_PLAYER_H
#define GAME_PLAYER_H

/* The functions of src/game/player.c, declared once for every file that calls them. */

#include "common.h"

void InitBrian(void);
void SpawnBrian(void);
void RespawnAtSavePoint(void);
void UpdateBrian(void);
void DrawBrian(Mtx* mtx);
void SetupBrianForBattle(void);
void BrianFieldMovement(ActorTransform* brian, PlayerState* state, AnimEntry* anims);
void BrianState_Wait(ActorTransform* brian, PlayerState* state, AnimEntry* anims);
void BrianState_Slide(ActorTransform* brian, PlayerState* state, AnimEntry* anims);
void BrianState_KnockedDown(ActorTransform* brian, PlayerState* state, AnimEntry* anims);
void BrianState_Defeated(ActorTransform* brian, PlayerState* state, AnimEntry* anims);
void BrianState_TalkEnd(ActorTransform* brian, PlayerState* state, AnimEntry* anims);
void BrianState_WalkIn(ActorTransform* brian, PlayerState* state, AnimEntry* anims);
void BrianState_StaffSwing(MonsterBattleData* brian, PlayerState* state, AnimEntry* anims);
void BrianState_Cast(MonsterBattleData* brian, PlayerState* state, AnimEntry* anims);
void BrianState_EnterDoor(ActorTransform* brian, PlayerState* state, AnimEntry* anims);
void BrianState_ThroughDoor(ActorTransform* brian, PlayerState* state, AnimEntry* anims);
void BrianState_PickUpItem(ActorTransform* brian, PlayerState* state, AnimEntry* anims);
void BrianState_UseItem(MonsterBattleData* brian, PlayerState* state, AnimEntry* anims);
void BrianState_Escape(ActorTransform* brian, PlayerState* state, AnimEntry* anims);
void BrianState_FaceAndMessage(ActorTransform* brian, PlayerState* state, AnimEntry* anims);
void BrianState_Stand(f32 analog_x, f32 analog_y, ActorTransform* brian, PlayerState* state);   /* stand */
void BrianState_Walk(f32 x, f32 y, ActorTransform* brian, PlayerState* state);   /* walk */
void BrianState_Run(f32 x, f32 y, ActorTransform* brian, PlayerState* state);   /* run */
void ApplyBrianMovement(ActorTransform* brian, AnimEntry* anims, Coordinates2D* offset);
void func_80006A50(void);
void BrianBattleMovement(u16 mask, ActorTransform* brian, PlayerState* state);   /* battle circle + collision step */
s32 FindTalkTarget(PlayerState* state, ActorTransform* brian);
void PushBrianOutOfEnemies(f32* dx, f32* dz, ActorTransform* brian);   /* push out of the enemies */
void ApplyFloorEffect(PlayerState* state, ActorTransform* brian);
void StartBrianArrival(void);
void BrianWalkThroughDoor(PlayerState* state, ActorTransform* brian);
void StartBrianPickUp(PlayerState* state, ActorTransform* brian, AnimEntry* anims);
void StartBrianUseItem(u8 itemID);
void LoadBrianAnimation(ActorTransform* brian);
s32 FindStaffTarget(ActorTransform* brian);
void StartBrianStaffSwing(PlayerState* state, ActorTransform* brian, AnimEntry* anims);
s32 RollStaffHit(s32 enemy, MonsterBattleData* actor);   /* staff hit test */
u16 CalcStaffPower(PlayerState* state);   /* staff damage */
u16 DamageBrian(u16 power, u16 flags, MonsterBattleData* attacker);   /* hurt Brian; returns the damage */
void StartBrianDodge(void);   /* Brian dodges */
s32 CanBrianAct(PlayerState* state, ActorTransform* brian);
s32 BrianSpellInput(PlayerState* state);
void BrianStartCast(PlayerState* state, ActorTransform* brian, AnimEntry* anims);
void CountBattleAgility(PlayerState* player);
void UpdateBrianGrowth(PlayerState* state, ActorTransform* brian);

#endif
