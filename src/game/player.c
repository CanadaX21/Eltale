#include "common.h"
BSS_ORDER_PAD(0)
#include "game/player.h"
#include "game/audio.h"
#include "game/boss.h"
#include "game/camera.h"
#include "game/collision.h"
#include "game/dma.h"
#include "game/dust.h"
#include "game/effects.h"
#include "game/encounter.h"
#include "game/enemy.h"
#include "game/hud.h"
#include "game/map.h"
#include "game/map_objects.h"
#include "game/math.h"
#include "game/message.h"
#include "game/model.h"
#include "game/screen_fade.h"
#include "game/shadow.h"
#include "game/signs.h"
#include "game/sound.h"
#include "game/spells.h"
#include "game/spirits.h"
#include "game/save_format.h"
#include "game/controller.h"
#include "game/main.h"
#include "spell_bank.h"
#include "segment_symbols.h"


/* ---- player.c's .bss (0x8007A8F0-0x8007AB70), in address order. IDO 5.3 lays a unit's .bss
   out by its variables' block numbers, so a declaration added or removed before them, in a header too, can change
   their order; BSS_ORDER_PAD keeps the original order and `python3 tools/bss_order.py fix` recomputes it
   (docs/DECLARATIONS.md). Names inside gBrianStats / gPlayerState that other code reads are members. The D_ names marked unreferenced are space
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
SignSlot* gSignTarget;                   /* the sign Brian can read (FindSignTarget) */
BrianData1 gBrianStats;                  /* Brian's stats */
PlayerState gPlayerState;                 /* Brian's state block */
s32 D_8007AB44;                         /* unreferenced */
Vec3f gBrianFloorPos;                       /* Brian's point on the floor (his shadow, the glows) */
s32 D_8007AB54;                         /* unreferenced (three words: IDO would 8-align an array this size) */
s32 D_8007AB58;                         /* unreferenced */
s32 D_8007AB5C;                         /* unreferenced */
s32 gActorLightAnim;                      /* the brightened light animator for Brian, NPCs, enemies, effects */

