#include "common.h"
BSS_ORDER_PAD(0)
#include "game/boss.h"
#include "game/camera.h"
#include "game/collision.h"
#include "game/dma.h"
#include "game/message.h"
#include "game/model.h"
#include "game/shadow.h"
#include "game/boss_anims.h"
#include "game/boss_tables.h"
#include "game/enemy.h"
#include "game/main.h"
#include "game/map.h"
#include "game/math.h"
#include "game/player.h"

/* ---- boss.c's .bss (0x8007C0B0-0x8007C280), in address order. IDO 5.3 lays a unit's .bss
   out by its variables' block numbers, so a declaration added or removed before them, in a header too, can change
   their order; BSS_ORDER_PAD keeps the original order and `python3 tools/bss_order.py fix` recomputes it
   (docs/DECLARATIONS.md).
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
  BossPlacement *data;
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
        dma_write((u8*)fileStart, &gBossVAddr, fileSize); // load the boss file from the ROM
        SpawnBoss(i, data->x, data->z, data->yrot, &gBoss);
        gBossLoaded = i + 1;
      }
      return;
    }
  }

}
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
        if (sqrtf(dx * dx + dz * dz) < gBossTalkRadii[gBossLoaded - 1]) {
            boss = gBossLoaded;
        }
    }
    return boss;
}
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
    e->currMP = base->maxHP;   /* from maxHP: maxMP is 0 in every record */
    e->actionID = ENEMY_ACTION_IDLE;
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
        e->flags |= ENEMY_FLAG_SUNK;
        m->transform.pos.y -= 30.0;
    }
    m->transform.yrot = yrot;
    m->transform.shadowXRot = gBossFloorPoll.sn_x;
    m->transform.shadowYRot = gBossFloorPoll.sn_y;
    m->transform.shadowZRot = gBossFloorPoll.sn_z;
    m->transform.actorFlags = 1;
    m->transform.stateFlags = 0;
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
    for (i = MESSAGE_TEXT_SIZE; i != 0; i--) {
        *dst++ = *src++;
    }
    gBossSpawnTransform = *(BossSpawnTransform*)&gBoss.battleData;
    if (boss == LAST_BOSS) {
        gBossSpawnTransform.transform.pos.x = -31.0f;
        gBossSpawnTransform.transform.pos.y = 139.0f;
        gBossSpawnTransform.transform.pos.z = -328.0f;
    }
    ApplyCameraSetup(gBossCameraSetups[boss], (MonsterBattleData*)&gBossSpawnTransform);
}
void OnBossDefeated(s32 boss) {
    u8* src;
    u8* dst;
    s32 count;

    if (boss == LAST_BOSS) {
        src = gBossSceneNames[boss];
        dst = gSpeakerName;
        count = 0x20;
        do {
            *dst++ = *src++;
            count--;
        } while (count != 0);

        src = gLastBossDefeatText;
        dst = gTextBuffer;
        count = MESSAGE_TEXT_SIZE;
        do {
            *dst++ = *src++;
            count--;
        } while (count != 0);

        OpenMessageWindow(1);
        gPlayerState.flags |= PLAYER_MESSAGE_HOLD;
        gGameState |= GAME_STATE_MAP_CHANGE;
        gModeCountdown = 0x40;
        gTransitionStyle = TRANSITION_WHITE_SLOW;
        gFadeOutAt = 0x3E;
        gFieldMode = FIELD_MODE_LAST_BOSS;
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
