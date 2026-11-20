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
void BrianState_Victory(ActorTransform* brian, PlayerState* state, AnimEntry* anims);
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
void StartBrianDeparture(void);
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

/* Brian's animations by ID (ActorTransform.animID; gBrianModelAnims, his model's table). Each is a file of its own,
   brian_anim_NN, which LoadBrianAnimation loads to BRIAN_ANIM_BUFFER while it plays. Named by the code that plays
   them (src/game/player.c, src/game/main.c). 4, 5, 6 and 19 are structurally verified animations with no identified normal-game
   playback path: no code sets them; their files are typed, and they load and play when set. */
#define BRIAN_ANIM_STAND         0     /* standing in the field (gBrianIdleAnim, HP at least a third) */
#define BRIAN_ANIM_WALK          1     /* walking (BrianState_Walk), and stepping to a door */
#define BRIAN_ANIM_RUN           2     /* running (BrianState_Run) */
#define BRIAN_ANIM_CAST          3     /* casting a spell at a target (BrianStartCast) */
#define BRIAN_ANIM_DODGE         7     /* an attack missed him (StartBrianDodge) */
#define BRIAN_ANIM_HIT           8     /* hit (DamageBrian) */
#define BRIAN_ANIM_DEFEATED      9     /* knocked out at HP 0 (BrianState_KnockedDown) */
#define BRIAN_ANIM_STAFF_SWING   10    /* the staff swing (StartBrianStaffSwing) */
#define BRIAN_ANIM_STAFF_COMBO   11    /* the combo's second swing (BrianState_StaffSwing) */
#define BRIAN_ANIM_SIGN_START    12    /* turning to read a sign, then BRIAN_ANIM_SIGN_LOOP */
#define BRIAN_ANIM_SIGN_LOOP     13    /* reading a sign */
#define BRIAN_ANIM_SIGN_END      14    /* after a sign's message (BrianState_FaceAndMessage) */
#define BRIAN_ANIM_STOP          15    /* stopping from speed, from its frame 3 (BrianState_Walk, BrianState_Run) */
#define BRIAN_ANIM_VICTORY       16    /* a battle won (UpdateBrian) */
#define BRIAN_ANIM_BATTLE_STAND  17    /* standing in battle (gBrianIdleAnim) */
#define BRIAN_ANIM_BATTLE_START  18    /* a battle's start (SetupBrianForBattle) */
#define BRIAN_ANIM_DOOR          20    /* through a door model, its door's flags' bit 0 clear (PlayerState.animCue) */
#define BRIAN_ANIM_DOOR_FLAG1    21    /* through a door model, its door's flags' bit 0 set (PlayerState.animCue) */
#define BRIAN_ANIM_USE_LOOP      22    /* an item or a spell on himself, after BRIAN_ANIM_USE_START */
#define BRIAN_ANIM_USE_START     23    /* an item or a spell on himself (StartBrianUseItem, BrianStartCast) */
#define BRIAN_ANIM_USE_END       24    /* after an item or a spell on himself */
#define BRIAN_ANIM_STAND_LOW_HP  25    /* standing with HP under a third (gBrianIdleAnim) */
#define BRIAN_ANIM_DOOR_KIND3    26    /* leaving through a door of kind 3 (BrianWalkThroughDoor) */
#define BRIAN_ANIM_NO_TARGET     27    /* A with nothing to act on, or a SIGN_NO_POSE sign */
#define BRIAN_ANIM_PICK_UP       28    /* taking an item from a map object, then BRIAN_ANIM_PICK_UP_END */
#define BRIAN_ANIM_PICK_UP_END   29

/* PlayerState.state: the handler gBrianStateHandlers runs (UpdateBrian) */
#define BRIAN_STATE_STAND         0   /* BrianFieldMovement (states 0-2) */
#define BRIAN_STATE_WALK          1
#define BRIAN_STATE_RUN           2
#define BRIAN_STATE_WAIT          3   /* BrianState_Wait: an action's animation, then BRIAN_STATE_STAND */
#define BRIAN_STATE_SLIDE         4   /* BrianState_Slide: stopping from speed */
#define BRIAN_STATE_HIT           5   /* BrianState_KnockedDown: hit (DamageBrian); at 0 HP, BRIAN_STATE_DEFEATED */
#define BRIAN_STATE_DEFEATED      6   /* BrianState_Defeated */
#define BRIAN_STATE_TALK          7   /* BrianState_TalkEnd */
#define BRIAN_STATE_WALK_IN       8   /* BrianState_WalkIn */
#define BRIAN_STATE_ENTER_DOOR    9   /* BrianState_EnterDoor */
#define BRIAN_STATE_THROUGH_DOOR  10  /* BrianState_ThroughDoor */
#define BRIAN_STATE_PICK_UP       11  /* BrianState_PickUpItem */
#define BRIAN_STATE_USE_ITEM      12  /* BrianState_UseItem */
#define BRIAN_STATE_STAFF         13  /* BrianState_StaffSwing: the first swing */
#define BRIAN_STATE_CAST          14  /* BrianState_Cast */
#define BRIAN_STATE_VICTORY       15  /* BrianState_Victory: a battle won */
#define BRIAN_STATE_STAFF_COMBO   16  /* BrianState_StaffSwing: the second swing */
#define BRIAN_STATE_SIGN          17  /* BrianState_FaceAndMessage */

