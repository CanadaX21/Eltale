/*same as 1D160.c from Quest64*/
#include "common.h"


extern sMapData* gMapHeader;
extern s32 gCurrentSubmap;
extern u16 gSubmapFlags;               /* the submap's flags (bit 0: random battles and monsters here (FieldLoop, ResetEnemies; SetupNPCs only without it)) */
extern EncounterZoneList* gSubmapEncounterZones;
typedef struct {
    s16 x;
    s16 z;
} EncounterCircle; /* names from vbhayden's Quest 64 scratches */
u32 getRandomNumber(u32 range);
extern f32 D_8006F528; /* 8100.0f: encounter circle max distance squared (90) */
extern f32 D_8006F52C; /* 2500.0f: encounter circle min distance squared (50) */
extern f32 gEncounterWalkDistance;               /* distance walked toward the next encounter roll */
extern u16 gEncounterChance;               /* encounter chance out of 2000, +50 per 50 units walked */
extern s16 gBattleIntroStep;          /* the battle intro camera's step: 0 looks at gBattleIntroEnemy first, 1 the view behind Brian */
extern s16 gBattleIntroTimer;         /* frames left in the intro step */
extern s16 gBattleIntroEnemy;         /* the enemy (gEnemies index) the intro looks at */
extern u16 gTurnDelay;
extern s16 D_8008BE56;
extern u16 gTurnIndex;   /* whose turn: index into gTurnOrder */
extern u16 gBattleState;              /* battle state flags */
extern f32 gBattleCenterX;
extern f32 gBattleCenterZ;
extern f32 gBattleRadius;
extern u16 gBattleCircleRing;
extern u16 gTurnOrder[];   /* turn order: enemy indices */
void RequestMusic(s8 bgm);
void SetupBrianForBattle(void);
void Init_MISS_Icon(void);
s32 SpawnBattleRing(u16 type, f32 x, f32 y, f32 z, f32 radius);
EncounterZone* FindEncounterZone(f32 x, f32 z, EncounterZoneList* zones);
EncounterCircle* IsInEncounterCircle(f32 brian_x, f32 brian_z);
void CreateEncounter(f32 spawn_x, f32 spawn_z, f32 rotation_angle, u16* encounter_table);
extern s32 gEnemyCount;              /* enemy count (Quest D_8007C990) */
extern EnemyAction gEnemies[];    /* enemy table (Quest D_8007C998) */
extern MonsterBattleData* gEnemyGroupHeads[];
extern s32 gEnemyGroupCount;
extern Coordinates2D gBattleCameraOffset;
extern u16 gTurnCount;
void SpawnEnemy(s32 enemy_type, f32 x, f32 z, f32 angle, MonsterBattleData* data, EnemyAction* enemy);
void SortTurnOrder(void);
void rotateCoordinatesByAngle(f32 angle, Coordinates2D* coordinates);

/* encounter packs (names from vbhayden's Quest 64 scratches) */
typedef struct {
    s32 enemy_type;
    s32 base_count;
    s32 count_variance;
} EnemySpawnData;

typedef struct {
    EnemySpawnData* spawn_data;
    u16 num_enemy_types;
    u16 position_index;
} EncounterPack;

typedef struct {
    Vec3f** position_lists;
    EncounterPack* pack_list;
    EncounterCircle* circles;
    s32 circle_count;
} EncounterSet;                      /* 0x10: a submap's circles and packs */
extern EncounterSet* gSubmapEncounterSet;

typedef struct {
    s32 enemy_type;
    s32 count;
} EnemySpawnInfo;

//#pragma GLOBAL_ASM("asm/nonmatchings/game/encounter/InitEncounters.s")
void InitEncounters(void) {

    gBattleState &= 0x8000;
    gEncounterWalkDistance = 0.0f;
    gEncounterChance = 0;
    if (gSubmapFlags & 1) {
        if (gMapHeader->encounters != 0) {
            gSubmapEncounterZones = (EncounterZoneList*)gMapHeader->encounters + gCurrentSubmap;
            gSubmapEncounterSet = (EncounterSet*)gMapHeader->unk34 + gCurrentSubmap;
            return;
        }
        gSubmapEncounterZones = NULL;
        gSubmapEncounterSet = NULL;
    }
}

