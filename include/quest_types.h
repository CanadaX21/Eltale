/* The types and constants the game's modules share (docs/ARCHITECTURE.md, docs/DECLARATIONS.md). The section
 * markers say which of Quest 64's headers (Mallos31/Quest @401fe0e) a type came from; the names, sizes and
 * comments are this game's. No extern or prototype lines: those are in the module headers. */
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
typedef struct unk_232f4_s{
    f32 x;
    f32 y;
}Coordinates2D;
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
    u16 flags;          /* DOOR_ (below the struct), and the door's kind in bits 8-11 */
    u16 keyItem;        /* the item DOOR_NEEDS_KEY / DOOR_REFUSES_KEY test */
    char unk18[2];
    u16 model;          /* the door model (index into gMapModels), copied to gPendingWarp.door when used */
    char unk1C[2];
    u16 dest[3];        /* 0x1E: map, submap and entrance the door leads to (SetDoorWarp) */
}MapDoor;              /* a door / warp trigger, 0x24 (gSubmapHeader->doors[]): a box of half size
                          halfWidth x halfDepth at (x, z), turned by angle (the way Brian must face) */
/* MapDoor.flags; gPendingWarp.doorFlags takes them when Brian walks into the door, then gArrivalFlags on the other side.
   Bits 8-11 are the door's kind, which StartBrianDeparture and SpawnBrian switch on: 1 walk out and walk in, 2 and 3
   leave through a door model (on arrival 2 comes through it, 3 walks in while it moves), 4 a door that opens. Kind 0 (two
   doors, both DOOR_LEAVE_STANDING) has no departure of its own: without DOOR_LEAVE_STANDING no countdown would
   start. Bit 0 (on 177 of the 538 doors) picks which way a door model is passed: BRIAN_ANIM_DOOR_FLAG1 and the door's
   other animation (gDoorAnimAlt) */
#define DOOR_ANY_FACING       0x02  /* the trigger works whichever way Brian faces (CheckDoorTriggers) */
#define DOOR_LEAVE_STANDING   0x04  /* Brian stands 30 frames instead of leaving by the kind (StartBrianDeparture) */
#define DOOR_ARRIVE_STANDING  0x08  /* Brian stands 15 frames instead of arriving by the kind (SpawnBrian) */
#define DOOR_NEEDS_KEY        0x10  /* opens only with keyItem in the inventory (CheckDoorKeyItem) */
#define DOOR_REFUSES_KEY      0x20  /* opens only without keyItem in the inventory (CheckDoorKeyItem) */
#define WARP_ARRIVAL         0x108  /* how a warp sets Brian down (gArrivalFlags: the Wings, Return, Exit, a defeat, a new
                                       game, the ending's scenes): DOOR_ARRIVE_STANDING, kind 1 */
typedef struct {
    f32 x;
    f32 z;
    f32 yrot;
    char unkC[2];
    u16 doorModel;               /* copied to gEntranceDoorModel */
    char unk10[4];
} MapEntrance;              /* 0x14: where Brian appears when he enters a submap (gSubmapHeader->entrances[gPendingWarp.entrance]) */
typedef struct {
    u16 modelSet;               /* this submap's model set (gMapHeader->modelSets[]) */
    u16 light;               /* its light record (gMapHeader->lights[]) */
    MapDoor* doors;
    s32 doorCount;          /* doors[] */
    MapEntrance* entrances;      /* entrances */
    char unk10[2];          /* as a u16, the number of entrances the data gives the submap; no code reads it */
    u16 area;               /* the area: its NPC blob (LoadNPCArea) and its monster set (LoadMonsterSet) */
    u16 flags;              /* SUBMAP_ (below the struct); copied to gSubmapFlags */
}Submap;                 /* a submap, 0x18 (gMapHeader->submaps[], gSubmapHeader) */
/* Submap.flags (gSubmapFlags) */
#define SUBMAP_ENCOUNTERS     0x1  /* random encounters (FieldLoop, InitEncounters) and the area's monster set
                                      (ResetEnemies: LoadMonsterSet); no NPCs are set up (SetupNPCs); the walking that trains agility counts
                                      800 units, not 1500 (UpdateBrianGrowth) */
#define SUBMAP_FLOOR_CAMERAS  0x2  /* the camera setup follows the floor Brian stands on (its floor flags' bits 1-3), and
                                      L does not cycle the setups (UpdateCamera) */
#define SUBMAP_NO_RETURN      0x4  /* the Return spell and the Wings do nothing here (SpellEffect_Return,
                                      CheckIfTeleportPossible) */
#define SUBMAP_NO_EXIT        0x8  /* the Exit spell does nothing here, and the compass is not drawn (SpellEffect_Exit,
                                      DrawHUD) */
