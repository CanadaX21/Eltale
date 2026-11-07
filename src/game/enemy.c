#include "common.h"
extern BossFxEntry* gBossEffectScripts[];    /* the boss effect scripts, by a boss action's low tag byte */
void GetRootMotionOffset(MonsterBattleData* actor, Coordinates2D* offset);
void MoveEnemy(s32, f32, f32, MonsterBattleData*, EnemyAction*);
void StartNPCText(Npc* npc, s32 textOffset);   /* a map object starts its text */
void ClearStatusEffects(MonsterBattleData *actor);
void SpawnEnemyLandingBurst(MonsterBattleData* actor);   /* the enemy's landing burst */
extern u8 gMonsterFileBuffer[]; // area monster file buffer (Quest gMonsterFileBuffer)
extern void (*gEnemyActionHandlers[])(MonsterBattleData*, EnemyAction*, AnimEntry*); // enemy action handlers (Quest D_8004C290)
extern u16 gBattleState; // battle state flags (0x100 = boss battle)
void UpdateBossEffectScript(void); // boss spell animation update (Quest func_8000B300)
extern s32 gBattleExp;
extern s32 gBattleStone;
extern Submap* gSubmapHeader;
extern sMapData* gMapHeader;        /* the current map's header */
extern u16 gSubmapFlags;   /* the submap's flags (bit 0: random battles and monsters here (FieldLoop, ResetEnemies; SetupNPCs only without it)) */
extern MonsterSet gMonsterSets[];
void dma_write(u8* devAddr, void* vaddr, u32 nbytes);
void Setup_MISS_Icon(MonsterBattleData* actor);
void PlaySound(u8 delay, s8 id, u8 level);
extern u16 gTurnIndex;
u32 getRandomNumber(u32 range);
/* enemy AI (names from vbhayden's Quest 64 scratches; the per-enemy state is EnemyAction, the options EnemyAIOption) */
extern u16 gMonsterCardTimer;
extern PlayerState gPlayerState;    /* Brian (Quest D_8007BAB8) */
extern u8 dialogue_bank_ROM_START[];
float sqrtf(float value);
EnemyAIOption* PickEnemyAIOption(EnemyAction* enemy, MonsterBattleData* actor);
void CommitEnemyAIOption(EnemyAction* enemy, MonsterBattleData* actor, EnemyAIOption* option);
void TurnActorToward(MonsterBattleData* actor, f32 x, f32 z);
void EndBattleTurn(MonsterBattleData* actor);
void rotateCoordinatesByAngle(f32 angle, Coordinates2D* coordinates);
void SetAnimation(ActorTransform *brian, s32 nextAnim, s32 nextFramerate, s32 afterAnim, s32 afterFramerate, u16 flags);
void SnapSpellCamera(s32 duration, MonsterBattleData* caster);
f32 Atan2(f32 dx, f32 dz);
void StartBossEffectScript(s32 id, EnemyAction* enemy);

void MoveActorWithCollision(f32 dx, f32 dz, ActorTransform* actor);   /* move an actor with collision */
extern u16 gFieldMode;
extern u8 gEndingNpcText[];
void LoadNPCArea(s32 area);
void SpawnNPCs(NpcRecord* rec, s32 count);
void StartEndingText(s32 index, u8* text);
/* other units' variables, declared here so enemy.c's .bss block below starts past the file's 32nd declaration */
extern Gfx* gDisplayListHead;
extern RomRange gNpcBlobs[];       /* each area's monster file in ROM */
extern u8 npc_held_objects_ROM_START[];               /* the battle overlay's code in ROM */
extern u8 npc_held_objects_ROM_END[];
extern u8 shannon_actor_ROM_START[];               /* loaded to 0x802A0000 */
extern u8 shannon_actor_ROM_END[];
extern u8 func_8020C0D0[];          /* the overlay area */
extern u8 gShannonActorData[];
extern s32 gCurrentSubmap;              /* current submap */
extern s32 gPropsNearCount;               /* props near Brian */
extern ModelTransform* gPropsNear[];
extern s32 gGameState;
extern s32 gModeCountdown;
extern s32 gMessageChoice;
extern MessageLines gMessageLines;
extern u8 gSaveChoiceYes[];
extern u8 gSaveChoiceNo[];
/* ---- enemy.c's .bss (0x8007AB70-0x8007C0B0), in address order. IDO 5.3 lays out the
   variables first declared among a file's first 32 declarations (headers included) after all later ones; this
   block starts past them (the externs above were moved up for that), so it keeps its order whatever the headers
   hold. Declaring any of these variables above it moves them. Names inside these blocks that other code reads by
   name (D_8007AC70, D_8007B930) are link names in undefined_syms.jp.txt. The D_ names
   marked unreferenced are space no code addresses. ---- */
