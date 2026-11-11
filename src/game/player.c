#include "common.h"
void BrianBattleMovement(u16 mask, ActorTransform* brian, PlayerState* state);   /* battle circle + collision step */
void BrianState_Stand(f32 analog_x, f32 analog_y, ActorTransform* brian, PlayerState* state);   /* stand */
void BrianState_Walk(f32 x, f32 y, ActorTransform* brian, PlayerState* state);   /* walk */
void BrianState_Run(f32 x, f32 y, ActorTransform* brian, PlayerState* state);   /* run */
void ApplyFloorEffect(PlayerState* state, ActorTransform* brian);
void UpdateBrianGrowth(PlayerState* state, ActorTransform* brian);
s32 RollStaffHit(s32 enemy, MonsterBattleData* actor);   /* staff hit test */
u16 CalcStaffPower(PlayerState* state);                    /* staff damage */
void EndBattleTurn(MonsterBattleData* actor);
s32 AnySpellActive(void);
void CountBattleAgility(PlayerState* player);
extern u16 gBattleState;   /* battle state flags */
extern f32 gTurnCenterX;   /* the turn\'s movement circle centre x */
extern f32 gTurnCenterZ;   /* and z */
extern SpellDef* gSpellDefs[];      /* each element's first spell record (spells.c) */
extern u16 gSpellMpCosts[];            /* MP cost by press count, [1..3] (spells.c) */
extern u16 gSpellBannerTimer;
void SnapSpellCamera(s32 duration, MonsterBattleData* caster);
extern f64 D_8006ED08; /* 0.68: speed decay in BrianState_Slide */
extern f64 D_8006EDC0; /* 0.025: stick magnitude -> speed in BrianState_Stand */
extern u16 gTurnDelay;
float sqrtf(float value);
void SetAnimation(ActorTransform *brian, s32 nextAnim, s32 nextFramerate, s32 afterAnim, s32 afterFramerate, u16 flags);
f32 Atan2(f32 x, f32 z);   /* atan2 */
void rotateCoordinatesByAngle(f32 angle, Coordinates2D* vec);
void FindFreeEffectSlot(ActorTransform* brian);
extern u16 gFieldMode;
void FindMapBGM(s32 map, s32 room, u16 fade);
extern AnimEntry* gBrianAnims[];   /* Brian's animation table per model */
void ApplyRootMotion(ActorTransform* actor, AnimEntry* anim, f32 scale);
void LoadBrianAnimation(ActorTransform* brian);
extern EnemyAction gEnemies[];   /* the enemies in battle */
void ApplyBrianMovement(ActorTransform* brian, AnimEntry* anims, Coordinates2D* offset);
extern RomRange gBrianAnimRom[];       /* Brian's animations in ROM */
void dma_write(u8* devAddr, void* vaddr, u32 nbytes);
extern s32 gCurrentMap;
extern s32 gCurrentSubmap;
extern PendingWarp gPendingWarp;
extern u32 gSpiritExpTable[];             /* EXP needed per spirit level */
extern s32 gNextMap;
extern RespawnLocation gRespawnLocationTbl[];
extern s32 gNextSubmap;
typedef struct {                    /* Brian's starting stats: the first 0x38 bytes of BrianData1 */
    s32 words[14];
} BrianStartStats;
extern BrianStartStats gNewGameStats;  /* his stats at a new game */
extern u16 gSpellCrossFlags;
extern u8 brian_model_ROM_START[];
extern u8 brian_model_ROM_END[];
extern u8 D_80206000[];
extern s32 gGameState;   /* game state flags */
extern s32 gMessageWindowsOpen;
void BuildPickupMessage(s32 itemID);
s32 OpenMessageWindow(s32 type);
void AddItemToInventory(u8 itemID);
void PlaySound(u8 delay, s8 id, u8 level);
void PollWorldCollision(f32 sample_x, f32 sample_y, f32 sample_z, u16 arg3_flags, CollisionPollingResult* result);   /* floor/wall poll */


extern s8 gStickX;                /* stick x */
extern s8 gStickY;                /* stick y */
extern u16 gSpellBannerFade;
extern Camera gCamera;
extern s32 gFieldFreeze;
extern s32 gInputFreeze;
/* ---- player.c's .bss (0x8007A8F0-0x8007AB70), in address order. IDO 5.3 lays a variable
   out by its FIRST declaration, and those first declared among the file's first 32 declarations (typedefs and
   variables, externs too) after all later ones: this block sits past them, so it comes out in order. Declaring
   any of these variables above it moves them. Names inside gBrianStats / gPlayerState that other code reads
   are members. The D_ names marked unreferenced are space
   no code addresses (unknown variables). ---- */
f32 gBrianStepPhase;
f32 D_8007A8F4;
f32 D_8007A8F8;
f32 gAgilityWalk;
f32 gAgilityWalkSafe;
s32 gNearBoss;
s32 gTextBoxDelay;                         /* frames until the text box opens */
u16 gRunFootstepTimer;                         /* footstep timer while running (Quest D_8007BC28) */
s32 gBrianIdleAnim;                         /* idle animation (Quest D_8007BC2C) */
s32 D_8007A914;                         /* unreferenced */
Coordinates2D gBrianMovement;               /* Brian's movement this frame (ApplyBrianMovement) */
s32 D_8007A920[2];                      /* unreferenced */
CollisionPollingResult gBrianFloorPoll;
s32 D_8007A944;                         /* unreferenced */
u8 D_8007A948[0x40];                    /* unreferenced */
f32 gBattleAgilityDistance;                         /* distance walked in battle toward the next agility point */
f32 gBrianSpawnX;                         /* Brian's spawn x */
f32 gBrianSpawnZ;
f32 gBrianSpawnYaw;
s32 gArrivalFlags;
s32 gEntranceDoorModel;
f32 gBrianMoveX;
f32 gBrianMoveZ;
f32 gBrianStepDistance;                         /* distance Brian walked this frame */
u16 gSavedMap;                         /* saved map */
u16 gSavedSubmap;                         /* saved submap */
u16 gSavedEntrance;
s32 gBattleExp;
s32 gBattleStone;
s32 gDroppedItem;                         /* the item dropped (0xFF: none) */
s32 gTalkTarget;                         /* the NPC Brian can talk to, 1-based (0 = none; FindTalkTarget) */
Spirit* gSpiritTarget;                   /* the spirit Brian can pick up (CheckSpiritPickup) */
MapObject* gObjectTarget;                /* the map object in front of Brian (FindMapObjectInFront) */
SignSlot* gSignTarget;                   /* the sign Brian can read (TryReadSign) */
BrianData1 gBrianStats;                  /* Brian's stats */
PlayerState gPlayerState;                 /* Brian's state block */
s32 D_8007AB44;                         /* unreferenced */
Vec3f gBrianFloorPos;                       /* Brian's point on the floor (his shadow, the glows) */
s32 D_8007AB54;                         /* unreferenced (three words: IDO would 8-align an array this size) */
s32 D_8007AB58;                         /* unreferenced */
s32 D_8007AB5C;                         /* unreferenced */
s32 gActorLightAnim;                      /* the brightened light animator for Brian, NPCs, enemies, effects */