typedef struct FontStrip {
     char texels[0x7B8];         /* 304 x 13 texels, CI4 */
} FontStrip;                     /* one strip of the font sheet (gFontSheet[], DrawString) */
/* ---- from Quest include/player.h ---- */
typedef struct {            /* one move an enemy's AI can pick (names from vbhayden's Quest 64 scratches) */
    u16 action_tag;         /* 0x00: AI_ACTION_SPELL, and below it the spell (element << 8 | index: gSpellDefs[element][index]);
                               in a boss battle the low byte also names the boss effect script (StartBossEffectScript) */
    u16 move_flags;         /* 0x02: 2 = keeps moving while winding up, 0xA = while casting (EnemyCastAttack) */
    u16 cast_delay;         /* 0x04: frames before it acts */
    u16 hold;               /* 0x06: frames held after the cast when next_anim is set (EnemyCastAttack) */
    s16 anim;               /* 0x08 */
    s16 next_anim;          /* 0x0A: -1 = back to animation 0 */
    s16 recover_anim;       /* 0x0C: played after the hold (EnemyCastAttack) */
    char unkE[2];           /* 0 in every option; never read */
    f32 max_range;          /* 0x10 */
    f32 hit_range;          /* 0x14: a melee hit's reach, times the attacker's scale (EnemyMeleeAttack) */
} EnemyAIOption;            /* 0x18: MonsterBaseData.aiOptions points at aiScriptNumber of these (PickEnemyAIOption) */
#define AI_ACTION_SPELL 0x8000  /* action_tag: the option casts the spell in the bits below */
typedef struct {
     u16 monsterType; 
     s16 monsterID;  /* the record's own index in its table: the model (gMonsterAnims[]) and name (gEnemyNames[])
                        index; loaded signed everywhere (SpawnEnemy, SpawnBoss, the battle card) */
     u16 currHP;     /* equal to maxHP in every record */
     u16 maxHP;
     u16 currMP;     /* 0x08: 0 in every record (the MP drain spell SpellEffect_DrainMP reads both as u16) */
     u16 maxMP;      /* 0x0A: 0 in every record; SpawnEnemy and SpawnBoss start an enemy's MP from maxHP */
     u16 agi;
     u16 def;
     s32 expValue;
     s32 stone;      /* 0x14: the stone it gives when defeated (EnemyKnockback: gBattleStone) */
     f32 radius;     /* 0x18: horizontal hit radius in model units: distance tests, floor polls, 2x = effect size
                        (Mallos31's Quest decomp: hitboxHeight; vbhayden's scratches: hitboxWidth) */
     f32 height;     /* 0x1C: hit height: 0.5x mid-body, 1x the over-enemy icon, 2x where effects start above
                        (Quest decomp: hitboxWidth) */
     f32 modelScale;
     u16 aiFlags;           /* MONSTER_AI_RANDOM (below the struct) */
     u16 element; 
     s16 hitAnim;           /* the animation it plays when hit (DamageEnemy) */
     u16 atk; 
     u16 aiScriptNumber;
     u16 unk2E;             /* 0 in every record; never read */
     EnemyAIOption* aiOptions;
     u8 itemDrop;
     u8 unk35;              /* 0 in every record; never read */
     u16 unk36;             /* 0 in every record; never read */
} MonsterBaseData; 
/* The elements (MonsterBaseData.element, BrianData1.spirits[], a spell's path[0], gElementMasks): the order of the C
   buttons that start a spell (BrianSpellInput), and the game's own names on its spell banner */
#define ELEMENT_FIRE   0   /* C up: Fire Ball */
#define ELEMENT_EARTH  1   /* C left: Rock */
#define ELEMENT_WATER  2   /* C down: Water Pillar */
#define ELEMENT_WIND   3   /* C right: Wind Cutter */
#define ELEMENT_NONE   4   /* no element: no icon on the monster card, and no pair rule in DamageEnemy */

#define MONSTER_AI_RANDOM  0x4  /* MonsterBaseData.aiFlags: PickEnemyAIOption takes a move at random */
typedef struct {
     u16 unk0;
     s16 modelID;
     u16 currHP;
     u16 maxHP;
     u16 currMP;
     u16 maxMP;
     u16 agi;
     u16 def;
     s32 exp;
     s32 stone;             /* 0x14: the pre-release currency "stone" (Quest 64 decomp community name): battles won add to it; nothing spends or shows it */
     f32 radius;            /* 0x18: Brian's collision radius (copied to ActorTransform.collisionScale); was colissionSize */
     f32 height;            /* 0x1C: as MonsterBaseData's: effects start at fractions of height x scale */
     f32 modelScale;        /* 0x20: InitBrian copies it into his transform's scale (MonsterBaseData's modelScale) */
     u8 spirits[4];         /* 0x24: by element (ELEMENT_FIRE .. ELEMENT_WIND) */
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
     f32 rootX;             /* 0x38: the root part's key position this frame, times the scale (ApplyRootMotion) */
     f32 rootY;
     f32 rootZ;
     f32 rootMoveX;         /* 0x44: the root part's move since the last frame (ApplyRootMotion; GetRootMotionOffset
                               turns it into the actor's step). A map object keeps its use point's x / z in
                               rootMoveX / rootMoveZ instead (SpawnMapObjects; FindMapObjectInFront) */
     f32 rootMoveY;
     f32 rootMoveZ;
     s16 modelID;           /* 0x50 */
     s16 animID;
     u16 animFrame;
     u16 totalAnimFrames;
     u16 animFramerate;      /* 0x58 */
     s16 afterAnim;          /* 0x5A: the animation to play when this one ends (SetAnimation) */
     u16 afterFramerate;     /* 0x5C: its framerate (AdvanceAnimation reads it unsigned) */
     u16 actorFlags;         /* 0x5E: 1 = an enemy (not Brian), 4 = the last boss */
     u16 stateFlags;         /* 0x60: the actor's state bits (ACTOR_, ANIM_ and NPC_, below); a map object keeps its
                                opening countdown here instead (MAPOBJ_OPENING) */
     u16 floorFlags;         /* 0x62: the floor poll's flags (Brian, enemies; bits 6-8 = the floor kind); a map object
                                keeps its opened-flag id here (IsMapObjectOpened) */
     union {
         MonsterBaseData* monster;   /* an enemy's: its entry in the monster table */
         BrianData1* player;         /* Brian's: gBrianStats (InitBrian) */
     } base;                 /* 0x64: the actor's base data. The two layouts agree up to 0x24 (HP, MP, agility,
                                defence, exp, radius, height, modelScale), and code that runs for any actor reads
                                that part through base.monster */
} ActorTransform;           /* 0x68 */
/* ActorTransform.stateFlags */
#define ACTOR_BUSY          0x0001  /* in an action: input, the battle turn and the Start menu wait for its end */
#define ACTOR_NO_MELEE_HIT  0x0002  /* a melee attack can't land (EnemyMeleeAttack): a dodge, or a hit with damage
                                       flag 4 (DamageBrian, DamageEnemy) */