RomRange* gAreaMonsterFile;                   /* the current area's monster file */
u16 gWalkingObjectCount;                         /* map objects walking, at most 3 */
Coordinates2D gNpcStep;               /* a walking object's step */
s32 D_8007AB80[2];                      /* unreferenced */
CollisionPollingResult gActorMovePoll;
s32 gNpcCount;                          /* NPCs loaded (gNpcs) */
s32 gNpcsDrawn;                         /* NPCs drawn this frame (within 300 units of Brian) */
s32 D_8007ABAC;                         /* unreferenced */
ActorTransform* gNpcsDrawnList[16];             /* their transforms */
Npc gNpcs[16];                /* the map objects, 0x84 each */
u8 gTextBuffer[0x4B0];                   /* a 0x4B0-byte buffer: enemy.c, ending_text.c and boss.c fill it */
u8 gSpeakerName[0x20];                    /* a 0x20-byte buffer */
s32 gEnemyCount;                         /* enemies in battle */
EnemyAction gEnemies[6];              /* the enemies in battle, 0x130 each (Quest D_8007C998) */
EnemyAction* gCardEnemy;                /* the enemy on the info card: the last one hit (EnemyHitReaction) */
s32 gEnemiesDrawn;                         /* enemies drawn this frame */
MonsterBattleData* gDrawnEnemies[6];       /* the enemies drawn this frame */
AnimEntry** gMonsterAnims;
u8** gEnemyNames;
u16 gTurnOrder[6];                      /* the enemies in turn order */
MonsterSet* gCurrentMonsterSet;
BossEffectScript gBossEffectScript;                   /* a boss's effect script player */
Coordinates2D gEnemyStep;
f32 D_8007C078;                         /* gEnemyStep's scale where SpawnEnemyLandingBurst uses it as an OddVector */
s32 D_8007C07C;                         /* unreferenced */
CollisionPollingResult gEnemyFloorPoll;      /* an enemy's spawn floor poll */
s32 D_8007C09C;                         /* unreferenced */
s32 D_8007C0A0[4];                      /* unreferenced */
s32 OpenMessageWindow(s32 type);
void SetupNPCs(void) {
    sMapList* list;

    gNpcCount = 0;
    if (!(gSubmapFlags & 1)) {
        LoadNPCArea(gSubmapHeader->area);
        list = gMapHeader->npcList;
        if (list != NULL) {
            SpawnNPCs(list->list, list->count);
        }
        gWalkingObjectCount = 0;
        if (gFieldMode == 1) {
            StartEndingText(0, gEndingNpcText);
            OpenMessageWindow(1);
        }
    }
}
void UpdateNPCs(void) {
    s32 n;
    Npc* obj;
    f32 dx;
    f32 dz;
    ActorTransform* xform;

    n = gNpcCount;
    obj = gNpcs;
    for (; n != 0; n--, obj++) {
        xform = &obj->transform;
        switch (obj->state) {
            case 0:
                if (obj->record->kind == 2) {
                    obj->timer--;
                    if (obj->timer == 0) {
                        if (gWalkingObjectCount < 3) {
                            obj->state = 1;
                            obj->timer = (getRandomNumber(2) + 1) * (obj->record->anims + 1)->frames;
                            SetAnimation(xform, 1, 1, 1, 1, 1);
                            dx = obj->spawnX - xform->pos.x;
                            dz = obj->spawnZ - xform->pos.z;
                            if (sqrtf(dx * dx + dz * dz) > 50.0) {
                                xform->yrot = Atan2(dx, dz);
                            } else {
                                xform->yrot = (f32)getRandomNumber(16) * 0.39269908169872414 - 1.5707963267948966;
                            }
                            gWalkingObjectCount++;
                        } else {
                            obj->timer = (getRandomNumber(2) + 1) * obj->record->anims->frames;
                        }
                    }
                }
                break;
            case 1:
                gNpcStep.x = 0.0f;
                gNpcStep.y = 0.4f;
                rotateCoordinatesByAngle(xform->yrot, &gNpcStep);
                MoveActorWithCollision(gNpcStep.x, gNpcStep.y, xform);
                obj->timer--;
                if (obj->timer == 0) {
                    obj->state = 0;
                    obj->timer = obj->record->anims->frames;
                    SetAnimation(xform, 0, 1, 0, 1, 1);
                    gWalkingObjectCount--;
                }
                break;
            case 2:
                if (obj->timer != 0) {
                    obj->timer--;
                    xform->yrot += obj->turnLeft / 15.0f;
                    if (obj->timer == 0) {
                        SetAnimation(xform, obj->record->talkAnim, 1, obj->record->talkAnim, 1, 1);
                    }
                }
                break;
            case 3:
                obj->timer--;
                xform->yrot -= obj->turnLeft / 15.0f;
                if (obj->timer == 0) {
                    obj->state = 0;
                    obj->timer = obj->record->anims->frames;
                    xform->yrot = obj->spawnYaw;
                    SetAnimation(xform, 0, 1, 0, 1, 1);
                }
                break;
        }
    }
}
void DrawModel(s32 flags, ActorTransform* actor, AnimEntry* anim, s32 mtxList);
void AdvanceAnimation(s32 framerate, ActorTransform* model, AnimEntry* anim);
void AddShadow(f32 x, f32 y, f32 z, f32 xrot, f32 yrot, f32 zrot, f32 size);   /* the shadow */
void DrawNPCs(Mtx* mtx) {
    Npc* obj;
    ActorTransform** list;
    ActorTransform* t;
    AnimEntry* anims;
    s32 i;
    s32 filter;
    f32 dx;
    f32 dz;
    f32 dist;

    gNpcsDrawn = 0;
    obj = gNpcs;
    list = gNpcsDrawnList;
    for (i = gNpcCount; i != 0; i--, obj++) {
        t = &obj->transform;
        dx = obj->transform.pos.x - gPlayerState.brian.transform.pos.x;
        dz = obj->transform.pos.z - gPlayerState.brian.transform.pos.z;
        dist = (dx * dx) + (dz * dz);
        if (dist < 90000.0f) {
            filter = (dist < 20000.0f) ? G_TF_BILERP : G_TF_POINT;
            gDPSetTextureFilter(gDisplayListHead++, filter);
            anims = obj->record->anims;
            DrawModel(0, t, &anims[t->animID], mtx);
            AdvanceAnimation(t->animFramerate, t, &anims[t->animID]);
            AddShadow(t->pos.x, t->pos.y, t->pos.z, t->shadowXRot, t->shadowYRot, t->shadowZRot, 5.0f);
            *list++ = t;
            gNpcsDrawn++;
        }
    }
}
void LoadNPCArea(s32 area) {
    u8* start;
    u8* end;

    gAreaMonsterFile = &gNpcBlobs[area];
    dma_write(npc_held_objects_ROM_START, func_8020C0D0, npc_held_objects_ROM_END - npc_held_objects_ROM_START);
    dma_write(shannon_actor_ROM_START, gShannonActorData, shannon_actor_ROM_END - shannon_actor_ROM_START);
    start = gAreaMonsterFile->start;
    end = gAreaMonsterFile->end;
    dma_write(start, gMonsterFileBuffer, end - start);
}
void PollWorldCollision(f32 sample_x, f32 sample_y, f32 sample_z, u16 arg3_flags, CollisionPollingResult* result);
void SpawnNPCs(NpcRecord* rec, s32 count) {
    Npc* obj = gNpcs;
    CollisionPollingResult floor;

    for (; count != 0; count--, rec++) {
        if (gCurrentSubmap == rec->submap) {
            obj->state = 0;
            obj->timer = 1;
            obj->record = rec;
            obj->transform.pos.x = rec->x;
            obj->transform.pos.z = rec->z;
            obj->transform.yrot = rec->yrot;
            obj->transform.xrot = 0.0f;
            obj->transform.zrot = 0.0f;
            obj->transform.scale = rec->scale;
            obj->transform.collisionScale = 50.0f;
            PollWorldCollision(obj->transform.pos.x, obj->transform.pos.y, obj->transform.pos.z, 0xA00, &floor);
            obj->transform.pos.y = floor.y;
            obj->transform.shadowXRot = floor.sn_x;
            obj->transform.shadowYRot = floor.sn_y;
            obj->transform.shadowZRot = floor.sn_z;
            SetAnimation(&obj->transform, 0, 1, 0, 1, 1);
            obj->spawnX = rec->x;
            obj->spawnZ = rec->z;
            obj->spawnYaw = rec->yrot;
            if (rec->flags & 1) {
                obj->transform.unk60 = 0x1000;
            } else {
                obj->transform.unk60 = 0;
            }
            obj++;
            gNpcCount++;
        }
    }
}
void PushNPCOutOfBrian(f32* dx, f32* dz, ActorTransform* actor);
void MoveActorWithCollision(f32 dx, f32 dz, ActorTransform* actor) {
    PushNPCOutOfBrian(&dx, &dz, actor);
    actor->xspeed = dx;
    actor->zspeed = dz;
    actor->pos.x += dx;
    actor->pos.z += dz;
    gActorMovePoll.x = 4.0f;
    PollWorldCollision(actor->pos.x, actor->pos.y, actor->pos.z, 0xA01, &gActorMovePoll);
    actor->pos.x = gActorMovePoll.x;
    actor->pos.y = gActorMovePoll.y;
    actor->pos.z = gActorMovePoll.z;
    actor->shadowXRot = gActorMovePoll.sn_x;
    actor->shadowYRot = gActorMovePoll.sn_y;
    actor->shadowZRot = gActorMovePoll.sn_z;
}
void PushNPCOutOfBrian(f32* dx, f32* dz, ActorTransform* actor) {
    f32 x;
    f32 z;
    f32 ox;
    f32 oz;
    f32 r;
    f32 dist;
    f32 f;
    f32 radius;
    s32 hit;
    s32 n;
    ActorTransform** list;
    ActorTransform* other;
    ModelTransform** props;
    ModelTransform* prop;
    ActorTransform* brian;

    brian = &gPlayerState.brian.transform;
    x = *dx + actor->pos.x;
    z = *dz + actor->pos.z;
    radius = actor->collisionScale * actor->scale;
    hit = 0;
    ox = x - brian->pos.x;
    oz = z - brian->pos.z;
    r = brian->collisionScale * brian->scale + radius;
    if (ox < r && -r < ox && oz < r && -r < oz) {
        dist = sqrtf(ox * ox + oz * oz);
        if (dist < r) {
            f = r / dist;
            hit = 1;
            ox *= f;
            oz *= f;
            *dx = brian->pos.x + ox - actor->pos.x;
            *dz = brian->pos.z + oz - actor->pos.z;
        }
    }
    n = gNpcsDrawn;
    if (n != 0) {
        list = gNpcsDrawnList;
        while (!hit && n != 0) {
            other = *list++;
            if (other != actor) {
                ox = x - other->pos.x;
                oz = z - other->pos.z;
                r = other->collisionScale * other->scale + radius;
                if (ox < r && -r < ox && oz < r && -r < oz) {
                    dist = sqrtf(ox * ox + oz * oz);
                    if (dist < r) {
                        f = r / dist;
                        hit = 1;
                        ox *= f;
                        oz *= f;
                        *dx = other->pos.x + ox - actor->pos.x;
                        *dz = other->pos.z + oz - actor->pos.z;
                    }
                }
            }
            n--;
        }
    }
    n = gPropsNearCount;
    if (n != 0) {
        props = gPropsNear;
        while (!hit && n != 0) {
            prop = *props++;
            ox = x - prop->x;
            oz = z - prop->z;
            r = radius + 2.0 * prop->model_scale;
            if (ox < r && -r < ox && oz < r && -r < oz) {
                dist = sqrtf(ox * ox + oz * oz);
                if (dist < r) {
                    f = r / dist;
                    hit = 1;
                    ox *= f;
                    oz *= f;
                    *dx = prop->x + ox - actor->pos.x;
                    *dz = prop->z + oz - actor->pos.z;
                }
            }
            n--;
        }
    }
}
u32 CountItem(u8 itemID);
void AddItemToInventory(u8 itemID);
u32 IsStoryFlagSet(u32 flag);
void StartNPCTalk(s32 index) {
    f32 turn;
    s32 taken;
    s32 text;
    f32 dx;
    f32 dz;
    ActorTransform* brian;

    brian = &gPlayerState.brian.transform;
    gNpcs[index].state = 2;
    if (gNpcs[index].record->flags & 2) {
        gNpcs[index].timer = 0;
    } else {
        gNpcs[index].timer = 0xF;
        dx = brian->pos.x - gNpcs[index].transform.pos.x;
        dz = brian->pos.z - gNpcs[index].transform.pos.z;
        turn = Atan2(dx, dz) - gNpcs[index].transform.yrot;
        if (turn < -3.141592653589793) {
            turn += 6.283185307179586;
        }
        if (turn > 3.141592653589793) {
            turn -= 6.283185307179586;
        }
        gNpcs[index].turnLeft = turn;
        gNpcs[index].timer = 0xF;
        SetAnimation(&gNpcs[index].transform, 1, 1, 1, 1, 1);
    }
    switch (gNpcs[index].record->talkType) {
        case 2:
            if (CountItem((u8)gNpcs[index].record->talkParam) == 0) {
                taken = 0;
                text = gNpcs[index].record->message;
            } else {
                taken = 1;
                text = gNpcs[index].record->altMessage;
            }
            StartNPCText(&gNpcs[index], text);
            if (taken == 0) {
                AddItemToInventory((u8)gNpcs[index].record->talkParam);
                gPlayerState.flags |= 0x40;
            }
            break;
        case 5:
            if (IsStoryFlagSet(gNpcs[index].record->talkParam) == 0) {
                text = gNpcs[index].record->message;
            } else {
                text = gNpcs[index].record->altMessage;
                if (gNpcs[index].record->talkParam == 7) {
                    gGameState |= 0x8080;
                    gModeCountdown = 0x16;
                }
            }
            StartNPCText(&gNpcs[index], text);
            break;
        case 6:
            StartNPCText(&gNpcs[index], gNpcs[index].record->message);
            gMessageLines.lineCount = 2;
            gMessageLines.lines[0] = gSaveChoiceYes;
            gMessageLines.lines[1] = gSaveChoiceNo;
            gMessageChoice = -1;
            gGameState |= 0x10;
            break;
        default:
            StartNPCText(&gNpcs[index], gNpcs[index].record->message);
            break;
    }
}
void ResumeNpcAfterTalk(s32 index) {
    NpcRecord* rec;

    rec = gNpcs[index].record;
    if (rec->kind == 2) {
        gNpcs[index].state = 0;
        gNpcs[index].timer = getRandomNumber(30) + 30;
        return;
    }
    if (rec->flags & 2) {
        gNpcs[index].state = 0;
        gNpcs[index].timer = rec->anims->frames;
        return;
    }
    gNpcs[index].state = 3;
    gNpcs[index].timer = 0xF;
    SetAnimation(&gNpcs[index].transform, 1, 1, 1, 1, 1);
}
void StartNPCText(Npc* npc, s32 textOffset) {
    s32 i;

    dma_write(textOffset + dialogue_bank_ROM_START, gTextBuffer, 0x4B0);
    if (npc->record->name != 0) {
        for (i = 0; i != 0x20; i++) {
            gSpeakerName[i] = (npc->record->name)[i];
        }
    } else {
        gSpeakerName[0] = 0xFF;
    }
}
void StartEndingText(s32 index, u8* text) {
    s32 i;
    u8* dst;
    Npc* npc;

    dst = gTextBuffer;

    for (i = 0x4B0; i != 0; i--) {
        *dst++ = *text++;
    }

    npc = &gNpcs[index];

    if (npc->record->name != 0) {
        for (i = 0; i != 0x20; i++) {
            gSpeakerName[i] = (npc->record->name)[i];
        }
    } else {
        gSpeakerName[0] = 0xFF;
    }
}
void LoadMonsterSet(void);
void ResetEnemies(void) {
    EnemyAction* enemy;
    s32 i;

    gEnemyCount = 0;
    gEnemiesDrawn = 0;
    enemy = gEnemies;
    i = 0;
    while (i < 6) {
        enemy->battleData.transform.modelID = -1;
        enemy++;
        i++;
    }
    if (gSubmapFlags & 1) {
        LoadMonsterSet();
    }
}
void UpdateEnemies(void) {
    EnemyAction* enemy;
    MonsterBattleData* actor;
    s32 n;

    n = gEnemyCount;
    if (n != 0) {
        for (enemy = gEnemies; n != 0; enemy++) {
            actor = &enemy->battleData;
            if (enemy->battleData.transform.modelID != -1) {
                gEnemyActionHandlers[enemy->actionID](actor, enemy, gMonsterAnims[actor->transform.modelID]);
                n--;
            }
        }
    }

    if (gBattleState & 0x100) {
        UpdateBossEffectScript();
    }
}
void DrawEnemies(Mtx* mtx) {
    EnemyAction* e;
    MonsterBattleData* m;
    MonsterBattleData** list;
    AnimEntry* anims;
    s32 n;
    f32 y;
    f32 lift;

    gEnemiesDrawn = 0;
    list = gDrawnEnemies;
    gDPSetTextureFilter(gDisplayListHead++, G_TF_BILERP);
    n = gEnemyCount;
    for (e = gEnemies; n != 0; e++) {
        m = &e->battleData;
        if (e->battleData.transform.modelID != -1) {
            y = m->transform.pos.y;
            anims = gMonsterAnims[m->transform.modelID];
            if (m->transform.base.monster->monsterType == FLYING) {
                lift = m->transform.base.monster->height * m->transform.scale;
                if (y < gPlayerState.brian.transform.pos.y) {
                    m->transform.pos.y = gPlayerState.brian.transform.pos.y;
                }
                m->transform.pos.y += lift;
                m->stats->flyY = m->transform.pos.y;
            }
            DrawModel(1, &m->transform, &anims[m->transform.animID], mtx);
            AdvanceAnimation(m->transform.animFramerate, &m->transform, &anims[m->transform.animID]);
            m->transform.pos.y = y;
            if (m->transform.base.monster->monsterType != MAMMON && !(m->transform.floorFlags & 0x8000)) {
                AddShadow(m->transform.pos.x, m->transform.pos.y, m->transform.pos.z, m->transform.shadowXRot, m->transform.shadowYRot, m->transform.shadowZRot,
                              m->transform.collisionScale * m->transform.scale);
            }
            *list++ = m;
            n--;
            gEnemiesDrawn++;
        }
    }
}
void LoadMonsterSet(void) {
    u8* start;
    u8* end;
    s32 size;
    MonsterSet* areaMonsterData;

    areaMonsterData = (gCurrentMonsterSet = &gMonsterSets[gSubmapHeader->area]);
    gMonsterAnims = areaMonsterData->anims;
    gEnemyNames = areaMonsterData->namePtr;
    start = areaMonsterData->fileStart;
    end = areaMonsterData->fileEnd;
    size = end - start;
    dma_write(start, gMonsterFileBuffer, size);
}
void ClearEnemies(void) {
    EnemyAction* enemy;
    s32 remaining;
    MonsterBattleData* actor;

    enemy = gEnemies;
    remaining = 6;
    do {
        actor = &enemy->battleData;
        if (actor->transform.modelID != -1) {
            ClearStatusEffects(actor);
            actor->transform.modelID = -1;
        }
        remaining--;
        enemy++;
    } while (remaining != 0);
    gEnemyCount = 0;
}
void SpawnEnemy(s32 id, f32 x, f32 z, f32 yrot, MonsterBattleData* m, EnemyAction* e) {
    u16 mask;
    MonsterBaseData* base;

    base = &gCurrentMonsterSet->baseStats[id];
    m->transform.modelID = base->monsterID;
    SetAnimation(&m->transform, 0, 1, 0, 1, 1);
    m->transform.animFrame = getRandomNumber(0x10);
    m->transform.scale = base->modelScale;
    m->transform.collisionScale = base->radius;
    m->stats = &e->battleData.statsBuf;
    m->transform.base.monster = base;
    e->monBaseData = base;
    e->currHP = base->maxHP;
    e->currMP = base->maxHP;
    e->actionID = 0;
    e->flags = 0;
    e->movementBoundSize = (f32)(u32)base->agi * 0.2857 + 17.0;
    e->pushZ = 0.0;
    e->pushX = 0.0;
    gEnemyFloorPoll.x = m->transform.base.monster->radius * m->transform.base.monster->modelScale;
    mask = 0xA01;
    PollWorldCollision(x, 0.0f, z, mask, &gEnemyFloorPoll);
    if (gEnemyFloorPoll.flags & 0x8000) {
        if (m->transform.base.monster->monsterType == ON_GROUND) {
            PollWorldCollision(x + 0.5, 0.0f, z + 0.5, mask, &gEnemyFloorPoll);
            if (gEnemyFloorPoll.flags & 0x8000) {
                gEnemyFloorPoll.y = gPlayerState.brian.transform.pos.y;
            }
        } else {
            gEnemyFloorPoll.y = gPlayerState.brian.transform.pos.y;
        }
    }
    m->transform.pos.x = gEnemyFloorPoll.x;
    m->transform.pos.y = gEnemyFloorPoll.y;
    m->transform.pos.z = gEnemyFloorPoll.z;
    m->transform.yrot = yrot;
    m->transform.shadowXRot = gEnemyFloorPoll.sn_x;
    m->transform.shadowYRot = gEnemyFloorPoll.sn_y;
    m->transform.shadowZRot = gEnemyFloorPoll.sn_z;
    m->transform.floorFlags = gEnemyFloorPoll.flags;
    m->transform.actorFlags = 1;
    m->transform.unk60 = 0;
    e->battleData.statsBuf.statusFlags = 0;
    e->battleData.statsBuf.hitMask = 0;
    e->battleData.statsBuf.atk = e->monBaseData->atk;
    e->battleData.statsBuf.atkScale = 0x10;
    e->battleData.statsBuf.agi = m->transform.base.monster->agi;
    e->battleData.statsBuf.def = m->transform.base.monster->def;
    e->battleData.statsBuf.moveScale = 1.0f;
    e->battleData.statsBuf.flyY = m->transform.pos.y;
}
void SortTurnOrder(void) {
    u32 i;
    s32 swaps;
    u16 tmp;
    u16 agi[6];

    for (i = 0; i < gEnemyCount; i++) {
        gTurnOrder[i] = i;
        agi[i] = gEnemies[i].monBaseData->agi;
    }
    do {
        swaps = 0;
        for (i = 0; i < gEnemyCount - 1; i++) {
            if (agi[i] < agi[i + 1]) {
                tmp = agi[i];
                agi[i] = agi[i + 1];
                agi[i + 1] = tmp;
                swaps++;
                tmp = gTurnOrder[i];
                gTurnOrder[i] = gTurnOrder[i + 1];
                gTurnOrder[i + 1] = tmp;
            }
        }
    } while (swaps != 0);
}
void FlagTurnEnemy(void) {
    gEnemies[gTurnOrder[gTurnIndex]].flags |= 1;
}
void EnemyHitReaction(MonsterBattleData* actor, EnemyAction* enemy, AnimEntry* anims) {
    EnemyAIOption* option;

    GetRootMotionOffset(actor, &gEnemyStep);
    MoveEnemy(0, gEnemyStep.x, gEnemyStep.y, actor, enemy);
    if (enemy->flags & 1) {
        gCardEnemy = enemy;
        gMonsterCardTimer = 0x3C;
        enemy->flags &= ~3;
        if (enemy->battleData.statsBuf.statusFlags & 8) {
            EndBattleTurn(actor);
            return;
        }
        option = PickEnemyAIOption(enemy, actor);
        if (option != NULL) {
            if (!(enemy->flags & 0x10)) {
                CommitEnemyAIOption(enemy, actor, option);
                return;
            }
            enemy->actionID = 1;
            enemy->tries = 4;
            if (gBattleState & 0x100) {
                enemy->actionTimer = anims[1].frames;
                SetAnimation(&actor->transform, 1, 1, 1, 1, 1);
                return;
            }
            enemy->actionTimer = anims[1].frames / 10;
            SetAnimation(&actor->transform, 1, 10, 1, 10, 1);
            return;
        }
        enemy->actionID = 1;
        enemy->tries = 4;
        if (gBattleState & 0x100) {
            enemy->actionTimer = anims[1].frames;
            SetAnimation(&actor->transform, 1, 1, 1, 1, 1);
            return;
        }
        enemy->actionTimer = anims[1].frames / 10;
        SetAnimation(&actor->transform, 1, 10, 1, 10, 1);
    }
}
void SelectEnemyAction(MonsterBattleData* actor, EnemyAction* enemy, AnimEntry* anims) {
    f32 dx;
    f32 dz;
    EnemyAIOption* randomized_option;
    f32 dist_to_player;

    TurnActorToward(actor, gPlayerState.brian.transform.pos.x, gPlayerState.brian.transform.pos.z);

    if (gBattleState & 0x100) {
        gEnemyStep.x = 0.0f;
        gEnemyStep.y = 0.4f;
        rotateCoordinatesByAngle(actor->transform.yrot, &gEnemyStep);
    } else {
        GetRootMotionOffset(actor, &gEnemyStep);
    }

    MoveEnemy(1, gEnemyStep.x, gEnemyStep.y, actor, enemy);

    enemy->actionTimer--;
    if (enemy->actionTimer == 0) {
        enemy->tries--;
        if (enemy->defaultOption != NULL) {
            dx = gPlayerState.brian.transform.pos.x - actor->transform.pos.x;
            dz = gPlayerState.brian.transform.pos.z - actor->transform.pos.z;

            dist_to_player = sqrtf((dx * dx) + (dz * dz)) - (gPlayerState.brian.transform.base.monster->radius * gPlayerState.brian.transform.scale);
            if (dist_to_player < (enemy->defaultOption->max_range * actor->transform.scale)) {
                CommitEnemyAIOption(enemy, actor, enemy->defaultOption);
                return;
            }
            if ((enemy->tries == 0) || (enemy->flags & 2)) {
                CommitEnemyAIOption(enemy, actor, enemy->defaultOption);
                return;
            }
            if (gBattleState & 0x100) {
                enemy->actionTimer = anims[1].frames;
                return;
            }
            enemy->actionTimer = anims[1].frames / 10;
            return;
        }

        randomized_option = PickEnemyAIOption(enemy, actor);
        if (randomized_option != NULL) {
            CommitEnemyAIOption(enemy, actor, randomized_option);
            return;
        }
        if ((enemy->tries == 0) || (enemy->flags & 2)) {
            enemy->actionID = 0;
            SetAnimation(&actor->transform, 0, 1, 0, 1, 1);
            EndBattleTurn(actor);
            return;
        }
        if (gBattleState & 0x100) {
            enemy->actionTimer = anims[1].frames;
            return;
        }
        enemy->actionTimer = anims[1].frames / 10;
    }
}
void EnemyKnockback(MonsterBattleData* actor, EnemyAction* enemy, AnimEntry* anims) {
    GetRootMotionOffset(actor, &gEnemyStep);
    MoveEnemy(0, gEnemyStep.x, gEnemyStep.y, actor, enemy);
    enemy->actionTimer--;
    if (enemy->actionTimer == 0) {
        if (enemy->currHP == 0) {
            ClearStatusEffects(actor);
            SpawnEnemyLandingBurst(actor);
            actor->transform.modelID = -1;
            actor->transform.unk60 &= ~1;
            gEnemyCount--;
            gBattleExp += actor->transform.base.monster->expValue;
            gBattleStone += actor->transform.base.monster->stone;
            PlaySound(0, 0xA, 0xFF);
            return;
        }
        enemy->actionID = 0;
        actor->transform.unk60 &= ~3;
    }
}
s32 EnemyAgilityCheck(MonsterBattleData* actor);
u16 CalcEnemyMeleeDamage(EnemyAction* enemy);
u16 DamageBrian(u16 power, u16 flags, MonsterBattleData* attacker);   /* hurt Brian; returns the damage */
void StartBrianHit(void);            /* Brian dodges */
void SpawnStaffHitEffect(f32 x, f32 y, f32 z);   /* hit sparks */
void SpawnDamageNumber(MonsterBattleData* target, s32 kind, s32 damage);   /* damage / heal digits */
void UpdateStatusVisuals(MonsterBattleData* actor, u16 statusBits, u16 kind, s16 duration);
void EnemyFaceBrian(MonsterBattleData* actor, EnemyAction* enemy, AnimEntry* anims) {
    TurnActorToward(actor, gPlayerState.brian.transform.pos.x, gPlayerState.brian.transform.pos.z);
    GetRootMotionOffset(actor, &gEnemyStep);
    MoveEnemy(0, gEnemyStep.x, gEnemyStep.y, actor, enemy);
    enemy->actionTimer--;
    if (enemy->actionTimer == 0) {
        enemy->actionID = 0;
        actor->transform.unk60 &= ~1;
        EndBattleTurn(actor);
    }
}
void EnemyMeleeAttack(MonsterBattleData* actor, EnemyAction* enemy, AnimEntry* anims) {
    u16 damage;
    f32 dx;
    f32 dz;
    f32 dist;
    EnemyAIOption* option;
    u16 hitFlags;
    u16 moveFlags;

    switch (enemy->phase) {
        case 0:
            moveFlags = enemy->defaultOption->move_flags;
            if (moveFlags & 2) {
                GetRootMotionOffset(actor, &gEnemyStep);
                MoveEnemy(0, gEnemyStep.x, gEnemyStep.y, actor, enemy);
            }
            enemy->actionTimer--;
            if (enemy->actionTimer == 0) {
                enemy->phase++;
                enemy->actionTimer = enemy->defaultOption->hold;
            }
            break;
        case 1:
            moveFlags = enemy->defaultOption->move_flags;
            if (moveFlags & 0xA) {
                GetRootMotionOffset(actor, &gEnemyStep);
                MoveEnemy(0, gEnemyStep.x, gEnemyStep.y, actor, enemy);
            }
            if (!(gPlayerState.brian.transform.unk60 & 2) && !(enemy->flags & 8)) {
                dx = gPlayerState.brian.transform.pos.x - actor->transform.pos.x;
                dz = gPlayerState.brian.transform.pos.z - actor->transform.pos.z;
                dist = sqrtf(dx * dx + dz * dz) - gPlayerState.brian.transform.scale * gPlayerState.stats->radius;
                if (dist < enemy->defaultOption->hit_range * actor->transform.scale) {
                    if (EnemyAgilityCheck(actor) != 0) {
                        /* damage holds the attack's power first, then what Brian took */
                        damage = CalcEnemyMeleeDamage(enemy);
                        hitFlags = 0x8004;
                        if (enemy->flags & 0x20) {
                            hitFlags = 0xC004;
                        }
                        damage = DamageBrian(damage, hitFlags, actor);
                        gEnemyStep.x = 0.0f;
                        gEnemyStep.y = 2.0f * (actor->transform.base.monster->radius * actor->transform.scale);
                        rotateCoordinatesByAngle(actor->transform.yrot, &gEnemyStep);
                        SpawnStaffHitEffect(actor->transform.pos.x + gEnemyStep.x, actor->transform.pos.y + actor->transform.base.monster->height * actor->transform.scale * 0.7f,
                                      actor->transform.pos.z + gEnemyStep.y);
                        if (enemy->battleData.statsBuf.statusFlags & 2) {
                            /* draining: the enemy heals a quarter of the damage, plus one */
                            damage = ((damage + 3) >> 2) + 1;
                            enemy->currHP += damage;
                            if (enemy->currHP > actor->transform.base.monster->maxHP) {
                                enemy->currHP = actor->transform.base.monster->maxHP;
                            }
                            SpawnDamageNumber(actor, 1, damage);
                            UpdateStatusVisuals(actor, 2, 1, 30);
                        }
                    } else {
                        StartBrianHit();
                        gPlayerState.stats->agiXP++;
                    }
                    enemy->flags |= 8;
                }
            }
            enemy->actionTimer--;
            if (enemy->actionTimer == 0) {
                option = enemy->defaultOption;
                enemy->phase++;
                if (option->next_anim == -1) {
                    enemy->actionTimer = anims[option->anim].frames - option->cast_delay;
                } else {
                    enemy->actionTimer = anims[option->recover_anim].frames;
                    SetAnimation(&actor->transform, option->recover_anim, 1, 0, 1, 1);
                    GetRootMotionOffset(actor, &gEnemyStep);
                }
                if (enemy->actionTimer <= 0) {
                    enemy->actionTimer = 1;
                }
                EndBattleTurn(actor);
            }
            break;
        case 2:
            GetRootMotionOffset(actor, &gEnemyStep);
            MoveEnemy(0, gEnemyStep.x, gEnemyStep.y, actor, enemy);
            enemy->actionTimer--;
            if (enemy->actionTimer == 0) {
                enemy->actionID = 0;
                actor->transform.unk60 &= ~1;
            }
            break;
    }
}
void SpawnSpell(MonsterBattleData* caster, u32 spell, s32 shot, u16 flags);   /* cast a spell */
void EnemyCastAttack(MonsterBattleData* actor, EnemyAction* enemy, AnimEntry* anims) {
    EnemyAIOption* option;

    switch (enemy->phase) {
        case 0:
            if (enemy->defaultOption->move_flags & 2) {
                GetRootMotionOffset(actor, &gEnemyStep);
                MoveEnemy(0, gEnemyStep.x, gEnemyStep.y, actor, enemy);
            }
            enemy->actionTimer--;
            if (enemy->actionTimer == 0) {
                SpawnSpell(actor, enemy->defaultOption->action_tag & ~0x8000, 0, 0);
                option = enemy->defaultOption;
                enemy->phase++;
                if (option->next_anim == -1) {
                    enemy->actionTimer = anims[option->anim].frames - option->cast_delay;
                } else {
                    enemy->phase++;
                    enemy->actionTimer = option->hold;
                }
                if (enemy->actionTimer <= 0) {
                    enemy->actionTimer = 0xF;
                }
                EndBattleTurn(actor);
            }
            break;
        case 1:
            if (enemy->defaultOption->move_flags & 0xA) {
                GetRootMotionOffset(actor, &gEnemyStep);
                MoveEnemy(0, gEnemyStep.x, gEnemyStep.y, actor, enemy);
            }
            enemy->actionTimer--;
            if (enemy->actionTimer == 0) {
                enemy->actionID = 0;
                actor->transform.unk60 &= ~1;
                enemy->pushX = 0.0f;
                enemy->pushZ = 0.0f;
            }
            break;
        case 2:
            if (enemy->defaultOption->move_flags & 0xA) {
                GetRootMotionOffset(actor, &gEnemyStep);
                MoveEnemy(0, gEnemyStep.x, gEnemyStep.y, actor, enemy);
            }
            enemy->actionTimer--;
            if (enemy->actionTimer == 0) {
                option = enemy->defaultOption;
                enemy->phase--;
                enemy->actionTimer = anims[option->recover_anim].frames;
                SetAnimation(&actor->transform, option->recover_anim, 1, 0, 1, 1);
            }
            if (enemy->actionTimer <= 0) {
                enemy->actionTimer = 0xF;
            }
            break;
    }
}
EnemyAIOption* PickEnemyAIOption(EnemyAction* enemy, MonsterBattleData* actor) {
    f32 dx;
    f32 dz;
    ActorTransform* brian;
    f32 dist;
    f32 range;
    f32 sum;
    s32 flag;
    u32 count;
    EnemyAIOption* option;
    EnemyAIOption* other;
    EnemyAIOption* sorted[5];
    u32 i;

    brian = &gPlayerState.brian.transform;
    enemy->flags &= ~0x10;
    dx = gPlayerState.brian.transform.pos.x - actor->transform.pos.x;
    dz = gPlayerState.brian.transform.pos.z - actor->transform.pos.z;
    dist = sqrtf((dx * dx) + (dz * dz)) - (brian->base.monster->radius * brian->scale);
    option = enemy->monBaseData->aiOptions;
    for (i = 0; i < enemy->monBaseData->aiScriptNumber; i++) {
        sorted[i] = option;
        option++;
    }
    do {
        flag = 0;
        for (i = 0; i < enemy->monBaseData->aiScriptNumber - 1; i++) {
            other = sorted[i];
            if (sorted[i + 1]->max_range < other->max_range) {
                sorted[i] = sorted[i + 1];
                sorted[i + 1] = other;
                flag++;
            }
        }
    } while (flag != 0);
    flag = 0;
    for (i = 0; i < enemy->monBaseData->aiScriptNumber; i++) {
        option = sorted[i];
        if (i < enemy->monBaseData->aiScriptNumber - 1) {
            range = option->max_range * actor->transform.scale;
            sum = (enemy->movementBoundSize * enemy->battleData.statsBuf.moveScale) + range;
            if (dist < range) {
                flag = 1;
                break;
            }
            if (dist < sum) {
                other = sorted[i + 1];
                range = other->max_range * actor->transform.scale;
                if (dist < range) {
                    if (getRandomNumber(2) == 0) {
                        option = other;
                    }
                    flag = 1;
                    if (option != other) {
                        enemy->flags |= 0x10;
                    }
                    break;
                }
            }
        } else {
            range = option->max_range * actor->transform.scale;
            sum = (enemy->movementBoundSize * enemy->battleData.statsBuf.moveScale) + range;
            if (dist < sum) {
                if (range < dist) {
                    enemy->flags |= 0x10;
                }
                flag = 1;
                break;
            }
        }
    }
    if (flag == 0) {
        option = NULL;
    }
    if (enemy->monBaseData->aiFlags & 4) {
        count = enemy->monBaseData->aiScriptNumber;
        option = &enemy->monBaseData->aiOptions[getRandomNumber(count)];
    }
    enemy->defaultOption = option;
    return option;
}

