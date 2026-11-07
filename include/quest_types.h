/* Types and #defines from Quest 64's headers (Mallos31/Quest @401fe0e), generated when the
 * decomp.wtf project was set up and edited by hand since (Eltale's names, sizes and comments).
 * No extern/prototype lines: Quest's declarations use Quest addresses. */
#ifndef _QUEST_TYPES_H_
#define _QUEST_TYPES_H_
#include <ultra64.h>

/* ---- from Quest include/structs.h ---- */
typedef float MtxF_t[4][4];
typedef union {
    MtxF_t mf;
    struct {
        float xx, yx, zx, wx,
              xy, yy, zy, wy,
              xz, yz, zz, wz,
              xw, yw, zw, ww;
    };
} MtxF;
typedef struct {
Vec3f pos;
Vec3f rot;
} PosRot; /* unused (nothing references it); kept: the header typedef count orders IDO .bss */
typedef struct
{
    u8 red;
    u8 green;
    u8 blue;
    u8 alpha;
}Color_RGBA8; /* unused (nothing references it); kept: the header typedef count orders IDO .bss */
typedef struct unk_232f4_s{
    f32 x;
    f32 y;
}Coordinates2D;
typedef struct unk_1d924_s
{
    s32 unk0;
    s16 unk4;
    s16 unk6;
    s32 unk8;
    s32 unkC;
    s32 unk10;
    s32 unk14;
}unk1d924s; /* unused (nothing references it); kept: the header typedef count orders IDO .bss */
typedef struct {
    s32 type;                       /* 0 and 2 keep the camera's position */
    f32 x;
    f32 y;
    f32 z;
    f32 local_focus_height;         /* 0x10 */
    f32 local_camera_height;        /* 0x14 */
    f32 ideal_planar_distance;      /* 0x18 */
    f32 min_angle_x;                /* 0x1C */
    f32 max_angle_x;
    f32 min_angle_y;                /* 0x24 */
    f32 max_angle_y;
    Vec3f view;                     /* 0x2C: copied to the camera's +0x108 ([0] = view cone, degrees) */
} CameraSetup;                      /* 0x38: one entry of a map's camera table (LoadCameraSetup) */
/* a camera setup (camera.c ApplyCameraSetup applies it; boss.c keeps two per boss at gBossCameraSetups) */
typedef struct {
    s32 type;      /* 0 = follow the target, 1 = fixed position */
    f32 x;
    f32 y;
    f32 z;
    f32 local_focus_height;    /* 0x10 */
    f32 local_camera_height;   /* 0x14 */
    f32 ideal_planar_distance; /* 0x18 */
} CameraValues;
typedef struct {
    f32 x;
    f32 z;
    f32 angle;
    f32 halfWidth;
    f32 halfDepth;
    u16 flags;          /* 2: no facing test; 0x10 opens only with keyItem, 0x20 only without it (CheckDoorKeyItem) */
    u16 keyItem;        /* the item the 0x10 / 0x20 flags test */
    char unk18[2];
    u16 model;          /* the door model (index into gMapModels), copied to gPendingWarp.door when used */
    char unk1C[2];
    u16 dest[3];        /* 0x1E: map, submap and entrance the door leads to (SetDoorWarp) */
}MapDoor;              /* a door / warp trigger, 0x24 (gSubmapHeader->doors[]): a box of half size
                          halfWidth x halfDepth at (x, z), turned by angle (the way Brian must face) */
typedef struct {
    f32 x;
    f32 z;
    f32 yrot;
    char unkC[2];
    u16 unkE;               /* copied to gEntranceDoorModel */
    char unk10[4];
} MapEntrance;              /* 0x14: where Brian appears when he enters a submap (gSubmapHeader->entrances[gEntrance]) */
typedef struct {
    u16 modelSet;               /* this submap's model set (gMapHeader->modelSets[]) */
    u16 light;               /* its light record (gMapHeader->lights[]) */
    MapDoor* doors;
    s32 doorCount;          /* doors[] */
    MapEntrance* entrances;      /* entrances */
    char unk10[2];
    u16 area;               /* the area: its NPC blob (LoadNPCArea) and its monster set (LoadMonsterSet) */
    u16 flags;              /* copied to gSubmapFlags: bit 0 a town; 2, 4, 8 tested by the camera,
                               the Return / Exit spells and the HUD */
}Submap;                 /* a submap, 0x18 (gMapHeader->submaps[], gSubmapHeader) */
typedef struct FontStrip {
     char texels[0x7B8];         /* 304 x 13 texels, CI4 */
} FontStrip;                     /* one strip of the font sheet (gFontSheet[], DrawString) */
/* ---- from Quest include/player.h ---- */
typedef struct {
    u8 unk0[0x18];
}AIScript;
typedef struct {            /* one move an enemy's AI can pick (names from vbhayden's Quest 64 scratches) */
    u16 action_tag;         /* 0x00: 0x8000 = a spell; low byte: a boss's special animation (StartBossEffectScript) */
    u16 move_flags;         /* 0x02: 2 = keeps moving while winding up, 0xA = while casting (EnemyCastAttack) */
    u16 cast_delay;         /* 0x04: frames before it acts */
    u16 hold;               /* 0x06: frames held after the cast when next_anim is set (EnemyCastAttack) */
    s16 anim;               /* 0x08 */
    s16 next_anim;          /* 0x0A: -1 = back to animation 0 */
    s16 recover_anim;       /* 0x0C: played after the hold (EnemyCastAttack) */
    char unkE[2];
    f32 max_range;          /* 0x10 */
    f32 hit_range;          /* 0x14: a melee hit's reach, times the attacker's scale (EnemyMeleeAttack) */
} EnemyAIOption;            /* 0x18: MonsterBaseData.AIScript points at aiScriptNumber of these (PickEnemyAIOption) */
typedef struct {
     u16 monsterType; 
     s16 monsterID;  /* loaded signed everywhere (SpawnEnemy, SpawnBoss, the battle card) */
     u16 currHP;
     u16 maxHP;
     u16 currMP;     /* 0x08 (was f32 unk8; the MP drain spell SpellEffect_DrainMP reads both as u16) */
     u16 maxMP;      /* 0x0A */
     u16 agi;
     u16 def;
     s32 expValue;
     s32 stone;      /* 0x14: the stone it gives when defeated (EnemyKnockback: gBattleStone) */
     f32 radius;     /* 0x18: horizontal hit radius in model units: distance tests, floor polls, 2x = effect size
                        (Mallos31's Quest decomp: hitboxHeight; vbhayden's scratches: hitboxWidth) */
     f32 height;     /* 0x1C: hit height: 0.5x mid-body, 1x the over-enemy icon, 2x where effects start above
                        (Quest decomp: hitboxWidth) */
     f32 modelScale;
     u16 aiFlags;           /* 4 = pick an option at random (PickEnemyAIOption) */
     u16 element; 
     s16 hitAnim;           /* the animation it plays when hit (DamageEnemy) */
     u16 atk; 
     u16 aiScriptNumber;
     u16 unk2E;
     EnemyAIOption* aiOptions;
     u8 itemDrop;
     u8 unk35;
     u16 unk36;
} MonsterBaseData; 
typedef struct {
     u16 unk0;
     s16 unk2;
     u16 currHP;
     u16 maxHP;
     u16 currMP;
     u16 maxMP;
     u16 agi;
     u16 def;
     s32 exp;
     s32 stone;             /* 0x14: the pre-release currency "stone", unused (Quest 64 decomp community name) */
     f32 radius;            /* 0x18: Brian's collision radius (copied to ActorTransform.collisionScale); was colissionSize */
     f32 height;            /* 0x1C: as MonsterBaseData's: effects start at fractions of height x scale */
     f32 modelScale;        /* 0x20: InitBrian copies it into his transform's scale (MonsterBaseData's modelScale) */
     u8 spirits[4];         /* 0x24: fire, earth, water, wind */
     u16 HPXP;
     u16 MPXP;
     u16 agiXP;
     u16 defXP;
     u8 HPLevel;
     u8 MPLevel;
     u8 agiLevel;
     u8 defLevel;
     u8 spiritLevel;
     u8 unk35;
     u8 unk36;
     u8 unk37;
}BrianData1;    /* 0x38 in Eltale: Brian's state block (PlayerState, gPlayerState) follows it at gBrianStats + 0x38, and the save
                   slot record copies exactly these 0x38 bytes (WriteSaveRecord). Quest's version ran on to 0x4C. */