extern ModelTransform* gMapModels;    /* the current map's placed models (doors) */
void StartDoorAnimation(s32 state, s32 frame, s32 flag, void* position);
void InitBrian(void) {
    PlayerState* state;

    dma_write(brian_model_ROM_START, D_80206000, brian_model_ROM_END - brian_model_ROM_START);
    *(BrianStartStats*)&gBrianStats = gNewGameStats;
    state = &gPlayerState;
    state->brian.transform.scale = gBrianStats.modelScale;
    state->brian.transform.collisionScale = gBrianStats.radius;
    state->brian.transform.modelID = gBrianStats.unk2;
    state->stats = &gBrianStats;
    state->animCue = 0;
    state->flags = 0;
    state->spellPresses = 0;
    state->brian.transform.animID = -1;
    state->brian.transform.animFrame = 0;
    state->brian.transform.animFramerate = 1;
    state->brian.transform.base.player = &gBrianStats;
    state->brian.stats = &gPlayerState.brian.statsBuf;
    state->brian.transform.actorFlags = 0;
    state->brian.transform.stateFlags = 0;
    gArrivalFlags = 0x108;
    gSavedMap = 0xD;
    gSavedSubmap = 0x11;
    gSavedEntrance = 0;
    gBattleAgilityDistance = 0.0f;
    D_8007A8F4 = D_8007A8F8 = 0.0f;
    gAgilityWalk = gAgilityWalkSafe = 0.0f;
}
extern u16 gSubmapFlags;               /* the submap's flags (bit 0: random battles and monsters here (FieldLoop, ResetEnemies; SetupNPCs only without it)) */
void SpawnBrian(void) {
    s32 sound;
    PlayerState* state;
    ActorTransform* brian;
    AnimEntry* anims;
    ModelTransform* door;

    state = &gPlayerState;
    brian = &state->brian.transform;
    gGameState &= ~0x10000;
    anims = gBrianAnims[brian->modelID];
    state->state = 0;
    state->floorTimer = 0;
    brian->pos.x = gBrianSpawnX;
    brian->pos.y = 0.0f;
    brian->pos.z = gBrianSpawnZ;
    brian->xrot = 0.0f;
    brian->zrot = 0.0f;
    brian->yrot = gBrianSpawnYaw;
    brian->xspeed = 0.0f;
    brian->yspeed = 0.0f;
    brian->zspeed = 0.0f;
    PollWorldCollision(brian->pos.x, brian->pos.y, brian->pos.z, 0xA00, &gBrianFloorPoll);
    brian->pos.y = gBrianFloorPoll.y;
    brian->shadowXRot = gBrianFloorPoll.sn_x;
    brian->shadowYRot = gBrianFloorPoll.sn_y;
    brian->shadowZRot = gBrianFloorPoll.sn_z;
    if (gBrianFloorPoll.flags & 0x8000) {
        brian->floorFlags = 0;
    } else {
        brian->floorFlags = gBrianFloorPoll.flags;
    }
    SetAnimation(brian, state->animCue ? state->animCue : gBrianIdleAnim, 1, gBrianIdleAnim, 1, 1);
    if (gArrivalFlags & 8) {
        state->state = 3;
        state->timer = 15;
        brian->stateFlags = ACTOR_BUSY;
    } else {
        switch ((gArrivalFlags >> 8) & 7) {
            case 1:
            case 4:
                state->state = 8;
                state->timer = 15;
                gBrianMovement.x = 0.0f;
                gBrianMovement.y = 20.0f;
                rotateCoordinatesByAngle(brian->yrot, &gBrianMovement);
                state->stepX = gBrianMovement.x;
                state->stepZ = gBrianMovement.y;
                brian->stateFlags = ACTOR_BUSY;
                PlaySound(5, 0x21, 0xFF);
                PlaySound(0xF, 0x21, 0xFF);
                break;
            case 2:
                state->state = 10;
                door = &gMapModels[gEntranceDoorModel];
                state->endYaw = brian->yrot = door->angle * 0.017453292519943295;
                if (gArrivalFlags & 1) {
                    state->timer = 0x15;
                    brian->animFrame = 0x55;
                    gBrianMovement.x = -115.61f;
                    sound = 0xE;
                    gBrianMovement.y = 15.746f;
                } else {
                    state->timer = 0x1F;
                    state->endYaw += 3.141592653589793;
                    brian->animFrame = 0x3C;
                    sound = 0x14;
                    gBrianMovement.x = -132.831f;
                    gBrianMovement.y = -120.681f;
                }
                rotateCoordinatesByAngle(-door->angle * 0.017453292519943295, &gBrianMovement);
                brian->pos.x = door->x - gBrianMovement.x * brian->scale;
                brian->pos.z = door->z + gBrianMovement.y * brian->scale;
                brian->stateFlags = ACTOR_BUSY;
                LoadBrianAnimation(brian);
                ApplyRootMotion(brian, &anims[brian->animID], brian->scale);
                StartDoorAnimation(1, brian->animFrame, gArrivalFlags, door);
                PlaySound(sound, 8, 0xFF);
                break;
            case 3:
                state->state = 8;
                state->timer = 15;
                gBrianMovement.x = 0.0f;
                gBrianMovement.y = 20.0f;
                rotateCoordinatesByAngle(brian->yrot, &gBrianMovement);
                state->stepX = gBrianMovement.x;
                state->stepZ = gBrianMovement.y;
                brian->stateFlags = ACTOR_BUSY;
                LoadBrianAnimation(brian);
                ApplyRootMotion(brian, &anims[brian->animID], brian->scale);
                StartDoorAnimation(1, brian->animFrame, gArrivalFlags, door);   /* BUG: door unset on this path */
                PlaySound(5, 0x21, 0xFF);
                PlaySound(0xF, 0x21, 0xFF);
                break;
        }
    }
    state->flags = 0;
    if (gPendingWarp.entrance == -1) {
        state->flags |= 0x10;
    }
    if (gGameState & 0x800) {
        brian->base.player->currHP = 1;
        brian->base.player->currMP = brian->base.player->maxMP;
        state->brian.stats->statusFlags = 0;
    }
    state->brian.stats->statusFlags = 0;
    gBrianStepDistance = gBrianStepPhase = 0.0f;
    gRunFootstepTimer = 0;
    gDroppedItem = 0xFF;
    gTalkTarget = 0;
    gSpiritTarget = 0;
    gObjectTarget = 0;
    gSignTarget = 0;
    if (state->stats->spirits[0] == 50) {
        state->flags |= 0x1000;
    }
    if (state->stats->spirits[1] == 50) {
        state->flags |= 0x2000;
    }
    if (state->stats->spirits[2] == 50) {
        state->flags |= 0x4000;
    }
    if (state->stats->spirits[3] == 50) {
        state->flags |= 0x8000;
    }
    if ((state->flags & 0xF000) == 0xF000) {
        state->flags |= 0x10000;
    }
}
void RespawnAtSavePoint(void) {
    int i;

    gPlayerState.stats->radius = 50.0f;
    gNextMap = gCurrentMap;
    gNextSubmap = gCurrentSubmap;
    gPendingWarp.entrance = 0;
    for(i = 0; i < 17; i++) {
        if ((gCurrentMap == gRespawnLocationTbl[i].map) && (gCurrentSubmap == gRespawnLocationTbl[i].submap)) {
            gBrianSpawnX = gRespawnLocationTbl[i].x;
            gBrianSpawnZ = gRespawnLocationTbl[i].z;
            gBrianSpawnYaw = gRespawnLocationTbl[i].yrot;
            gPendingWarp.entrance = -1;
            break;
        }
    }
    gCurrentMap = -1;
    gCurrentSubmap = -1;
}
extern u16 gButtonsPressed;               /* buttons pressed this frame */
extern u16 gSpiritGainGlow;               /* the spirit-gain window's glow frames */
extern s32 gMarkerScreenY;                /* the marker's screen y (0xF0 = off screen) */
extern u8 gFadeRampBlack16[16][4];                   /* screen fade settings (StartScreenFade) */
extern void (*gBrianStateHandlers[])();            /* Brian's state handlers (defined below) */
void StartScreenFade(s32 frames, u16 flags, void* ramp);
void ClearStatusEffects(MonsterBattleData* actor);
s32 FindStaffTarget(ActorTransform* brian);
s32 FindTalkTarget(PlayerState* state, ActorTransform* brian);
Spirit* CheckSpiritPickup(ActorTransform* brian);
MapObject* FindMapObjectInFront(ActorTransform* brian);
SignSlot* TryReadSign(ActorTransform* brian);
s32 IsNearBoss(void);
void StartBossScene(s32 boss);
void StartBrianStaffSwing(PlayerState* state, ActorTransform* brian, AnimEntry* anims);
void StartNPCTalk(s32 index);
void SetSpiritFound(u16 flag);
s32 CheckIfInventoryFull(void);
void OpenMapObject(ActorTransform* obj);
void StartBrianPickUp(PlayerState* state, ActorTransform* brian, AnimEntry* anims);
void LoadSignText(SignRef* sign);
s32 CanBrianAct(PlayerState* state, ActorTransform* brian);
s32 BrianSpellInput(PlayerState* state);
void BrianStartCast(PlayerState* state, ActorTransform* brian, AnimEntry* anims);
u32 CountItem(u8 itemID);
u32 CountInventory(void);
void UpdateBrian(void) {
    PlayerState* state;
    ActorTransform* brian;
    AnimEntry* anims;
    u8* spirits;

    state = &gPlayerState;
    brian = &state->brian.transform;
    anims = gBrianAnims[brian->modelID];
    if (state->stats->currHP * 3 < state->stats->maxHP) {
        gBrianIdleAnim = 0x19;
    } else {
        gBrianIdleAnim = (gBattleState & 1) ? 0x11 : 0;
    }
    gBrianMoveX = gBrianMoveZ = 0.0f;
    if (!(gGameState & 0x87)) {
        if (gBattleState & 1) {
            if (!(gBattleState & 2) && !(brian->stateFlags & ACTOR_BUSY)) {
                state->staffTarget = FindStaffTarget(brian);
            }
        } else if (!(brian->stateFlags & ACTOR_BUSY)) {
            gTalkTarget = FindTalkTarget(state, brian);
            if (gTalkTarget == 0) {
                gSpiritTarget = CheckSpiritPickup(brian);
                if (gSpiritTarget == NULL) {
                    gObjectTarget = FindMapObjectInFront(brian);
                    if (gObjectTarget == NULL) {
                        gSignTarget = TryReadSign(brian);
                        if (gSignTarget == NULL) {
                            if ((gNearBoss = IsNearBoss()) != 0) {
                                StartBossScene(gNearBoss - 1);
                                gTextBoxDelay = 30;
                                gGameState |= 2;
                                state->flags |= 1;
                            }
                        }
                    }
                }
            }
        }
    }
    if (!(gGameState & 0xF)) {
        if (gButtonsPressed & (A_BUTTON | Z_TRIG)) {
            if ((gBattleState & 1) && !(brian->stateFlags & ACTOR_BUSY)) {
                if (!(gBattleState & 6) && AnySpellActive() == 0) {
                    if (state->staffTarget != 0) {
                        StartBrianStaffSwing(state, brian, anims);
                    } else {
                        CountBattleAgility(state);
                        state->staffTarget = 0;
                        EndBattleTurn(&state->brian);
                    }
                }
            } else if (!(brian->stateFlags & ACTOR_BUSY)) {
                if (gTalkTarget != 0) {
                    gTalkTarget--;
                    state->state = 7;
                    brian->xrot = brian->zrot = brian->xspeed = brian->zspeed = 0.0f;
                    SetAnimation(brian, gBrianIdleAnim, 1, gBrianIdleAnim, 1, 1);
                    StartNPCTalk(gTalkTarget);
                    OpenMessageWindow(gMarkerScreenY < 0x46 ? 0 : 1);
                    gGameState |= 2;
                } else if (gSpiritTarget != NULL) {
                    spirits = state->stats->spirits;
                    if (spirits[0] != 50 || spirits[1] != 50 || spirits[2] != 50 || spirits[3] != 50) {
                        gGameState |= 8;
                        state->state = 0;
                        SetSpiritFound(gSpiritTarget->flag);
                        gSpiritTarget->pickedUp |= 1;
                        gSpiritGainGlow = 8;
                        PlaySound(0, 1, 0xFF);
                    }
                } else if (CheckIfInventoryFull() != 0 && gObjectTarget != NULL) {
                    OpenMapObject(&gObjectTarget->transform);
                    gBrianMovement.x = 0.0f;
                    gBrianMovement.y = brian->scale * 171.0f;
                    rotateCoordinatesByAngle(gObjectTarget->transform.yrot, &gBrianMovement);
                    brian->pos.x = gObjectTarget->transform.pos.x + gBrianMovement.x;
                    brian->pos.z = gObjectTarget->transform.pos.z + gBrianMovement.y;
                    brian->yrot = gObjectTarget->transform.yrot;
                    state->endYaw = gObjectTarget->transform.yrot + 3.141592653589793;
                    state->itemId = gObjectTarget->transform.modelID;
                    StartBrianPickUp(state, brian, anims);
                    gObjectTarget = NULL;
                } else if (gSignTarget != NULL) {
                    LoadSignText((SignRef*)gSignTarget);   /* SignRef: LoadSignText's type for this record */
                    state->state = 0x11;
                    brian->stateFlags |= ACTOR_BUSY;
                    brian->xrot = brian->zrot = brian->xspeed = brian->zspeed = 0.0f;
                    if (gSignTarget->npc->flags & 2) {
                        state->timer = 0xF;
                        state->flags |= 0x200;
                        SetAnimation(brian, 0x1B, 1, gBrianIdleAnim, 1, 1);
                    } else {
                        state->timer = 0xF;
                        SetAnimation(brian, 0xC, 1, 0xD, 1, 1);
                    }
                    gSignTarget = NULL;
                } else {
                    state->state = 3;
                    state->timer = 0xF;
                    brian->xrot = brian->zrot = brian->xspeed = brian->zspeed = 0.0f;
                    brian->stateFlags |= ACTOR_BUSY;
                    SetAnimation(brian, 0x1B, 1, gBrianIdleAnim, 1, 1);
                }
            }
        } else if (CanBrianAct(state, brian)) {
            if (!(gGameState & 4) && (gButtonsPressed & R_TRIG)) {
                gGameState ^= 1;
                StartScreenFade(6, 2, gFadeRampBlack16);
                PlaySound(0, 1, 0xFF);
                brian->stateFlags |= ACTOR_BUSY;
            }
            if (!(gGameState & 1) && !(state->brian.stats->statusFlags & 0x40) && BrianSpellInput(state)) {
                BrianStartCast(state, brian, anims);
            }
        }
    } else if (gGameState & 4) {
        if (BrianSpellInput(state)) {
            BrianStartCast(state, brian, anims);
        }
    } else if ((gGameState & 1) && (gButtonsPressed & (B_BUTTON | R_TRIG))) {
        gGameState ^= 1;
        StartScreenFade(6, 4, gFadeRampBlack16[5]);
        PlaySound(0, 2, 0xFF);
        brian->stateFlags &= ~ACTOR_BUSY;
    }
    if (gBattleState & 8) {
        gBattleState &= ~8;
        brian->base.player->exp += gBattleExp;
        brian->base.player->stone += gBattleStone;
        if (gDroppedItem != 0xFF && (CountItem(gDroppedItem) >= 3 || CountInventory() >= 0x8D)) {
            gDroppedItem = 0xFF;
        }
        ClearStatusEffects(&state->brian);
        state->brian.statsBuf.statusFlags = 0;
        state->state = 0xF;
        state->timer = anims[16].frames;
        brian->stateFlags |= ACTOR_BUSY;
        brian->xrot = brian->zrot = brian->xspeed = brian->zspeed = 0.0f;
        SetAnimation(brian, 0x10, 1, gBrianIdleAnim, 1, 1);
    } else if (gBattleState & 0x10) {
        ClearStatusEffects(&state->brian);
        state->brian.statsBuf.statusFlags = 0;
    }
    gBrianStateHandlers[state->state](brian, state, anims);
    gBrianStepDistance = sqrtf(gBrianMoveX * gBrianMoveX + gBrianMoveZ * gBrianMoveZ);
    if (!(gBattleState & 1) && brian->base.player->currMP < brian->base.player->maxMP) {
        gBrianStepPhase += gBrianStepDistance;
        if (gBrianStepPhase > 60.0) {
            gBrianStepPhase -= 60.0;
            brian->base.player->currMP++;
        }
    }
}
extern Gfx* gDisplayListHead;
extern Gfx D_10000A0[];             /* Brian's render setup */
extern Vec3f gPartPositions[];           /* recorded part positions (model.c) */
void StepLightAnim();   /* RULE-EXCEPTION R25: unprototyped; this caller passes a second argument the definition does not take */
void DrawModel(s32 flags, ActorTransform* actor, AnimEntry* anim, s32 mtxList);
void AdvanceAnimation(s32 framerate, ActorTransform* model, AnimEntry* anim);
void AddShadow(f32 x, f32 y, f32 z, f32 xrot, f32 yrot, f32 zrot, f32 size);
void DrawBrian(Mtx* mtx) {
    s32 flags;
    PlayerState* state;
    ActorTransform* brian;
    AnimEntry* anims;

    gSPDisplayList(gDisplayListHead++, D_10000A0);
    StepLightAnim(gActorLightAnim, &gDisplayListHead);
    state = &gPlayerState;
    brian = &state->brian.transform;
    if (state->state == 0xA || state->state == 0xB) {
        flags = 1;
    } else {
        flags = 0;
    }
    anims = gBrianAnims[brian->modelID];
    LoadBrianAnimation(brian);
    DrawModel(flags, brian, &anims[brian->animID], mtx);
    AdvanceAnimation(brian->animFramerate, brian, &anims[brian->animID]);
    LoadBrianAnimation(brian);
    PollWorldCollision(gPartPositions[0].x, brian->pos.y, gPartPositions[0].z, 0xA00, &gBrianFloorPoll);
    gBrianFloorPos.x = gPartPositions[0].x;
    gBrianFloorPos.y = gBrianFloorPoll.y;
    gBrianFloorPos.z = gPartPositions[0].z;
    AddShadow(gBrianFloorPos.x, gBrianFloorPos.y, gBrianFloorPos.z, brian->shadowXRot, brian->shadowYRot, brian->shadowZRot,
                  brian->collisionScale * brian->scale);
}