void CommitEnemyAIOption(EnemyAction* enemy, MonsterBattleData* actor, EnemyAIOption* option) {
    s32 isSpell;
    s32 nextAnim;

    isSpell = option->action_tag & 0x8000;
    if ((isSpell != 0) && (enemy->battleData.statsBuf.statusFlags & 0x40)) {
        enemy->actionID = 0;
        SetAnimation(&actor->transform, 0, 1, 0, 1, 1);
        EndBattleTurn(actor);
        return;
    }
    if (isSpell != 0) {
        enemy->actionID = 5;
    } else {
        enemy->actionID = 4;
    }

    enemy->phase = 0;
    enemy->actionTimer = option->cast_delay;
    enemy->flags &= ~0x28;

    actor->transform.unk60 = 1;
    actor->transform.yrot = Atan2(gPlayerState.brian.transform.pos.x - actor->transform.pos.x, gPlayerState.brian.transform.pos.z - actor->transform.pos.z);

    if (option->next_anim == -1) {
        nextAnim = 0;
    } else {
        nextAnim = option->next_anim;
    }
    SetAnimation(&actor->transform, option->anim, 1, nextAnim, 1, 1);
    SnapSpellCamera(option->cast_delay + 0xF, actor);

    if (enemy->actionTimer == 0) {
        enemy->actionTimer = 1;
    }
    if (gBattleState & 0x100) {
        StartBossEffectScript(option->action_tag & 0xFF, enemy);
    }
}
void TurnActorToward(MonsterBattleData* actor, f32 targetX, f32 targetZ) {
    f32 diff;

    targetX -= actor->transform.pos.x;
    targetZ -= actor->transform.pos.z;
    diff = Atan2(targetX, targetZ) - actor->transform.yrot;
    if (diff > 3.141592653589793) {
        diff -= 6.283185307179586;
    } else if (diff < -3.141592653589793) {
        diff += 6.283185307179586;
    }
    actor->transform.yrot += diff * 0.05;
    if (actor->transform.yrot > 3.141592653589793) {
        actor->transform.yrot -= 6.283185307179586;
    } else if (actor->transform.yrot < -3.141592653589793) {
        actor->transform.yrot += 6.283185307179586;
    }
}
extern f32 gBattleCenterX;               /* the battle circle's centre x */
extern f32 gBattleCenterZ;               /* and z */
extern f32 gTurnCenterX;               /* the turn\'s movement circle centre x */
extern f32 gTurnCenterZ;               /* and z */
extern f32 gBattleRadius;               /* the battle circle's radius */
void PushEnemyOut(f32* dx, f32* dz, MonsterBattleData* actor);
void MoveEnemy(s32 leash, f32 dx, f32 dz, MonsterBattleData* m, EnemyAction* e) {
    f32 dist;
    f32 ddx;
    f32 ddz;
    f32 reach;
    MonsterBaseData* base;
    u16 mask;

    if (e->pushX != 0.0f || e->pushZ != 0.0f) {
        dx += e->pushX;
        dz += e->pushZ;
        e->pushX *= 0.9f;
        e->pushZ *= 0.9f;
    }
    if (leash != 0) {
        ddx = (m->transform.pos.x + m->transform.xspeed) - gTurnCenterX;
        ddz = (m->transform.pos.z + m->transform.zspeed) - gTurnCenterZ;
        dist = sqrtf((ddx * ddx) + (ddz * ddz));
        reach = e->movementBoundSize * e->battleData.statsBuf.moveScale;
        if (reach < dist) {
            dx = ((reach * ddx) / dist - m->transform.pos.x) + gTurnCenterX;
            dz = ((reach * ddz) / dist - m->transform.pos.z) + gTurnCenterZ;
            e->flags |= 2;
        }
    }
    ddx = (m->transform.pos.x + m->transform.xspeed) - gBattleCenterX;
    ddz = (m->transform.pos.z + m->transform.zspeed) - gBattleCenterZ;
    dist = sqrtf((ddx * ddx) + (ddz * ddz));
    if (gBattleRadius < dist) {
        dx = (((gBattleRadius - 0.5) * ddx) / dist - m->transform.pos.x) + gBattleCenterX;
        dz = (((gBattleRadius - 0.5) * ddz) / dist - m->transform.pos.z) + gBattleCenterZ;
    }
    PushEnemyOut(&dx, &dz, m);
    base = m->transform.base.monster;
    m->transform.xspeed = dx;
    mask = 0xA00;
    m->transform.zspeed = dz;
    m->transform.pos.x += dx;
    m->transform.pos.z += dz;
    if (base->monsterType == ON_GROUND && e->actionID != 0) {
        mask = 0xA01;
    }
    gEnemyFloorPoll.x = base->radius * base->modelScale;
    PollWorldCollision(m->transform.pos.x, m->transform.pos.y, m->transform.pos.z, mask, &gEnemyFloorPoll);
    m->transform.pos.x = gEnemyFloorPoll.x;
    m->transform.pos.y = gEnemyFloorPoll.y;
    m->transform.pos.z = gEnemyFloorPoll.z;
    if (e->flags & 4) {
        m->transform.pos.y -= 30.0;
    }
    m->transform.shadowXRot = gEnemyFloorPoll.sn_x;
    m->transform.shadowYRot = gEnemyFloorPoll.sn_y;
    m->transform.shadowZRot = gEnemyFloorPoll.sn_z;
    m->transform.floorFlags = gEnemyFloorPoll.flags;
}
void ApplyRootMotion(ActorTransform* actor, AnimEntry* anim, f32 scale);
void GetRootMotionOffset(MonsterBattleData* actor, Coordinates2D* offset) {
    AnimEntry* anims = gMonsterAnims[actor->transform.modelID];

    ApplyRootMotion(&actor->transform, &anims[actor->transform.animID], actor->transform.scale);
    offset->x = -actor->transform.unkDoor44;
    offset->y = actor->transform.unkDoor4C;
    rotateCoordinatesByAngle(actor->transform.yrot, offset);
}
void PushEnemyOut(f32* dx, f32* dz, MonsterBattleData* m) {
    f32 x;
    f32 z;
    f32 ox;
    f32 oz;
    f32 r;
    f32 dist;
    f32 f;
    f32 radius;
    s32 hit;
    s32 n;
    MonsterBattleData** list;
    MonsterBattleData* other;
    ModelTransform** props;
    ModelTransform* prop;
    ActorTransform* brian;

    brian = &gPlayerState.brian.transform;
    x = *dx + m->transform.pos.x;
    z = *dz + m->transform.pos.z;
    radius = m->transform.collisionScale * m->transform.scale;
    hit = 0;
    ox = x - brian->pos.x;
    oz = z - brian->pos.z;
    r = brian->collisionScale * brian->scale + radius;
    if (ox < r && -r < ox && oz < r && -r < oz) {
        dist = sqrtf(ox * ox + oz * oz);
        if (dist < r) {
            f = r / dist;
            hit = 1;
            ox *= f;
            oz *= f;
            *dx = brian->pos.x + ox - m->transform.pos.x;
            *dz = brian->pos.z + oz - m->transform.pos.z;
        }
    }
    n = gEnemiesDrawn;
    if (n != 0) {
        list = gDrawnEnemies;
        while (!hit && n != 0) {
            other = *list++;
            if (other != m) {
                ox = x - other->transform.pos.x;
                oz = z - other->transform.pos.z;
                r = other->transform.collisionScale * other->transform.scale + radius;
                if (ox < r && -r < ox && oz < r && -r < oz) {
                    dist = sqrtf(ox * ox + oz * oz);
                    if (dist != 0.0f && dist < r) {
                        f = r / dist;
                        hit++;
                        ox *= f;
                        oz *= f;
                        *dx = other->transform.pos.x + ox - m->transform.pos.x;
                        *dz = other->transform.pos.z + oz - m->transform.pos.z;
                    }
                }
            }
            n--;
        }
    }
    n = gPropsNearCount;
    if (n != 0) {
        props = gPropsNear;
        while (!hit && n != 0) {
            prop = *props++;
            ox = x - prop->x;
            oz = z - prop->z;
            r = radius + 2.0 * prop->model_scale;
            if (ox < r && -r < ox && oz < r && -r < oz) {
                dist = sqrtf(ox * ox + oz * oz);
                if (dist < r) {
                    f = r / dist;
                    hit++;
                    ox *= f;
                    oz *= f;
                    *dx = prop->x + ox - m->transform.pos.x;
                    *dz = prop->z + oz - m->transform.pos.z;
                }
            }
            n--;
        }
    }
}
s32 EnemyAgilityCheck(MonsterBattleData *actor)
{
  s16 chance;
  s32 agi;
  agi = actor->stats->agi;
  chance = (agi * 100) / (agi + ((gPlayerState.brian.stats->agi + 7) >> 3));
  return getRandomNumber(100) < chance;
}
extern u8 gCritFade[];             /* the critical hit cue (spells.c plays it too) */
void StartScreenFade(s32 frames, u16 flags, void* ramp);
u16 CalcEnemyMeleeDamage(EnemyAction* enemy) {
    u16 damage = enemy->battleData.statsBuf.atk;

    damage = (damage * enemy->battleData.statsBuf.atkScale) >> 4;
    if (getRandomNumber(0x20) == 0) {
        damage += damage >> 1;
        StartScreenFade(4, 0, gCritFade);
        enemy->flags |= 0x20;
    }
    if (enemy->battleData.statsBuf.statusFlags & 1) {
        damage >>= 1;
    }
    return damage;
}
extern s32 gElementMasks[];             /* element bits per element */
u16 DamageEnemy(s32 index, u16 damage, u16 element, u16 flags, MonsterBattleData* attacker) {
    EnemyAction* enemy;
    MonsterBattleData* actor;
    f32 angle;
    AnimEntry* anims;

    enemy = &gEnemies[index];
    anims = gMonsterAnims[enemy->battleData.transform.modelID];
    if (enemy->currHP != 0) {
        actor = &enemy->battleData;
        if (damage) {
            if (!(flags & 0x8000)) {
                switch (gElementMasks[enemy->monBaseData->element] | gElementMasks[element]) {
                    case 1:
                    case 2:
                    case 4:
                    case 8:
                        damage >>= 1;
                        break;
                    case 5:
                    case 10:
                        damage += damage >> 2;
                        break;
                }
            } else {
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
                    damage += damage >> 1;
                }
            }
            angle = attacker->stats->atk;
            angle /= attacker->stats->atk + enemy->battleData.statsBuf.def;
            damage = (u32)((f32)(u32)damage * angle);
            damage += getRandomNumber(sqrtf((f32)(u32)damage) * 0.75 + 1.0);
            if (!damage) {
                damage++;
            }
            if (enemy->currHP < damage) {
                enemy->currHP = 0;
            } else {
                enemy->currHP -= damage;
            }
            if (enemy->battleData.statsBuf.statusFlags & 0x200) {
                enemy->currMP += damage;
                if (actor->transform.base.monster->maxMP < enemy->currMP) {
                    enemy->currMP = actor->transform.base.monster->maxMP;
                }
                UpdateStatusVisuals(actor, 0x200, 2, 0x1E);
            }
            SpawnDamageNumber(actor, (flags & 0x4000) ? 0x8003 : 0, damage);
        }
        enemy->actionID = 2;
        actor->transform.unk60 |= 1;
        if (flags & 4) {
            actor->transform.unk60 |= 2;
        }
        if (gBattleState & 0x100) {
            enemy->actionTimer = anims[enemy->monBaseData->hitAnim].frames;
            SetAnimation(&actor->transform, enemy->monBaseData->hitAnim, 1, 0, 1, 1);
        } else {
            enemy->actionTimer = anims[enemy->monBaseData->hitAnim].frames / 10;
            SetAnimation(&actor->transform, enemy->monBaseData->hitAnim, 10, 0, 1, 1);
        }
    }
    if (flags & 0x8000) {
        PlaySound(0, 0x18, 0xFF);
    }
    return damage;
}