typedef struct {                /* an actor's place, motion, model and animation (vbhayden's Quest 64 scratches:
                                   ActorTransform; was BrianData2): the head of MonsterBattleData (Brian, enemies),
                                   and the whole transform of an NPC or a map object */
     Vec3f pos;             /* 0x00 */
     f32 xrot;
     f32 yrot;
     f32 zrot;
     f32 xspeed;
     f32 yspeed;
     f32 zspeed;
     f32 scale;             /* 0x24 */
     f32 collisionScale;    /* 0x28: entity-to-entity collision size (times scale; enemy.c), also the shadow size; was shadowScale */
     f32 shadowXRot;
     f32 shadowYRot;
     f32 shadowZRot;
     f32 unkDoor38;         /* 0x38-0x40: the root part's key position this frame, times the scale (ApplyRootMotion) */
     f32 unkDoor3C;
     f32 unkDoor40;
     f32 unkDoor44;         /* 0x44-0x4C: the root part's move since the last frame (ApplyRootMotion); a map object
                               keeps its door point's x / z in 0x44 / 0x4C (FindMapObjectInFront) */
     f32 unkDoor48;
     f32 unkDoor4C;
     s16 modelID;           /* 0x50 */
     s16 animID;
     u16 animFrame;
     u16 totalAnimFrames;
     u16 animFramerate;      /* 0x58 */
     s16 afterAnim;          /* 0x5A: the animation to play when this one ends (SetAnimation) */
     u16 afterFramerate;     /* 0x5C: its framerate (AdvanceAnimation reads it unsigned) */
     u16 actorFlags;         /* 0x5E: 1 = an enemy (not Brian), 4 = the last boss */
     u16 unk60;              /* 0x60: flags; 0x200 / 0x400 are set by SetAnimation / AdvanceAnimation and steer
                                ApplyRootMotion */
     u16 floorFlags;         /* 0x62: the floor poll's flags (Brian, enemies; bits 6-8 = the floor kind); a map object
                                keeps its opened-flag id here (IsMapObjectOpened) */
     union {
         MonsterBaseData* monster;   /* an enemy's: its entry in the monster table */
         BrianData1* player;         /* Brian's: gBrianStats (InitBrian) */
     } base;                 /* 0x64: the actor's base data. The two layouts agree up to 0x24 (HP, MP, agility,
                                defence, exp, radius, height, modelScale), and code that runs for any actor reads
                                that part through base.monster */
} ActorTransform;           /* 0x68 */
typedef struct unk_D_80086DC0_s
{
    s32 mode;                       /* 0x00: the camera mode (a setup's type; 4 = spell, 5 = transition) */
    s32 timer;                      /* 0x04: frames of a timed override (flag 0x40) */
    u16 flags;                      /* 0x08: 0x10, 0x40 = set up / following (InitRoomCamera, SnapSpellCamera) */
    s16 unkA;
    f32 cam_x;                      /* 0x0C: the camera position (vbhayden: cam_x) */
    f32 cam_y;
    f32 cam_z;
    f32 focus_x;                    /* 0x18: the point it looks at */
    f32 focus_y;
    f32 focus_z;
    f32 pitch;                      /* 0x24 */
    f32 target_yaw;                 /* 0x28: the target's yaw (ClampCameraToTarget keeps the camera within 45 deg of it) */
    f32 yaw;                        /* 0x2C: the camera's yaw, radians (DrawMapProps view cone) */
    f32 unk30;  /* zeroed by InitRoomCamera */
    f32 focus_speed_y;              /* 0x34: the focus height's speed (CameraFollowHeight) */
    f32 unk38;
    f32 orbit_speed;                /* 0x3C: (CameraTurnToAngle) */
    f32 cam_speed_x;  /* 0x40 */
    f32 cam_speed_y;  /* 0x44 */
    f32 cam_speed_z;  /* 0x48 */
    f32 ideal_planar_distance; /* 0x4C */
    f32 unk50;
    f32 orbit_yaw;                  /* 0x54: the angle the camera offset is turned by around the target */
    f32 local_camera_height;   /* 0x58 */
    f32 local_focus_height;    /* 0x5C */
    f32 min_angle_x;  /* 0x60 */
    f32 max_angle_x;  /* 0x64 */
    f32 min_angle_y;  /* 0x68 */
    f32 max_angle_y;  /* 0x6C */
    f32 pull;                  /* 0x70: towards the wanted position */
    f32 drag;                  /* 0x74: the share of its speed it keeps per frame */
    f32 orbit_pull;            /* 0x78 */
    f32 orbit_drag;            /* 0x7C */
    ActorTransform* target_transform;    /* 0x80 */
    f32 unk84;                 /* (ClampCameraToTarget: the target's yaw turned round, within -pi .. pi) */
}Camera; /* camera state, 0x88 (camera.c's gCamera); field names from vbhayden's Quest 64 scratches.
                   Quest's camera goes on to 0x120; Eltale keeps those parts as separate variables (camera.c's
                   gProjectionMtxF ... gCameraSetups, each with its own lui in LoadCameraSetup). */
/* ---- from player.c ---- */
/* ---- from Quest include/monsters.h ---- */
#define ON_GROUND 0
#define FLYING 1
#define MAMMON 2
typedef struct {
    s16 timer;              /* a view of ProjSlot (effects.c) */
    s16 unk2;
    s16 unk4;
    s16 unk6;
    u16 flags;
    char unkA[2];
    f32 x;                  /* 0x0C: kept on the monster (DrawMonsterStatusIcons) */
    f32 y;
    f32 z;
}StatusFxObject;
typedef struct {
 u16  statusFlags;    /* 0x00: status bits; bit 0 halves Brian's staff damage (CalcStaffPower) */
 u8 statusTimers[0x10];       /* 0x02: per-status counters (CountDownStatuses counts them down) */
 u8 status[7];
 char unk19[0x9];
 s8 statusTimer[7];   /* 0x22: frames left per status, 0 = no limit (DrawMonsterStatusIcons counts them down) */
 char unk29[0x9];
 s8 statusFxTimer[0x12];
 StatusFxObject* statusFx[4];
 char unk54[0x30];
 u16 atk;      /* 0x84: times atkScale >> 4 = an enemy's hit (CalcEnemyMeleeDamage); Brian's is the four
                  spirit levels' total, his staff power (SetupBrianForBattle, CalcStaffPower) */
 u16 agi;
 u16 def;      /* 0x88; Brian's agi / def are copied from his stats at battle start */
 u16 atkScale; /* 0x8A: attack scale in 16ths: 0x10 at battle start (SetupBrianForBattle), 0x18 /
                  0x20 from a spell (spells.c), back to 0x10 when it ends */
 u16 hitMask;    /* 0x8C: spell hit mask (gHitMaskBits bits: TestSpellHitMask / SetSpellHitMask;
                  ClearHitMasks clears Brian's and every enemy's) */
 char unk8E[2];
 f32 moveScale; /* 0x90: movement scale, 1.0; 0.5 / 2.0 from a spell (spells.c); Brian's battle
                   movement radius is PlayerState.moveRadius times this */
 f32 flyY;
} ActorStats;
typedef struct {
     ActorTransform transform;     /* 0x00: place, model, animation, base data */
     ActorStats* stats;            /* 0x68: points at statsBuf (SpawnEnemy; InitBrian for Brian) */
     ActorStats statsBuf;          /* 0x6C */
} MonsterBattleData;               /* 0x104: an actor in battle, an enemy's (EnemyAction.battleData) or Brian's
                                      (PlayerState.brian = gBrian); the name is the Quest 64 decomps' */