void SetupBrianForBattle(void) {
    /* Battle start: Brian's battle values into his state block: movement radius
       agi * 0.2857 + 17, the spirit total, staff base power 16, agility / defence copies, and unk10 = the total
       less each element's excess over total / 4, then / 10 + 1, at most 16. Then state 3 for the length of
       animation entry 18 (+0xDA) */
    PlayerState* state;
    AnimEntry* anims;
    BrianData1* stats;
    s32 quarter;
    s32 i;

    state = &gPlayerState;
    gBattleExp = gBattleStone = 0;
    stats = state->brian.transform.base.player;
    anims = gBrianAnims[state->brian.transform.modelID];
    state->moveRadius = (f32)(u32)stats->agi * 0.2857 + 17.0;
    state->brian.statsBuf.statusFlags = 0;
    state->brian.statsBuf.hitMask = 0;
    state->brian.statsBuf.atk = state->stats->spirits[0] + state->stats->spirits[1] + state->stats->spirits[2] +
                    state->stats->spirits[3];
    state->brian.statsBuf.atkScale = 0x10;
    quarter = state->brian.statsBuf.atk >> 2;
    state->brian.statsBuf.agi = stats->agi;
    state->brian.statsBuf.def = stats->def;
    state->comboWindow = state->brian.statsBuf.atk;
    state->brian.statsBuf.moveScale = 1.0f;
    for (i = 0; i < 4; i++) {
        if (quarter < state->stats->spirits[i]) {
            state->comboWindow = state->comboWindow - state->stats->spirits[i] + quarter;
        }
    }
    state->comboWindow = state->comboWindow / 10 + 1;
    if (state->comboWindow > 16) {
        state->comboWindow = 16;
    }
    state->state = 3;
    state->timer = anims[18].frames;
    state->staffTarget = 0;
    state->savedYRot = state->brian.transform.yrot;
    state->flags &= ~0x20;
    state->brian.transform.stateFlags |= ACTOR_BUSY;
    if (gBrianIdleAnim != 0x19) {
        gBrianIdleAnim = 0x11;
    }
    SetAnimation(&state->brian.transform, 0x12, 1, gBrianIdleAnim, 1, 1);
}
extern Dust gRunDustTemplate;              /* the running dust template */
Dust* SpawnDust(Dust* dust);
void StartScriptedBattle(void);
void BrianFieldMovement(ActorTransform* brian, PlayerState* state, AnimEntry* anims) {
    f32 x;
    f32 y;

    x = -(f32)gStickX;
    y = -(f32)gStickY;
    if (x > -8.0f && x < 8.0f) {
        x = 0.0f;
    }
    if (y > -8.0f && y < 8.0f) {
        y = 0.0f;
    }
    if (x != 0.0f || y != 0.0f) {
        gBrianMovement.x = x;
        gBrianMovement.y = y;
        rotateCoordinatesByAngle(gCamera.target_yaw, &gBrianMovement);
        x = gBrianMovement.x;
        y = gBrianMovement.y;
    }
    UpdateBrianGrowth(state, brian);
    if (gFieldFreeze != 0) {
        x = y = 0.0f;
    } else if (gInputFreeze != 0) {
        x = y = 0.0f;
    } else if (gGameState & 0x300F) {
        x = y = 0.0f;
    } else if (state->brian.statsBuf.statusFlags & 8) {
        x = y = 0.0f;
    }
    if ((gBattleState & 1) && (gBattleState & 0x202)) {
        x = y = 0.0f;
    } else if (gTurnDelay != 0) {
        x = y = 0.0f;
    }
    if (state->flags & 1) {
        if (gTextBoxDelay != 0) {
            if (--gTextBoxDelay == 0) {
                OpenMessageWindow(0);
            }
        } else if (gMessageWindowsOpen == 0) {
            state->flags &= ~1;
            gGameState &= ~2;
            StartScriptedBattle();
        }
    } else if (state->flags & 2) {
        if (gMessageWindowsOpen == 0) {
            state->flags &= ~2;
            gGameState &= ~2;
        }
    } else if (state->flags & 0x10) {
        if (gMessageWindowsOpen == 0) {
            state->flags &= ~0x10;
            gGameState &= ~2;
        }
    }
    switch (state->state) {
        case 2:
            BrianState_Run(x, y, brian, state);
            break;
        case 1:
            BrianState_Walk(x, y, brian, state);
            break;
        default:
            BrianState_Stand(x, y, brian, state);
            break;
    }
    BrianBattleMovement(0xA01, brian, state);
    if (state->state == 2) {
        gRunFootstepTimer += brian->animFramerate;
        if (gRunFootstepTimer >= 151) {
            gRunFootstepTimer = 0;
            PlaySound(0, 0x21, 0xFF);
        }
        if (state->timer != 0) {
            if (!(state->timer & 3)) {
                gRunDustTemplate.x = brian->pos.x;
                gRunDustTemplate.y = brian->pos.y + 1.0f;
                gRunDustTemplate.z = brian->pos.z;
                gRunDustTemplate.vx = 0.0f;
                gRunDustTemplate.vy = 0.2f;
                gRunDustTemplate.vz = 0.0f;
                SpawnDust(&gRunDustTemplate);
            }
            state->timer--;
        }
    }
    ApplyFloorEffect(state, brian);
}
void BrianState_Wait(ActorTransform* brian, PlayerState* state, AnimEntry* anims) {
    brian->zspeed = 0.0f;
    brian->xspeed = 0.0f;
    state->timer--;
    if (state->timer == 0) {
        state->state = 0;
        brian->stateFlags &= ~(ACTOR_BUSY | ACTOR_NO_MELEE_HIT);
    }
}
void BrianState_Slide(ActorTransform* brian, PlayerState* state, AnimEntry* anims) {
    BrianBattleMovement(0xA01, brian, state);
    brian->xspeed *= 0.68;
    brian->zspeed *= 0.68;
    state->timer--;
    if (!state->timer) {
        state->state = 0;
        brian->zspeed = 0.0f;
        brian->xspeed = 0.0f;
        brian->stateFlags &= ~ACTOR_BUSY;
    }
}
extern s32 gModeCountdown;
extern u16 gTransitionStyle;
extern u16 gFadeOutAt;
extern u16 gXPTable[];             /* XP needed per stat level */
extern u16 gItemBarSlide;
extern u16 gHudSlideState;
void RequestMusic(s8 song);         /* music request (sound.c) */
void BrianState_KnockedDown(ActorTransform* brian, PlayerState* state, AnimEntry* anims) {
    BrianData1* stats;

    brian->zspeed = 0.0f;
    brian->xspeed = 0.0f;
    state->timer--;
    if (!state->timer) {
        stats = state->stats;
        brian->xspeed = 0.0f;
        brian->zspeed = 0.0f;
        if (stats->currHP == 0) {
            state->state = 6;
            state->timer = 0xE6;
            SetAnimation(brian, 9, 1, gBrianIdleAnim, 1, 1);
            ClearStatusEffects((MonsterBattleData*)brian);
            RequestMusic(0x1E);
            gGameState |= 0x880;
            gModeCountdown = state->timer;
            gTransitionStyle = 2;
            gFadeOutAt = 0x5A;
            state->subTimer = 0xC8;
            gPendingWarp.map = gSavedMap;
            gPendingWarp.submap = gSavedSubmap;
            gPendingWarp.entrance = gSavedEntrance;
            gBrianSpawnX = 0.0f;
            gBrianSpawnZ = 0.0f;
            gBrianSpawnYaw = 0.0f;
            gArrivalFlags = 0x108;
            gPlayerState.animCue = 0;
            gHudSlideState = 1;
            gItemBarSlide = 0xC;
            return;
        }
        state->state = 0;
        brian->stateFlags &= ~(ACTOR_BUSY | ACTOR_NO_MELEE_HIT);
    }
}
void BrianState_Defeated(ActorTransform* brian, PlayerState* state, AnimEntry* anims) {
    ApplyBrianMovement(brian, anims, &gBrianMovement);
    brian->xspeed = gBrianMovement.x;
    brian->zspeed = gBrianMovement.y;
    BrianBattleMovement(0xA01, brian, state);
    state->timer--;
    if (state->timer == state->subTimer) {
        gCamera.ideal_planar_distance = 250.0f;
        gCamera.local_camera_height = 150.0f;
        gCamera.pull = 0.01f;
    }
}
extern s32 gMessageChoice;
extern Npc gNpcs[];      /* the map objects */
void ResumeNpcAfterTalk(s32 index);
s32 StartMusicFadeOut(u32 frames);      /* start the music fade-out (src/game/audio.c) */
void BrianState_TalkEnd(ActorTransform* brian, PlayerState* state, AnimEntry* anims) {
    brian->zspeed = 0.0f;
    brian->xspeed = 0.0f;
    if (gMessageWindowsOpen == 0) {
        ResumeNpcAfterTalk(gTalkTarget);
        state->state = 0;
        brian->stateFlags &= ~ACTOR_BUSY;
        gGameState &= ~2;
        if (gGameState & 0x10) {
            gGameState &= ~0x10;
            if (gNpcs[gTalkTarget].record->talkType == 6 && gMessageChoice == 0) {
                gGameState |= 0x480;
                gModeCountdown = 0x23;
                gFadeOutAt = 0x1E;
                gSavedMap = gCurrentMap;
                gSavedSubmap = gCurrentSubmap;
                state->state = 3;
                state->timer = 0x41;
                brian->stateFlags |= ACTOR_BUSY;
                StartMusicFadeOut(0x3C);
            }
        } else if (state->flags & 0x40) {
            state->flags &= ~0x40;
            PlaySound(0, 0x3B, 0xFF);
        }
        gTalkTarget = 0;
    }
}
void BrianState_WalkIn(ActorTransform* brian, PlayerState* state, AnimEntry* anims) {
    BrianState_Walk(state->stepX, state->stepZ, brian, state);
    BrianBattleMovement(0xA01, brian, state);
    state->timer--;
    if (!state->timer) {
        state->state = 0;
        brian->zspeed = 0.0f;
        brian->xspeed = 0.0f;
        brian->stateFlags &= ~ACTOR_BUSY;
    }
}
extern u8 gCritFade[];             /* the critical hit cue */
u16 DamageEnemy(s32 enemy, u16 damage, u16 element, u16 flags, MonsterBattleData* attacker);   /* deal damage */
void ShowEnemyMiss(MonsterBattleData* actor);
void SpawnDamageNumber(MonsterBattleData* target, s32 kind, s32 damage);
void UpdateStatusVisuals(MonsterBattleData* actor, u16 statusBits, u16 kind, s16 duration);
void SpawnStaffHitEffect(f32 x, f32 y, f32 z);   /* staff hit effect */
void BrianState_StaffSwing(MonsterBattleData* brian, PlayerState* state, AnimEntry* anims) {
    u16 dmg;
    s32 flags;

    brian->transform.zspeed = 0.0f;
    brian->transform.xspeed = 0.0f;
    if (--state->timer == 0) {
        if (RollStaffHit(state->staffTarget - 1, brian) != 0) {
            dmg = CalcStaffPower(state);
            if (state->state == 0x10) {
                dmg = dmg * 0.5;
            }
            flags = 0x8004;
            if (state->flags & 0x400) {
                flags = 0xC004;
                state->flags &= ~0x400;
            }
            dmg = DamageEnemy(state->staffTarget - 1, dmg, (state->spellId >> 8) & 0xF, flags, brian);
            if (dmg != 0) {
                brian->transform.base.player->currMP++;
                if (brian->transform.base.player->currMP > brian->transform.base.player->maxMP) {
                    brian->transform.base.player->currMP = brian->transform.base.player->maxMP;
                }
                if (state->brian.statsBuf.statusFlags & 2) {
                    dmg = ((dmg + 3) >> 2) + 1;
                    brian->transform.base.player->currHP += dmg;
                    if (brian->transform.base.player->currHP > brian->transform.base.player->maxHP) {
                        brian->transform.base.player->currHP = brian->transform.base.player->maxHP;
                    }
                    SpawnDamageNumber(brian, 1, dmg);
                    UpdateStatusVisuals(brian, 2, 1, 0x1E);
                }
                gBrianMovement.x = 0.0f;
                gBrianMovement.y = brian->transform.base.player->radius * brian->transform.scale * 2.0f;
                rotateCoordinatesByAngle(brian->transform.yrot, &gBrianMovement);
                SpawnStaffHitEffect(brian->transform.pos.x + gBrianMovement.x, brian->transform.pos.y + brian->transform.base.player->height * brian->transform.scale * 0.7f,
                              brian->transform.pos.z + gBrianMovement.y);
            }
            state->stats->HPXP++;
        } else {
            ShowEnemyMiss(&gEnemies[state->staffTarget - 1].battleData);
        }
        if (gEnemies[state->staffTarget - 1].currHP == 0) {
            state->flags &= ~0x100;
        }
        if (state->state == 0xD && (state->flags & 0x100)) {
            state->state = 0x10;
            state->timer = 0x12;
            state->flags &= ~0x100;
            brian->transform.stateFlags |= ACTOR_BUSY;
            SetAnimation(&brian->transform, 0xB, 1, gBrianIdleAnim, 1, 1);
            gCamera.timer = 0x23;
            return;
        }
        state->state = 3;
        if (state->state == 0xD) {   /* BUG: the state was just set to 3, so the first arm never runs */
            state->timer = anims[10].frames - 20;
        } else {
            state->timer = anims[11].frames - 22;
        }
        state->staffTarget = 0;
        state->flags &= ~0x100;
        CountBattleAgility(state);
        state->staffTarget = 0;
        EndBattleTurn(brian);
        return;
    }
    if (state->state == 0xD && state->comboWindow >= state->timer && (gButtonsPressed & (A_BUTTON | Z_TRIG))) {
        state->flags |= 0x100;
    }
}
void SpawnSpell(MonsterBattleData* caster, u32 spell, s32 shot, u16 flags);   /* cast the spell */
void BrianState_Cast(MonsterBattleData* brian, PlayerState* state, AnimEntry* anims) {
    brian->transform.xspeed = brian->transform.zspeed = 0.0f;
    if (state->subTimer != 0) {
        if (--state->subTimer == 0) {
            SpawnSpell(brian, state->spellId, 0, (state->flags & 0x10000) ? 0x10 : 0);
        }
    } else if (--state->timer == 0) {
        if (AnySpellActive() != 0) {
            state->timer = 8;
        } else {
            brian->transform.stateFlags &= ~ACTOR_BUSY;
            if (state->flags & 4) {
                state->state = 3;
                state->timer = anims[24].frames;
                state->flags &= ~4;
                SetAnimation(&brian->transform, 24, 1, gBrianIdleAnim, 1, 1);
            } else {
                state->state = 0;
            }
            CountBattleAgility(state);
            state->staffTarget = 0;
            EndBattleTurn(brian);
        }
    }
}
extern f64 D_8006ED18;  /* pi */
extern f64 D_8006ED20;  /* 2 pi */
extern f64 D_8006ED28;  /* -pi */
extern f64 D_8006ED30;  /* 2 pi */
extern f64 D_8006ED38;  /* 2 pi */
extern f64 D_8006ED40;  /* 0.3: turn rate */
extern f64 D_8006ED48;  /* -pi */
void BrianState_EnterDoor(ActorTransform* brian, PlayerState* state, AnimEntry* anims) {
    f32 diff;

    brian->zspeed = 0.0f;
    brian->xspeed = 0.0f;
    brian->pos.x += state->stepX;
    brian->pos.z += state->stepZ;
    diff = state->savedYRot - brian->yrot;
    if (diff > 3.141592653589793) {
        diff -= 6.283185307179586;
    } else if (diff < -3.141592653589793) {
        diff += 6.283185307179586;
    }
    brian->yrot += diff * 0.3;
    if (brian->yrot > 3.141592653589793) {
        brian->yrot -= 6.283185307179586;
    } else if (brian->yrot < -3.141592653589793) {
        brian->yrot += 6.283185307179586;
    }
    state->timer--;
    if (!state->timer) {
        state->state = 0xA;
        state->timer = 0x50;
        SetAnimation(brian, state->animCue, 1, gBrianIdleAnim, 1, 1);
        LoadBrianAnimation(brian);
        ApplyRootMotion(brian, &anims[brian->animID], brian->scale);
        brian->yrot = state->endYaw;
        PlaySound(0x1A, 8, 0xFF);
    }
}
void BrianState_ThroughDoor(ActorTransform* brian, PlayerState* state, AnimEntry* anims) {
    ApplyBrianMovement(brian, anims, &gBrianMovement);
    brian->xspeed = gBrianMovement.x;
    brian->zspeed = gBrianMovement.y;
    BrianBattleMovement(0xA00, brian, state);
    state->timer--;
    if (!state->timer) {
        state->state = 0;
        brian->stateFlags &= ~ACTOR_BUSY;
        brian->yrot = state->endYaw;
    }
}
void BrianState_PickUpItem(ActorTransform* brian, PlayerState* state, AnimEntry* anims) {
    if (state->timer != 0) {
        ApplyBrianMovement(brian, anims, &gBrianMovement);
        brian->xspeed = gBrianMovement.x;
        brian->zspeed = gBrianMovement.y;
        BrianBattleMovement(0xA00, brian, state);
        if (state->timer == 0x20) {
            BuildPickupMessage(state->itemId);
            OpenMessageWindow(1);
            AddItemToInventory(state->itemId);
            gGameState |= 2;
            PlaySound(0, 0x3B, 0xFF);
        }
        state->timer--;
        if (!state->timer) {
            brian->yrot = state->endYaw;
            SetAnimation(brian, gBrianIdleAnim, 1, gBrianIdleAnim, 1, 1);
        }
    } else if (gMessageWindowsOpen == 0) {
        state->state = 0;
        brian->stateFlags &= ~ACTOR_BUSY;
        gGameState &= ~2;
        gTalkTarget = 0;
    }
}
extern ItemData gItemData[];
void UseItem(u8 itemID, MonsterBattleData* brian);
void BrianState_UseItem(MonsterBattleData* brian, PlayerState* state, AnimEntry* anims) {
    state->timer--;
    if (!state->timer) {
        UseItem(state->itemId, brian);
        if (gItemData[state->itemId].type == 2) {
            CountBattleAgility(state);
            state->staffTarget = 0;
            EndBattleTurn(brian);
            return;
        }
        state->subTimer = 0xF;
        CountBattleAgility(state);
        return;
    }
    state->subTimer--;
    if (!state->subTimer) {
        if (AnySpellActive() != 0) {
            state->subTimer = 8;
            return;
        }
        state->state = 3;
        state->timer = 0xF;
        SetAnimation(&brian->transform, 0x18, 1, gBrianIdleAnim, 1, 1);
        state->staffTarget = 0;
        EndBattleTurn(brian);
    }
}
void BrianState_Escape(ActorTransform* brian, PlayerState* state, AnimEntry* anims) {
    f32 diff;

    brian->zspeed = 0.0f;
    brian->xspeed = 0.0f;
    if (state->timer < 0x26) {
        diff = state->savedYRot - brian->yrot;
        if (diff < -3.141592653589793) {
            diff += 6.283185307179586;
        } else if (diff > 3.141592653589793) {
            diff -= 6.283185307179586;
        }
        brian->yrot += diff * 0.2;
        if (brian->yrot < -3.141592653589793) {
            brian->yrot += 6.283185307179586;
        } else if (brian->yrot > 3.141592653589793) {
            brian->yrot -= 6.283185307179586;
        }
    }
    state->timer--;
    if (state->timer == 0) {
        state->state = 0;
        brian->stateFlags &= ~ACTOR_NO_MELEE_HIT;
        if (gFieldMode != 4) {
            FindMapBGM(gCurrentMap, gCurrentSubmap, 0x1E);
        }
    }
}
void BrianState_FaceAndMessage(ActorTransform* brian, PlayerState* state, AnimEntry* anims) {
    f32 diff;

    diff = state->savedYRot - brian->yrot;
    if (diff < -3.141592653589793) {
        diff += 6.283185307179586;
    } else if (diff > 3.141592653589793) {
        diff -= 6.283185307179586;
    }
    brian->yrot += diff * 0.2;
    if (brian->yrot < -3.141592653589793) {
        brian->yrot += 6.283185307179586;
    } else if (brian->yrot > 3.141592653589793) {
        brian->yrot -= 6.283185307179586;
    }
    if (state->timer != 0) {
        state->timer--;
        if (!state->timer) {
            OpenMessageWindow(1);
            gGameState |= 2;
        }
    } else if (gMessageWindowsOpen == 0) {
        state->state = 3;
        if (state->flags & 0x200) {
            state->timer = 2;
        } else {
            state->timer = 8;
            SetAnimation(brian, 0xE, 1, gBrianIdleAnim, 1, 1);
        }
        state->flags &= ~0x200;
        gGameState &= ~2;
        gTalkTarget = 0;
    }
}
/* Brian's state handlers by PlayerState.state, called (brian, state, anims) by UpdateBrian. Declared without a
   prototype: two handlers (BrianState_Wait, BrianState_Slide) still have placeholder parameter types. */