/* LoadBrianAnimation's buffer: the animation playing is loaded here, below his model. An immediate in its code (a
   symbol would compile to one more instruction); the linker asserts that every animation file is linked here and
   ends below the model, and the model below the overlay slot (undefined_syms.jp.txt). */
#define BRIAN_ANIM_BUFFER        ((void*)0x80200000)

/* The state src/game/player.c defines (.bss), in its storage order: the order of these declarations is the
 * unit's .bss layout (include/bss_order.h). */
extern f32 gBrianStepPhase;
extern f32 D_8007A8F4;
extern f32 D_8007A8F8;
extern f32 gAgilityWalk;
extern f32 gAgilityWalkSafe;
extern s32 gNearBoss;
extern s32 gTextBoxDelay;   /* frames until the text box opens */
extern u16 gRunFootstepTimer;   /* footstep timer while running (Quest D_8007BC28) */
extern s32 gBrianIdleAnim;   /* idle animation (Quest D_8007BC2C) */
extern s32 D_8007A914;   /* unreferenced */
extern Coordinates2D gBrianMovement;   /* Brian's movement this frame (ApplyBrianMovement) */
extern s32 D_8007A920[2];   /* unreferenced */
extern CollisionPollingResult gBrianFloorPoll;
extern s32 D_8007A944;   /* unreferenced */
extern u8 D_8007A948[0x40];   /* unreferenced */
extern f32 gBattleAgilityDistance;   /* distance walked in battle toward the next agility point */
extern f32 gBrianSpawnX;   /* Brian's spawn x */
extern f32 gBrianSpawnZ;
extern f32 gBrianSpawnYaw;   /* Brian's spawn yaw */
extern s32 gArrivalFlags;
extern s32 gEntranceDoorModel;
extern f32 gBrianMoveX;
extern f32 gBrianMoveZ;
extern f32 gBrianStepDistance;   /* distance Brian walked this frame */
extern u16 gSavedMap;   /* saved map */
extern u16 gSavedSubmap;   /* saved submap */
extern u16 gSavedEntrance;
extern s32 gBattleExp;
extern s32 gBattleStone;
extern s32 gDroppedItem;   /* the item dropped (0xFF: none) */
extern s32 gTalkTarget;   /* the NPC Brian can talk to, 1-based (0 = none; FindTalkTarget) */
extern Spirit* gSpiritTarget;   /* the spirit Brian can pick up (CheckSpiritPickup) */
extern MapObject* gObjectTarget;   /* the map object in front of Brian (FindMapObjectInFront) */
extern SignSlot* gSignTarget;   /* the sign Brian can read (FindSignTarget) */
extern BrianData1 gBrianStats;   /* Brian's stats */
extern PlayerState gPlayerState;   /* Brian's state block */
extern s32 D_8007AB44;   /* unreferenced */
extern Vec3f gBrianFloorPos;   /* Brian's point on the floor (his shadow, the glows) */
extern s32 D_8007AB54;   /* unreferenced (three words: IDO would 8-align an array this size) */
extern s32 D_8007AB58;   /* unreferenced */
extern s32 D_8007AB5C;   /* unreferenced */
extern s32 gActorLightAnim;   /* the brightened light animator for Brian, NPCs, enemies, effects */

/* The data src/game/player.c defines. */
extern void (*gBrianStateHandlers[])();   /* Brian's state handlers (defined below) */
extern s32 gStaffSpiritScale[];

/* Link names inside the blocks above (undefined_syms.jp.txt): one place in a block, named for the code that
 * addresses it by that name. */
extern s32 D_8007A9E0;   /* a link name inside gBrianStats: gBrianStats + 0x10 (undefined_syms.jp.txt) */
extern u8 D_8007AA04;   /* a link name inside gBrianStats: gBrianStats + 0x34 (undefined_syms.jp.txt) */

#endif