typedef struct {
     u16 actionID;   /* 0x00: the handler (gEnemyActionHandlers) */
     s16 actionTimer;
     u16 tries;      /* 0x04: AI attempts left */
     u16 phase;      /* 0x06: the attack's step (EnemyMeleeAttack / EnemyCastAttack switch on it) */
     u16 flags;      /* 0x08: 2 = give up */
     u16 currHP;     /* 0x0A */
     u16 currMP;     /* 0x0C (was maxHP; drained by SpellEffect_DrainMP, capped by the base data's maxMP) */
     u16 unkE;
     f32 movementBoundSize;
     s32 unk14;
     s32 unk18;
     EnemyAIOption* defaultOption;   /* 0x1C */
     f32 pushX;      /* 0x20: a push the boss effect script gives (flag 8); MoveEnemy adds it and decays it by 0.9 */
     f32 pushZ;      /* 0x24; battleData is at 0x2C and the struct is 0x130 (enemy table gEnemies) */
     MonsterBaseData* monBaseData;
     MonsterBattleData battleData;
} EnemyAction; 
typedef struct PlayerState {        /* Brian's state block (gPlayerState; Quest's BrianRoot) */
    u16 state;                      /* 0x00 (DrawBrian reads it with lhu) */
    u16 staffTarget;                       /* 0x02: the enemy targeted by the staff, 1-based (BrianState_StaffSwing) */
    u16 timer;                      /* 0x04: frames left in the state */
    u16 animCue;                       /* 0x06: an animation cue (SetAnimation plays it; the Exit / Return spells
                                       clear it; Quest names this offset doorSide) */
    s32 flags;                      /* 0x08: bit 0x20 = left the battle circle (escape); 0x1000 << element =
                                       that element at 50 spirits (gSpiritMaxBits), 0x10000 = all four
                                       (both set by SpiritGainWindow) */
    f32 moveRadius;                       /* times brian.statsBuf.moveScale: Brian's battle movement radius */
    s32 comboWindow;                      /* 0x10: battle start: spirit total less each element's excess over total / 4,
                                       then / 10 + 1, at most 16 (SetupBrianForBattle) */
    BrianData1* stats;              /* 0x14: gBrianStats */
    MonsterBattleData brian;        /* 0x18: Brian in battle and in the field (gBrian): his
                                       stats pointer at 0x80 points at the statsBuf at 0x84 */
    u8 itemId;
    u8 path[3];                     /* 0x11D: the spell's directions so far, 0-3 (BrianSpellInput) */
    u16 spellPresses;               /* 0x120: 1-3 button presses: the MP cost index (gSpellMpCosts) */
    u16 spellId;                    /* 0x122: the spell being cast: element << 8 | index into gSpellDefs */
    u16 subTimer;
    char unk126[2];
    u16 floorTimer;                 /* 0x128: frames to the next hit from a damaging floor (ApplyFloorEffect) */
    char unk12A[2];
    f32 stepX;
    f32 stepZ;
    f32 savedYRot;                  /* 0x134 */
    f32 endYaw;
} PlayerState;
/* ---- from Quest include/npc.h ---- */
typedef struct {
    char unk0[0x2];
    s16 unk2;
    char unk4[0xA];
    s16 unkE;
    
}NPCData3; /* unused (nothing references it); kept: the header typedef count orders IDO .bss */
typedef struct {
    char unk0[0x4];
    u8 unk4;
    char unk5[0xB];
    NPCData3* unk10;
    char unk14[4];
    u16 unk18;
}NPCData2; /* unused (nothing references it); kept: the header typedef count orders IDO .bss */
typedef struct {
u16 action;
u16 actionTimer;
f32 xWanderOrigin; 
f32 zWanderOrigin; 
f32 yRotReturn; 
f32 unk10; 
PosRot PosRot;
f32 unk2C; 
f32 unk30;
f32 unk34;
f32 scale;
f32 collisionSize;
Vec3f shadowRot;
char unk4C[0x14];
u16 unk60;
char unk62[4];
s16 animID;
u16 animFrames;
u16 totalFrames;
u16 animSpeed;
s16 unkAnimID; 
s16 unk70;
char unk72[0x2];
u16 unk74;
char unk76[0xA];
NPCData2* unk80;    
}NPCData; /* unused (nothing references it); kept: the header typedef count orders IDO .bss */
/* ---- from Quest include/hud.h ---- */
#define U_CBUTTONS	CONT_E
#define L_CBUTTONS	CONT_C
#define R_CBUTTONS	CONT_F
#define D_CBUTTONS	CONT_D
typedef struct {
    void* image;            /* 0x00: the CI texture (LoadHUDTexture) */
    u16 ci4;                /* 0x04: 0 = CI8, else CI4 (loaded as 8-bit at half width) */
    u16 unk6;
    u32 width;              /* 0x08: texels per row */
    s32 unkC;
}HudImage;
typedef struct {
    s16 unk0;
    s16 unk2;
    s16 unk4;
    u16 unk6;
    u16 unk8;
    s16 unkA;
    u16 unkC;
    u16 unkE;
    char unk10[0x16];
    u16 unk26;
    u16 unk28;
    u16 unk2A;
}unk203d0s2; /* unused (nothing references it); kept: the header typedef count orders IDO .bss */
typedef struct {
    char unk0[0x20];
    unk203d0s2* unk20;
    char unk24[0x6C];
    u16 unk90;    
}unk203d0s; /* unused (nothing references it); kept: the header typedef count orders IDO .bss */
typedef struct unk_20888_s{
    u16 displayTimer;
    s16 unk2;
    Vec3f* pos;
    f32 offsetX;
    f32 offsetY;
    f32 offsetZ;
    u16 texS;       /* where the icon is on the status symbols image */
    u16 texT;
    u16 width;
    u16 height;
}MissIcon;
typedef struct {
    char unk0[0x24];
    u8 unk24;
    u8 unk25;
    u8 unk26;
    u8 unk27;
}unk1f3dcTEST2; /* unused (nothing references it); kept: the header typedef count orders IDO .bss */ 
typedef struct temp4 {
 u8 unk_00;
 char unk_01[5];
} temp4;
typedef struct temp2 {
 char unk_00[0x14];
 Vec3f unk_14;
 char unk_20[0x64];
} temp2; 
/* ---- from Quest include/inventory.h ---- */
typedef struct unk_22260_s3{
    u16 canBeUsed;
    u16 type;
    s16 itemArg1;
    s16 itemArg2;
    u16 unk8;
    u16 unkA;
}ItemData;
/* ---- from Quest include/bgm.h ---- */
typedef struct {
    s16 map;
    s16 submap;
    s8 bgm;
    s8 pad;
}bgmData; /* unused (nothing references it); kept: the header typedef count orders IDO .bss */
typedef struct unk26658_s{
    u8 delay;
    s8 id;
    u8 volume;
}DelayedSound;
/* ---- from Quest include/31A10.h ---- */
typedef short int16_t;
typedef int16_t   qs510_t;
#define qs510(n)                      ((qs510_t)((n)*0x0400))
typedef struct {
    s32 unk0;
    s32 unk4;
    s32 unk8;
    s32 unkC;
}temps; /* unused (nothing references it); kept: the header typedef count orders IDO .bss */
typedef struct TextPalette {
     char colours[0x20];         /* 16 x RGBA5551 */
} TextPalette;                   /* unused: gTextPalettes is read as u16 colours; kept: the header typedef count orders IDO .bss */
/* ---- from Quest include/screen_dim.h ---- */
#define SCREEN_DIMMABLE 1
#define USE_TIMER 2
#define SCREEN_BRIGHTENING 4
/* ---- from Quest include/nnsched.h ---- */
#include <ultra64.h>
#define NN_SC_STACKSIZE    0x2000  
#define NN_SC_SWAPBUFFER   0x0040  
#define NN_SC_RETRACE_MSG  1  
#define NN_SC_DONE_MSG     2  
#define NN_SC_PRE_NMI_MSG  3  
#define NN_SC_GTASKEND_MSG 4  
#define NN_SC_MAX_MESGS    8  
#define NN_SC_STACKSIZE    0x1000
#define OS_SC_SWAPBUFFER   0x0040  /* as in the SDK sched.h */
/* gGameMode (Quest64-Decomp's names, the last two are Eltale's) */
#define MODE_GAMEPLAY      1
#define MODE_PAUSED        2
#define MODE_TITLE         3
#define MODE_SPLASH        4
#define MODE_CREDITS       5
#define MODE_ENDING        6
#define MODE_AFTER_CREDITS 7
#define NN_SC_PRI          120  
#define NN_SC_AUDIO_PRI    110  
#define NN_SC_GRAPHICS_PRI 100  
#define VIDEO_MSG       666  
#define RSP_DONE_MSG    667  
#define RDP_DONE_MSG    668  
#define PRE_NMI_MSG     669  
#define NN_SC_PERF
#define NN_SC_PERF_NUM    4
#define NN_SC_GTASK_NUM   8   
#define NN_SC_AUTASK_NUM   4  
typedef short NNScMsg;  
typedef struct SCClient_s {  
    struct SCClient_s   *next;  
    OSMesgQueue         *msgQ;  
} NNScClient;
typedef struct SCTask_s {  
    struct SCTask_s     *next;  
    u32                 state;
    u32			flags;
    void		*framebuffer;  
    OSTask              list;
    OSMesgQueue         *msgQ;
    OSMesg              msg;
} NNScTask;
typedef struct {  
  
  NNScMsg   retraceMsg;
  NNScMsg   prenmiMsg;
  
  OSMesgQueue audioRequestMQ;
  OSMesg      audioRequestBuf[NN_SC_MAX_MESGS];
  OSMesgQueue graphicsRequestMQ;
  OSMesg      graphicsRequestBuf[NN_SC_MAX_MESGS];
  
  OSMesgQueue retraceMQ;
  OSMesg      retraceMsgBuf[NN_SC_MAX_MESGS];
  OSMesgQueue rspMQ;
  OSMesg      rspMsgBuf[NN_SC_MAX_MESGS];
  OSMesgQueue rdpMQ;
  OSMesg      rdpMsgBuf[NN_SC_MAX_MESGS];
  
  OSMesgQueue waitMQ;
  OSMesg      waitMsgBuf[NN_SC_MAX_MESGS];
  
  OSThread    schedulerThread;	
  OSThread    audioThread;	
  OSThread    graphicsThread;	
  
  NNScClient  *clientList;
  
  NNScTask    *curGraphicsTask;
  NNScTask    *curAudioTask;
  NNScTask    *graphicsTaskSuspended;
  
  
  u32         firstTime;  
} NNSched;
typedef struct {
  u32 gtask_cnt;                        
  u32 autask_cnt;                       
  u64 retrace_time;                     
  u64 gtask_stime[NN_SC_GTASK_NUM];   
  u64 rdp_etime[NN_SC_GTASK_NUM];     
  u64 rsp_etime[NN_SC_GTASK_NUM];     
  u64 autask_stime[NN_SC_AUTASK_NUM]; 
  u64 autask_etime[NN_SC_AUTASK_NUM]; 
  u32 endflag;
} NNScPerf; /* unused (nothing references it); kept: the header typedef count orders IDO .bss */
#ifdef NN_SC_PERF
#endif 
/* ---- from Quest include/guint.h ---- */
typedef union
{
	struct
	{
		unsigned int hi;
		unsigned int lo;
	} word;

	double	d;
} du; /* unused (nothing references it); kept: the header typedef count orders IDO .bss */
typedef union
{
	unsigned int	i;
	float		f;
} fu; /* unused (nothing references it); kept: the header typedef count orders IDO .bss */
#ifndef __GL_GL_H__
typedef	float	Matrix[4][4];
#endif
#define ROUND(d)	(int)(((d) >= 0.0) ? ((d) + 0.5) : ((d) - 0.5))
#define	ABS(d)		((d) > 0) ? (d) : -(d)
/* ---- from Quest include/osint.h ---- */
#include <os_internal.h>
typedef struct __OSEventState
{
    OSMesgQueue *messageQueue;
    OSMesg message;
} __OSEventState;
/* ---- from Quest include/viint.h ---- */
#include <os_internal.h>
#define OS_TV_TYPE_PAL 0
#define OS_TV_TYPE_NTSC 1
#define OS_TV_TYPE_MPAL 2
#define VI_STATE_01 0x01
#define VI_STATE_XSCALE_UPDATED 0x02
#define VI_STATE_YSCALE_UPDATED 0x04
#define VI_STATE_08 0x08         
#define VI_STATE_10 0x10         
#define VI_STATE_BLACK 0x20      
#define VI_STATE_REPEATLINE 0x40 
#define VI_STATE_FADE 0x80       
#define VI_CTRL_ANTIALIAS_MODE_3 0x00300 
#define VI_CTRL_ANTIALIAS_MODE_2 0x00200 
#define VI_CTRL_ANTIALIAS_MODE_1 0x00100 
#define VI_SCALE_MASK 0xfff 
#define VI_2_10_FPART_MASK 0x3ff
#define VI_SUBPIXEL_SH 0x10
#define BURST(hsync_width, color_width, vsync_width, color_start) \
    (hsync_width | (color_width << 8) | (vsync_width << 16) | (color_start << 20))