void (*gBrianStateHandlers[])() = {
    BrianFieldMovement, BrianFieldMovement, BrianFieldMovement, BrianState_Wait, BrianState_Slide, BrianState_KnockedDown,
    BrianState_Defeated, BrianState_TalkEnd, BrianState_WalkIn, BrianState_EnterDoor, BrianState_ThroughDoor, BrianState_PickUpItem,
    BrianState_UseItem, BrianState_StaffSwing, BrianState_Cast, BrianState_Escape, BrianState_StaffSwing, BrianState_FaceAndMessage,
};
void BrianState_Stand(f32 analog_x, f32 analog_y, ActorTransform* brian, PlayerState* state) {
    f32 desired_speed;
    s32 idleAnim;

    brian->zspeed = 0.0f;
    brian->xspeed = 0.0f;

    if (analog_x != 0.0f || analog_y != 0.0f) {
        desired_speed = sqrtf((analog_x * analog_x) + (analog_y * analog_y)) * 0.025;

        if (desired_speed > 2.0) {
            desired_speed = 2.0f;
        }
        if (desired_speed > 1.5) {
            state->state = 2;
            state->timer = 0x1E;
            gRunFootstepTimer = 0;
            return;
        }

        state->state = 1;
        return;
    }

    idleAnim = gBrianIdleAnim;

    if ((idleAnim != brian->animID) && (idleAnim != brian->afterAnim)) {
        SetAnimation(brian, idleAnim, 1, idleAnim, 1, 1);
    }
}
void BrianState_Walk(f32 x, f32 y, ActorTransform* brian, PlayerState* state) {
    s32 anim;
    s32 next;
    s32 rate;
    s32 frame;
    s32 loop;
    f32 angle;
    f32 s;
    f32 want;
    f32 prev;
    f32 turn;

    prev = sqrtf(brian->xspeed * brian->xspeed + brian->zspeed * brian->zspeed);
    frame = 0;
    if (x != 0.0f || y != 0.0f) {
        want = sqrtf(x * x + y * y) * 0.025;
        if (want > 2.0) {
            x = 2.0 * x / want;
            y = 2.0 * y / want;
            want = 2.0f;
        }
        angle = Atan2(x, y) - brian->yrot;
        if (angle > 3.141592653589793) {
            angle -= 6.283185307179586;
        } else if (angle < -3.141592653589793) {
            angle += 6.283185307179586;
        }
        if (angle > 0.7853981633974483) {
            angle = 0.7853982f;
        } else if (angle < -0.7853981633974483) {
            angle = -0.7853982f;
        }
        if (angle >= 0.0f) {
            turn = angle;
        } else {
            turn = -angle;
        }
        want = want * (1.0 - turn / 0.7853981633974483);
        brian->yrot += angle * 0.5;
        if (brian->yrot > 3.141592653589793) {
            brian->yrot -= 6.283185307179586;
        } else if (brian->yrot < -3.141592653589793) {
            brian->yrot += 6.283185307179586;
        }
        gBrianMovement.x = 0.0f;
        s = prev + (want - prev) * 0.85;
        gBrianMovement.y = s;
        rotateCoordinatesByAngle(brian->yrot, &gBrianMovement);
        brian->xspeed = gBrianMovement.x;
        brian->zspeed = gBrianMovement.y;
        anim = 1;
        next = 1;
        loop = 0;
        rate = want * 14.0f + 3.0f;
        if (want > 1.5 || s > 1.5) {
            state->state = 2;
            state->timer = 30;
            gRunFootstepTimer = 0;
        }
    } else {
        brian->xspeed *= 0.82;
        brian->zspeed *= 0.82;
        if (prev > 1.0) {
            anim = 15;
            next = gBrianIdleAnim;
            frame = 3;
            state->state = 4;
            state->timer = 8;
            brian->xrot = 0.0f;
            brian->zrot = 0.0f;
            brian->xspeed *= 0.9;
            brian->zspeed *= 0.9;
            rate = 1;
            loop = 1;
            FindFreeEffectSlot(brian);
        } else {
            anim = 1;
            next = 1;
            loop = 0;
            rate = prev * 14.0f + 3.0f;
            if (prev < 0.1) {
                state->state = 0;
                brian->xspeed = 0.0f;
                brian->zspeed = 0.0f;
            }
        }
    }
    if (anim != brian->animID && anim != brian->afterAnim) {
        SetAnimation(brian, anim, rate, next, rate, loop);
        brian->animFrame = frame;
    } else {
        brian->animFramerate = rate;
    }
}
void BrianState_Run(f32 x, f32 y, ActorTransform* brian, PlayerState* state) {
    s32 anim;
    s32 next;
    s32 rate;
    s32 frame;
    s32 loop;
    f32 angle;
    f32 s;
    f32 want;
    f32 prev;

    prev = sqrtf(brian->xspeed * brian->xspeed + brian->zspeed * brian->zspeed);
    if (x != 0.0f || y != 0.0f) {
        want = sqrtf(x * x + y * y) * 0.025;
        if (want > 2.0) {
            x = 2.0 * x / want;
            y = 2.0 * y / want;
            want = 2.0f;
        }
        angle = Atan2(x, y) - brian->yrot;
        if (angle > 3.141592653589793) {
            angle -= 6.283185307179586;
        } else if (angle < -3.141592653589793) {
            angle += 6.283185307179586;
        }
        brian->yrot += angle * 0.4;
        if (brian->yrot > 3.141592653589793) {
            brian->yrot -= 6.283185307179586;
        } else if (brian->yrot < -3.141592653589793) {
            brian->yrot += 6.283185307179586;
        }
        gBrianMovement.x = 0.0f;
        s = prev + (want - prev) * 0.2;
        gBrianMovement.y = s;
        rotateCoordinatesByAngle(brian->yrot, &gBrianMovement);
        brian->xspeed = gBrianMovement.x;
        brian->zspeed = gBrianMovement.y;
        anim = 2;
        next = 2;
        frame = 0;
        rate = want * 14.0f + 3.0f;
        loop = 0;
        if (s < 1.0 && want < 1.0) {
            state->state = 1;
        }
    } else {
        anim = 15;
        next = gBrianIdleAnim;
        frame = 3;
        state->state = 4;
        state->timer = 8;
        brian->xrot = 0.0f;
        brian->zrot = 0.0f;
        brian->xspeed *= 0.9;
        brian->zspeed *= 0.9;
        rate = 1;
        loop = 1;
        FindFreeEffectSlot(brian);
    }
    if (anim != brian->animID && anim != brian->afterAnim) {
        SetAnimation(brian, anim, rate, next, rate, loop);
        brian->animFrame = frame;
    } else {
        brian->animFramerate = rate;
    }
}
void ApplyBrianMovement(ActorTransform* brian, AnimEntry* anims, Coordinates2D* offset) {
    LoadBrianAnimation(brian);
    ApplyRootMotion(brian, &anims[brian->animID], brian->scale);
    offset->x = -brian->rootMoveX;
    offset->y = brian->rootMoveZ;
    rotateCoordinatesByAngle(brian->yrot, offset);
}
void func_80006A50(void) {
}
extern f32 gBattleCenterX;               /* battle arena centre x */
extern f32 gBattleCenterZ;               /* and z */
extern f32 gBattleRadius;               /* battle arena radius */
void PushBrianOutOfEnemies(f32* xspeed, f32* zspeed, ActorTransform* brian);   /* push out of the enemies */
void BrianBattleMovement(u16 mask, ActorTransform* brian, PlayerState* state) {
    f32 dx;
    f32 dz;
    f32 dist;
    f32 radius;
    BrianData1* data;

    if ((gBattleState & 1) && !(gBattleState & 6)) {
        dx = (brian->pos.x + brian->xspeed) - gTurnCenterX;
        dz = (brian->pos.z + brian->zspeed) - gTurnCenterZ;
        dist = sqrtf(dx * dx + dz * dz);
        radius = state->moveRadius * state->brian.statsBuf.moveScale;
        if (radius < dist) {
            brian->xspeed = ((radius * dx) / dist - brian->pos.x) + gTurnCenterX;
            brian->zspeed = ((radius * dz) / dist - brian->pos.z) + gTurnCenterZ;
        }
        dx = (brian->pos.x + brian->xspeed) - gBattleCenterX;
        dz = (brian->pos.z + brian->zspeed) - gBattleCenterZ;
        dist = sqrtf(dx * dx + dz * dz);
        if (gBattleState & 0x100) {
            if (gBattleRadius < dist) {
                brian->xspeed = ((gBattleRadius * dx) / dist - brian->pos.x) + gBattleCenterX;
                brian->zspeed = ((gBattleRadius * dz) / dist - brian->pos.z) + gBattleCenterZ;
            }
        } else if (gBattleRadius < dist) {
            state->flags |= 0x20;
        } else {
            state->flags &= ~0x20;
        }
    }
    PushBrianOutOfEnemies(&brian->xspeed, &brian->zspeed, brian);
    gBrianMoveX = brian->xspeed;
    gBrianMoveZ = brian->zspeed;
    dx = brian->pos.x + brian->xspeed;
    dz = brian->pos.z + brian->zspeed;
    data = brian->base.player;
    gBrianFloorPoll.x = data->radius * data->modelScale;
    PollWorldCollision(dx, brian->pos.y, dz, mask, &gBrianFloorPoll);
    brian->xspeed = gBrianFloorPoll.x - brian->pos.x;
    brian->zspeed = gBrianFloorPoll.z - brian->pos.z;
    brian->pos.x = gBrianFloorPoll.x;
    brian->pos.y = gBrianFloorPoll.y;
    brian->pos.z = gBrianFloorPoll.z;
    brian->shadowXRot = gBrianFloorPoll.sn_x;
    brian->shadowYRot = gBrianFloorPoll.sn_y;
    brian->shadowZRot = gBrianFloorPoll.sn_z;
    brian->floorFlags = gBrianFloorPoll.flags;
}
extern s32 gNpcsDrawn;               /* NPCs drawn this frame */
extern ActorTransform* gNpcsDrawnList[];     /* their transforms */
s32 FindTalkTarget(PlayerState* state, ActorTransform* brian) {
    s32 found;
    s32 i;
    ActorTransform** list;
    ActorTransform* npc;
    Npc* obj;
    f32 dx;
    f32 dy;
    f32 dz;

    found = 0;
    if (!(gGameState & 0x448D) && !(state->flags & 0x10)) {
        list = gNpcsDrawnList;
        for (i = gNpcsDrawn; i != 0 && found == 0; i--) {
            npc = *list++;
            dx = npc->pos.x - brian->pos.x;
            dy = npc->pos.y - brian->pos.y;
            dz = npc->pos.z - brian->pos.z;
            if (dy > -15.0 && dy < 15.0) {
                if (dx * dx + dz * dz < ((npc->stateFlags & NPC_WIDE_TALK) ? 625.0f : 225.0f)) {
                    gBrianMovement.x = dx;
                    gBrianMovement.y = dz;
                    rotateCoordinatesByAngle(brian->yrot, &gBrianMovement);
                    if (gBrianMovement.y >= 0.0f && gBrianMovement.x > -6.0 && gBrianMovement.x < 6.0) {
                        found++;
                        for (obj = gNpcs; &obj->transform != npc; obj++) {
                            found++;
                        }
                    }
                }
            }
        }
    }
    return found;
}
extern s32 gEnemiesDrawn;
extern MonsterBattleData* gDrawnEnemies[];
extern s32 gPropsNearCount;
extern ModelTransform* gPropsNear[];
void PushBrianOutOfEnemies(f32* dx, f32* dz, ActorTransform* brian) {
    f32 ddx;
    f32 ddz;
    f32 rr;
    f32 nx;
    f32 nz;
    f32 r;
    f32 d;
    f32 f;
    s32 hits;
    s32 n;
    MonsterBattleData** list;
    MonsterBattleData* other;
    ModelTransform** props;
    ModelTransform* prop;

    r = brian->collisionScale * brian->scale;
    nx = *dx + brian->pos.x;
    nz = *dz + brian->pos.z;
    hits = 0;
    n = gEnemiesDrawn;
    if (n != 0) {
        for (list = gDrawnEnemies; hits == 0 && n != 0; n--) {
            other = *list++;
            ddx = nx - other->transform.pos.x;
            ddz = nz - other->transform.pos.z;
            rr = (other->transform.collisionScale * other->transform.scale) + r;
            if (ddx < rr && -rr < ddx && ddz < rr && -rr < ddz) {
                d = sqrtf((ddx * ddx) + (ddz * ddz));
                if (d < rr) {
                    f = rr / d;
                    hits = 1;
                    ddx *= f;
                    ddz *= f;
                    *dx = (other->transform.pos.x + ddx) - brian->pos.x;
                    *dz = (other->transform.pos.z + ddz) - brian->pos.z;
                }
            }
        }
    }
    n = gNpcsDrawn;
    if (n != 0) {
        for (list = (MonsterBattleData**)gNpcsDrawnList; hits == 0 && n != 0; n--) {
            other = *list++;
            ddx = nx - other->transform.pos.x;
            ddz = nz - other->transform.pos.z;
            rr = (other->transform.collisionScale * other->transform.scale) + r;
            if (ddx < rr && -rr < ddx && ddz < rr && -rr < ddz) {
                d = sqrtf((ddx * ddx) + (ddz * ddz));
                if (d < rr) {
                    f = rr / d;
                    hits = 1;
                    ddx *= f;
                    ddz *= f;
                    *dx = (other->transform.pos.x + ddx) - brian->pos.x;
                    *dz = (other->transform.pos.z + ddz) - brian->pos.z;
                }
            }
        }
    }
    n = gPropsNearCount;
    if (n != 0) {
        for (props = gPropsNear; hits == 0 && n != 0; n--) {
            prop = *props++;
            ddx = nx - prop->x;
            ddz = nz - prop->z;
            rr = r + 2.0 * prop->model_scale;
            if (ddx < rr && -rr < ddx && ddz < rr && -rr < ddz) {
                d = sqrtf((ddx * ddx) + (ddz * ddz));
                if (d < rr) {
                    f = rr / d;
                    hits = 1;
                    ddx *= f;
                    ddz *= f;
                    *dx = (prop->x + ddx) - brian->pos.x;
                    *dz = (prop->z + ddz) - brian->pos.z;
                }
            }
        }
    }
}
u16 DamageBrian(u16 power, u16 flags, MonsterBattleData* attacker);   /* hurt Brian; returns the damage */
void ApplyFloorEffect(PlayerState* state, ActorTransform* brian) {
    f32 dx;
    f32 dz;
    u16 floor;

    floor = (brian->floorFlags >> 6) & 7;
    switch (floor) {
        case 0:
            break;
        case 1:
            if (state->floorTimer == 0) {
                DamageBrian(1, 4, (MonsterBattleData*)brian);   /* Brian's whole actor: the floor hits him as his own attacker */
            }
            state->floorTimer--;
            break;
        case 2:
            dx = brian->shadowXRot * (1.0f + 1.0f);
            brian->pos.x += dx;
            dz = brian->shadowZRot * (1.0f + 1.0f);
            brian->pos.z += dz;
            brian->xspeed += dx;
            brian->zspeed += dz;
            break;
    }
}
void BrianWalkThroughDoor(PlayerState* state, ActorTransform* brian);
void StartBrianArrival(void) {
    /* Brian arrives through a door / warp: his state by the entrance kind (gPendingWarp.doorFlags). Through pointer
       locals to the block, like DrawBrian: direct gPlayerState.x accesses keep the block's address in a
       register, the target addresses each field symbolically */
    s32 kind;
    PlayerState* state;
    MonsterBattleData* brian;
    ModelTransform* door;

    state = &gPlayerState;
    brian = &state->brian;
    kind = (gPendingWarp.doorFlags >> 8) & 0xF;
    gArrivalFlags = gPendingWarp.doorFlags;
    if (gPendingWarp.doorFlags & 4) {
        state->state = 3;
        state->timer = 30;
        state->animCue = 0;
        brian->transform.stateFlags |= ACTOR_BUSY;
        SetAnimation(&brian->transform, gBrianIdleAnim, 1, gBrianIdleAnim, 1, 1);
        gModeCountdown = 0x13;
        return;
    }
    switch (kind) {
        case 1:                         /* placed at the entrance, facing gPendingWarp.yaw, 40 units in */
            state->state = 8;
            state->timer = 30;
            state->animCue = 0;
            brian->transform.yrot = gPendingWarp.yaw;
            brian->transform.zrot = 0.0f;
            brian->transform.xrot = 0.0f;
            gBrianMovement.x = 0.0f;
            gBrianMovement.y = 40.0f;
            rotateCoordinatesByAngle(brian->transform.yrot, &gBrianMovement);
            state->stepX = gBrianMovement.x;
            state->stepZ = gBrianMovement.y;
            brian->transform.stateFlags |= ACTOR_BUSY;
            PlaySound(5, 0x21, 0xFF);
            PlaySound(0xF, 0x21, 0xFF);
            gModeCountdown = 0x13;
            break;
        case 2:
        case 3:                         /* walks out of the door (BrianWalkThroughDoor) */
            BrianWalkThroughDoor(state, &brian->transform);
            if (gPendingWarp.doorFlags & 1) {  /* both arms store 50 (the bytes have the test) */
                gModeCountdown = 50;
            } else {
                gModeCountdown = 50;
            }
            brian->transform.stateFlags |= ACTOR_BUSY;
            break;
        case 4:
            door = &gMapModels[gPendingWarp.door];
            StartDoorAnimation(1, 0, gPendingWarp.doorFlags, door);
            state->state = 3;
            state->timer = 30;
            state->animCue = 0;
            brian->transform.stateFlags |= ACTOR_BUSY;
            SetAnimation(&brian->transform, gBrianIdleAnim, 1, gBrianIdleAnim, 1, 1);
            gModeCountdown = 0x19;
            break;
    }
}
void BrianWalkThroughDoor(PlayerState* state, ActorTransform* brian) {
    /* Brian walks out of a door: StartDoorAnimation starts the door's animation; door kind 3 (bits 8-11 of the entrance
       kind) = state 3 for 75 frames, anim 0x1A, sound 0x1E; otherwise state 9 for 8 frames, sliding to the door's
       offset point (gBrianMovement holds three floats here: x / z offset and a yaw offset in degrees) rotated by the
       door's angle, with the per-frame step in unk12C / unk130 and the yaw to end on in savedYRot */
    PendingWarp* warp;
    ModelTransform* door;
    f32 yaw;
    f32 frames;
    s32 cue;
    u16 kind;

    warp = &gPendingWarp;
    door = &gMapModels[warp->door];
    StartDoorAnimation(0, 0, warp->doorFlags, door);
    kind = (warp->doorFlags >> 8) & 0xF;
    if (kind == 3) {
        state->animCue = 0;
        SetAnimation(brian, 0x1A, 1, 1, 0xA, 1);
        state->state = 3;
        state->timer = 0x4B;
        brian->xrot = 0.0f;
        brian->zrot = 0.0f;
        brian->xspeed = 0.0f;
        brian->zspeed = 0.0f;
        PlaySound(0x1E, 9, 0xFF);
    } else {
        state->state = 9;
        state->timer = 8;
        SetAnimation(brian, 1, 0xA, 1, 0xA, 1);
        if (warp->doorFlags & 1) {
            cue = 0x15;
            ((Vec3f*)&gBrianMovement)->x = -133.0f;
            ((Vec3f*)&gBrianMovement)->y = -92.0f;
            ((Vec3f*)&gBrianMovement)->z = 4.0f;
        } else {
            cue = 0x14;
            ((Vec3f*)&gBrianMovement)->x = -119.0f;
            ((Vec3f*)&gBrianMovement)->y = 125.0f;
            ((Vec3f*)&gBrianMovement)->z = 162.0f;
        }
        state->animCue = cue;
        yaw = door->angle * 0.017453292519943295;
        rotateCoordinatesByAngle(-yaw, &gBrianMovement);
        frames = (u32)state->timer;
        state->stepX = (door->x - gBrianMovement.x * brian->scale - brian->pos.x) / frames;
        state->stepZ = (door->z + gBrianMovement.y * brian->scale - brian->pos.z) / frames;
        state->endYaw = yaw;
        yaw = yaw + ((Vec3f*)&gBrianMovement)->z * 0.017453292519943295;
        if (yaw > 3.141592653589793) {
            yaw -= 6.283185307179586;
        } else if (yaw < -3.141592653589793) {
            yaw += 6.283185307179586;
        }
        state->savedYRot = yaw;
        brian->xrot = 0.0f;
        brian->zrot = 0.0f;
    }
    brian->stateFlags |= ACTOR_BUSY;
}
void StartBrianPickUp(PlayerState* state, ActorTransform* brian, AnimEntry* anims) {
    SetAnimation(brian, 0x1C, 1, 0x1D, 1, 1);
    LoadBrianAnimation(brian);
    ApplyRootMotion(brian, &anims[brian->animID], brian->scale);
    state->state = 0xB;
    state->timer = 0x5A;
    brian->stateFlags |= ACTOR_BUSY;
}
void StartBrianUseItem(u8 itemID) {
    AnimEntry* anims = gBrianAnims[gPlayerState.brian.transform.modelID];
    gPlayerState.itemId = itemID;
    gPlayerState.state = 0xC;
    gPlayerState.timer = anims[23].frames;
    gPlayerState.brian.transform.stateFlags |= ACTOR_BUSY;
    SetAnimation(&gPlayerState.brian.transform, 0x17, 1, 0x16, 1, 1);
}
void LoadBrianAnimation(ActorTransform* brian) {
    static s32 sLoaded = -1;            /* the animation whose data is at 0x80200000 */
    RomRange* rom;

    if (sLoaded != brian->animID) {
        sLoaded = brian->animID;
        rom = &gBrianAnimRom[sLoaded];
        dma_write(rom->start, (void*)0x80200000, rom->end - rom->start);
    }
}