void ShowEnemyMiss(MonsterBattleData* actor) {
    Setup_MISS_Icon(actor);
    PlaySound(0, 0x17, 0xFF);
}
s32 GetEnemyIndex(MonsterBattleData* battleData) {
    s32 i;

    for (i = 0; i < 6; i++) {
        if (battleData == &gEnemies[i].battleData) {
            break;
        }
    }
    return i;
}
extern SpellAnimDef gEnemySpellEffects[];     /* enemy spell effects */
extern EffectSpawnDef gEffectModels[];   /* effect models */
void*SpawnEffectObject(u16 colour, u16 duration, f32 x, f32 y, f32 z, f32 angle, EffectSpawnDef* model, OddVector* size, ActorTransform* actor);
void QueueSparkleBurst(s32 kind, s32 frames, f32 x, f32 y, f32 z, f32 size);
void SpawnEnemyLandingBurst(MonsterBattleData* actor) {
    f32 y;
    EffectSpawnDef* model;
    SpellAnimDef* def;

    def = &gEnemySpellEffects[4];
    ((OddVector*)&gEnemyStep)->x = def->land_size_x;
    ((OddVector*)&gEnemyStep)->y = def->land_size_y;
    ((OddVector*)&gEnemyStep)->scale = actor->transform.collisionScale;
    model = &gEffectModels[def->land_model];
    if (actor->transform.base.monster->monsterType == FLYING) {
        y = actor->stats->flyY;
    } else {
        y = actor->transform.pos.y + actor->transform.base.monster->height * 0.5 * actor->transform.scale;
    }
    SpawnEffectObject(def->land_color, def->land_duration, actor->transform.pos.x, y, actor->transform.pos.z, actor->transform.yrot, model, (OddVector*)&gEnemyStep, &actor->transform);
    QueueSparkleBurst(2, 0x1E, actor->transform.pos.x, actor->transform.base.monster->height * actor->transform.scale + y, actor->transform.pos.z,
                  actor->transform.base.monster->radius * actor->transform.scale * 3.6);
}
void StopBossEffectScript(void) {
    gBossEffectScript.entry = NULL;
}
void StartBossEffectScript(s32 id, EnemyAction* enemy) {
    gBossEffectScript.frame = 0;
    gBossEffectScript.entry = gBossEffectScripts[id];
    gBossEffectScript.enemy = enemy;
    gBossEffectScript.battleData = &gEnemies[0].battleData;
}
void StartCameraShake(s32 kind);
extern SpellDef* gSpellDefs[];
typedef struct {
    s16 model;
    u16 color;
    u16 duration;
    u16 unk6;
    f32 size_x;
    f32 size_y;
} SpellAnimStage;                    /* 0x10: one of a SpellAnimDef's stages; a boss script counts them flat across the table */
void UpdateBossEffectScript(void) {
    BossFxEntry* fx;
    EnemyAction* boss;
    f32 x;
    f32 y;
    f32 z;
    SpellDef* def;
    BossEffectScript* script;
    MonsterBattleData* actor;
    s32 element;
    s32 index;
    SpellAnimStage* stage;
    EffectSpawnDef* model;
    ActorTransform* brian;

    brian = &gPlayerState.brian.transform;
    script = &gBossEffectScript;
    if (script->entry != NULL) {
        script->frame++;
        fx = script->entry;
        if (fx->frame < script->frame) {
            actor = script->battleData;
            if (fx->flags & 8) {
                gEnemyStep.x = fx->x;
                gEnemyStep.y = fx->z;
                boss = gEnemies;
                rotateCoordinatesByAngle(actor->transform.yrot, &gEnemyStep);
                boss->pushX = gEnemyStep.x;
                boss->pushZ = gEnemyStep.y;
            } else {
                stage = &((SpellAnimStage*)gEnemySpellEffects)[fx->stage];
                model = &gEffectModels[stage->model];
                gEnemyStep.x = fx->x;
                gEnemyStep.y = fx->z;
                rotateCoordinatesByAngle(actor->transform.yrot + fx->angle, &gEnemyStep);
                if (fx->flags & 2) {
                    x = brian->pos.x;
                    y = brian->pos.y;
                    z = brian->pos.z;
                } else {
                    x = actor->transform.pos.x;
                    y = actor->transform.pos.y;
                    z = actor->transform.pos.z;
                }
                x += gEnemyStep.x * actor->transform.scale;
                y += fx->y * actor->transform.scale;
                z += gEnemyStep.y * actor->transform.scale;
                ((OddVector*)&gEnemyStep)->x = stage->size_x;
                ((OddVector*)&gEnemyStep)->y = stage->size_y;
                if (script->enemy->defaultOption->action_tag & 0x8000) {
                    element = script->enemy->defaultOption->action_tag >> 8;
                    index = script->enemy->defaultOption->action_tag & 0xFF;
                    def = &gSpellDefs[element & 0xF][index];
                    ((OddVector*)&gEnemyStep)->scale = def->radius;
                } else {
                    ((OddVector*)&gEnemyStep)->scale = actor->transform.collisionScale;
                }
                SpawnEffectObject(stage->color, stage->duration, x, y, z, actor->transform.yrot + fx->angle, model, (OddVector*)&gEnemyStep,
                              &actor->transform);
            }
            if (fx->flags & 4) {
                StartCameraShake(1);
            }
            if (fx->flags & 1) {
                script->entry = NULL;
            } else {
                script->entry++;
            }
        }
    }
}