#define WIDTH(v) v
#define VSYNC(v) v
#define HSYNC(duration, leap) (duration | (leap << 16))
#define LEAP(upper, lower) ((upper << 16) | lower)
#define START(start, end) ((start << 16) | end)
#define FTOFIX(val, i, f) ((u32)(val * (f32)(1 << f)) & ((1 << (i + f)) - 1))
#define F210(val) FTOFIX(val, 2, 10)
#define SCALE(scaleup, off) (F210((1.0f / (f32)scaleup)) | (F210((f32)off) << 16))
#define VCURRENT(v) v 
#define ORIGIN(v) v
#define VINTR(v) v
#define HSTART START
typedef struct
{
     f32 factor;
     u16 offset;
     u32 scale;
} __OSViScale;
typedef struct
{
     u16 state;
     u16 retraceCount;
     void *framep;
     OSViMode *modep;
     u32 control;
     OSMesgQueue *msgq;
     OSMesg msg;
     __OSViScale x;
     __OSViScale y;
} __OSViContext;
/* ---- typedefs from Quest .c files under src ---- */
/* 12740.c */
typedef struct unk_1205c_s
{
    char unk0[0x60];
    u16 unk60;
    u16 unk62;
}unk1205cs; /* unused (nothing references it); kept: the header typedef count orders IDO .bss */
/* 12740.c */
typedef struct {
    f32 x;
    f32 y;
    f32 z;
    f32 randomOffset;
    u16 pickedUp;
    s16 animTimer;
    u8 flag;                        /* 0x14: its found flag (SetSpiritFound) */
    char unk15[0x3];
}Spirit;
/* 12740.c */
typedef struct {
    f32 unk0;
    f32 unk4;
    f32 unk8;
    f32 unkC;
    f32 unk10;
}unk12358s; /* unused (a view of a shared type); kept: removing it moves IDO's .bss order */
/* 12740.c */
typedef struct unkD_80086EE0_s
{
    s32 phase;                  /* CameraMode_AfterBattle: 0 = waiting, 1 = easing back */
    s32 timer;                  /* frames left in the phase */
    s32 savedMode;              /* the camera mode to restore */
    u16 savedFlags;             /* the camera flags to restore (lhu) */
    s16 unkE;
    f32 orbitYaw;               /* the camera's orbit yaw for the transition (the yaw when the battle began) */
    f32 orbitPull;
    f32 orbitDrag;
}CameraTransition;
/* 15440.c */
typedef struct {
    s32 unk0;
    u16 unk4;
    char unk6[0x36];
}unk15b50s; /* unused (nothing references it); kept: the header typedef count orders IDO .bss */
/* 15440.c */
typedef struct unk_16ffc_s{
    char unk0[0x24];
    s32 unk24;
    s32 unk28;
}unk16ffcs; /* unused (nothing references it); kept: the header typedef count orders IDO .bss */
/* 15440.c */
typedef struct unk_164e4_s{
    char unk0[0xA];
    u16 unkA;
}unk164e4s; /* unused (nothing references it); kept: the header typedef count orders IDO .bss */
/* 15440.c */
typedef struct {
    f32 unk0;
    f32 unk4;
    f32 unk8;
}tempStruct14F48_3; /* unused (replaced by SpellSlot / SpellDef / MonsterBattleData / SpellFx); kept: removing it can move IDO's .bss order */
/* 15440.c */
typedef struct {
    u16 unlockLevel;    /* 0x00: the element level needed */
    u16 flags;          /* 0x02: 1 battle, 2 field, 4 carries on after a hit, 8 aim by its own direction */
    u16 presses;        /* 0x04: 1-3, the MP cost index */
    u8 path[3];         /* 0x06: the directions; path[0] = the element */
    u8 unk9;
    u16 lifetime;       /* 0x0A: frames (object +0x04) */
    u16 power;          /* 0x0C */
    u16 hitRate;        /* 0x0E: % */
    u16 effect;         /* 0x10: effect routine */
    u16 movement;       /* 0x12: movement routine */
    u16 heightSnapping; /* 0x14: 1 = stop at the ground */
    char unk16[2];
    u16 targetType; /* Quest: x2C_spell_def->targetType (2 = self) */
    char unk1A[2];
    f32 unk1C;
    char unk20[0xA];
    u16 gravity;    /* 0x2A: bit 0 = falls (Quest "angle") */
}tempStruct14F48_2; /* unused (replaced by SpellSlot / SpellDef / MonsterBattleData / SpellFx); kept: removing it can move IDO's .bss order */ /* spell definition */
typedef struct {
    u16 unk0;
    char unk2[6];
    u16 unk8;
} tempStruct14F48_4; /* unused (replaced by SpellSlot / SpellDef / MonsterBattleData / SpellFx); kept: removing it can move IDO's .bss order */ /* spell effect object (Quest unk34) */
/* 15440.c */
typedef struct {
    s16 unk0;
    s16 unk2;
    u16 range;      /* 0x04 */
    u16 unk6;
    u16 unk8;
    u16 flags;      /* 0x0A: bit 0 = hit the ground */
    f32 unkC;       /* x */
    f32 unk10;      /* y */
    f32 unk14;      /* z */
    f32 vx;         /* 0x18 */
    f32 vy;         /* 0x1C */
    f32 vz;         /* 0x20 */
    s32 caster; /* Quest: x24_caster */
    tempStruct14F48_3* unk28;
    tempStruct14F48_2* unk2C;
    s32 unk30;
    tempStruct14F48_4* unk34;
}tempStruct14F48; /* unused (replaced by SpellSlot / SpellDef / MonsterBattleData / SpellFx); kept: removing it can move IDO's .bss order */ /* spell object; field names from vbhayden's Quest 64 scratches */
/* 1850.c */
    typedef unsigned            uintptr_t;
