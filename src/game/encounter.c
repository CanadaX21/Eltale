/*same as 1D160.c from Quest64*/
#include "common.h"
BSS_ORDER_PAD(0)
#include "game/encounter.h"
#include "game/boss.h"
#include "game/camera.h"
#include "game/effects.h"
#include "game/enemy.h"
#include "game/hud.h"
#include "game/math.h"
#include "game/player.h"
#include "game/sound.h"
#include "game/spells.h"
#include "game/boss_anims.h"
#include "game/map.h"


#include "map_format.h"   /* the encounter records a map holds: EncounterCircle, EnemySpawnData, EncounterPack, EncounterSet */

typedef struct {
    s32 enemy_type;
    s32 count;
} EnemySpawnInfo;

//#pragma GLOBAL_ASM("asm/nonmatchings/game/encounter/InitEncounters.s")
/* ---- encounter.c's .bss (0x8008BE40-0x8008BFC0): the battle and encounter state. Defined in their storage order, the order its header declares them in: IDO 5.3 lays the unit's
   .bss out by those declarations (include/bss_order.h), and it 8-aligns every variable of 8 bytes or more. The
   D_ names marked unreferenced are space no code addresses (unknown variables). ---- */
Coordinates2D gBattleCameraOffset;
s32 D_8008BE48;                          /* unreferenced: the offset's third word (offsets of this kind -
                                            gSpellOffset, gEnemyStep, gHudVector - are OddVectors whose third
                                            word is a scale; no code reads this one) */
s16 gBattleIntroStep;                    /* the battle intro camera's step: 0 looks at gBattleIntroEnemy first, 1 the
                                            view behind Brian */
s16 gBattleIntroTimer;                   /* frames left in the intro step */
s16 gBattleIntroEnemy;                   /* the enemy (gEnemies index) the intro looks at */
u16 gBattleState;                        /* the BATTLE_ bits (game/encounter.h) */
u16 gTurnDelay;
s16 D_8008BE56;
u16 gTurnIndex;                          /* whose turn: index into gTurnOrder */
u16 gTurnCount;
f32 gBattleCenterX;
f32 gBattleCenterZ;
f32 gTurnCenterX;                        /* the turn's movement circle centre x */
f32 gTurnCenterZ;
f32 gBattleRadius;
u8 D_8008BE70[0x100];                    /* unreferenced (256 bytes; Quest 64 has the same span after its radius) */
MonsterBattleData* gEnemyGroupHeads[8];  /* 8 fill the space to gEnemyGroupCount */
s32 gEnemyGroupCount;
u16 gBattleCircleRing;
u16 gTurnRing;
EncounterZoneList* gSubmapEncounterZones;
EncounterSet* gSubmapEncounterSet;
AnimEntry** gFieldMonsterAnims;          /* the field's gMonsterAnims, kept while a boss battle uses gBossAnimTables */
u8** gFieldEnemyNames;                   /* the field's gEnemyNames, kept while a boss battle uses gBossNames */
s32 gBattleBoss;
f32 gEncounterWalkDistance;              /* distance walked toward the next encounter roll */
u16 gEncounterChance;                    /* encounter chance out of 2000, +50 per 50 units walked */

void InitEncounters(void) {

    gBattleState &= BATTLE_NO_ENCOUNTERS;
    gEncounterWalkDistance = 0.0f;
    gEncounterChance = 0;
    if (gSubmapFlags & SUBMAP_ENCOUNTERS) {
        if (gMapHeader->encounters != 0) {
            gSubmapEncounterZones = (EncounterZoneList*)gMapHeader->encounters + gCurrentSubmap;
            gSubmapEncounterSet = gMapHeader->encounterSets + gCurrentSubmap;
            return;
        }
        gSubmapEncounterZones = NULL;
        gSubmapEncounterSet = NULL;
    }
}