void InitBrian(void) {
    PlayerState* state;

    dma_write(brian_model_ROM_START, brian_model_VRAM, brian_model_ROM_END - brian_model_ROM_START);
    gBrianStats = gNewGameStats;
    state = &gPlayerState;
    state->brian.transform.scale = gBrianStats.modelScale;
    state->brian.transform.collisionScale = gBrianStats.radius;
    state->brian.transform.modelID = gBrianStats.modelID;
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
    gArrivalFlags = WARP_ARRIVAL;
    gSavedMap = 0xD;
    gSavedSubmap = 0x11;
    gSavedEntrance = 0;
    gBattleAgilityDistance = 0.0f;
    D_8007A8F4 = D_8007A8F8 = 0.0f;
    gAgilityWalk = gAgilityWalkSafe = 0.0f;
}
void SpawnBrian(void) {
    s32 sound;
    PlayerState* state;
    ActorTransform* brian;
    AnimEntry* anims;
    ModelTransform* door;

    state = &gPlayerState;
    brian = &state->brian.transform;
    gGameState &= ~GAME_STATE_HIDE_BRIAN;
    anims = gBrianAnims[brian->modelID];
    state->state = BRIAN_STATE_STAND;
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
    if (gArrivalFlags & DOOR_ARRIVE_STANDING) {
        state->state = BRIAN_STATE_WAIT;
        state->timer = 15;
        brian->stateFlags = ACTOR_BUSY;
    } else {
        switch ((gArrivalFlags >> 8) & 7) {
            case 1:
            case 4:
                state->state = BRIAN_STATE_WALK_IN;
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
                state->state = BRIAN_STATE_THROUGH_DOOR;
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
                state->state = BRIAN_STATE_WALK_IN;
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
        state->flags |= PLAYER_MESSAGE_HOLD;
    }
    if (gGameState & GAME_STATE_RESPAWN) {
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
        state->flags |= PLAYER_AT_50(ELEMENT_FIRE);
    }
    if (state->stats->spirits[1] == 50) {
        state->flags |= PLAYER_AT_50(ELEMENT_EARTH);
    }
    if (state->stats->spirits[2] == 50) {
        state->flags |= PLAYER_AT_50(ELEMENT_WATER);
    }
    if (state->stats->spirits[3] == 50) {
        state->flags |= PLAYER_AT_50(ELEMENT_WIND);
    }
    if ((state->flags & PLAYER_AT_50_BITS) == PLAYER_AT_50_BITS) {
        state->flags |= PLAYER_ALL_AT_50;
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
void UpdateBrian(void) {
    PlayerState* state;
    ActorTransform* brian;
    AnimEntry* anims;
    u8* spirits;

    state = &gPlayerState;
    brian = &state->brian.transform;
    anims = gBrianAnims[brian->modelID];
    if (state->stats->currHP * 3 < state->stats->maxHP) {
        gBrianIdleAnim = BRIAN_ANIM_STAND_LOW_HP;
    } else {
        gBrianIdleAnim = (gBattleState & BATTLE_ACTIVE) ? BRIAN_ANIM_BATTLE_STAND : BRIAN_ANIM_STAND;
    }
    gBrianMoveX = gBrianMoveZ = 0.0f;
    if (!(gGameState & (GAME_STATE_ITEM_MENU | GAME_STATE_MESSAGE | GAME_STATE_SPELL_INPUT | GAME_STATE_MAP_CHANGE))) {
        if (gBattleState & BATTLE_ACTIVE) {
            if (!(gBattleState & BATTLE_ENEMY_TURN) && !(brian->stateFlags & ACTOR_BUSY)) {
                state->staffTarget = FindStaffTarget(brian);
            }
        } else if (!(brian->stateFlags & ACTOR_BUSY)) {
            gTalkTarget = FindTalkTarget(state, brian);
            if (gTalkTarget == 0) {
                gSpiritTarget = CheckSpiritPickup(brian);
                if (gSpiritTarget == NULL) {
                    gObjectTarget = FindMapObjectInFront(brian);
                    if (gObjectTarget == NULL) {
                        gSignTarget = FindSignTarget(brian);
                        if (gSignTarget == NULL) {
                            if ((gNearBoss = IsNearBoss()) != 0) {
                                StartBossScene(gNearBoss - 1);
                                gTextBoxDelay = 30;
                                gGameState |= GAME_STATE_MESSAGE;
                                state->flags |= PLAYER_BOSS_SCENE;
                            }
                        }
                    }
                }
            }
        }
    }
    if (!(gGameState & (GAME_STATE_ITEM_MENU | GAME_STATE_MESSAGE | GAME_STATE_SPELL_INPUT | GAME_STATE_SPIRIT_WINDOW))) {
        if (gButtonsPressed & (A_BUTTON | Z_TRIG)) {
            if ((gBattleState & BATTLE_ACTIVE) && !(brian->stateFlags & ACTOR_BUSY)) {
                if (!(gBattleState & (BATTLE_ENEMY_TURN | BATTLE_TURN_START)) && AnySpellActive() == 0) {
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
                    state->state = BRIAN_STATE_TALK;
                    brian->xrot = brian->zrot = brian->xspeed = brian->zspeed = 0.0f;
                    SetAnimation(brian, gBrianIdleAnim, 1, gBrianIdleAnim, 1, 1);
                    StartNPCTalk(gTalkTarget);
                    OpenMessageWindow(gMarkerScreenY < 0x46 ? 0 : 1);
                    gGameState |= GAME_STATE_MESSAGE;
                } else if (gSpiritTarget != NULL) {
                    spirits = state->stats->spirits;
                    if (spirits[0] != 50 || spirits[1] != 50 || spirits[2] != 50 || spirits[3] != 50) {
                        gGameState |= GAME_STATE_SPIRIT_WINDOW;
                        state->state = BRIAN_STATE_STAND;
                        SetSpiritFound(gSpiritTarget->flag);
                        gSpiritTarget->pickedUp |= 1;
                        gSpiritGainGlow = 8;
                        PlaySound(0, 1, 0xFF);
                    }
                } else if (InventoryHasRoom() != 0 && gObjectTarget != NULL) {
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
                    state->state = BRIAN_STATE_SIGN;
                    brian->stateFlags |= ACTOR_BUSY;
                    brian->xrot = brian->zrot = brian->xspeed = brian->zspeed = 0.0f;
                    if (gSignTarget->sign->flags & SIGN_NO_POSE) {
                        state->timer = 0xF;
                        state->flags |= PLAYER_SIGN_NO_POSE;
                        SetAnimation(brian, BRIAN_ANIM_NO_TARGET, 1, gBrianIdleAnim, 1, 1);
                    } else {
                        state->timer = 0xF;
                        SetAnimation(brian, BRIAN_ANIM_SIGN_START, 1, BRIAN_ANIM_SIGN_LOOP, 1, 1);
                    }
                    gSignTarget = NULL;
                } else {
                    state->state = BRIAN_STATE_WAIT;
                    state->timer = 0xF;
                    brian->xrot = brian->zrot = brian->xspeed = brian->zspeed = 0.0f;
                    brian->stateFlags |= ACTOR_BUSY;
                    SetAnimation(brian, BRIAN_ANIM_NO_TARGET, 1, gBrianIdleAnim, 1, 1);
                }
            }
        } else if (CanBrianAct(state, brian)) {
            if (!(gGameState & GAME_STATE_SPELL_INPUT) && (gButtonsPressed & R_TRIG)) {
                gGameState ^= GAME_STATE_ITEM_MENU;
                StartScreenFade(6, FADE_HOLD, gFadeRampBlack16);
                PlaySound(0, 1, 0xFF);
                brian->stateFlags |= ACTOR_BUSY;
            }
            if (!(gGameState & GAME_STATE_ITEM_MENU) && !(state->brian.stats->statusFlags & STATUS_SILENCE) && BrianSpellInput(state)) {
                BrianStartCast(state, brian, anims);
            }
        }
    } else if (gGameState & GAME_STATE_SPELL_INPUT) {
        if (BrianSpellInput(state)) {
            BrianStartCast(state, brian, anims);
        }
    } else if ((gGameState & GAME_STATE_ITEM_MENU) && (gButtonsPressed & (B_BUTTON | R_TRIG))) {
        gGameState ^= GAME_STATE_ITEM_MENU;
        StartScreenFade(6, FADE_BRIGHTEN, gFadeRampBlack16[5]);
        PlaySound(0, 2, 0xFF);
        brian->stateFlags &= ~ACTOR_BUSY;
    }
    if (gBattleState & BATTLE_WON) {
        gBattleState &= ~BATTLE_WON;
        brian->base.player->exp += gBattleExp;
        brian->base.player->stone += gBattleStone;
        if (gDroppedItem != 0xFF && (CountItem(gDroppedItem) >= 3 || CountInventory() >= 0x8D)) {
            gDroppedItem = 0xFF;
        }
        ClearStatusEffects(&state->brian);
        state->brian.statsBuf.statusFlags = 0;
        state->state = BRIAN_STATE_VICTORY;
        state->timer = anims[BRIAN_ANIM_VICTORY].frames;
        brian->stateFlags |= ACTOR_BUSY;
        brian->xrot = brian->zrot = brian->xspeed = brian->zspeed = 0.0f;
        SetAnimation(brian, BRIAN_ANIM_VICTORY, 1, gBrianIdleAnim, 1, 1);
    } else if (gBattleState & BATTLE_ESCAPED) {
        ClearStatusEffects(&state->brian);
        state->brian.statsBuf.statusFlags = 0;
    }
    gBrianStateHandlers[state->state](brian, state, anims);
    gBrianStepDistance = sqrtf(gBrianMoveX * gBrianMoveX + gBrianMoveZ * gBrianMoveZ);
    if (!(gBattleState & BATTLE_ACTIVE) && brian->base.player->currMP < brian->base.player->maxMP) {
        gBrianStepPhase += gBrianStepDistance;
        if (gBrianStepPhase > 60.0) {
            gBrianStepPhase -= 60.0;
            brian->base.player->currMP++;
        }
    }
}
void StepLightAnim();   /* RULE-EXCEPTION R25: unprototyped; this caller passes a second argument the definition does not take */
void DrawBrian(Mtx* mtx) {
    s32 flags;
    PlayerState* state;
    ActorTransform* brian;
    AnimEntry* anims;

    gSPDisplayList(gDisplayListHead++, D_10000A0);
    StepLightAnim(gActorLightAnim, &gDisplayListHead);
    state = &gPlayerState;
    brian = &state->brian.transform;
    if (state->state == BRIAN_STATE_THROUGH_DOOR || state->state == BRIAN_STATE_PICK_UP) {
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
    /* Battle start: Brian's battle values into his state block: the movement radius agi * 0.2857 + 17; his
       battle attack, the spirit total, at atkScale 0x10 (sixteenths: 1.0); agility and defence copied; no statuses;
       comboWindow, the last frames of a staff swing in which A or Z starts the combo: the total less each element's
       excess over total / 4, then / 10 + 1, at most 16. Then BRIAN_STATE_WAIT for the length of the battle-start
       animation */
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
    state->state = BRIAN_STATE_WAIT;
    state->timer = anims[BRIAN_ANIM_BATTLE_START].frames;
    state->staffTarget = 0;
    state->savedYRot = state->brian.transform.yrot;
    state->flags &= ~PLAYER_OUTSIDE_CIRCLE;
    state->brian.transform.stateFlags |= ACTOR_BUSY;
    if (gBrianIdleAnim != BRIAN_ANIM_STAND_LOW_HP) {
        gBrianIdleAnim = BRIAN_ANIM_BATTLE_STAND;
    }
    SetAnimation(&state->brian.transform, BRIAN_ANIM_BATTLE_START, 1, gBrianIdleAnim, 1, 1);
}
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
    } else if (gGameState & (GAME_STATE_ITEM_MENU | GAME_STATE_MESSAGE | GAME_STATE_SPELL_INPUT | GAME_STATE_SPIRIT_WINDOW | GAME_STATE_UNSET_1000 | GAME_STATE_UNSET_2000)) {
        x = y = 0.0f;
    } else if (state->brian.statsBuf.statusFlags & STATUS_IMMOBILE) {
        x = y = 0.0f;
    }
    if ((gBattleState & BATTLE_ACTIVE) && (gBattleState & (BATTLE_ENEMY_TURN | BATTLE_INTRO))) {
        x = y = 0.0f;
    } else if (gTurnDelay != 0) {
        x = y = 0.0f;
    }
    if (state->flags & PLAYER_BOSS_SCENE) {
        if (gTextBoxDelay != 0) {
            if (--gTextBoxDelay == 0) {
                OpenMessageWindow(0);
            }
        } else if (gMessageWindowsOpen == 0) {
            state->flags &= ~PLAYER_BOSS_SCENE;
            gGameState &= ~GAME_STATE_MESSAGE;
            StartScriptedBattle();
        }
    } else if (state->flags & PLAYER_PICKUP_MESSAGE) {
        if (gMessageWindowsOpen == 0) {
            state->flags &= ~PLAYER_PICKUP_MESSAGE;
            gGameState &= ~GAME_STATE_MESSAGE;
        }
    } else if (state->flags & PLAYER_MESSAGE_HOLD) {
        if (gMessageWindowsOpen == 0) {
            state->flags &= ~PLAYER_MESSAGE_HOLD;
            gGameState &= ~GAME_STATE_MESSAGE;
        }
    }
    switch (state->state) {
        case BRIAN_STATE_RUN:
            BrianState_Run(x, y, brian, state);
            break;
        case BRIAN_STATE_WALK:
            BrianState_Walk(x, y, brian, state);
            break;
        default:
            BrianState_Stand(x, y, brian, state);
            break;
    }
    BrianBattleMovement(0xA01, brian, state);
    if (state->state == BRIAN_STATE_RUN) {
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
        state->state = BRIAN_STATE_STAND;
        brian->stateFlags &= ~(ACTOR_BUSY | ACTOR_NO_MELEE_HIT);
    }
}
void BrianState_Slide(ActorTransform* brian, PlayerState* state, AnimEntry* anims) {
    BrianBattleMovement(0xA01, brian, state);
    brian->xspeed *= 0.68;
    brian->zspeed *= 0.68;
    state->timer--;
    if (!state->timer) {
        state->state = BRIAN_STATE_STAND;
        brian->zspeed = 0.0f;
        brian->xspeed = 0.0f;
        brian->stateFlags &= ~ACTOR_BUSY;
    }
}
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
            state->state = BRIAN_STATE_DEFEATED;
            state->timer = 0xE6;
            SetAnimation(brian, BRIAN_ANIM_DEFEATED, 1, gBrianIdleAnim, 1, 1);
            ClearStatusEffects((MonsterBattleData*)brian);
            RequestMusic(0x1E);
            gGameState |= GAME_STATE_MAP_CHANGE | GAME_STATE_RESPAWN;
            gModeCountdown = state->timer;
            gTransitionStyle = TRANSITION_BLACK_SLOW;
            gFadeOutAt = 0x5A;
            state->subTimer = 0xC8;
            gPendingWarp.map = gSavedMap;
            gPendingWarp.submap = gSavedSubmap;
            gPendingWarp.entrance = gSavedEntrance;
            gBrianSpawnX = 0.0f;
            gBrianSpawnZ = 0.0f;
            gBrianSpawnYaw = 0.0f;
            gArrivalFlags = WARP_ARRIVAL;
            gPlayerState.animCue = 0;
            gHudSlideState = 1;
            gItemBarSlide = 0xC;
            return;
        }
        state->state = BRIAN_STATE_STAND;
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
void BrianState_TalkEnd(ActorTransform* brian, PlayerState* state, AnimEntry* anims) {
    brian->zspeed = 0.0f;
    brian->xspeed = 0.0f;
    if (gMessageWindowsOpen == 0) {
        ResumeNpcAfterTalk(gTalkTarget);
        state->state = BRIAN_STATE_STAND;
        brian->stateFlags &= ~ACTOR_BUSY;
        gGameState &= ~GAME_STATE_MESSAGE;
        if (gGameState & GAME_STATE_INN_CHOICE) {
            gGameState &= ~GAME_STATE_INN_CHOICE;
            if (gNpcs[gTalkTarget].record->talkType == NPC_TALK_INN && gMessageChoice == 0) {
                gGameState |= GAME_STATE_SAVE_MENU | GAME_STATE_MAP_CHANGE;      /* the rest, then the save mode */
                gModeCountdown = 0x23;
                gFadeOutAt = 0x1E;
                gSavedMap = gCurrentMap;
                gSavedSubmap = gCurrentSubmap;
                state->state = BRIAN_STATE_WAIT;
                state->timer = 0x41;
                brian->stateFlags |= ACTOR_BUSY;
                StartMusicFadeOut(0x3C);
            }
        } else if (state->flags & PLAYER_ITEM_GIVEN) {
            state->flags &= ~PLAYER_ITEM_GIVEN;
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
        state->state = BRIAN_STATE_STAND;
        brian->zspeed = 0.0f;
        brian->xspeed = 0.0f;
        brian->stateFlags &= ~ACTOR_BUSY;
    }
}
void BrianState_StaffSwing(MonsterBattleData* brian, PlayerState* state, AnimEntry* anims) {
    u16 dmg;
    s32 flags;

    brian->transform.zspeed = 0.0f;
    brian->transform.xspeed = 0.0f;
    if (--state->timer == 0) {
        if (RollStaffHit(state->staffTarget - 1, brian) != 0) {
            dmg = CalcStaffPower(state);
            if (state->state == BRIAN_STATE_STAFF_COMBO) {
                dmg = dmg * 0.5;
            }
            flags = HIT_PHYSICAL | HIT_NO_MELEE;
            if (state->flags & PLAYER_STAFF_CRITICAL) {
                flags = HIT_PHYSICAL | HIT_CRITICAL | HIT_NO_MELEE;
                state->flags &= ~PLAYER_STAFF_CRITICAL;
            }
            dmg = DamageEnemy(state->staffTarget - 1, dmg, (state->spellId >> 8) & 0xF, flags, brian);
            if (dmg != 0) {
                brian->transform.base.player->currMP++;
                if (brian->transform.base.player->currMP > brian->transform.base.player->maxMP) {
                    brian->transform.base.player->currMP = brian->transform.base.player->maxMP;
                }
                if (state->brian.statsBuf.statusFlags & STATUS_DRAIN) {
                    dmg = ((dmg + 3) >> 2) + 1;
                    brian->transform.base.player->currHP += dmg;
                    if (brian->transform.base.player->currHP > brian->transform.base.player->maxHP) {
                        brian->transform.base.player->currHP = brian->transform.base.player->maxHP;
                    }
                    SpawnDamageNumber(brian, 1, dmg);
                    UpdateStatusVisuals(brian, STATUS_DRAIN, 1, 0x1E);
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
            state->flags &= ~PLAYER_COMBO_INPUT;
        }
        if (state->state == BRIAN_STATE_STAFF && (state->flags & PLAYER_COMBO_INPUT)) {
            state->state = BRIAN_STATE_STAFF_COMBO;
            state->timer = 0x12;
            state->flags &= ~PLAYER_COMBO_INPUT;
            brian->transform.stateFlags |= ACTOR_BUSY;
            SetAnimation(&brian->transform, BRIAN_ANIM_STAFF_COMBO, 1, gBrianIdleAnim, 1, 1);
            gCamera.timer = 0x23;
            return;
        }
        state->state = BRIAN_STATE_WAIT;
        if (state->state == BRIAN_STATE_STAFF) {   /* BUG: the state was just set to 3, so the first arm never runs */
            state->timer = anims[BRIAN_ANIM_STAFF_SWING].frames - 20;
        } else {
            state->timer = anims[BRIAN_ANIM_STAFF_COMBO].frames - 22;
        }
        state->staffTarget = 0;
        state->flags &= ~PLAYER_COMBO_INPUT;
        CountBattleAgility(state);
        state->staffTarget = 0;
        EndBattleTurn(brian);
        return;
    }
    if (state->state == BRIAN_STATE_STAFF && state->comboWindow >= state->timer && (gButtonsPressed & (A_BUTTON | Z_TRIG))) {
        state->flags |= PLAYER_COMBO_INPUT;
    }
}
void BrianState_Cast(MonsterBattleData* brian, PlayerState* state, AnimEntry* anims) {
    brian->transform.xspeed = brian->transform.zspeed = 0.0f;
    if (state->subTimer != 0) {
        if (--state->subTimer == 0) {
            SpawnSpell(brian, state->spellId, 0, (state->flags & PLAYER_ALL_AT_50) ? SPELLSLOT_BOOSTED : 0);
        }
    } else if (--state->timer == 0) {
        if (AnySpellActive() != 0) {
            state->timer = 8;
        } else {
            brian->transform.stateFlags &= ~ACTOR_BUSY;
            if (state->flags & PLAYER_CAST_ON_SELF) {
                state->state = BRIAN_STATE_WAIT;
                state->timer = anims[BRIAN_ANIM_USE_END].frames;
                state->flags &= ~PLAYER_CAST_ON_SELF;
                SetAnimation(&brian->transform, BRIAN_ANIM_USE_END, 1, gBrianIdleAnim, 1, 1);
            } else {
                state->state = BRIAN_STATE_STAND;
            }
            CountBattleAgility(state);
            state->staffTarget = 0;
            EndBattleTurn(brian);
        }
    }
}
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
        state->state = BRIAN_STATE_THROUGH_DOOR;
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
        state->state = BRIAN_STATE_STAND;
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
            gGameState |= GAME_STATE_MESSAGE;
            PlaySound(0, 0x3B, 0xFF);
        }
        state->timer--;
        if (!state->timer) {
            brian->yrot = state->endYaw;
            SetAnimation(brian, gBrianIdleAnim, 1, gBrianIdleAnim, 1, 1);
        }
    } else if (gMessageWindowsOpen == 0) {
        state->state = BRIAN_STATE_STAND;
        brian->stateFlags &= ~ACTOR_BUSY;
        gGameState &= ~GAME_STATE_MESSAGE;
        gTalkTarget = 0;
    }
}
void BrianState_UseItem(MonsterBattleData* brian, PlayerState* state, AnimEntry* anims) {
    state->timer--;
    if (!state->timer) {
        UseItem(state->itemId, brian);
        if (gItemData[state->itemId].type == ITEM_TYPE_WINGS) {
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
        state->state = BRIAN_STATE_WAIT;
        state->timer = 0xF;
        SetAnimation(&brian->transform, BRIAN_ANIM_USE_END, 1, gBrianIdleAnim, 1, 1);
        state->staffTarget = 0;
        EndBattleTurn(brian);
    }
}
void BrianState_Victory(ActorTransform* brian, PlayerState* state, AnimEntry* anims) {
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
        state->state = BRIAN_STATE_STAND;
        brian->stateFlags &= ~ACTOR_NO_MELEE_HIT;
        if (gFieldMode != FIELD_MODE_LAST_BOSS) {
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
            gGameState |= GAME_STATE_MESSAGE;
        }
    } else if (gMessageWindowsOpen == 0) {
        state->state = BRIAN_STATE_WAIT;
        if (state->flags & PLAYER_SIGN_NO_POSE) {
            state->timer = 2;
        } else {
            state->timer = 8;
            SetAnimation(brian, BRIAN_ANIM_SIGN_END, 1, gBrianIdleAnim, 1, 1);
        }
        state->flags &= ~PLAYER_SIGN_NO_POSE;
        gGameState &= ~GAME_STATE_MESSAGE;
        gTalkTarget = 0;
    }
}
/* Brian's state handlers by PlayerState.state, called (brian, state, anims) by UpdateBrian. Declared without a
   prototype: two handlers (BrianState_Wait, BrianState_Slide) still have placeholder parameter types. */
void (*gBrianStateHandlers[])() = {
    BrianFieldMovement, BrianFieldMovement, BrianFieldMovement, BrianState_Wait, BrianState_Slide, BrianState_KnockedDown,
    BrianState_Defeated, BrianState_TalkEnd, BrianState_WalkIn, BrianState_EnterDoor, BrianState_ThroughDoor, BrianState_PickUpItem,
    BrianState_UseItem, BrianState_StaffSwing, BrianState_Cast, BrianState_Victory, BrianState_StaffSwing, BrianState_FaceAndMessage,
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
            state->state = BRIAN_STATE_RUN;
            state->timer = 0x1E;
            gRunFootstepTimer = 0;
            return;
        }

        state->state = BRIAN_STATE_WALK;
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
        anim = BRIAN_ANIM_WALK;
        next = BRIAN_ANIM_WALK;
        loop = 0;
        rate = want * 14.0f + 3.0f;
        if (want > 1.5 || s > 1.5) {
            state->state = BRIAN_STATE_RUN;
            state->timer = 30;
            gRunFootstepTimer = 0;
        }
    } else {
        brian->xspeed *= 0.82;
        brian->zspeed *= 0.82;
        if (prev > 1.0) {
            anim = BRIAN_ANIM_STOP;
            next = gBrianIdleAnim;
            frame = 3;
            state->state = BRIAN_STATE_SLIDE;
            state->timer = 8;
            brian->xrot = 0.0f;
            brian->zrot = 0.0f;
            brian->xspeed *= 0.9;
            brian->zspeed *= 0.9;
            rate = 1;
            loop = 1;
            StartDustEmitter(brian);
        } else {
            anim = BRIAN_ANIM_WALK;
            next = BRIAN_ANIM_WALK;
            loop = 0;
            rate = prev * 14.0f + 3.0f;
            if (prev < 0.1) {
                state->state = BRIAN_STATE_STAND;
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
        anim = BRIAN_ANIM_RUN;
        next = BRIAN_ANIM_RUN;
        frame = 0;
        rate = want * 14.0f + 3.0f;
        loop = 0;
        if (s < 1.0 && want < 1.0) {
            state->state = BRIAN_STATE_WALK;
        }
    } else {
        anim = BRIAN_ANIM_STOP;
        next = gBrianIdleAnim;
        frame = 3;
        state->state = BRIAN_STATE_SLIDE;
        state->timer = 8;
        brian->xrot = 0.0f;
        brian->zrot = 0.0f;
        brian->xspeed *= 0.9;
        brian->zspeed *= 0.9;
        rate = 1;
        loop = 1;
        StartDustEmitter(brian);
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
void BrianBattleMovement(u16 mask, ActorTransform* brian, PlayerState* state) {
    f32 dx;
    f32 dz;
    f32 dist;
    f32 radius;
    BrianData1* data;

    if ((gBattleState & BATTLE_ACTIVE) && !(gBattleState & (BATTLE_ENEMY_TURN | BATTLE_TURN_START))) {
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
        if (gBattleState & BATTLE_BOSS) {
            if (gBattleRadius < dist) {
                brian->xspeed = ((gBattleRadius * dx) / dist - brian->pos.x) + gBattleCenterX;
                brian->zspeed = ((gBattleRadius * dz) / dist - brian->pos.z) + gBattleCenterZ;
            }
        } else if (gBattleRadius < dist) {
            state->flags |= PLAYER_OUTSIDE_CIRCLE;
        } else {
            state->flags &= ~PLAYER_OUTSIDE_CIRCLE;
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
    if (!(gGameState & (GAME_STATE_ITEM_MENU | GAME_STATE_SPELL_INPUT | GAME_STATE_SPIRIT_WINDOW | GAME_STATE_MAP_CHANGE | GAME_STATE_SAVE_MENU | GAME_STATE_UNSET_4000)) && !(state->flags & PLAYER_MESSAGE_HOLD)) {
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
void StartBrianDeparture(void) {
    /* Brian leaves through the door or warp he walked into: his state by the door's kind (gPendingWarp.doorFlags). Through pointer
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
    if (gPendingWarp.doorFlags & DOOR_LEAVE_STANDING) {
        state->state = BRIAN_STATE_WAIT;
        state->timer = 30;
        state->animCue = 0;
        brian->transform.stateFlags |= ACTOR_BUSY;
        SetAnimation(&brian->transform, gBrianIdleAnim, 1, gBrianIdleAnim, 1, 1);
        gModeCountdown = 0x13;
        return;
    }
    switch (kind) {
        case 1:                         /* walks 40 units on, facing the door's angle (gPendingWarp.yaw) */
            state->state = BRIAN_STATE_WALK_IN;
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
            state->state = BRIAN_STATE_WAIT;
            state->timer = 30;
            state->animCue = 0;
            brian->transform.stateFlags |= ACTOR_BUSY;
            SetAnimation(&brian->transform, gBrianIdleAnim, 1, gBrianIdleAnim, 1, 1);
            gModeCountdown = 0x19;
            break;
    }
}
void BrianWalkThroughDoor(PlayerState* state, ActorTransform* brian) {
    /* Brian walks out of a door: StartDoorAnimation starts the door's animation; door kind 3 (bits 8-11 of the door's
       flags) = BRIAN_STATE_WAIT for 75 frames, BRIAN_ANIM_DOOR_KIND3, sound 0x1E; otherwise BRIAN_STATE_ENTER_DOOR
       for 8 frames, sliding to the door's
       offset point (gBrianMovement holds three floats here: x / z offset and a yaw offset in degrees) rotated by the
       door's angle, with the per-frame step in stepX / stepZ and the yaw to end on in savedYRot */
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
        SetAnimation(brian, BRIAN_ANIM_DOOR_KIND3, 1, BRIAN_ANIM_WALK, 0xA, 1);
        state->state = BRIAN_STATE_WAIT;
        state->timer = 0x4B;
        brian->xrot = 0.0f;
        brian->zrot = 0.0f;
        brian->xspeed = 0.0f;
        brian->zspeed = 0.0f;
        PlaySound(0x1E, 9, 0xFF);
    } else {
        state->state = BRIAN_STATE_ENTER_DOOR;
        state->timer = 8;
        SetAnimation(brian, BRIAN_ANIM_WALK, 0xA, BRIAN_ANIM_WALK, 0xA, 1);
        if (warp->doorFlags & 1) {
            cue = BRIAN_ANIM_DOOR_FLAG1;
            ((Vec3f*)&gBrianMovement)->x = -133.0f;
            ((Vec3f*)&gBrianMovement)->y = -92.0f;
            ((Vec3f*)&gBrianMovement)->z = 4.0f;
        } else {
            cue = BRIAN_ANIM_DOOR;
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
    SetAnimation(brian, BRIAN_ANIM_PICK_UP, 1, BRIAN_ANIM_PICK_UP_END, 1, 1);
    LoadBrianAnimation(brian);
    ApplyRootMotion(brian, &anims[brian->animID], brian->scale);
    state->state = BRIAN_STATE_PICK_UP;
    state->timer = 0x5A;
    brian->stateFlags |= ACTOR_BUSY;
}
void StartBrianUseItem(u8 itemID) {
    AnimEntry* anims = gBrianAnims[gPlayerState.brian.transform.modelID];
    gPlayerState.itemId = itemID;
    gPlayerState.state = BRIAN_STATE_USE_ITEM;
    gPlayerState.timer = anims[BRIAN_ANIM_USE_START].frames;
    gPlayerState.brian.transform.stateFlags |= ACTOR_BUSY;
    SetAnimation(&gPlayerState.brian.transform, BRIAN_ANIM_USE_START, 1, BRIAN_ANIM_USE_LOOP, 1, 1);
}
void LoadBrianAnimation(ActorTransform* brian) {
    static s32 sLoaded = -1;            /* the animation whose file is at BRIAN_ANIM_BUFFER */
    RomRange* rom;

    if (sLoaded != brian->animID) {
        sLoaded = brian->animID;
        rom = &gBrianAnimRom[sLoaded];
        dma_write(rom->start, BRIAN_ANIM_BUFFER, rom->end - rom->start);
    }
}

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

    state->state = BRIAN_STATE_STAFF;
    state->timer = 0x14;
    state->flags &= ~PLAYER_STAFF_CRITICAL;
    brian->stateFlags |= ACTOR_BUSY;
    SetAnimation(brian, BRIAN_ANIM_STAFF_SWING, 1, gBrianIdleAnim, 1, 1);
    dx = gEnemies[state->staffTarget - 1].battleData.transform.pos.x - brian->pos.x;
    dz = gEnemies[state->staffTarget - 1].battleData.transform.pos.z - brian->pos.z;
    brian->yrot = Atan2(dx, dz);
    SnapSpellCamera(0x23, (MonsterBattleData*)brian);
}
s32 RollStaffHit(s32 enemy, MonsterBattleData* actor) {
    s16 chance;
    s32 agi;
    EnemyAction* e = &gEnemies[enemy];

    agi = actor->stats->agi;
    chance = (agi * 100) / (agi + ((e->battleData.stats->agi + 7) >> 3));
    return getRandomNumber(100) < chance;
}
/* CalcStaffPower's damage scale (x/64) by how many spirits are above a quarter of their total: 0 to 3. Four
   cannot be (four spirits each above a quarter of their total would exceed it), so the four entries are the table */
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
    if (getRandomNumber((state->flags & PLAYER_ALL_AT_50) ? 4 : 0x20) == 0) {
        damage = damage + (damage >> 1);
        StartScreenFade(4, 0, gCritFade);
        state->flags |= PLAYER_STAFF_CRITICAL;
    }
    if (state->brian.statsBuf.statusFlags & STATUS_SHRUNK) {
        damage >>= 1;
    }
    return damage;
}
u16 DamageBrian(u16 power, u16 flags, MonsterBattleData* attacker) {
    PlayerState* state;
    f32 angle;
    MonsterBattleData* actor;
    AnimEntry* anims;

    state = &gPlayerState;
    if (state->state != BRIAN_STATE_DEFEATED) {
        actor = &state->brian;
        anims = gBrianAnims[state->brian.transform.modelID];
        if (power) {
            if (flags & HIT_PHYSICAL) {
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
            if (state->brian.statsBuf.statusFlags & STATUS_MP_ABSORB) {
                actor->transform.base.player->currMP += power;
                if (actor->transform.base.player->maxMP < actor->transform.base.player->currMP) {
                    actor->transform.base.player->currMP = actor->transform.base.player->maxMP;
                }
                UpdateStatusVisuals(actor, STATUS_MP_ABSORB, 2, 0x1E);
            }
            state->stats->HPXP++;
            state->stats->defXP++;
            SpawnDamageNumber(actor, (flags & HIT_CRITICAL) ? 0x8003 : 0, power);
        }
        state->state = BRIAN_STATE_HIT;
        state->timer = anims[BRIAN_ANIM_HIT].frames;
        actor->transform.stateFlags |= ACTOR_BUSY;
        if (flags & HIT_NO_MELEE) {
            actor->transform.stateFlags |= ACTOR_NO_MELEE_HIT;
        }
        SetAnimation(&actor->transform, BRIAN_ANIM_HIT, 1, gBrianIdleAnim, 1, 1);
        if (power) {
            StartCameraShake(2);
        }
        if (flags & HIT_PHYSICAL) {
            PlaySound(0, 0x18, 0xFF);
        }
    }
    return power;
}

void StartBrianDodge(void) {
    /* An attack missed Brian (Quest twin func_80006F6C): unless he is defeated or out of HP, BRIAN_STATE_WAIT for
       the length of his dodge animation (BRIAN_ANIM_DODGE), busy and melee-proof, the MISS icon, sound 0x17. A
       melee miss also counts toward his agility (EnemyMeleeAttack's agiXP); a spell that misses him does not */
    PlayerState* state;
    MonsterBattleData* brian;
    AnimEntry* anims;

    state = &gPlayerState;
    brian = &state->brian;
    if (state->state != BRIAN_STATE_DEFEATED && state->stats->currHP != 0) {
        anims = gBrianAnims[brian->transform.modelID];
        state->state = BRIAN_STATE_WAIT;
        state->timer = anims[BRIAN_ANIM_DODGE].frames;
        brian->transform.stateFlags |= ACTOR_BUSY | ACTOR_NO_MELEE_HIT;
        SetAnimation(&brian->transform, BRIAN_ANIM_DODGE, 1, gBrianIdleAnim, 1, 1);
        Setup_MISS_Icon(brian);
        PlaySound(0, 0x17, 0xFF);
    }
}
s32 CanBrianAct(PlayerState* state, ActorTransform* brian) {
    s32 ret = 0;

    if (!(brian->stateFlags & ACTOR_BUSY) && !(gGameState & GAME_STATE_MAP_CHANGE) && !(gBattleState & BATTLE_WON)) {
        if (gBattleState & BATTLE_ACTIVE) {
            if (!(gBattleState & (BATTLE_ENEMY_TURN | BATTLE_INTRO)) && AnySpellActive() == 0 && gTurnDelay == 0) {
                ret = 1;
            }
        } else {
            ret = 1;
        }
    }
    return ret;
}
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
            dir = SPELL_DIR_UP;
        } else if (gButtonsPressed & D_CBUTTONS) {
            dir = SPELL_DIR_DOWN;
        } else if (gButtonsPressed & L_CBUTTONS) {
            dir = SPELL_DIR_LEFT;
        } else if (gButtonsPressed & R_CBUTTONS) {
            dir = SPELL_DIR_RIGHT;
        }
        state->path[state->spellPresses] = dir;
        state->spellPresses++;
        if (GetSpellAvailability(GetSpellFromPath(state->spellPresses, state->path), state->stats->currMP) & (SPELL_LOW_MP | SPELL_LOW_SPIRITS)) {
            sound = 0x13;
            state->spellPresses--;
        } else if (!(gGameState & GAME_STATE_SPELL_INPUT)) {
            sound = 0x17;
            gGameState |= GAME_STATE_SPELL_INPUT;
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
            gGameState &= ~GAME_STATE_SPELL_INPUT;
            ret = 1;
        }
    } else if ((gButtonsPressed & B_BUTTON) && (gGameState & GAME_STATE_SPELL_INPUT)) {
        if (state->spellPresses) {
            state->spellPresses--;
            gSpellBannerFade = 8;
            gSpellCrossFlags &= ~1;
        }
        if (!state->spellPresses) {
            gGameState &= ~GAME_STATE_SPELL_INPUT;
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
    stats->currMP -= gSpellMpCosts[state->spellPresses - 1];
    spell = gSpellDefs[(state->spellId >> 8) & 0xF] + (state->spellId & 0xFF);

    if (spell->target_type == SPELL_TARGET_SELF) {
        anim = BRIAN_ANIM_USE_START;
        nextAnim = BRIAN_ANIM_USE_LOOP;
        state->flags |= PLAYER_CAST_ON_SELF;
    } else {
        anim = BRIAN_ANIM_CAST;
        nextAnim = gBrianIdleAnim;
    }

    frames = anims[anim].frames;

    state->state = BRIAN_STATE_CAST;
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

    if (gBattleState & BATTLE_ACTIVE) {
        dx = player->brian.transform.pos.x - gTurnCenterX;
        dz = player->brian.transform.pos.z - gTurnCenterZ;
        gBattleAgilityDistance += sqrtf(dx * dx + dz * dz);
        while (gBattleAgilityDistance > 35.0) {
            gBattleAgilityDistance -= 35.0;
            player->stats->agiXP++;
        }
    }
}
void UpdateBrianGrowth(PlayerState* state, ActorTransform* brian) {
    u16 need;
    s32 count;
    u8* spirits;

    count = 0;
    if (!(gBattleState & BATTLE_ACTIVE)) {
        if (gSubmapFlags & SUBMAP_ENCOUNTERS) {
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
        if (state->flags & PLAYER_AT_50(ELEMENT_WATER)) {
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
        if (state->flags & PLAYER_AT_50(ELEMENT_WIND)) {
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
        if (state->flags & PLAYER_AT_50(ELEMENT_EARTH)) {
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
    if (!(gGameState & GAME_STATE_SPIRIT_WINDOW)) {
        if (state->stats->spiritLevel >= 98) {
            brian->base.player->exp = 0;
        } else if (brian->base.player->exp >= gSpiritExpTable[state->stats->spiritLevel]) {
            brian->base.player->exp -= gSpiritExpTable[state->stats->spiritLevel];
            if (state->stats->spiritLevel < 98) {
                state->stats->spiritLevel++;
            }
            spirits = state->stats->spirits;
            if (spirits[0] != 50 || spirits[1] != 50 || spirits[2] != 50 || spirits[3] != 50) {
                gGameState |= GAME_STATE_SPIRIT_WINDOW;
                state->state = BRIAN_STATE_STAND;
                gSpiritGainGlow = 8;
                PlaySound(0, 1, 0xFF);
            }
        }
    }
    if (count != 0) {
        PlaySound(4, 0x35, 0xFF);
    }
    if (!(gGameState & (GAME_STATE_MESSAGE | GAME_STATE_SPIRIT_WINDOW)) && gDroppedItem != 0xFF) {
        BuildPickupMessage(gDroppedItem);
        OpenMessageWindow(1);
        AddItemToInventory(gDroppedItem);
        gDroppedItem = 0xFF;
        state->flags |= PLAYER_PICKUP_MESSAGE;
        gGameState |= GAME_STATE_MESSAGE;
        PlaySound(0, 0x3B, 0xFF);
    }
    if (!(gGameState & (GAME_STATE_MESSAGE | GAME_STATE_SPIRIT_WINDOW))) {
        brian->stateFlags &= ~ACTOR_BUSY;
    }
}