#define ANIM_LOOPED         0x0200  /* the animation wrapped round this frame (AdvanceAnimation; ApplyRootMotion) */
#define ANIM_CHANGED        0x0400  /* a new animation started this frame (SetAnimation, AdvanceAnimation) */
#define NPC_WIDE_TALK       0x1000  /* an NPC Brian can talk to from 25 units, not 15 (its record's flag bit 0) */
#define MAPOBJ_OPENING      0x8000  /* a map object opening: the low bits count its frames down (OpenMapObject,
                                       DrawMapObjects) */
typedef struct unk_D_80086DC0_s
{
    s32 mode;                       /* 0x00: the camera mode, gCameraModeHandlers' index: 0 and 2 the field, 1
                                       fixed, 3 aimed fixed, 4 a battle (and its spells), 5 after a battle */
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
    f32 arrivalYaw;                 /* the camera's yaw from Brian's facing for the next submap's start (InitRoomCamera):
                                  ClampCameraToTarget keeps it as Brian leaves through a door (within 45
                                  degrees of behind him); behind him (pi) after a warp */
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
 u16  statusFlags;    /* 0x00: the STATUS_ bits (below the struct) */
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

/* ActorStats.statusFlags: one bit per status slot (the bit is 1 << slot). gApplyStatusHandlers[slot] sets it, and
   CountDownStatuses clears it when the slot's turns (statusTimers[slot]) run out. The effects are the handlers' and
   the readers'. */
#define STATUS_SHRUNK          0x0001  /* half size; its staff, melee and spell damage halved (slot 0; in a boss battle it
                                          misses everyone) */
#define STATUS_DRAIN           0x0002  /* its staff and melee hits heal it a quarter of the damage, plus one (slot 1) */
#define STATUS_ATTACK_UP       0x0004  /* atkScale 0x18 (level 1) or 0x20 (level 2) (slot 2) */
#define STATUS_IMMOBILE        0x0008  /* an enemy's turns end at once (EnemyIdle); Brian cannot move (BrianFieldMovement
                                          zeroes his stick) but can still act. Its handler also sets ACTOR_BUSY, which
                                          the end of the hit reaction the spell starts clears (EnemyKnockback, or
                                          BrianState_KnockedDown for Brian, whom UpdateBrianGrowth then keeps clear
                                          while he stands or walks) (slot 3; in a boss battle it misses everyone) */
#define STATUS_MOVE_SCALE      0x0010  /* moveScale 0.5 (level 1) or 2.0 (level 2) (slot 4) */
#define STATUS_AGILITY_SCALE   0x0020  /* agi x2 (level 1), x4 (2) or /2 (3) (slot 5) */
#define STATUS_SILENCE         0x0040  /* cannot cast (slot 6; in a boss battle it misses everyone) */
#define STATUS_REVEALED        0x0080  /* the monster card shows its stats and element (slot 7) */
#define STATUS_SPELL_BARRIER   0x0100  /* another's spell makes no roll and has no effect on it, though one that does not
                                          pierce still ends there; a self-target spell is not stopped (UpdateSpells)
                                          (slot 8) */
#define STATUS_MP_ABSORB       0x0200  /* the damage it takes is added to its MP (slot 9) */
#define STATUS_DEFENCE_SCALE   0x0400  /* def /2 (level 1), x0.75 (2), x1.5 (3) or x2 (4) (slot 10) */
#define STATUS_SLOT11          0x0800  /* set by its handler; no code reads it, and no spell's effects name the slot
                                          (slot 11) */

/* The flags DamageEnemy and DamageBrian take. A staff swing and an enemy's melee hit pass HIT_PHYSICAL | HIT_NO_MELEE,
   and HIT_CRITICAL with them for a critical; a spell passes its SpellDef.on_hit, with HIT_CRITICAL for a critical, so
   on_hit's SPELL_PIERCES (4) arrives here as HIT_NO_MELEE. */
#define HIT_NO_MELEE   0x0004  /* the target is ACTOR_NO_MELEE_HIT while it reacts */
#define HIT_CRITICAL   0x4000  /* shown in the critical number style (SpawnDamageNumber) */
#define HIT_PHYSICAL   0x8000  /* no element; from behind (over 135 degrees off the target's facing) x1.5; the hit
                                  sound */
typedef struct {
     ActorTransform transform;     /* 0x00: place, model, animation, base data */
     ActorStats* stats;            /* 0x68: points at statsBuf (SpawnEnemy; InitBrian for Brian) */
     ActorStats statsBuf;          /* 0x6C */
} MonsterBattleData;               /* 0x104: an actor in battle, an enemy's (EnemyAction.battleData) or Brian's
                                      (gPlayerState.brian); the name is the Quest 64 decomps' */
