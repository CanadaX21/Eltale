#include "common.h"
extern PlayerState gPlayerState;
float sqrtf(float value);
extern f32 gBossTalkRadii[];
extern BossData gBossPlacements[];
extern s32 gCurrentMap;
extern s32 gCurrentSubmap;
u32 IsStoryFlagSet(u32 flag);
extern bossFile gBossFiles[];
extern void* gBossVAddr;
void dma_write(u8* devAddr, void* vaddr, u32 nbytes);
void SpawnBoss(s32 id, f32 x, f32 z, f32 yrot, EnemyAction* e);
extern u8 gBitMasks[];

typedef struct {
    ActorTransform transform;
    ActorStats* stats;
} BossSpawnTransform;               /* 0x6C: the head of a MonsterBattleData, the boss's as StartBossScene copies it */
/* ---- boss.c's .bss (0x8007C0B0-0x8007C280), in address order. IDO 5.3 lays out the
   variables first declared among a file's first 32 declarations (typedefs and variables, externs too) after all
   later ones; this block is all inside those 32 and the file declares no other .bss, so it keeps its order.
   Adding declarations above it (headers included: recount after a header edit), or declaring these
   variables there, moves them.
   ---- */
CollisionPollingResult gBossFloorPoll;      /* the boss spawn's floor poll (SpawnBoss) */
s32 D_8007C0CC;                         /* unreferenced */
BossSpawnTransform gBossSpawnTransform;          /* the boss's spawn transform */
u8 gStoryFlags[4];                       /* bosses beaten, one bit each (SetStoryFlag) */
s32 gBossLoaded;                         /* a boss is loaded */
s32 D_8007C144;                         /* unreferenced */
EnemyAction gBoss;                 /* the boss (an EnemyAction: battleData at 0x2C) */
s32 D_8007C278[2];                      /* unreferenced */


//#pragma GLOBAL_ASM("asm/nonmatchings/game/boss/ResetStoryFlags.s")
void ResetStoryFlags(void) {
    gStoryFlags[0] = 0;
}

void LoadMapBoss(void)
{
  s32 fileStart;
  s32 fileEnd;
  int i;
  s32 fileSize;
  BossData *data;
  gBossLoaded = 0;
    
  for (i = 0; i < 8; i++)
  {
    data = &gBossPlacements[i];
    if ((gCurrentMap == data->bossMap) && (gCurrentSubmap == data->bossSubmap))
    {
      if (IsStoryFlagSet(i) == 0)
      {
        fileStart = gBossFiles[data->bossNumber].fileStart;
        fileEnd = gBossFiles[data->bossNumber].fileEnd;
        fileSize = fileEnd - fileStart;
        dma_write((u8*)fileStart, &gBossVAddr, fileSize); //load boss file from RAM
        SpawnBoss(i, data->x, data->y, data->z, &gBoss);
        gBossLoaded = i + 1;
      }
      return;
    }
  }

}
extern AnimEntry* gBossAnimTables[];      /* the boss models' animation tables */
extern Gfx* gDisplayListHead;
void DrawModel(s32 flags, ActorTransform* actor, AnimEntry* anim, s32 mtxList);
void AdvanceAnimation(s32 framerate, ActorTransform* model, AnimEntry* anim);
void AddShadow(f32 x, f32 y, f32 z, f32 xrot, f32 yrot, f32 zrot, f32 size);   /* the shadow */
void DrawBoss(Mtx* mtxList) {
    EnemyAction* boss;
    MonsterBattleData* actor;
    AnimEntry* anims;

    if (gBossLoaded != 0) {
        boss = &gBoss;
        actor = &boss->battleData;
        anims = gBossAnimTables[actor->transform.modelID];
        gDPSetTextureFilter(gDisplayListHead++, G_TF_BILERP);
        DrawModel(1, &actor->transform, &anims[actor->transform.animID], mtxList);
        AdvanceAnimation(actor->transform.animFramerate, &actor->transform, &anims[actor->transform.animID]);
        if (actor->transform.base.monster->monsterType != MAMMON) {
            AddShadow(actor->transform.pos.x, actor->transform.pos.y, actor->transform.pos.z, actor->transform.shadowXRot, actor->transform.shadowYRot,
                          actor->transform.shadowZRot, actor->transform.collisionScale * actor->transform.scale);
        }
    }
}