void RollRandomEncounter(void) {
    f32 x;
    f32 z;
    f32 dx;
    f32 dz;
    s32 index;
    EncounterZone* region;
    EncounterCircle* circle;
    f32 angle;

    if (!(gBattleState & BATTLE_ACTIVE) && !(gBattleState & BATTLE_NO_ENCOUNTERS) && !(gPlayerState.brian.transform.stateFlags & ACTOR_BUSY) && gBrianStepDistance != 0.0f) {
        gEncounterWalkDistance += gBrianStepDistance;
        if (gEncounterWalkDistance >= 50.0) {
            gEncounterChance += 50;
            if ((u32)getRandomNumber(2000) < gEncounterChance) {
                x = gPlayerState.brian.transform.pos.x;
                z = gPlayerState.brian.transform.pos.z;
                circle = IsInEncounterCircle(x, z);
                if (circle != NULL) {
                    region = FindEncounterZone(x, z, gSubmapEncounterZones);
                    if (region != NULL) {
                        gEncounterChance = 0;
                        dx = x - circle->x;
                        dz = z - circle->z;
                        angle = Atan2(dx, dz);
                        CreateEncounter(circle->x, circle->z, angle, region->encounters);
                        gBattleCenterX = circle->x;
                        gBattleCenterZ = circle->z;
                        gBattleState = BATTLE_ACTIVE | BATTLE_TURN_START | BATTLE_INTRO;
                        D_8008BE56 = 0;
                        gTurnDelay = 20;
                        if (gPlayerState.stats->agi < gEnemies[gTurnOrder[0]].monBaseData->agi) {
                            gTurnIndex = 0;
                            gBattleState |= BATTLE_ENEMY_TURN;
                        } else {
                            gTurnIndex = gTurnCount - 1;
                        }
                        SetupBrianForBattle();
                        Init_MISS_Icon();
                        gBattleRadius = 100.0f;
                        gBattleCircleRing = SpawnBattleRing(0, gBattleCenterX, gPlayerState.brian.transform.pos.y, gBattleCenterZ, gBattleRadius * 1.04);
                        gBattleIntroStep = 0;
                        gBattleIntroTimer = 1;
                        index = getRandomNumber(gEnemyCount);
                        gBattleIntroEnemy = index;
                        RequestMusic(13);
                    }
                }
            }
            gEncounterWalkDistance = 0.0f;
        }
    }
}
void StartScriptedBattle(void) {
    s32 music;

    if (gBossLoaded == 7) {   /* boss 6 (gBossLoaded is the boss number + 1): its battle is centred at 0, 0 */
        gBattleCenterX = 0.0f;
        gBattleCenterZ = 0.0f;
    } else {
        gBattleCenterX = gBoss.battleData.transform.pos.x;
        gBattleCenterZ = gBoss.battleData.transform.pos.z;
    }

    gBattleState = BATTLE_ACTIVE | BATTLE_TURN_START | BATTLE_BOSS | BATTLE_INTRO;
    D_8008BE56 = 0;
    gTurnDelay = 0;
    gFieldMonsterAnims = gMonsterAnims;
    gFieldEnemyNames = gEnemyNames;
    gBattleBoss = gBossLoaded;
    gMonsterAnims = gBossAnimTables;
    gEnemyNames = gBossNames;
    SpawnBoss(gBossLoaded - 1, gBattleCenterX, gBattleCenterZ, gBoss.battleData.transform.yrot, gEnemies);
    StopBossEffectScript();
    gBossLoaded = 0;
    gTurnCount = *(u32*)&gEnemyCount = 1;

    SortTurnOrder();
    gEnemyGroupCount = 1;
    gEnemyGroupHeads[0] = &gEnemies[0].battleData;
    gBattleIntroStep = 1;
    gBattleIntroTimer = 1;
    gBattleIntroEnemy = 0;
    gTurnIndex = gTurnCount - 1;

    if (gBattleBoss != LAST_BOSS + 1) {   /* gBattleBoss is the placement row + 1 */
        music = 0;
    } else {
        music = 0x29;
    }

    RequestMusic(music);
    SetupBrianForBattle();
    Init_MISS_Icon();
    gBattleRadius = gBossBattleRadii[gBattleBoss];
    gBattleCircleRing = SpawnBattleRing(0, gBattleCenterX, gBoss.battleData.transform.pos.y, gBattleCenterZ, gBattleRadius * 1.04);
}
void BattleStartCamera(void) {
    f32 dx;
    f32 dz;
    f32 angle;
    MonsterBattleData* enemy;

    /* RULE-EXCEPTION R19: gBattleIntroStep, gBattleIntroTimer and gBattleIntroEnemy are declared s16 for
       StartScriptedBattle (declared u16, its constant 1 gets a register of its own), but this function
       reads them unsigned (lhu, u16 wrap), so its reads carry (u16) casts. */
    switch ((u16)gBattleIntroStep) {
        case 0:
            enemy = &gEnemies[(u16)gBattleIntroEnemy].battleData;
            dx = enemy->transform.pos.x - gPlayerState.brian.transform.pos.x;
            dz = enemy->transform.pos.z - gPlayerState.brian.transform.pos.z;
            angle = Atan2(dx, dz);
            if (getRandomNumber(100) > 50) {
                angle += 3.141592653589793;
                if (angle < -3.141592653589793) {
                    angle += 6.283185307179586;
                } else if (angle > 3.141592653589793) {
                    angle -= 6.283185307179586;
                }
            }
            angle += 0.39269908169872414;
            if (angle < -3.141592653589793) {
                angle += 6.283185307179586;
            } else if (angle > 3.141592653589793) {
                angle -= 6.283185307179586;
            }
            gBattleCameraOffset.x = 0.0f;
            if (enemy->transform.base.monster->height < enemy->transform.base.monster->radius) {
                gBattleCameraOffset.y = 2.0 * (enemy->transform.base.monster->radius * enemy->transform.scale) + 15.0;
            } else {
                gBattleCameraOffset.y = 2.0 * (enemy->transform.base.monster->height * enemy->transform.scale) + 15.0;
            }
            gCamera.ideal_planar_distance = gBattleCameraOffset.y * 1.5;
            rotateCoordinatesByAngle(angle, &gBattleCameraOffset);
            gCamera.cam_x = enemy->transform.pos.x + gBattleCameraOffset.x;
            gCamera.cam_z = enemy->transform.pos.z + gBattleCameraOffset.y;
            gCamera.cam_y = enemy->transform.pos.y + 15.0;
            if (gCamera.cam_y < gPlayerState.brian.transform.pos.y) {
                gCamera.cam_y = gPlayerState.brian.transform.pos.y;
            }
            gCamera.cam_speed_x = gCamera.cam_speed_y = gCamera.cam_speed_z = 0.0f;
            gCamera.mode = 0;
            gCamera.orbit_speed = 0.0f;
            gCamera.local_camera_height = 15.0f;
            gCamera.target_transform = &enemy->transform;
            gCamera.pull = 0.01f;
            gCameraTransition.orbitYaw = gCamera.yaw;
            gBattleIntroStep = (u16)gBattleIntroStep + 1;
            gBattleIntroTimer = 60;
            break;
        case 1:
            if ((u16)(gBattleIntroTimer = (u16)gBattleIntroTimer - 1) == 0) {
                gBattleState &= ~BATTLE_INTRO;
                LoadCameraSetup(gCameraSetupIndex);
                dx = gBattleCenterX - gPlayerState.brian.transform.pos.x;
                dz = gBattleCenterZ - gPlayerState.brian.transform.pos.z;
                angle = Atan2(dx, dz) + 3.141592653589793;
                if (angle > 3.141592653589793) {
                    angle -= 6.283185307179586;
                } else if (angle < -3.141592653589793) {
                    angle += 6.283185307179586;
                }
                gBattleCameraOffset.x = 0.0f;
                gBattleCameraOffset.y = 50.0f;
                rotateCoordinatesByAngle(angle, &gBattleCameraOffset);
                gCamera.cam_x = gPlayerState.brian.transform.pos.x + gBattleCameraOffset.x;
                gCamera.cam_y = gPlayerState.brian.transform.pos.y + 30.0;
                gCamera.mode = 4;
                gCamera.flags |= 0x10;
                gCamera.cam_z = gPlayerState.brian.transform.pos.z + gBattleCameraOffset.y;
                gCamera.cam_speed_x = gCamera.cam_speed_y = gCamera.cam_speed_z = 0.0f;
                gCamera.orbit_speed = 0.0f;
                if ((gBattleState & BATTLE_BOSS) && gBattleBoss == LAST_BOSS + 1) {
                    gCamera.local_camera_height = 50.0f;
                    gCamera.local_focus_height = 28.0f;
                    gCamera.ideal_planar_distance = 180.0f;
                } else {
                    gCamera.local_camera_height += gBattleCameraMapAdjust[gCurrentMap].x;
                    gCamera.ideal_planar_distance += gBattleCameraMapAdjust[gCurrentMap].y;
                }
                gCamera.target_transform = &gPlayerState.brian.transform;
                gCamera.pull = 0.02f;
                gBattleCameraYawOffset = 0.0f;
            }
            break;
    }
}
void StepBattleRound(void) {
    s32 done;
    f32 size;
    MonsterBattleData* actor;

    if (!(gBattleState & BATTLE_TURN_START)) {
        return;
    }
    if (gTurnDelay != 0) {
        gTurnDelay--;
        return;
    }
    actor = (gBattleState & BATTLE_ENEMY_TURN) ? &gEnemies[gTurnOrder[gTurnIndex]].battleData : &gPlayerState.brian;
    if (actor->transform.stateFlags & ACTOR_BUSY) {
        return;
    }
    done = FALSE;
    if (gEnemyCount == 0 || (gBattleState & BATTLE_ENEMIES_DOWN)) {
        gBattleState |= BATTLE_WON;
        done = TRUE;
        if (gBattleState & BATTLE_BOSS) {
            SetStoryFlag(gBattleBoss - 1);
            OnBossDefeated(gBattleBoss - 1);
            gMonsterAnims = gFieldMonsterAnims;
            gEnemyNames = gFieldEnemyNames;
        }
        gDroppedItem = RollItemDrop();
    } else if (gBattleState & BATTLE_ESCAPED) {
        done = TRUE;
    } else if (!(gBattleState & BATTLE_BOSS) && (gPlayerState.flags & PLAYER_OUTSIDE_CIRCLE)) {
        gBattleState |= BATTLE_ESCAPED;
        done = TRUE;
    }
    if (done) {
        gBattleState &= ~BATTLE_ACTIVE;
        gMonsterCardTimer = 0;
        StartGroundRing(gBattleCircleRing, gPlayerState.brian.transform.pos.x, gPlayerState.brian.transform.pos.y,
                        gPlayerState.brian.transform.pos.z);
        ClearEnemies();
        StartAfterBattleCamera(gCameraSetupIndex);
        if (gBattleState & BATTLE_BOSS) {
            gBattleState &= ~BATTLE_BOSS;
        }
        if (gBattleState & BATTLE_ESCAPED) {
            FindMapBGM(gCurrentMap, gCurrentSubmap, 0);
        } else {
            UpdateBGMDelay(0x2B, 8);
        }
        return;
    }
    gBattleState &= ~BATTLE_TURN_START;
    if (gBattleState & BATTLE_ENEMY_TURN) {
        FlagTurnEnemy();
    }
    gTurnCenterX = actor->transform.pos.x;
    gTurnCenterZ = actor->transform.pos.z;
    if (gBattleState & BATTLE_ENEMY_TURN) {
        size = gEnemies[gTurnOrder[gTurnIndex]].movementBoundSize;
    } else {
        size = gPlayerState.moveRadius;
    }
    actor = (gBattleState & BATTLE_ENEMY_TURN) ? &gEnemies[gTurnOrder[gTurnIndex]].battleData : &gPlayerState.brian;
    size *= actor->stats->moveScale;
    size += actor->transform.base.monster->radius * actor->transform.scale;
    gTurnRing = SpawnBattleRing((gBattleState & BATTLE_ENEMY_TURN) ? 1 : 2, gTurnCenterX, actor->transform.pos.y, gTurnCenterZ,
                               size * 1.04);
}
void EndBattleTurn(MonsterBattleData* actor) {
    u32 left;
    EnemyAction* enemy;
    u16 index;

    CountDownStatuses(actor);
    if (gBattleState & BATTLE_ACTIVE) {
        if (!(gBattleState & BATTLE_ENEMY_TURN)) {
            left = gTurnCount;
            while (left != 0) {
                if (++gTurnIndex >= gTurnCount) {
                    gTurnIndex = 0;
                }
                index = gTurnOrder[gTurnIndex];
                enemy = &gEnemies[index];
                if (enemy->currHP != 0 && enemy->battleData.transform.modelID != -1) {
                    break;
                }
                left--;
            }
            if (left == 0) {
                gBattleState |= BATTLE_ENEMIES_DOWN;
            }
            gTurnDelay = 20;
        } else {
            gTurnDelay = 0;
        }
        StartGroundRing(gTurnRing, actor->transform.pos.x, actor->transform.pos.y, actor->transform.pos.z);
        gBattleState ^= BATTLE_ENEMY_TURN;
        gBattleState |= BATTLE_TURN_START;
    }
}
EncounterZone* FindEncounterZone(f32 x, f32 z, EncounterZoneList* zones) {
    s32 n;
    EncounterZone* zone;

    zone = NULL;
    if (zones != NULL) {
        n = zones->count;
        zone = zones->zones;
        
        while (n != 0) {
            if (!(zone->x <= x) || !(zone->z <= z) || !((x - zone->x) <= zone->width) || !((z - zone->z) <= zone->depth)) {
                n--;
                zone++;
            } else {
                break;
            }
        }
        if (n == 0) {
            zone = NULL;
        }
    }
    return zone;
}
EncounterCircle* IsInEncounterCircle(f32 brian_x, f32 brian_z) {
    f32 dx;
    f32 dz;
    f32 dist_sqr;
    s32 circles_in_range;
    s32 circle_count;
    EncounterCircle* circle_ptr;
    EncounterCircle* found_encounters[6];

    circles_in_range = 0;

    if (gSubmapEncounterSet != NULL) {
        circle_count = gSubmapEncounterSet->circle_count;
        circle_ptr = gSubmapEncounterSet->circles;

        while (circles_in_range < 6 && circle_count != 0) {
            dx = circle_ptr->x - brian_x;
            dz = circle_ptr->z - brian_z;
            dist_sqr = (dx * dx + dz * dz);

            if (2500.0f <= dist_sqr && dist_sqr <= 8100.0f) {
                found_encounters[circles_in_range++] = circle_ptr;
            }

            circle_ptr++;
            circle_count--;
        }

        if (circles_in_range == 0) {
            circle_ptr = NULL;
        } else {
            circle_ptr = found_encounters[getRandomNumber(circles_in_range)];
        }
    } else {
        circle_ptr = NULL;
    }

    return circle_ptr;
}
void CreateEncounter(f32 spawn_x, f32 spawn_z, f32 rotation_angle, u16* encounter_table) {
    EnemyAction* current_enemy;
    EnemySpawnInfo* spawn_ptr;
    EncounterPack* selected_pack;
    EnemySpawnData* spawn_data;
    Vec3f* positions;
    s32 enemies_to_spawn;
    s32 num_packs;
    s32 selected_pack_id;
    s32 enemy_type_count;
    EnemySpawnInfo spawn_info[7];

    num_packs = *encounter_table++;
    selected_pack_id = getRandomNumber(num_packs);
    selected_pack = &gSubmapEncounterSet->pack_list[encounter_table[selected_pack_id]];
    positions = &(*gSubmapEncounterSet->position_lists)[selected_pack->position_index];

    enemy_type_count = selected_pack->num_enemy_types;
    spawn_data = selected_pack->spawn_data;

    current_enemy = gEnemies;
    spawn_ptr = spawn_info;
    gEnemyGroupCount = 0;

    for (; enemy_type_count != 0; enemy_type_count--, spawn_data++) {
        enemies_to_spawn = spawn_data->base_count;

        if (spawn_data->count_variance != 0) {
            enemies_to_spawn += getRandomNumber(spawn_data->count_variance + 1);
        }

        spawn_ptr->enemy_type = spawn_data->enemy_type;
        spawn_ptr->count = enemies_to_spawn;
        spawn_ptr++;

        gEnemyCount += enemies_to_spawn;

        gEnemyGroupHeads[gEnemyGroupCount] = &current_enemy->battleData;
        gEnemyGroupCount++;

        for (; enemies_to_spawn != 0; enemies_to_spawn--, current_enemy++, positions++) {
            gBattleCameraOffset.x = positions->x;
            gBattleCameraOffset.y = positions->y;

            rotateCoordinatesByAngle(rotation_angle, &gBattleCameraOffset);

            SpawnEnemy(spawn_data->enemy_type, spawn_x + gBattleCameraOffset.x, spawn_z + gBattleCameraOffset.y,
                          rotation_angle, &current_enemy->battleData, current_enemy);
        }
    }

    SortTurnOrder();
    gTurnCount = (s16)gEnemyCount;
}
void func_800200F0(void) {
}

u8 RollItemDrop(void) {
    u8 item;

    /* RULE-EXCEPTION R30: item first holds the drop chance (1 in 3 with earth at 50, else 1 in 4); a separate chance
       local (any type, either declaration order, a ternary, an early return, item preset to 0xFF) keeps the result
       in v1 where the target has a0. */
    if (gPlayerState.flags & PLAYER_AT_50(ELEMENT_EARTH)) {
        item = 3;
    } else {
        item = 4;
    }
    if ((gBattleState & BATTLE_BOSS) || (getRandomNumber(item) == 0)) {
        item = gEnemies[getRandomNumber(gTurnCount)].monBaseData->itemDrop;
    } else {
        item = 0xFF;    /* none */
    }
    return item;
}
void func_8002018C(void) {
}