typedef struct {
     u16 actionID;   /* 0x00: the handler gEnemyActionHandlers runs: ENEMY_ACTION_ (below the struct) */
     s16 actionTimer;
     u16 tries;      /* 0x04: AI attempts left */
     u16 phase;      /* 0x06: the attack's step (EnemyMeleeAttack / EnemyCastAttack switch on it) */
     u16 flags;      /* 0x08: ENEMY_FLAG_ (below the struct) */
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

/* EnemyAction.actionID: the handler gEnemyActionHandlers runs for the enemy each frame (UpdateEnemies) */
#define ENEMY_ACTION_IDLE       0  /* EnemyIdle: idle; at its turn the card, and the choice of a move */
#define ENEMY_ACTION_APPROACH   1  /* SelectEnemyAction: walks toward Brian, then a move or the turn's end */
#define ENEMY_ACTION_HIT        2  /* EnemyKnockback: hit (DamageEnemy); at 0 HP it dies */
#define ENEMY_ACTION_FACE       3  /* EnemyFaceBrian: no code sets it */
#define ENEMY_ACTION_MELEE      4  /* EnemyMeleeAttack */
#define ENEMY_ACTION_CAST       5  /* EnemyCastAttack */

/* EnemyAction.flags */
#define ENEMY_FLAG_TURN          0x01  /* its turn has come (FlagTurnEnemy); EnemyIdle starts the turn and clears it */
#define ENEMY_FLAG_AT_LEASH      0x02  /* it reached its turn's movement limit (MoveEnemy): the approach ends */
#define ENEMY_FLAG_SUNK          0x04  /* placed 30 units lower (MoveEnemy; SpawnBoss sets it for Mammon) */
#define ENEMY_FLAG_HIT_RESOLVED  0x08  /* this melee swing has hit or missed (EnemyMeleeAttack) */
#define ENEMY_FLAG_OUT_OF_RANGE  0x10  /* PickEnemyAIOption chose a move out of reach: it approaches first */
#define ENEMY_FLAG_CRITICAL      0x20  /* its melee hit is critical (CalcEnemyMeleeDamage) */
typedef struct PlayerState {        /* Brian's state block (gPlayerState; Quest's BrianRoot) */
    u16 state;                      /* 0x00 (DrawBrian reads it with lhu) */
    u16 staffTarget;                       /* 0x02: the enemy targeted by the staff, 1-based (BrianState_StaffSwing) */
    u16 timer;                      /* 0x04: frames left in the state */
    u16 animCue;                       /* 0x06: an animation cue (SetAnimation plays it; the Exit / Return spells
                                       clear it; Quest names this offset doorSide) */
    s32 flags;                      /* 0x08: PLAYER_ (below) */
    f32 moveRadius;                       /* times brian.statsBuf.moveScale: Brian's battle movement radius */
    s32 comboWindow;                      /* 0x10: battle start: spirit total less each element's excess over total / 4,
                                       then / 10 + 1, at most 16 (SetupBrianForBattle) */
    BrianData1* stats;              /* 0x14: gBrianStats */
    MonsterBattleData brian;        /* 0x18: Brian in battle and in the field (gPlayerState.brian): his
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

/* PlayerState.flags */
#define PLAYER_BOSS_SCENE      0x0001  /* a boss's scene is on: its battle starts when the message closes */
#define PLAYER_PICKUP_MESSAGE  0x0002  /* the battle drop's message is on (UpdateBrianGrowth sets it only when it gives
                                          the drop): GAME_STATE_MESSAGE ends with it */
#define PLAYER_CAST_ON_SELF    0x0004  /* the spell being cast is on himself (BrianStartCast, BrianState_Cast) */
#define PLAYER_OUTSIDE_CIRCLE  0x0020  /* outside the battle circle: he escapes at the next turn (StepBattleRound) */
#define PLAYER_COMBO_INPUT     0x0100  /* A or Z in the staff swing's window: a second swing follows */
#define PLAYER_SIGN_NO_POSE    0x0200  /* reading a SIGN_NO_POSE sign: no BRIAN_ANIM_SIGN_END when its message closes
                                          (UpdateBrian sets it, BrianState_FaceAndMessage clears it) */
#define PLAYER_STAFF_CRITICAL  0x0400  /* the staff swing is critical (CalcStaffPower) */
#define PLAYER_AT_50(element)  (0x1000 << (element))   /* that element's spirits reached 50 (SpiritGainWindow,
                                                     gSpiritMaxBits) */
#define PLAYER_AT_50_BITS      0xF000   /* the four together */
#define PLAYER_ALL_AT_50       0x10000  /* set with the fourth (SpiritGainWindow) */
#define PLAYER_MESSAGE_HOLD    0x0010  /* a message the game opened keeps GAME_STATE_MESSAGE until every message window
                                          has closed (BrianFieldMovement), and no one can be talked to meanwhile
                                          (FindTalkTarget): set when Brian is put down away from an entrance
                                          (SpawnBrian at entrance -1: a new game, the Wings, a respawn at a listed place, the ending's
                                          and the staff credits' scenes)
                                          and by the last boss's last words
                                          (OnBossDefeated) */
#define PLAYER_ITEM_GIVEN      0x0040  /* an NPC gave an item (NPC_TALK_ITEM): its sound plays when the talk ends
                                          (BrianState_TalkEnd) */
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
    s32 unkC;               /* 0x0C: the image's height in texels (45 the HUD font's, 80 the info window's);
                               no code reads it */
}HudImage;
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
/* ---- from Quest include/inventory.h ---- */
typedef struct unk_22260_s3{
    u16 canBeUsed;      /* ITEM_USE_ and ITEM_NOT_BOSS (below the struct) */
    u16 type;           /* ITEM_TYPE_ */
    s16 itemArg1;
    s16 itemArg2;
    u16 unk8;           /* 0 in every item record; never read */
    u16 unkA;           /* the same */
}ItemData;