s32 IsNearBoss(void) {
    s32 boss;
    f32 dx;
    f32 dz;

    boss = 0;
    if (gBossLoaded != 0) {
        dx = gBoss.battleData.transform.pos.x - gPlayerState.brian.transform.pos.x;
        dz = gBoss.battleData.transform.pos.z - gPlayerState.brian.transform.pos.z;
        if (sqrtf(dx * dx + dz * dz) < gBossTalkRadii[gBossLoaded]) {
            boss = gBossLoaded;
        }
    }
    return boss;
}
extern MonsterBaseData gBossBaseData[];    /* the bosses' base data */
extern MonsterBaseData D_803AB9A8;      /* Mammon's base data (boss 7) */
extern MonsterBaseData gMammonBaseData; /* Mammon's base data again (= D_803AB9A8): the second test names it so */
void PollWorldCollision(f32 sample_x, f32 sample_y, f32 sample_z, u16 arg3_flags, CollisionPollingResult* result);
void SetAnimation(ActorTransform *brian, s32 nextAnim, s32 nextFramerate, s32 afterAnim, s32 afterFramerate, u16 flags);
void SpawnBoss(s32 id, f32 x, f32 z, f32 yrot, EnemyAction* e) {
    MonsterBaseData* base;
    MonsterBattleData* m;

    base = &gBossBaseData[id];
    m = &e->battleData;
    e->battleData.transform.modelID = base->monsterID;
    SetAnimation(&m->transform, 0, 1, 0, 1, 1);
    m->transform.scale = base->modelScale;
    m->transform.collisionScale = base->radius;
    m->transform.base.monster = base;
    m->stats = &e->battleData.statsBuf;
    e->monBaseData = base;
    e->currHP = base->maxHP;
    e->currMP = base->maxHP;
    e->actionID = 0;
    e->flags = 0;
    e->movementBoundSize = sqrtf((u32)base->agi) * 3.846 + 17.838;
    e->battleData.statsBuf.statusFlags = 0;
    e->battleData.statsBuf.atk = e->monBaseData->atk;
    e->battleData.statsBuf.agi = m->transform.base.monster->agi;
    e->battleData.statsBuf.def = m->transform.base.monster->def;
    e->battleData.statsBuf.moveScale = 1.0f;
    PollWorldCollision(x, 0.0f, z, 0xA00, &gBossFloorPoll);
    m->transform.pos.x = gBossFloorPoll.x;
    m->transform.pos.y = gBossFloorPoll.y;
    m->transform.pos.z = gBossFloorPoll.z;
    if (base == &D_803AB9A8) {
        e->flags |= 4;
        m->transform.pos.y -= 30.0;
    }
    m->transform.yrot = yrot;
    m->transform.shadowXRot = gBossFloorPoll.sn_x;
    m->transform.shadowYRot = gBossFloorPoll.sn_y;
    m->transform.shadowZRot = gBossFloorPoll.sn_z;
    m->transform.actorFlags = 1;
    m->transform.unk60 = 0;
    if (base == &gMammonBaseData) {
        m->transform.actorFlags |= 4;
    }
}

u32 IsStoryFlagSet(u32 flag) {
    return (u8)(gStoryFlags[flag >> 3] & gBitMasks[flag & 7]);
}
void SetStoryFlag(u16 flag) {
    gStoryFlags[flag >> 3] |= gBitMasks[flag & 7];
}
extern u8* gBossSceneNames[];             /* boss scene names (0x20 bytes each) */
extern u8* gBossSceneTexts[];             /* boss scene texts (0x4B0 bytes each) */
extern u8 gTextBuffer[];
extern u8 gSpeakerName[];
extern CameraValues gBossCameraSetups[][2]; /* per boss: two camera setups */
void ApplyCameraSetup(CameraValues* setup, MonsterBattleData* target);
void StartBossScene(s32 boss) {
    s32 i;
    u8* src;
    u8* dst;

    src = gBossSceneNames[boss];
    dst = gSpeakerName;
    for (i = 0x20; i != 0; i--) {
        *dst++ = *src++;
    }
    src = gBossSceneTexts[boss];
    dst = gTextBuffer;
    for (i = 0x4B0; i != 0; i--) {
        *dst++ = *src++;
    }
    gBossSpawnTransform = *(BossSpawnTransform*)&gBoss.battleData;
    if (boss == 7) {
        gBossSpawnTransform.transform.pos.x = -31.0f;
        gBossSpawnTransform.transform.pos.y = 139.0f;
        gBossSpawnTransform.transform.pos.z = -328.0f;
    }
    ApplyCameraSetup(gBossCameraSetups[boss], (MonsterBattleData*)&gBossSpawnTransform);
}
s32 OpenMessageWindow(s32 type);
extern u16 gBossHpBonus[];
extern u16 gBossMpBonus[];
extern u16 gFieldMode;
extern u16 gTransitionStyle;
extern u16 gFadeOutAt;
extern u8 gLastBossDefeatText[];
extern BrianData1 gBrianStats;
extern s32 gGameState;
extern s32 gModeCountdown;
void OnBossDefeated(s32 boss) {
    u8* src;
    u8* dst;
    s32 count;

    if (boss == 7) {
        src = gBossSceneNames[boss];
        dst = gSpeakerName;
        count = 0x20;
        do {
            *dst++ = *src++;
            count--;
        } while (count != 0);

        src = gLastBossDefeatText;
        dst = gTextBuffer;
        count = 0x4B0;
        do {
            *dst++ = *src++;
            count--;
        } while (count != 0);

        OpenMessageWindow(1);
        gPlayerState.flags |= 0x10;
        gGameState |= 0x80;
        gModeCountdown = 0x40;
        gTransitionStyle = 3;
        gFadeOutAt = 0x3E;
        gFieldMode = 4;
    } else {
        gBrianStats.maxHP += gBossHpBonus[boss];
        if (gBrianStats.maxHP > 500) {
            gBrianStats.maxHP = 500;
        }
        gBrianStats.currHP = gBrianStats.maxHP;

        gBrianStats.maxMP += gBossMpBonus[boss];
        if (gBrianStats.maxMP > 500) {
            gBrianStats.maxMP = 500;
        }
        gBrianStats.currMP = gBrianStats.maxMP;
    }
}