extern f32 gBrianStepDistance;               /* distance Brian walked this frame */
extern PlayerState gPlayerState;
f32 Atan2(f32 x, f32 z);     /* atan2 */
void RollRandomEncounter(void) {
    f32 x;
    f32 z;
    f32 dx;
    f32 dz;
    s32 index;
    EncounterZone* region;
    EncounterCircle* circle;
    f32 angle;

    if (!(gBattleState & 1) && !(gBattleState & 0x8000) && !(gPlayerState.brian.transform.unk60 & 1) && gBrianStepDistance != 0.0f) {
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
                        gBattleState = 0x205;
                        D_8008BE56 = 0;
                        gTurnDelay = 20;
                        if (gPlayerState.stats->agi < gEnemies[gTurnOrder[0]].monBaseData->agi) {
                            gTurnIndex = 0;
                            gBattleState |= 2;
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
extern f32 gBossBattleRadii[];
extern AnimEntry** gMonsterAnims;
extern u8** gEnemyNames;
extern s32 gBossLoaded;
extern EnemyAction gBoss;
extern void* gFieldMonsterAnims;
extern void* gFieldEnemyNames;
extern s32 gBattleBoss;
extern AnimEntry* gBossAnimTables[];
extern u8 gBossNames[];
void SpawnBoss(s32 arg0, f32 x, f32 z, f32 angle, EnemyAction* enemy);
void StopBossEffectScript(void);
void StartScriptedBattle(void) {
    s32 music;

    if (gBossLoaded == 7) {
        gBattleCenterX = 0.0f;
        gBattleCenterZ = 0.0f;
    } else {
        gBattleCenterX = gBoss.battleData.transform.pos.x;
        gBattleCenterZ = gBoss.battleData.transform.pos.z;
    }

    gBattleState = 0x305;
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

    if (gBattleBoss != 8) {
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
extern u16 gTurnRing;
extern s32 gDroppedItem;               /* the item dropped */
extern u16 gMonsterCardTimer;
extern s32 gCameraSetupIndex;
extern s32 gCurrentMap;
extern f32 gTurnCenterX;               /* the turn\'s movement circle centre x */
extern f32 gTurnCenterZ;
void SetStoryFlag(u16 flag);
void OnBossDefeated(s32 boss);
u8 RollItemDrop(void);              /* the item an enemy drops */
void StartGroundRing(s32 index, f32 x, f32 y, f32 z);
void ClearEnemies(void);
void StartAfterBattleCamera(s32 index);
void FindMapBGM(s32 map, s32 room, u16 fade);
void UpdateBGMDelay(s8 song, u16 fade);
void FlagTurnEnemy(void);
void LoadCameraSetup(s32 index);
extern Camera gCamera;
extern CameraTransition gCameraTransition;
extern f32 gBattleCameraYawOffset;
extern Vec3f gBattleCameraMapAdjust[];          /* per map: battle camera height / distance adjustments */
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
                gBattleState &= ~0x200;
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
                if ((gBattleState & 0x100) && gBattleBoss == 8) {
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

    if (!(gBattleState & 4)) {
        return;
    }
    if (gTurnDelay != 0) {
        gTurnDelay--;
        return;
    }
    actor = (gBattleState & 2) ? &gEnemies[gTurnOrder[gTurnIndex]].battleData : &gPlayerState.brian;
    if (actor->transform.unk60 & 1) {
        return;
    }
    done = FALSE;
    if (gEnemyCount == 0 || (gBattleState & 0x400)) {
        gBattleState |= 8;
        done = TRUE;
        if (gBattleState & 0x100) {
            SetStoryFlag(gBattleBoss - 1);
            OnBossDefeated(gBattleBoss - 1);
            gMonsterAnims = gFieldMonsterAnims;
            gEnemyNames = gFieldEnemyNames;
        }
        gDroppedItem = RollItemDrop();
    } else if (gBattleState & 0x10) {
        done = TRUE;
    } else if (!(gBattleState & 0x100) && (gPlayerState.flags & 0x20)) {
        gBattleState |= 0x10;
        done = TRUE;
    }
    if (done) {
        gBattleState &= ~1;
        gMonsterCardTimer = 0;
        StartGroundRing(gBattleCircleRing, gPlayerState.brian.transform.pos.x, gPlayerState.brian.transform.pos.y,
                        gPlayerState.brian.transform.pos.z);
        ClearEnemies();
        StartAfterBattleCamera(gCameraSetupIndex);
        if (gBattleState & 0x100) {
            gBattleState &= ~0x100;
        }
        if (gBattleState & 0x10) {
            FindMapBGM(gCurrentMap, gCurrentSubmap, 0);
        } else {
            UpdateBGMDelay(0x2B, 8);
        }
        return;
    }
    gBattleState &= ~4;
    if (gBattleState & 2) {
        FlagTurnEnemy();
    }
    gTurnCenterX = actor->transform.pos.x;
    gTurnCenterZ = actor->transform.pos.z;
    if (gBattleState & 2) {
        size = gEnemies[gTurnOrder[gTurnIndex]].movementBoundSize;
    } else {
        size = gPlayerState.moveRadius;
    }
    actor = (gBattleState & 2) ? &gEnemies[gTurnOrder[gTurnIndex]].battleData : &gPlayerState.brian;
    size *= actor->stats->moveScale;
    size += actor->transform.base.monster->radius * actor->transform.scale;
    gTurnRing = SpawnBattleRing((gBattleState & 2) ? 1 : 2, gTurnCenterX, actor->transform.pos.y, gTurnCenterZ,
                               size * 1.04);
}
void CountDownStatuses(MonsterBattleData* actor);
void EndBattleTurn(MonsterBattleData* actor) {
    u32 left;
    EnemyAction* enemy;
    u16 index;

    CountDownStatuses(actor);
    if (gBattleState & 1) {
        if (!(gBattleState & 2)) {
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
                gBattleState |= 0x400;
            }
            gTurnDelay = 20;
        } else {
            gTurnDelay = 0;
        }
        StartGroundRing(gTurnRing, actor->transform.pos.x, actor->transform.pos.y, actor->transform.pos.z);
        gBattleState ^= 2;
        gBattleState |= 4;
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
    if (gPlayerState.flags & 0x2000) {
        item = 3;
    } else {
        item = 4;
    }
    if ((gBattleState & 0x100) || (getRandomNumber(item) == 0)) {
        item = gEnemies[getRandomNumber(gTurnCount)].monBaseData->itemDrop;
    } else {
        item = 0xFF;    /* none */
    }
    return item;
}
void func_8002018C(void) {
}