/* ItemData.canBeUsed (items 20-24 also carry 4, which no code reads) */
#define ITEM_USE_FIELD   0x1
#define ITEM_USE_BATTLE  0x2
#define ITEM_NOT_BOSS    0x8   /* not in a boss battle (CheckIfSpellItemUsable) */
/* ItemData.type: the handlers gItemCheckHandlers and gItemUseHandlers */
#define ITEM_TYPE_NONE   0     /* NoEffectItem: never usable (the key items, 20-24, among them) */
#define ITEM_TYPE_HEAL   1     /* itemArg1 HP, itemArg2 MP */
#define ITEM_TYPE_WINGS  2     /* itemArg1 the destination (gWingData) */
#define ITEM_TYPE_SPELL  3     /* itemArg1 the spell, cast SPELLSLOT_SURE */
#define ITEM_TYPE_UNUSED_F 0xF /* no item record has it: RemoveKeyItem, which nothing calls, takes out the first
                                 item of this type whose itemArg1 is the id it is given */
/* ---- from Quest include/bgm.h ---- */
typedef struct unk26658_s{
    u8 delay;
    s8 id;
    u8 volume;
}DelayedSound;
/* ---- from Quest include/31A10.h ---- */
typedef short int16_t;
typedef int16_t   qs510_t;
#define qs510(n)                      ((qs510_t)((n)*0x0400))
/* ---- from Quest include/nnsched.h ---- */
#include <ultra64.h>
#define NN_SC_SWAPBUFFER   0x0040  
#define NN_SC_RETRACE_MSG  1  
#define NN_SC_DONE_MSG     2  
#define NN_SC_PRE_NMI_MSG  3  
#define NN_SC_GTASKEND_MSG 4  
#define NN_SC_MAX_MESGS    8  
#define NN_SC_STACKSIZE    0x1000  /* each scheduler thread's stack (Quest 64's nnsched.h has 0x2000) */
#define OS_SC_SWAPBUFFER   0x0040  /* as in the SDK sched.h */
/* gGameMode: which loop mainproc runs. Quest64-Decomp names modes 1 to 5 MODE_GAMEPLAY, MODE_PAUSED, MODE_TITLE,
   MODE_SPLASH and MODE_CREDITS and defines no 0, 6 or 7; this decomp keeps MODE_GAMEPLAY, MODE_TITLE and
   MODE_SPLASH, names 2 MODE_MENU and 5 MODE_EPILOGUE for what each mode does and shows, and names 0, 6 and 7
   itself */
#define MODE_NONE          0  /* none yet: mainproc starts the field loop again (FieldLoop's countdown to a new
                                 place, EndSequenceScene) */
#define MODE_GAMEPLAY      1
#define MODE_MENU          2  /* the Start menu, and its save pages for the inn's save and the title's load and
                                 delete (EnterSaveMode); MenuLoop */
#define MODE_TITLE         3
#define MODE_SPLASH        4
#define MODE_EPILOGUE      5  /* the epilogue: a narration scrolls over a painting of the castle in the clouds, with no
                                 credits (EpilogueLoop; the overlay's segment keeps the historical name
                                 credits_overlay); it starts the credits' song; then MODE_STAFF_CREDITS (BuildFrame) */
#define MODE_ENDING        6  /* the ending's seven scenes (EndingLoop, gEndingScenes); then MODE_EPILOGUE */
#define MODE_STAFF_CREDITS 7  /* the staff credits: fourteen scenes of the world with their credit images, then End
                                 (StaffCreditsLoop, gSceneSequence); then a new game (EndSequenceScene) */
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
#ifdef NN_SC_PERF
#endif 
/* ---- from Quest include/guint.h ---- */
#ifndef __GL_GL_H__
typedef	float	Matrix[4][4];
#endif
#define ROUND(d)	(int)(((d) >= 0.0) ? ((d) + 0.5) : ((d) - 0.5))
#define	ABS(d)		((d) > 0) ? (d) : -(d)
/* ---- from Quest include/osint.h ---- */
#include <os_internal.h>
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
/* 25290.c */
typedef struct {
    s32 volume;
}MusicVolume;
/* 2F1E0.c */
/* The bytes of text a message holds: Message.text and .page, and gTextBuffer (enemy.c). StartNPCText reads this
   many from the dialogue bank, the ending and the boss scenes copy this many into gTextBuffer, and
   text/encoding.json's NPC part gives the same limit (tools/text_build.py stops the build if the two differ). */