extern u16 gTurnCount;               /* enemies in gTurnOrder */
extern u16 gTurnOrder[];             /* the enemies in hit-test order */
s32 FindStaffTarget(ActorTransform* brian) {
    u16* order;
    s32 hit;
    u32 i;
    MonsterBattleData* m;
    f32 reach;
    f32 er;
    f32 dx;                  /* then the vertical gap (RULE-EXCEPTION R30: a separate local takes another register) */
    f32 dz;                  /* then the enemy height (same) */
    f32 d;
    MonsterBaseData* base;

    reach = brian->base.player->radius * brian->scale;
    hit = 0;
    order = gTurnOrder;
    i = 0;
    if (gTurnCount != 0) {
        do {
            if (gEnemies[*order].battleData.transform.modelID != -1) {
                m = &gEnemies[*order].battleData;
                dx = m->transform.pos.x - brian->pos.x;
                dz = m->transform.pos.z - brian->pos.z;
                d = sqrtf((dx * dx) + (dz * dz));
                er = m->transform.base.monster->radius * m->transform.scale;
                if (d < (reach + er) + 10.0) {
                    gBrianMovement.x = dx;
                    gBrianMovement.y = dz;
                    base = m->transform.base.monster;
                    if (base->monsterType == FLYING) {
                        dx = m->stats->flyY - brian->pos.y;
                        dz = 2.0 * base->radius;
                    } else if (base->monsterType == MAMMON) {
                        dz = 0.0f;
                        dx = 0.0f;
                    } else {
                        dz = base->height;
                        dx = m->transform.pos.y - brian->pos.y;
                    }
                    if (dx < 0.0f) {
                        dx = -dx;
                    }
                    dx -= (dz * m->transform.scale) + (brian->base.player->height * brian->scale);
                    if (dx <= 0.0f) {
                        rotateCoordinatesByAngle(brian->yrot, &gBrianMovement);
                        er = er * 1.5;
                        if (gBrianMovement.y >= 0.0f && -er < gBrianMovement.x && gBrianMovement.x < er) {
                            hit = *order + 1;
                            break;
                        }
                    }
                }
            }
            i++;
            order++;
        } while (i < gTurnCount);
    }
    return hit;
}
void StartBrianStaffSwing(PlayerState* state, ActorTransform* brian, AnimEntry* anims) {
    f32 dx;
    f32 dz;

    state->state = 0xD;
    state->timer = 0x14;
    state->flags &= ~0x400;
    brian->stateFlags |= ACTOR_BUSY;
    SetAnimation(brian, 0xA, 1, gBrianIdleAnim, 1, 1);
    dx = gEnemies[state->staffTarget - 1].battleData.transform.pos.x - brian->pos.x;
    dz = gEnemies[state->staffTarget - 1].battleData.transform.pos.z - brian->pos.z;
    brian->yrot = Atan2(dx, dz);
    SnapSpellCamera(0x23, (MonsterBattleData*)brian);
}
u32 getRandomNumber(u32 range);
s32 RollStaffHit(s32 enemy, MonsterBattleData* actor) {
    s16 chance;
    s32 agi;
    EnemyAction* e = &gEnemies[enemy];

    agi = actor->stats->agi;
    chance = (agi * 100) / (agi + ((e->battleData.stats->agi + 7) >> 3));
    return getRandomNumber(100) < chance;
}
/* damage scale (x/64) by the number of strong spirits; guess: 4 entries (count reaches 4, and the word read
   then is the 0 after the table) */