/* 1850.c */
typedef struct OSScTask_s {
    struct OSScTask_s   *next;          
    u32                 state;
    u32			flags;
    void		*framebuffer;	

    OSTask              list;
    OSMesgQueue         *msgQ;
    OSMesg              msg;
#ifndef _FINALROM                       
    OSTime              startTime;      
    OSTime              totalTime;      
#endif                                  
} OSScTask;                             
/* 1850.c */
typedef struct {
    OSScTask task;
    char unk58[0xD0F0];
    OSMesg unkD148;
    void *unkD14C; 
}unk111Cs;
/* 191F0.c */
typedef struct {
    u8 unk0;
    u8 unk1;
    u8 unk2;
    u8 unk3;
}unk19918s; /* unused (nothing references it); kept: the header typedef count orders IDO .bss */
/* 191F0.c */
typedef struct {
    s32 unk0;
    s32 unk4;
    s32 unk8;
    s32 unkC;
    s32 unk10;
    s32 unk14;
    s32 unk18;
    s32 unk1C;
}unk18684s; /* unused (nothing references it); kept: the header typedef count orders IDO .bss */
/* 191F0.c */
typedef struct {
    s32 unk0;
    char unk4[0x14];
    s32 unk18;
    char unk1C[0x14];
    s32 unk30;
    char unk34[0x14];
    s32 unk48;
    char unk4C[0x14];
}unk19324s; /* unused (nothing references it); kept: the header typedef count orders IDO .bss */
/* 191F0.c */
typedef struct {
    u16 unk0;
    s16 unk2;
    s16 unk4;
    s16 unk6;
    s16 unk8;
    s16 unkA;
    f32 unkC;
    f32 unk10;
    f32 unk14;
    f32 unk18;
    f32 unk1C;
    f32 unk20;
    f32 unk24;
    f32 unk28;
    f32 unk2C;
    f32 unk30;
    s32 unk34; 
    s16 unk38;
    s16 unk3A;
    s32 unk3C;
    s32 unk40;
    s32 unk44;
    s32 unk48;
    s32 unk4C;
    s32 unk50;
    s32 unk54;
}unk1a238s; /* unused (nothing references it); kept: the header typedef count orders IDO .bss */
/* 191F0.c */
/* 191F0.c */
typedef struct {
 MonsterBattleData* target;   /* the actor the digits float over (DrawDamageNumbers reads its stats->flyY) */
 u8 damageDigits[8];
 u16 damageDisplayTimer;
 char unkE[2];
 s32 kind;
 s32 numDigits;
 s32 stackIndex;    
}DamageDigs; 
/* 191F0.c */
typedef struct {
    s32 unk0;
    f32 unk4;
    f32 unk8;
    f32 unkC;
    f32 unk10;
    void* unk14;
} unkSmoke; /* unused (nothing references it); kept: the header typedef count orders IDO .bss */ 
/* 191F0.c */
/* 1D160.c */
typedef struct {
    s16 x;
    s16 z;
    s16 width;
    s16 depth;
    u16 encounters[8];   /* 0x08: the region's encounter table (CreateEncounter) */
}EncounterZone;
/* 1D160.c */
typedef struct {
    EncounterZone* zones;
    s32 count;
}EncounterZoneList;
/* 1D160.c */
typedef struct unk_1c560_s{
    char unk0[0x30];
    s32 unk30;
    s32 unk34;
}unk1c560s; /* unused (nothing references it); kept: the header typedef count orders IDO .bss */ 
/* 1D160.c */
typedef struct {
    void* stats;
    Vec3f* pos;
    Vec3f* rot;
    f32 unk1C;
    f32 unk20;
    f32 unk24;
    f32 scale;
    f32 collisionScale;
    Vec3f* shadowPos;
    f32 unk38;
    f32 unk3C;
    f32 unk40;
    f32 unk44;
    f32 unk48;
    f32 unk4C;
    s16 modelID;
    s16 animID;
    u16 animFrame;
    u16 totalFrameCnt;
    u16 animSpeed;
    u16 nextAnimID;
    s16 unk5C;
    s16 unk5E;
    u16 unk60;
    u16 unk62;
    MonsterBaseData *unk64;
    ActorStats *unk68;
    ActorStats unk6C;
}unkEnemy; /* unused (nothing references it); kept: the header typedef count orders IDO .bss */
/* 1D160.c */
typedef struct {
    s16 unk0;
    s16 unk2;
}unkEncounterSomething; /* unused (nothing references it); kept: the header typedef count orders IDO .bss */
/* 1D160.c */
typedef struct {
    void* unk0;
    void* unk4;
    unkEncounterSomething* unk8;
    s32 unkC; 
}unkEncounterData; /* unused (nothing references it); kept: the header typedef count orders IDO .bss */
/* 1E4B0.c */
typedef struct {
    void* unk0;
    u16 unk4;
    u16 unk6;
    f32 unk8;
    f32 unkC;
    f32 unk10;
    s8 unk14;
    s8 unk15;
    u8 unk16;
    char unk17; 
    s32 unk18;
    char unk1C[4];
}unk1e138s; /* unused (nothing references it); kept: the header typedef count orders IDO .bss */
/* 1E4B0.c */
typedef struct {
    char unk0[0x52];
    s16 unk52;
    u16 unk54;
    u16 unk56;
    s16 unk58;
    s16 unk5A;
    u16 unk5C;
    s16 unk5E;
    u16 unk60;
}unk1e0acs; /* unused (a view of a shared type); kept: removing it moves IDO's .bss order */
/* 1E4B0.c */
typedef struct {
    s16 unk0;
    s16 unk2;
}unk1e0acs2; /* unused (a view of a shared type); kept: removing it moves IDO's .bss order */
/* 23740.c */
typedef struct {
 Vec3f pos;
 f32 yrot;
 f32 zrot;
 f32 xrot;
 Vec3f scale;
 f32 unk24;
 f32 unk28;
 f32 unk2C;
 s16 unk30; 
 u16 startingFrame;
}LimbKeyframe; /* unused (nothing references it); kept: the header typedef count orders IDO .bss */
/* 23740.c */
typedef struct {
    LimbKeyframe* unk0; 
    u16 unk4;
    u16 numFramesInAnim;
    f32 unk8;
    f32 unkC;
    f32 unk10;
    s8 unk14;
    s8 unk15;
    u8 unk16;
    char unk17; 
    s32 unk18;
    char unk1C[4];
}unk22b40s; /* unused (nothing references it); kept: the header typedef count orders IDO .bss */
/* 25290.c */
typedef struct unk_2513c_s{
    s32 unk0;
    s32 unk4;
    u8 unk8;
    u8 unk9;
    u16 unkA;
    ALHeap* unkC;
    s32 unk10;
    s32 unk14;
    s32 unk18;
    char unk1C[0x174];
    s32 unk190;
    char unk194[8];
    s32 unk19C;
    s32 unk1A0;
    u8 unk1A4;
    u8 unk1A5;
    s16 unk1A6;
    s32 unk1A8;
    s32 unk1AC;
    s32 unk1B0;
    s32 unk1B4;
    char unk1B8[0x174];
    s32 unk32C;
}unk2513cs; /* unused (nothing references it); kept: the header typedef count orders IDO .bss */
/* 25290.c */
typedef struct {
    s32 volume;
}MusicVolume;
/* 25290.c */
typedef struct {
    s32 unk0;
    s32 unk4;
}unkVolume2; /* unused (nothing references it); kept: the header typedef count orders IDO .bss */
/* 2F1E0.c */
typedef struct Message
{
    s32 flags; 
    s16 xpos; 
    s16 ypos; 
    s16 width; 
    s16 height; 
    s16 quillTimer; 
    u8 id;                          /* 0x0E: the message id the window shows (FindMessageWindow); 0x80 = none */
    u8 charDelay;                   /* 0x0F: frames per character */
    s8 charTimer;                   /* 0x10: frames left until the next character */
    s8 unk11;
    u16 textPos;                    /* 0x12: how far the text is typed (index into text) */
    u8 heldChar;                    /* 0x14: the character at textPos, held back while text[textPos] is 0xFF */
    s8 choice;                      /* 0x15: the option the hand points at (choice boxes) */
    u8 stickHeld;                   /* 0x16: set while the stick stays pushed (one step per push) */
    u8 name[0x20];                  /* 0x17: speaker name, 0xFF-terminated (0xFF first = no name tag) */
    char unk37;
    s16 lineCount;                      /* lines in use */
    s16 unk3A;
    u8 lines[4][0x18];              /* 0x3C: the window's lines, 0xFF-terminated */
    u8 text[0x4B0];                 /* 0x9C: the message text (Eltale's 0x4B0, Quest 0x400): slots are 0x9FC */
    u8 page[0x4B0];                 /* 0x54C: the page being shown (PageMessage fills it, DrawMessageText draws it) */
} Message; 
/* 328F0.c */
typedef struct {  
    s16 unk0;
    s16 unk2;
    u8 unk4;
    u8 unk5;
    s8 unk6;
    s8 unk7;
    s32 unk8;
    s32 unkC;    
}unk32218s; /* unused (nothing references it); kept: the header typedef count orders IDO .bss */
/* 328F0.c */
typedef struct {
    u8 unk0;
    u8 unk1;
    u8 unk2;
}unk31cf0s; /* unused (nothing references it); kept: the header typedef count orders IDO .bss */
/* 328F0.c */
/* 33D0.c */
typedef struct unk_1d924_s2
{
    f32 unk0;
    f32 unk4;
    f32 unk8;
    f32 unkC;
    f32 unk10;
    f32 unk14;
    f32 unk18;
    f32 unk1C;
    f32 unk20;
    f32 unk24;
    f32 unk28;
    f32 unk2C;
    f32 unk30;
    f32 unk34;
    f32 unk38;
    f32 unk3C;
    f32 unk40;
    f32 unk44;
    f32 unk48;
    f32 unk4C;
    s16 unk50;
    s16 unk52;
    f32 unk54;
    char unk56[0x8];
    u16 unk60;
}unk1d924s2; /* unused (nothing references it); kept: the header typedef count orders IDO .bss */
/* 33D0.c */
typedef struct {
    s16 unk0;
    s16 unk2;
    s16 unk4;
}unk6604s; /* unused (a view of a shared type); kept: removing it moves IDO's .bss order */
/* 33D0.c */
typedef struct {
    s32 start;
    s32 end;
}fileBounds; /* unused (nothing references it); kept: the header typedef count orders IDO .bss */
/* 33D0.c */
typedef struct {
    s16 unk0;
    s16 unk2;
    s16 unk4;
    char unk6[0x5E];
    s16 unk64;
    char unk66[0xE];
    u16 unk74;
    char unk76[0xA0];
    s16 unk116;
    s8 unk118;
}unkBAB8; /* unused (nothing references it); kept: the header typedef count orders IDO .bss */
/* 33D0.c */
typedef struct {
    s16 unk0;
    s16 unk2;
    s16 unk4;
}unk69fcs; /* unused (nothing references it); kept: the header typedef count orders IDO .bss */
/* 33D0.c */
typedef struct unk_3f5c_s{
    char unk0[0x18];
    f32 unk18;
    f32 unk1C;
    f32 unk20;
    char unk24[0x3C];
    u16 unk60;
}unk3f5cs; /* unused (a view of a shared type); kept: removing it moves IDO's .bss order */
/* 33D0.c */
typedef struct unk_3f5c_s2{
    s16 unk0;
    s16 unk2;
    u16 unk4;
}unk3f5cs2; /* unused (a view of a shared type); kept: removing it moves IDO's .bss order */
/* 33D0.c */
typedef struct {
    u16 map;
    u16 submap;
    f32 x;
    f32 z;
    f32 yrot;
}RespawnLocation;
/* 33D0.c */
typedef struct unkStruct4 {
    char unk00[4];
    u16 unk4;
    u16 unk6;
    u16 unk8;
    u16 unkA;
    u16 unkC;
    u16 unkE;
    s32 unk10;
} unkStruct4;
/* 33D0.c */
typedef struct unkStruct3 {
    char unk00[0x64];
    unkStruct4* unk64;
} unkStruct3;
/* 33D0.c */
typedef struct ElementLevels {
    u8 fire;
    u8 earth;
    u8 water;
    u8 wind;
} ElementLevels;
/* 33D0.c */
typedef struct unkStruct2 {
    char unk_00[0x24];
    ElementLevels elements;
    u16 unk28;
    u16 unk2A;
    u16 unk2C;
    u16 unk2E;
    u8 unk30;
    u8 unk31;
    u8 unk32;
    u8 unk33;
    u8 unk34;
} unkStruct2;
/* 33D0.c */
typedef struct unkStruct {
    u16 unk0;
    char unk_02[6];
    u16 unk8;
    char unk_0A[6];
    unkStruct2* unk10;

    
    
} unkStruct;
/* 33D0.c */
typedef struct {
     u8 unk_000[0x56];
     s16 unk_056;           /* frames Brian stays hurt (StartBrianHit) */
     u8 unk_058[0xA];
     s16 unk_062;           /* frames of state 5, Brian hit by an enemy (DamageBrian) */
     u8 unk_064[0x76];
     s16 unk_0DA;           /* Brian's timer when a battle starts (SetupBrianForBattle) */
     u8 unk_0DC[0x3A];
     s16 unk_116;           /* frames of state 0xC (StartBrianUseItem) */
} struct_80053D00;  /* unused (the fields are AnimEntry[n].frames: gBrianAnims is AnimEntry*[]); kept for the declaration count */
/* 8650.c */
typedef struct {
    f32 x;
    f32 y;
    f32 z;
    f32 rx;                         /* 0x0C: the ModelKey channels: position, rotation, scale */
    f32 ry;
    f32 rz;
    f32 sx;                         /* 0x18 */
    f32 sy;
    f32 sz;
    f32 tension;                    /* 0x24: Kochanek-Bartels spline parameters (SampleSpline) */
    f32 continuity;
    f32 bias;
    u16 linear;                     /* 0x30: nonzero = straight interpolation into this key */
    u16 frame;                      /* 0x32 */
} ModelKeyFrame;                    /* 0x34: one stored key of a part's animation */
typedef struct {
    ModelKeyFrame* keys;            /* 0x00 */
    u16 keyCount;                   /* 0x04: 1 = a still pose */
    u16 frames;                     /* 0x06: key frames (SampleSpline samples them) */
    Vec3f pivot;                    /* 0x08: subtracted after the part's rotation (ApplyPivotOffset) */
    s8 parent;                      /* 0x14: parent part + 1, 0 = a root part */
    s8 dlist;                       /* 0x15: display list index, -1 = none */
    u8 flags;                       /* 0x16: bit 0 = face the camera, bit 1 = record its position */
    char unk17;
    s32* dlistFrames;               /* 0x18: per-frame display list indices, or NULL */
    u16 dlistFrameCount;            /* 0x1C */
    char unk1E[2];
} ModelPart;                        /* 0x20: one part of a model's animation (also a camera path, ending.c) (the Quest recomp's Unk8) */
typedef struct {                    /* 0xC: one animation of a model's table (Brian's: ApplyBrianMovement's anims; an enemy's: gMonsterAnims[model]) (Quest's StrangeAnimPair; the recomp's Unk9) */
    s16 count;                      /* parts */
    s16 frames;                     /* 0x02: its length, which a state takes as its timer */
    ModelPart* parts;
    Gfx** dlists;                   /* 0x08 */
} AnimEntry;
typedef struct {
    s16 unk0;
    s16 unk2;               /* frames standing between walks (times 1-2, UpdateNPCs) */
    char unk4[0xA];
    s16 unkE;               /* frames per walk (times 1-2) */
}unk8948s3;
/* 8650.c */
typedef struct {
    u16 submap;               /* the submap it appears in */
    u16 talkType;               /* its kind? 6: after talking to it Brian is sent to map gCurrentMap/C0 (BrianState_TalkEnd) */
    u8 kind;
    u8 flags;                /* bit 0: transform.unk60 |= 0x1000 at spawn; bit 1: doesn't turn to face Brian */
    u16 talkParam;               /* kind 5: a flag (IsStoryFlagSet); kind 2: the low byte is an item (StartNPCTalk) */
    u16 message;               /* the text when talked to (StartNPCText) */
    u16 altMessage;               /* the other text: item already taken / flag set (StartNPCTalk) */
    s32 unkC;
    AnimEntry* anims;
    u8* name;
    u16 talkAnim;              /* the animation it plays when talked to (UpdateNPCs) */
    char unk1A[2];
    f32 x;                  /* 0x1C */
    f32 z;
    f32 yrot;
    f32 scale;              /* 0x28 */
}NpcRecord;                 /* a map object's record, 0x2C (the map header's list, SpawnNPCs) */
/* 8650.c */
typedef struct unk_1d8b0_s
{
    char unk0[0x52];
    s16 unk52;
    s16 unk54;
    s16 unk56;
    s16 unk58;
    s16 unk5A;
    s16 unk5C;
    s16 unk5E;
    u16 unk60;
}unk1d8b0s; /* unused (nothing references it); kept: the header typedef count orders IDO .bss */
/* 8650.c */
typedef struct {
    u16 state;               /* state: 0 stand, 1 walk, 2 turn to Brian, 3 turn back (UpdateNPCs) */
    u16 timer;               /* frames left in the state */
    f32 spawnX;               /* the record's x, z and yrot at spawn */
    f32 spawnZ;
    f32 spawnYaw;
    f32 turnLeft;              /* the turn still to make toward Brian when talked to (StartNPCTalk) */
    ActorTransform transform;   /* 0x14 */
    char unk7C[4];
    NpcRecord* record;       /* its record */
}Npc;                  /* a map object, 0x84 (gNpcs[]) */
/* 8650.c */
typedef struct {
    s32 start;
    s32 end;
}NPCFile; /* unused (nothing references it); kept: the header typedef count orders IDO .bss */
/* 97B0.c */
typedef struct {
    s16 unk0;
    s16 unk2;
    s16 unk4;
    s16 unk6;
    u16 unk8;
}unk202e4s3; /* unused (nothing references it); kept: the header typedef count orders IDO .bss */
/* 97B0.c */
typedef struct {
    char unk0[0x12];
    u8 unk12[7];
    char unk19[0x2B];
    unk202e4s3* unk44[4];
}unk202e4s2; /* unused (nothing references it); kept: the header typedef count orders IDO .bss */
/* 97B0.c */
typedef struct {
    char unk0[0x68];
    unk202e4s2* unk68;
}unk202e4s; /* unused (nothing references it); kept: the header typedef count orders IDO .bss */
/* 97B0.c */
typedef struct {
    u16 frame;          /* fires once the script's frame counter passes this */
    u16 stage;          /* index of a 16-byte effect stage in gEnemySpellEffects */
    u16 flags;          /* 1 = last entry, 2 = at Brian, 4 = StartCameraShake(1), 8 = push the boss instead */
    char unk6[2];
    f32 x;              /* offset, rotated by the boss's yaw + angle and scaled by its scale */
    f32 y;
    f32 z;
    f32 angle;
} BossFxEntry;          /* 0x18: one step of a boss effect script (UpdateBossEffectScript) */
typedef struct {
    u16 frame;                      /* frames since the boss effect script started (UpdateBossEffectScript) */
    s16 unk2;
    BossFxEntry* entry;             /* the current entry (from gBossEffectScripts[n]); NULL = stopped */
    EnemyAction* enemy;             /* the boss running it (its default option's tag picks the effect radius) */
    MonsterBattleData* battleData;  /* gEnemies[0].battleData: where the effects spawn */
}BossEffectScript;         /* gBossEffectScript: a boss's effect script player (StartBossEffectScript / UpdateBossEffectScript) */
/* 97B0.c */
typedef struct {
    char unk0[0x90];
    u16 unk90;
    char unk92[0x82];
    u16 unk114;
    char unk116[0x4];
    u16 unk11A;
}unkac84s; /* unused (nothing references it); kept: the header typedef count orders IDO .bss */
/* 97B0.c */
typedef struct {
    u8* fileStart;
    u8* fileEnd;
    AnimEntry** anims;
    MonsterBaseData* baseStats; 
    void* namePtr;
}MonsterSet; 
/* 97B0.c */
typedef struct {
    f32 unk0;
    char unk4[0x4];
    f32 unk8;
    char unkC[0x4];
    f32 unk10;
}unka3f4s; /* unused (nothing references it); kept: the header typedef count orders IDO .bss */
/* C120.c */
typedef struct {
    char unk0[0x24];
    f32 bossX;
    f32 bossY;
    f32 bossZ;
}unkD_7D1A8s; /* unused (nothing references it); kept: the header typedef count orders IDO .bss */
/* C120.c */
typedef struct {
     u16 bossMap;
     u16 bossSubmap;
     u16 bossNumber;
     u16 unk6; 
     void* modelVram;
     f32 x;
     f32 y;
     f32 z;
}BossData;
/* C120.c */
typedef struct {
    s32 fileStart;
    s32 fileEnd;
}bossFile;
/* C910.c */
typedef struct unk_db38_s 
{
    s32 timer;
    s32 unkDoorState;
    s32 unkCurrDoorAnimFrame;
    s32 unkDoorFlag; 
    void* position; 
    f32 xSlideOffset;
    f32 yRotation;
}DoorAnimData;
/* C910.c */
/* C910.c */
/* C910.c */
typedef struct {
    f32 x;
    f32 y;
    f32 z;
    f32 angle;        /* degrees */
    f32 model_scale;
    s16 index;        /* into the geometry table gMapGeometry */
    u16 flags;
} ModelTransform;                   /* 0x18: a placed map model (gMapModels[]; doors use x/z/angle, BrianWalkThroughDoor) */
typedef struct unk_e3c4_s 
{
    s32 active;             /* 0 = free; copied from the same prop last frame, else 1 */
    f32 x;
    f32 y;
    f32 z;
    f32 angle;              /* the model's placed angle, degrees (ModelTransform.angle) */
    f32 spin;               /* radians: a random multiple of pi / 4 at first, minus pi / 200 a frame */
    ModelTransform* prop;   /* matched against the previous frame's records */
}SpinningProp;              /* a spinning prop: gSpinningProps1 / gSpinningProps0, one per frame (AddSpinningProp) */
/* FA60.c */
typedef struct unk_11170_s {
    s32 mode;                       /* 0 free, 1 used, 2 used with brightened lights (StepLightAnim) */
    s32 colourMode;                 /* the env colour channel: 0 off, 1 once, 2 hold the last key, 3 loop */
    s32 colourCount;                /* steps left */
    s32 colourLoopCount;            /* the step count a loop restarts with */
    s32* colourKey;                 /* the current step: r, g, b (3 s32 per step) */
    s32* colourLoop;                /* where a loop restarts */
    s32 lightMode;                  /* the light channel: same modes */
    s32 lightLoopCount;
    s32 lightCount;
    s32* lightKey;                  /* 6 s32 per step: ambient scale r, g, b, light scale r, g, b (0x100 = 1) */
    s32* lightLoop;
    s32* dirs;      /* 0x2C: the light directions StepLightAnim gives SetupLights; NULL = gLightDirections */
}LightAnim;
/* camera.c */
typedef struct unk_23df4_s
{
    f32 unk0;
    f32 unk4;
    f32 unk8;
    f32 unkC;
    f32 unk10;
    f32 unk14;
    f32 unk18;
    f32 unk1C;
    f32 unk20;
    f32 unk24;
    f32 unk28;
}unk23df4s; /* unused (nothing references it); kept: the header typedef count orders IDO .bss */
/* hud.c */
typedef struct STRUCT_D_803A8DD8 {
    char unk00[10];
} STRUCT_D_803A8DD8;
/* hud.c */
typedef struct VEC4_MEM {
    float unk0;
    float unk4;
    float unk8;
    float unkC;
} VEC4_MEM; /* unused (gHudVector is an OddVector); kept: the header typedef count orders IDO .bss */
/* hud.c */
typedef struct StatusFxDef {
    s16 model;      /* into gStatusFxModels */
    s16 color;
    u16 flags;      /* also the duration argument; 0x100 = at mid-body */
    f32 sizeX;
    f32 sizeY;
} StatusFxDef;
/* inventory.c */
typedef struct unk_22260_s{
    char unk0[0x64];
    s32 unk64;
}unk22260s; /* unused (nothing references it); kept: the header typedef count orders IDO .bss */
/* inventory.c */
typedef struct unk_2260c_s{
    s16 unk0;
    s16 unk2;
    s16 unk4;
}unk2260cs; /* unused (nothing references it); kept: the header typedef count orders IDO .bss */
/* inventory.c */
typedef struct {
    char unk0[4];
    u16 unk4;
    u16 unk6;    
}unk2234c4; /* unused (nothing references it); kept: the header typedef count orders IDO .bss */
/* inventory.c */
typedef struct {
    f32 width;
    f32 height;
    f32 unk8;
}ItemLightRingData; /* unused (gItemLightRingSize is an OddVector); kept: the header typedef count orders IDO .bss */
/* inventory.c */
typedef struct {
    s16 unk0;
    u16 unk2;
    u16 healItemLightColor;
    char unk6[2];
    f32 healItemLightWidth;
    f32 healItemLightHeight;
    s16 unk10;
    u16 unk12;
    u16 wingLightColor;
    char unk16[2];
    f32 wingLightWidth;
    f32 wingLightHeight;
}BaseItemLightRingData;
/* inventory.c */
typedef struct {
    u16 unk0;
    u16 unk2;
    u16 unk4;
    u16 unk6;
    char unk8[2];
}unk2234c3; /* unused (EffectSpawnDef has this layout: gItemEffectModels); kept: the header typedef count orders IDO .bss */
/* inventory.c */
typedef struct {
    s32 map;
    s32 submap;
    s32 entrance;
    u16 doorFlags;           /* the entrance kind: bits 8-11 how Brian arrives, bit 2 / bit 0 flags (StartBrianArrival, read with lhu) */
    char unkE[2];
    s32 door;          /* the door: index into gMapModels */
    f32 yaw;          /* Brian's facing on arrival */
}PendingWarp;
/* inventory.c */
typedef struct {
    u16 mapID;
    u16 submapID;
    f32 xpos;
    f32 zpos;
    f32 yrot;
}unk803a9190s;
/* shadow.c */
typedef struct {
    Mtx unk0[3];
    Mtx unkC0[1];
} mkst;
typedef struct {
    mkst mtx;                      /* the frame's matrices (the draw functions index mtx.unkC0[] past its end) */
    char unk100[0x8940];
    LookAt lookat;                 /* 0x8A40: the camera's look-at (UpdateCamera) */
    char unk8A60[0x10];
    Gfx glist[0xA00];              /* 0x8A70: the display list (BuildFrame) */
} FrameDynamic;                    /* 0xDA70: a frame's dynamic block, segment 2 (Quest's unk_58; was in main.c) */
typedef struct {                   /* one frame's graphics task (Quest unk111Cs, larger in Eltale; name invented) */
    OSScTask task;
    FrameDynamic dyn;
    NNScMsg msg;                   /* 0xDAC8: what the scheduler sends back when the task is done (type 2) */
    char unkDACA[2];
    void* framebuffer;             /* 0xDACC */
} GfxFrame;
/* shadow.c */
typedef struct unk_Shadow_s{
    f32 unk0;
    f32 unk4;
    f32 unk8;
    f32 unkC;
    f32 unk10;
    f32 unk14;
    f32 unk18;
}ShadowRec;
/* initialize.c */
typedef struct
{
    unsigned int inst1;
    unsigned int inst2;
    unsigned int inst3;
    unsigned int inst4;
} __osExceptionVector;
/* DA68F0.c */
typedef struct {
    s16 unk0;
    u16 unk2;
    f32 unk4;
    f32 unk8;
}unkTitleScreen; /* unused (nothing references it); kept: the header typedef count orders IDO .bss */
/* DC1620.c */
typedef struct {
    char unk0[4];
    s16 unk4;
    s16 unk6;
    char unk8[0x14];
    f32 unk1C;
    f32 unk20;
    s16 unk24;
    s16 unk26;
    s16 unk28;
    s16 unk2A;
    s16 unk2C;
    char unk2E[0x6];
    s16 unk34;
    char unk36[2];
    s32 unk38;
    u8 pad3C[0x14];
}unkOvl2s; /* unused (nothing references it); kept: the header typedef count orders IDO .bss */
/* ---- from Quest src/1BCD0.c (Eltale dust.c): one puff of dust / one particle (0x40) ---- */
typedef struct {
/*0x00*/ u16 frames;
/*0x02*/ u8 flags;
/*0x03*/ char pad;
/*0x04*/ void* update;
/*0x08*/ u8 texFmt;
/*0x09*/ u8 texWidth;
/*0x0A*/ u8 texHeight;
/*0x0B*/ u8 texCms;
/*0x0C*/ u8 texCmt;
/*0x0D*/ char unkD[3];
/*0x10*/ u8* dustTexture;
/*0x14*/ u8* dustDL;
/*0x18*/ f32 x;
/*0x1C*/ f32 y;
/*0x20*/ f32 z;
/*0x24*/ f32 roll;
/*0x28*/ f32 scaleX;
/*0x2C*/ f32 scaleY;
/*0x30*/ f32 vx;
/*0x34*/ f32 vy;
/*0x38*/ f32 vz;
/*0x3C*/ u8 colour;     /* index into the colour table gParticleColours (RGBA) */
/*0x3D*/ char unk3D[3];
}Dust;