#define MESSAGE_TEXT_SIZE 0x4B0
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
    u8 text[MESSAGE_TEXT_SIZE];     /* 0x9C: the message text (Eltale's 0x4B0, Quest 0x400): slots are 0x9FC */
    u8 page[MESSAGE_TEXT_SIZE];     /* 0x54C: the page being shown (PageMessage fills it, DrawMessageText draws it) */
} Message; 
/* 33D0.c */
typedef struct {
    u16 map;
    u16 submap;
    f32 x;
    f32 z;
    f32 yrot;
}RespawnLocation;
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
    u16 linear;                     /* 0x30: nonzero = straight interpolation into this key (SampleSpline); 0 in every
                                       key of the boss files, the monster sets and the NPC blobs */
    u16 frame;                      /* 0x32: where the key sits; a part's keys run in increasing frame from 0 to
                                       the part's frames (SampleSpline) */
} ModelKeyFrame;                    /* 0x34: one stored key of a part's animation */
typedef struct {
    ModelKeyFrame* keys;            /* 0x00 */
    u16 keyCount;                   /* 0x04: keys' length; 1 = a still pose (SampleSpline) */
    u16 frames;                     /* 0x06: key frames (SampleSpline samples them) */
    Vec3f pivot;                    /* 0x08: subtracted after the part's rotation (ApplyPivotOffset) */
    s8 parent;                      /* 0x14: parent part + 1, 0 = a root part */
    s8 dlist;                       /* 0x15: display list index, -1 = none */
    u8 flags;                       /* 0x16: bit 0 = face the camera, bit 1 = record its position */
    char unk17;                     /* 0x17: no code reads it; 0 in every boss part */
    s32* dlistFrames;               /* 0x18: per-frame display list indices, or NULL */
    u16 dlistFrameCount;            /* 0x1C: dlistFrames' length (DrawModelPart) */
    char unk1E[2];                  /* 0x1E: no code reads them; 0 in every boss part */
} ModelPart;                        /* 0x20: one part of a model's animation (also a camera path, staff_credits.c) (the Quest recomp's Unk8) */
typedef struct {                    /* 0xC: one animation of a model's table (Brian's: ApplyBrianMovement's anims; an enemy's: gMonsterAnims[model]) (Quest's StrangeAnimPair; the recomp's Unk9) */
    s16 count;                      /* parts' length (DrawModel) */
    s16 frames;                     /* 0x02: its length, which a state takes as its timer */
    ModelPart* parts;
    Gfx** dlists;                   /* 0x08: the model's display-list table, which a part's dlist indexes */
} AnimEntry;
/* 8650.c */
typedef struct {
    u16 submap;               /* the submap it appears in */
    u16 talkType;               /* NPC_TALK_ (below the struct): what talking to it does (StartNPCTalk) */
    u8 kind;                 /* NPC_KIND_WANDERS, or a value no code tells apart */
    u8 flags;                /* NPC_REC_ (below the struct) */
    u16 talkParam;               /* NPC_TALK_BOSS_FLAG: the boss whose flag IsStoryFlagSet tests; NPC_TALK_ITEM: the
                                    item, in its low byte (StartNPCTalk) */
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
}NpcRecord;                 /* an NPC's record, 0x2C (the map header's NPC list, SpawnNPCs) */
/* NpcRecord.talkType: what talking to the NPC does (StartNPCTalk). The 169 records use 1, 2, 5 and 6 */
#define NPC_TALK_MESSAGE    1  /* its message (a value the switch does not name does the same) */
#define NPC_TALK_ITEM       2  /* gives the item talkParam each time Brian holds none of it (CountItem): message
                                  then, altMessage while he holds one; PLAYER_ITEM_GIVEN plays its sound when the talk ends */
#define NPC_TALK_BOSS_FLAG  5  /* message until the boss talkParam is beaten (IsStoryFlagSet), altMessage after; with
                                  LAST_BOSS it would also restart the game (GAME_STATE_RESTART), but no record has it */
#define NPC_TALK_INN        6  /* the inn: its message with a Yes / No (GAME_STATE_INN_CHOICE); Yes rests Brian and opens
                                  the save (BrianState_TalkEnd) */
/* NpcRecord.flags */
#define NPC_REC_WIDE_TALK   0x1  /* can be talked to from 25 units, not 15 (NPC_WIDE_TALK at spawn) */
#define NPC_REC_NO_TURN     0x2  /* does not turn to face Brian when talked to */
/* NpcRecord.kind */
#define NPC_KIND_WANDERS    2  /* walks about between stands (UpdateNPCs, ResumeNpcAfterTalk); no code tells the other
                                  values (0, 1, 3) apart */