s32 gStaffSpiritScale[] = { 64, 64, 57, 60 };
u16 CalcStaffPower(PlayerState* state) {
    s32 count;
    s32 i;
    u16 damage;
    u16 total;
    u16 quarter;

    damage = state->brian.statsBuf.atk;
    quarter = state->brian.statsBuf.atk >> 2;
    total = state->brian.statsBuf.atk;
    count = 0;
    for (i = 0; i < 4; i++) {
        if (quarter < state->stats->spirits[i]) {
            total = total - state->stats->spirits[i] + quarter;
            count++;
        }
    }
    damage = (damage >> 1) + total;
    if (count != 0) {
        damage = (gStaffSpiritScale[count] * damage) >> 6;
    }
    damage = (damage * state->brian.statsBuf.atkScale) >> 4;
    if (getRandomNumber((state->flags & 0x10000) ? 4 : 0x20) == 0) {
        damage = damage + (damage >> 1);
        StartScreenFade(4, 0, gCritFade);
        state->flags |= 0x400;
    }
    if (state->brian.statsBuf.statusFlags & 1) {
        damage >>= 1;
    }
    return damage;
}
void StartCameraShake(s32 kind);
u16 DamageBrian(u16 power, u16 flags, MonsterBattleData* attacker) {
    PlayerState* state;
    f32 angle;
    MonsterBattleData* actor;
    AnimEntry* anims;

    state = &gPlayerState;
    if (state->state != 6) {
        actor = &state->brian;
        anims = gBrianAnims[state->brian.transform.modelID];
        if (power) {
            if (flags & 0x8000) {
                angle = Atan2(attacker->transform.pos.x - actor->transform.pos.x,
                              attacker->transform.pos.z - actor->transform.pos.z) - actor->transform.yrot;
                if (angle > 3.141592653589793) {
                    angle -= 6.283185307179586;
                } else if (angle < -3.141592653589793) {
                    angle += 6.283185307179586;
                }
                if (angle < 0.0f) {
                    angle = -angle;
                }
                if (angle > 2.356194490192345) {
                    power += power >> 1;
                }
            }
            angle = attacker->stats->atk;
            angle /= attacker->stats->atk + state->brian.statsBuf.def;
            power = (u32)((f32)(u32)power * angle);
            power += getRandomNumber(sqrtf((f32)(u32)power) * 0.75 + 1.0);
            if (!power) {
                power++;
            }
            if (actor->transform.base.player->currHP < power) {
                actor->transform.base.player->currHP = 0;
            } else {
                actor->transform.base.player->currHP -= power;
            }
            if (state->brian.statsBuf.statusFlags & 0x200) {
                actor->transform.base.player->currMP += power;
                if (actor->transform.base.player->maxMP < actor->transform.base.player->currMP) {
                    actor->transform.base.player->currMP = actor->transform.base.player->maxMP;
                }
                UpdateStatusVisuals(actor, 0x200, 2, 0x1E);
            }
            state->stats->HPXP++;
            state->stats->defXP++;
            SpawnDamageNumber(actor, (flags & 0x4000) ? 0x8003 : 0, power);
        }
        state->state = 5;
        state->timer = anims[8].frames;
        actor->transform.stateFlags |= ACTOR_BUSY;
        if (flags & 4) {
            actor->transform.stateFlags |= ACTOR_NO_MELEE_HIT;
        }
        SetAnimation(&actor->transform, 8, 1, gBrianIdleAnim, 1, 1);
        if (power) {
            StartCameraShake(2);
        }
        if (flags & 0x8000) {
            PlaySound(0, 0x18, 0xFF);
        }
    }
    return power;
}