typedef struct {
    u8* start;              /* ROM */
    u8* end;
} RomRange;                 /* a block of ROM to DMA (dma_write(start, ram, end - start)) */
/* ---- from Quest src/1D160.c (Eltale encounter.c): a map's header (gMapHeader) ---- */
typedef struct {
    u16 count;
    NpcRecord* list;
} sMapList;                 /* a map's objects: SetupNPCs hands them to SpawnNPCs */
typedef struct {
    Gfx* setup;             /* drawn once before the first visible prop of this type */
    Gfx* model;             /* drawn for each visible prop */
} MapPropGfx;               /* per prop type (DrawMapProps) */

/* ---- from collision.c ---- */
/* ---- from collision.c: a submap's model set (gMapModelData) ---- */
typedef struct {
    u16 transform_count;    /* placed models at 0x18 */
    u16 model_count;        /* 0x02: untiled models, indices at non_tiled */
    s16 tile_columns;       /* 0x04 */
    s16 tile_rows;          /* 0x06 */
    f32 min_x;              /* 0x08 */
    f32 min_z;              /* 0x0C */
    f32 tile_w;             /* 0x10 (Quest: aabb_max_x) */
    f32 tile_d;             /* 0x14 (Quest: aabb_max_z) */
    ModelTransform* transforms;  /* 0x18 (Quest: x18_vertices); gMapModels */
    u16* tile_offsets;      /* 0x1C (Quest: vertex_pairs) */
    u16* wall_defs;         /* 0x20 */
    u16* non_tiled;         /* 0x24 */
    u16 tile_flags;         /* 0x28: 0 = map not tiled */
    u16 unk2A;
} MapModelData;             /* names from vbhayden's Quest 64 scratches */
typedef struct {
    s32 lineCount;          /* lines in use */
    s32 unk4;
    u8* lines[4];           /* the lines to copy into the window */
} MessageLines;             /* gMessageLines */
typedef struct {
    s32 r;
    s32 g;
    s32 b;
} Rgb32;
typedef struct {
    s32 near;
    s32 far;
} FogRange;
typedef struct {
    s32 time;               /* the key applies from this time of day */
    Rgb32 colours[4];       /* ambient + three lights */
    FogRange fog;
} LightKey;                 /* 0x3C */
typedef struct {
    LightKey* keys;         /* 0x00: the day's light keys */
    s32 count;              /* 0x04 */
    s32 hasDirection;       /* 0x08: the record sets the light direction */
    s32 direction[3];       /* 0x0C */
    u16 flags;              /* flags (gLightRecordFlags) */
    char unk1A[2];
    s32 skyRecord;              /* sky record (gSkyRecords[]) */
} MapLight;                 /* 0x20: a submap's light record (gSubmapLight) */
typedef struct {
    s16 a;                          /* vertex indices */
    s16 b;
    s16 c;
    u16 flags;                      /* tested against the poll's face mask; returned in CollisionPollingResult.flags */
    f32 nx;                         /* the face's plane normal */
    f32 ny;
    f32 nz;
} GeometryFace;                     /* 0x14 */
typedef struct {
    s16 x;
    s16 y;
    s16 z;
    u16 unk6;
} GeometryVertex;                   /* 8 bytes */
typedef struct {
    s16 x_min;
    s16 x_max;
    s16 y_min;
    s16 y_max;
    s16 z_min;
    s16 z_max;
    s32 unkC;
    Gfx* dl;                /* 0x10: the model's display list */
    GeometryFace* faces;
    u16 face_count;
    u16 unk1A;
    GeometryVertex* vertices;
} ModelGeometry;                    /* 0x20: gMapGeometry[ModelTransform.index] (names from vbhayden's Quest 64 scratches) */
typedef struct {
    f32 x;
    f32 z;
} VertexData;
typedef struct {
    s32 index;  /* first wall index in NavMeshData.indices */
    s32 count;  /* number of walls */
} VertexGroup;
typedef struct {
    VertexData* vertices;
    s32* indices;
    VertexGroup* groups;
    s32 group_count;
} NavMeshData;
typedef struct map_data_s{
    u16 flags;               /* flags: & 7 = load the extra block (2 = also D_851F0..D_C8DE0), 4 = set the return
                               point on entry, 8 = set the exit point on leaving */
    char unk2[2];
    u8* romStart;               /* the map's ROM block */
    u8* romEnd;               /* its end */
    void* ram;             /* the RAM it loads to */
    ModelGeometry* geometry;   /* geometry table (gMapGeometry) */
    MapModelData* modelSets;    /* model sets (gMapModelData) */
    Submap* submaps;       /* submaps (gSubmapHeader) */
    MapLight* lights;        /* light records (gSubmapLight) */
    MapPropGfx* propGfx;      /* prop display lists by type */
    void* cameras;            /* the submaps' camera tables (camera.c's MapCameras[], InitRoomCamera); NULL = none */
    NavMeshData* cameraNavmesh;              /* wall navmesh, camera (gWallNavmeshCamera) */
    NavMeshData* actorNavmesh;              /* wall navmesh, actors (gWallNavmeshActors) */
    s32 encounters;              /* encounter tables, 8 bytes per submap */
    s32 unk34;              /* 0x10 bytes per submap */
    sMapList* npcList;      /* the NPC records and their count (SetupNPCs) */
    s32 regionBlock;        /* the region block loaded with the map (gRegionBlocks[]) */
    char unk40[4];
}sMapData;                  /* 0x44: a map's header (gMapTable[map]; the current one is gMapHeader) */
/* ---- from collision.c: what a floor poll (PollWorldCollision) returns ---- */
typedef struct {
    f32 x;
    f32 y;
    f32 z;
    f32 sn_x;  /* surface normal */
    f32 sn_y;
    f32 sn_z;
    u16 flags;  /* 0x18 */
} CollisionPollingResult;