/* 8650.c */
typedef struct {
    u16 state;               /* NPC_STATE_ (below the struct) */
    u16 timer;               /* frames left in the state */
    f32 spawnX;               /* the record's x, z and yrot at spawn */
    f32 spawnZ;
    f32 spawnYaw;
    f32 turnLeft;              /* the whole turn toward Brian when talked to (StartNPCTalk), made in 15 equal steps */
    ActorTransform transform;   /* 0x14 */
    char unk7C[4];
    NpcRecord* record;       /* its record */
}Npc;                  /* an NPC, 0x84 (gNpcs[]) */
/* Npc.state (UpdateNPCs) */
#define NPC_STATE_STAND      0  /* stands; a NPC_KIND_WANDERS one sets off when its timer runs out */
#define NPC_STATE_WALK       1  /* walks, then stands again */
#define NPC_STATE_TALK       2  /* talked to: turns to Brian (StartNPCTalk) and holds until the talk ends */
#define NPC_STATE_TURN_BACK  3  /* turns back to its own facing (ResumeNpcAfterTalk), then stands */
/* 97B0.c */
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
    s16 model;
    u16 color;
    u16 flags;
    u16 unk6;
    f32 size_x;
    f32 size_y;
} SpellAnimStage;                    /* 0x10: one of a SpellAnimDef's stages; a boss script counts them flat across the table */
/* 97B0.c */
typedef struct {
    u8* fileStart;
    u8* fileEnd;
    AnimEntry** anims;
    MonsterBaseData* baseStats; 
    u8** namePtr;           /* the set's enemy names, by monsterID (LoadMonsterSet makes it gEnemyNames) */
}MonsterSet; 
/* C120.c */
typedef struct {
     u16 bossMap;
     u16 bossSubmap;
     u16 bossNumber;         /* its boss file (gBossFiles[]) */
     u16 unk6;               /* 0 in every placement */
     AnimEntry* anims;       /* the boss model's animation table, the same as its gBossAnimTables entry; no code reads it */
     f32 x;
     f32 z;
     f32 yrot;               /* the boss's facing (SpawnBoss's yrot) */
}BossPlacement;                   /* 0x18: one boss placement (gBossPlacements[8], LoadMapBoss) */
/* C120.c */
typedef struct {
    s32 fileStart;
    s32 fileEnd;
}bossFile;
/* C910.c */
typedef struct unk_db38_s 
{
    s32 timer;
    s32 state;
    s32 frame;
    s32 doorFlags; 
    void* model;     /* the door's model (its ModelTransform: StartDoorAnimation; FindDoorAnim matches it) */
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
/* hud.c */
typedef struct StatusFxDef {
    s16 model;      /* into gStatusFxModels */
    s16 color;
    u16 flags;      /* SpawnEffectObject's flags argument (ProjSlot.flags); 0x100 = at mid-body */
    f32 sizeX;
    f32 sizeY;
} StatusFxDef;
/* inventory.c */
typedef struct {
    s16 healLightModel;
    u16 healLightColour;
    u16 healLightFlags;
    char unk6[2];
    f32 healItemLightWidth;
    f32 healItemLightHeight;
    s16 wingLightModel;
    u16 wingLightColour;
    u16 wingLightFlags;
    char unk16[2];
    f32 wingLightWidth;
    f32 wingLightHeight;
}BaseItemLightRingData;           /* the item lights (UseHealingItem, UseWings): each an effect model
                                     (gItemEffectModels), the colour and the flags SpawnEffectObject takes, and
                                     its size */
/* inventory.c */
typedef struct {
    s32 map;
    s32 submap;
    s32 entrance;
    u16 doorFlags;           /* the door's flags (MapDoor.flags, DOOR_): how Brian leaves (StartBrianDeparture, read
                                with lhu); SpawnBrian reads them again from gArrivalFlags for his arrival */
    char unkE[2];
    s32 door;          /* the door: index into gMapModels */
    f32 yaw;          /* the door's angle (SetDoorWarp): the way Brian faces as he leaves through it */
}PendingWarp;
/* inventory.c */
typedef struct {
    u16 mapID;
    u16 submapID;
    f32 xpos;
    f32 zpos;
    f32 yrot;
}WingDest;                /* a Wings destination (gWingData[itemArg1], UseWings): the map, the submap and Brian's place */
/* shadow.c */
typedef struct {
    Mtx list[3];        /* the frame's matrices from the start, the draw functions' mtxList: [0] the projection and
                           [2] the view (UpdateCamera), [1] the HUD's orthographic projection (InitRoomCamera) */
    Mtx drawMtx[1];     /* list[3] on: one matrix per draw, gMatrixHead counts them (segment 2 + 3 + n); the code
                           indexes past this member's end, into the rest of FrameDynamic */
} mkst;
typedef struct {
    mkst mtx;                      /* the frame's matrices */
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
    f32 x;
    f32 y;
    f32 z;
    f32 xrot;
    f32 yrot;
    f32 zrot;
    f32 size;
}ShadowRec;
/* initialize.c */
typedef struct
{
    unsigned int inst1;
    unsigned int inst2;
    unsigned int inst3;
    unsigned int inst4;
} __osExceptionVector;
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
/*0x14*/ Gfx* dustDL;
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
} sMapList;                 /* a map's NPC records: SetupNPCs hands them to SpawnNPCs */
typedef struct {
    Gfx* setup;             /* drawn once before the first visible prop of this type */
    Gfx* model;             /* drawn for each visible prop */
} MapPropGfx;               /* per prop type (DrawMapProps) */
typedef struct {
    u8* rom;                         /* 0x800 bytes to gSkyPalettes, then 0x4000 to gSkyTexture */
    Gfx* dl;
} SkyRecord;                         /* gSkyRecords[], picked by MapLight.unk1C */

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
    u16 flags;               /* MAP_ (below the struct); copied to gMapFlags */
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
    struct EncounterSet* encounterSets; /* the submaps' encounter sets, one EncounterSet each (include/map_format.h) */
    sMapList* npcList;      /* the NPC records and their count (SetupNPCs) */
    s32 regionBlock;        /* the region block loaded with the map (gRegionBlocks[]) */
    char unk40[4];          /* the last byte is the number of submaps the data gives the map; no code reads it */
}sMapData;                  /* 0x44: a map's header (gMapTable[map]; the current one is gMapHeader) */
/* sMapData.flags (gMapFlags): the kind of place. gMapTable gives 0x1 to maps 0, 1, 4, 6 and 8, 0x2 to maps 13 to 25,
   0x4 to maps 2, 3, 5, 7 and 9 to 12, 0x8 to maps 26 to 35; the names say what each holds and what the code does
   with it */