void Setup_MISS_Icon(MonsterBattleData* actor);
void StartBrianDodge(void) {
    /* An attack missed Brian (Quest twin func_80006F6C): unless he is in state 6 or out of HP, state 3 for the
       length of his dodge animation (entry 7's frames at +0x56), busy and melee-proof, the MISS icon, sound 0x17 */
    PlayerState* state;
    MonsterBattleData* brian;
    AnimEntry* anims;

    state = &gPlayerState;
    brian = &state->brian;
    if (state->state != 6 && state->stats->currHP != 0) {
        anims = gBrianAnims[brian->transform.modelID];
        state->state = 3;
        state->timer = anims[7].frames;
        brian->transform.stateFlags |= ACTOR_BUSY | ACTOR_NO_MELEE_HIT;
        SetAnimation(&brian->transform, 7, 1, gBrianIdleAnim, 1, 1);
        Setup_MISS_Icon(brian);
        PlaySound(0, 0x17, 0xFF);
    }
}
s32 CanBrianAct(PlayerState* state, ActorTransform* brian) {
    s32 ret = 0;

    if (!(brian->stateFlags & ACTOR_BUSY) && !(gGameState & 0x80) && !(gBattleState & 8)) {
        if (gBattleState & 1) {
            if (!(gBattleState & 0x202) && AnySpellActive() == 0 && gTurnDelay == 0) {
                ret = 1;
            }
        } else {
            ret = 1;
        }
    }
    return ret;
}
s32 GetSpellAvailability(u32 spell, u16 arg1);
s32 GetSpellFromPath(u16 presses, u8* path);
s32 GetSpellAvailability(u32 spell, u16 arg1);
s32 GetSpellFromPath(u16 presses, u8* path);
s32 BrianSpellInput(PlayerState* state) {
    s32 dir;
    s32 sound;
    s32 ret;
    u32 prev;
    u16 presses;

    ret = 0;
    presses = state->spellPresses;
    if (presses < 3 && (gButtonsPressed & (U_CBUTTONS | D_CBUTTONS | L_CBUTTONS | R_CBUTTONS))) {
        prev = state->spellPresses;
        if (gButtonsPressed & U_CBUTTONS) {
            dir = 0;
        } else if (gButtonsPressed & D_CBUTTONS) {
            dir = 2;
        } else if (gButtonsPressed & L_CBUTTONS) {
            dir = 1;
        } else if (gButtonsPressed & R_CBUTTONS) {
            dir = 3;
        }
        state->path[state->spellPresses] = dir;
        state->spellPresses++;
        if (GetSpellAvailability(GetSpellFromPath(state->spellPresses, state->path), state->stats->currMP) & 5) {
            sound = 0x13;
            state->spellPresses--;
        } else if (!(gGameState & 4)) {
            sound = 0x17;
            gGameState |= 4;
        } else {
            sound = 0x17;
        }
        PlaySound(0, sound, 0xFF);
        if (prev != state->spellPresses) {
            gSpellBannerFade = 8;
            gSpellCrossFlags |= 1;
            if (state->spellPresses == 1) {
                gSpellCrossFlags |= 2;
            } else {
                gSpellCrossFlags &= ~2;
            }
        }
    } else if (state->spellPresses != 0 && (gButtonsPressed & (A_BUTTON | Z_TRIG))) {
        state->spellId = GetSpellFromPath(state->spellPresses, state->path);
        if (GetSpellAvailability(state->spellId, state->stats->currMP) == 0) {
            gGameState &= ~4;
            ret = 1;
        }
    } else if ((gButtonsPressed & B_BUTTON) && (gGameState & 4)) {
        if (state->spellPresses) {
            state->spellPresses--;
            gSpellBannerFade = 8;
            gSpellCrossFlags &= ~1;
        }
        if (!state->spellPresses) {
            gGameState &= ~4;
        }
        PlaySound(0, 0x13, 0xFF);
    }
    return ret;
}