/* ---- from spells.c (enemy.c uses it too) ---- */
/* spell animation definition (tempStruct14F48.unk30; names from vbhayden's Quest 64 scratches) */
typedef struct {
    s16 model;      /* 0x00: index into gEffectModels, < 0 = none */
    u16 color;      /* 0x02 */
    u16 duration;   /* 0x04 */
    u16 unk6;
    f32 size_x;     /* 0x08 */
    f32 size_y;     /* 0x0C */
    s16 trail_model;    /* 0x10: the trail stage (see SpellAnimStages) */
    u16 trail_rate;     /* 0x12: also the trail piece's colour argument (SpawnSpellTrail) */
    u16 trail_flags;    /* 0x14: also its duration argument; 0x10 = turn by lifetime x pi / 10 */
    u16 unk16;
    f32 trail_size_x;   /* 0x18 */
    f32 trail_size_y;   /* 0x1C */
    s16 land_model;     /* 0x20: the landing stage (StartSpellLandVisual), < 0 = none */
    u16 land_color;     /* 0x22 */
    u16 land_duration;  /* 0x24 */
    u16 unk26;
    f32 land_size_x;    /* 0x28 */
    f32 land_size_y;    /* 0x2C */
    s16 hit_model;      /* 0x30: the hit stage (StartSpellHitVisual), < 0 = none */
    u16 hit_color;      /* 0x32 */
    u16 hit_flags;      /* 0x34: also its duration argument; 0x40 = on the caster, 0x100 = at mid-body */
    u16 unk36;
    f32 hit_size_x;     /* 0x38 */
    f32 hit_size_y;     /* 0x3C */
    s8 sound;       /* 0x40: < 0 = none (on spawn) */
    u8 delay;       /* 0x41 */
    s8 hit_sound;   /* 0x42: when a hit resolves (StartSpellHitVisual) */
    u8 hit_delay;   /* 0x43 */
    s8 land_sound;  /* 0x44: on landing (StartSpellLandVisual) */
    u8 land_delay;  /* 0x45 */
    s8 trail_sound; /* 0x46: each trail piece (SpawnSpellTrail) */
    u8 trail_delay; /* 0x47 */
} SpellAnimDef;     /* 0x48: the spell's effect record */
/* ---- from player.c: a model's animation table entry ---- */
typedef struct {
    f32 x;
    f32 y;
    f32 z;
    f32 unkC;
    f32 unk10;
    f32 unk14;
    f32 scaleX;
    f32 scaleY;
    f32 scaleZ;
} ModelKey;                         /* a part's sampled key (SampleSpline) */