#define MAP_TOWN       0x01  /* a town: time passes (UpdateTimeOfDay); leaving it for a MAP_FIELD sets the Return
                                point (LoadMap); the compass is drawn */
#define MAP_INTERIOR   0x02  /* buildings and rooms: LoadMap also loads the interiors' shared block; the Wings do
                                nothing here */
#define MAP_FIELD      0x04  /* the open land: time passes, the Return spell works only where the flags are exactly
                                MAP_FIELD, the compass is drawn */
#define MAP_DUNGEON    0x08  /* entering it sets the Exit point (LoadMap); the Exit spell works here (but in maps 30
                                and 34), the Wings do not; the compass is drawn */
#define MAP_FOG_SKY    0x80  /* the screen behind the scene takes the fog colour, not black (StepLightAnim): map 0,
                                maps 26 to 29 and map 35 */
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
    u16 flags;   /* 0x04: SpawnEffectObject's flags argument for the effect (ProjSlot.flags) */
    u16 unk6;
    f32 size_x;     /* 0x08 */
    f32 size_y;     /* 0x0C */
    s16 trail_model;    /* 0x10: the trail stage (each stage laid out as a SpellAnimStage) */
    u16 trail_rate;     /* 0x12: its high byte the frames between two trail pieces (UpdateSpells); the
                           whole value is also the piece's colour argument (SpawnSpellTrail), whose low byte
                           SpawnEffectObject keeps */
    u16 trail_flags;    /* 0x14: its flags argument (ProjSlot.flags); 0x10 = turn by lifetime x pi / 10 */
    u16 unk16;
    f32 trail_size_x;   /* 0x18 */
    f32 trail_size_y;   /* 0x1C */
    s16 land_model;     /* 0x20: the landing stage (StartSpellLandVisual), < 0 = none */
    u16 land_color;     /* 0x22 */
    u16 land_flags;  /* 0x24: the landing effect's flags argument */
    u16 unk26;
    f32 land_size_x;    /* 0x28 */
    f32 land_size_y;    /* 0x2C */
    s16 hit_model;      /* 0x30: the hit stage (StartSpellHitVisual), < 0 = none */
    u16 hit_color;      /* 0x32 */
    u16 hit_flags;      /* 0x34: its flags argument; 0x40 = on the caster, 0x100 = at mid-body */
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
    f32 yrot;
    f32 xrot;
    f32 zrot;
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
    f32 useX;                       /* -> transform.rootMoveX: the use point (FindMapObjectInFront: Brian within 300 of it, squared) */
    f32 useZ;                      /* -> transform.rootMoveZ */
    f32 useYaw;                      /* -> transform.yspeed: the facing Brian needs, within 45 degrees, and the use box's angle */
    f32 useHalfWidth;                      /* -> transform.xspeed: the use box's half width; zspeed its half depth */
    f32 useHalfDepth;                      /* -> transform.zspeed */
    u8 openedFlagId;                       /* 0x20: -> transform.floorFlags, the id of its opened flag; IsMapObjectOpened picks the animation
                                       from it */
    u8 item;                       /* -> transform.modelID: the item Brian takes when he opens it (UpdateBrian) */
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
    char unk0[0x1C];
    u16 nameOffset;                      /* offset into D_D37700 of the 0x20 bytes for gSpeakerName */
    u16 textOffset;                      /* offset of the 0x200 bytes for gTextBuffer */
} SignText;                       /* a SignObject as LoadSignText reads it: its name and text offsets */
typedef struct {
    SignText* text;
    f32 readYaw;
} SignRef;                        /* a SignSlot as LoadSignText reads it (UpdateBrian passes gSignTarget): text is
                                     SignSlot.sign seen as a SignText, readYaw is SignSlot.angle */

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
    s32 flags;                      /* 0x18: SIGN_ */
    u16 name;                       /* 0x1C: its name, an offset from D_D37700 (LoadSignText: 0x20 bytes) */
    u16 text;                       /* 0x1E: its text, an offset from D_D37700 (0x200 bytes; SignText reads both) */
} SignObject;                   /* 0x20; 38 of them at gSigns */
/* SignObject.flags */
#define SIGN_ANY_SIDE  0x1  /* can be read from any side, not only from its front (FindSignTarget); no sign has it */
#define SIGN_NO_POSE   0x2  /* Brian does not take the reading pose: BRIAN_ANIM_NO_TARGET instead of SIGN_START, and no
                               SIGN_END after the message (UpdateBrian, PLAYER_SIGN_NO_POSE); one sign has it, a
                               signpost on map 28 */
typedef struct {
    SignObject* sign;
    f32 angle;                      /* Brian's yaw minus the direction to the sign, set when found (SignRef.readYaw) */
    s32 unk8;
} SignSlot;                     /* 0xC: a sign in the room (gRoomSigns); LoadSignText reads it as a SignRef */

/* ---- floor / navigation mesh data: shared by collision.c and lighting.c ---- */
typedef struct {
    s32 time;                       /* the period starts at this time of day */
    s32 palette;                    /* index into gSkyPalettes */
    s32 fadeFrames;
} DayPeriod;                     /* 0xC: one period of the day (4 at gDayPalettePeriods) */
/* Area collision mesh (names from vbhayden's Quest 64 scratches) */


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