void BrianStartCast(PlayerState* state, ActorTransform* brian, AnimEntry* anims) {
    BrianData1* stats;
    SpellDef* spell;
    s32 anim;
    s32 nextAnim;
    u16 frames;

    stats = brian->base.player;
    stats->currMP -= gSpellMpCosts[state->spellPresses];
    spell = gSpellDefs[(state->spellId >> 8) & 0xF] + (state->spellId & 0xFF);

    if (spell->target_type == 2) {
        anim = 0x17;
        nextAnim = 0x16;
        state->flags |= 4;
    } else {
        anim = 3;
        nextAnim = gBrianIdleAnim;
    }

    frames = anims[anim].frames;

    state->state = 0xE;
    state->subTimer = 0x12;
    state->spellPresses = 0;
    state->timer = frames;

    brian->xspeed = 0.0f;
    brian->zspeed = 0.0f;
    brian->stateFlags |= ACTOR_BUSY;

    SetAnimation(brian, anim, 1, nextAnim, 1, 1);
    SnapSpellCamera(0x2D, (MonsterBattleData*) brian);
    gSpellBannerTimer = 0x1E;
}
void CountBattleAgility(PlayerState* player) {
    f32 dx;
    f32 dz;

    if (gBattleState & 1) {
        dx = player->brian.transform.pos.x - gTurnCenterX;
        dz = player->brian.transform.pos.z - gTurnCenterZ;
        gBattleAgilityDistance += sqrtf(dx * dx + dz * dz);
        while (gBattleAgilityDistance > 35.0) {
            gBattleAgilityDistance -= 35.0;
            player->stats->agiXP++;
        }
    }
}
void StartLevelUpEffect(s32 stat);        /* level-up effect */
void UpdateBrianGrowth(PlayerState* state, ActorTransform* brian) {
    u16 need;
    s32 count;
    u8* spirits;

    count = 0;
    if (!(gBattleState & 1)) {
        if (gSubmapFlags & 1) {
            gAgilityWalk += gBrianStepDistance;
            if (gAgilityWalk > 800.0) {
                gAgilityWalk -= 800.0;
                state->stats->agiXP++;
            }
        } else {
            gAgilityWalkSafe += gBrianStepDistance;
            if (gAgilityWalkSafe > 1500.0) {
                gAgilityWalkSafe -= 1500.0;
                state->stats->agiXP++;
            }
        }
    }
    if (brian->base.player->maxHP < 500) {
        if (state->stats->HPXP >= gXPTable[state->stats->HPLevel]) {
            state->stats->HPXP -= gXPTable[state->stats->HPLevel];
            brian->base.player->maxHP++;
            brian->base.player->currHP++;
            if (state->stats->HPLevel < 54) {
                state->stats->HPLevel++;
            }
            StartLevelUpEffect(0);
            count = 1;
        }
    } else {
        state->stats->HPXP = 0;
    }
    if (brian->base.player->maxMP < 500) {
        need = gXPTable[state->stats->MPLevel];
        if (state->flags & 0x4000) {
            need *= 3;
        } else {
            need *= 4;
        }
        if (state->stats->MPXP >= need) {
            state->stats->MPXP -= need;
            brian->base.player->maxMP++;
            brian->base.player->currMP++;
            if (state->stats->MPLevel < 54) {
                state->stats->MPLevel++;
            }
            StartLevelUpEffect(2);
            count++;
        }
    } else {
        state->stats->MPXP = 0;
    }
    if (brian->base.player->agi < 255) {
        need = gXPTable[state->stats->agiLevel];
        if (state->flags & 0x8000) {
            need = (f32)need * 0.8;
        }
        if (state->stats->agiXP >= need) {
            state->stats->agiXP -= need;
            brian->base.player->agi++;
            if (state->stats->agiLevel < 54) {
                state->stats->agiLevel++;
            }
            StartLevelUpEffect(3);
            count++;
        }
    } else {
        state->stats->agiXP = 0;
    }
    if (brian->base.player->def < 255) {
        need = gXPTable[state->stats->defLevel];
        if (state->flags & 0x2000) {
            need = (f32)need * 0.8;
        }
        if (state->stats->defXP >= need) {
            state->stats->defXP -= need;
            brian->base.player->def++;
            if (state->stats->defLevel < 54) {
                state->stats->defLevel++;
            }
            StartLevelUpEffect(1);
            count++;
        }
    } else {
        state->stats->defXP = 0;
    }
    if (!(gGameState & 8)) {
        if (state->stats->spiritLevel >= 98) {
            brian->base.player->exp = 0;
        } else if (brian->base.player->exp >= gSpiritExpTable[state->stats->spiritLevel]) {
            brian->base.player->exp -= gSpiritExpTable[state->stats->spiritLevel];
            if (state->stats->spiritLevel < 98) {
                state->stats->spiritLevel++;
            }
            spirits = state->stats->spirits;
            if (spirits[0] != 50 || spirits[1] != 50 || spirits[2] != 50 || spirits[3] != 50) {
                gGameState |= 8;
                state->state = 0;
                gSpiritGainGlow = 8;
                PlaySound(0, 1, 0xFF);
            }
        }
    }
    if (count != 0) {
        PlaySound(4, 0x35, 0xFF);
    }
    if (!(gGameState & 0xA) && gDroppedItem != 0xFF) {
        BuildPickupMessage(gDroppedItem);
        OpenMessageWindow(1);
        AddItemToInventory(gDroppedItem);
        gDroppedItem = 0xFF;
        state->flags |= 2;
        gGameState |= 2;
        PlaySound(0, 0x3B, 0xFF);
    }
    if (!(gGameState & 0xA)) {
        brian->stateFlags &= ~ACTOR_BUSY;
    }
}