/* the Controller Pak notes per controller (gPakNotes; controller.c scans them, save_menu.c lists the saves) */
typedef struct {
    s16 flags;                      /* 0x00: 0x8000 = a pak is inserted (InitControllers) */
    s8 ext[16];                     /* 0x02: an extension letter per note (0x1A = 'A' in the pak font) */
    s8 status;                      /* 0x12: 0 ok, -1 no response, -2 not a standard pad, -3 no controller */
    u8 freePages;                   /* 0x13: free pak pages (ScanControllerPak), 0xFF before a scan */
} PakNoteExts;                      /* 0x14 per controller (gPakNotes) */

/* ---- from map.c ---- */
typedef struct {
    f32 x;
    f32 y;
    f32 z;
    f32 shadow;                     /* index into gShadowSizes, stored as a float */
    f32 scale;
    s16 type;                       /* selects gMapHeader->propGfx[type] */
} MapProp;                          /* a map prop drawn by DrawMapProps */

/* ---- from collision.c / lighting.c ---- */

/* ---- map objects, area spots and the map tables: shared by map_objects.c, spirits.c and signs.c ---- */
typedef struct {
    f32 x;
    f32 z;                          /* stored negated: the spirit's z is -z */
    u8 flag;                        /* its collected flag (IsSpiritFound) */
} SpiritSpot;                     /* 0xC: one spirit spot */
typedef struct {
    u16 map;
    u16 submap;
    u16 count;
    SpiritSpot* spots;
} SubmapSpirits;                      /* 0xC: the spirit spots of one submap (43 at gSubmapSpiritTable) */
typedef struct {
    ActorTransform transform;
    s32 unk68;
} MapObject;                     /* 0x6C: a map object drawn by DrawMapObjects */
typedef struct {
    f32 x;                          /* 0x00 */
    f32 z;
    f32 yrot;
    f32 unkC;                       /* -> transform.unkDoor44 */
    f32 unk10;                      /* -> transform.unkDoor4C */
    f32 unk14;                      /* -> transform.yspeed */
    f32 unk18;                      /* -> transform.xspeed */
    f32 unk1C;                      /* -> transform.zspeed */
    u8 flags;                       /* 0x20: -> transform.unk62; IsMapObjectOpened picks the animation from it */
    u8 unk21;                       /* -> transform.modelID */
    char unk22[2];
} MapObjRec;                        /* 0x24: one placed object of a submap (SpawnMapObjects) */
typedef struct {
    u16 count;
    MapObjRec* recs;
} MapObjList;                       /* per submap */
typedef struct {
    s32 map;
    MapObjList* submaps;
} MapObjTable;                      /* gMapObjectTable[19]: the maps that place these objects */
typedef struct {
    char unk0[0x10];
    u8* texture;                    /* 0x10: the frame (0x100 bytes each in gSparkleFrames) */
    char unk14[4];
    Vec3f pos;                      /* 0x18 */
} unk8004E658s; /* unused (nothing references it); kept: the header typedef count orders IDO .bss */                     /* the spirit sparkle drawn by SpawnDust */
typedef struct {
    char unk0[0x1C];
    u16 unk1C;                      /* offset into D_D37700 of the 0x20 bytes for gSpeakerName */
    u16 unk1E;                      /* offset of the 0x200 bytes for gTextBuffer */
} SignText;
typedef struct {
    SignText* unk0;
    f32 unk4;
} SignRef;                        /* what gSignTarget points to (player.c's UpdateBrian passes it) */

/* ---- NPC records: shared by spirits.c and signs.c ---- */
typedef struct {
    u16 flags;                      /* bit 0 picks the colour from gSparkleEnvColors */
    char unk2[0x16];
    f32 x;                          /* 0x18 */
    f32 y;
    f32 z;
    char unk24[0xC];
    f32 velX;                       /* 0x30 */
    f32 velY;
    f32 velZ;
    u8 phase;                       /* 0x3C: wobble angle, 64 steps per turn */
} SpiritSparkle;
typedef struct {
    s32 map;
    s32 submap;
    f32 x;
    f32 z;
    f32 yaw;                        /* 0x10 */
    f32 radius;                     /* 0x14: talk range */
    s32 flags;                      /* 0x18: bit 0 = can be talked to from any side */
    s32 unk1C;
} SignObject;                   /* 0x20; 38 of them at gSigns */
typedef struct {
    SignObject* npc;
    f32 angle;                      /* Brian's yaw minus the direction to the NPC, set when found */
    s32 unk8;
} SignSlot;                     /* 0xC */

/* ---- floor / navigation mesh data: shared by collision.c and lighting.c ---- */
typedef struct {
    s32 time;                       /* the period starts at this time of day */
    s32 palette;                    /* index into gSkyPalettes */
    s32 fadeFrames;
} DayPeriod;                     /* 0xC: one period of the day (4 at gDayPalettePeriods) */
/* Area collision mesh (names from vbhayden's Quest 64 scratches) */

/* ---- the save data block: shared by sound.c and save_menu.c ---- */
typedef struct {
    u8 spirits[0x20];               /* 0x00: the spirit flags (gObjectsOpened) */
    u8 flags[0x10];                 /* 0x20: gSpiritsFound's flags */
    u8 items[0x97];                 /* 0x30: the inventory (gInventory gInventory and the byte after it) */
    u8 story;                       /* 0xC7: the boss flags (gStoryFlags) */
} SaveData;                     /* 0xC8: the save data block (copied out of the 0xE0-byte pak read buffer) */

/* effect objects (moved here from spells.c) */
/* spell effect spawn parameters (names from vbhayden's Quest 64 scratches) */
typedef struct {
    u16 sizeModeX;   /* x scaling mode: 0 hitbox width, 1 hitbox height, 3 vector scale */
    u16 sizeModeY;   /* y scaling mode */
    u16 gfx;    /* into gProjGfx */
    u16 texture; /* into gProjModelDefs */
    u16 anim;   /* into gProjAnims */
} EffectSpawnDef;
typedef struct {
    f32 x;          /* width: times the actor's radius or height, or times scale, or as is (EffectSpawnDef's modes) */
    f32 y;          /* height, the same way */
    f32 scale;      /* the base size for mode 3 (a spell's radius, an enemy's collisionScale, 1.0) */
} OddVector;        /* an effect's size for SpawnEffectObject (vbhayden's Quest 64 name) */

#endif
