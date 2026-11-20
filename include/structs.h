#ifndef _STRUCTS_H_
#define _STRUCTS_H_

typedef struct Vec3f {
  union {
    struct {
      f32 x;
      f32 y;
      f32 z;
    };
    f32 f[3];
  };
} Vec3f;

/* The spell record: gSpellDefs[element] points at each element's first one;
   field names from vbhayden's Quest 64 scratches. Used by spells.c (the spells) and player.c (Brian casting). */
typedef struct {
    u16 minSpirits;
    u16 on_hit;         /* 0x02: SPELL_IN_BATTLE, SPELL_IN_FIELD, SPELL_PIERCES, SPELL_AIM_ALONG (below) */
    u16 presses;        /* 0x04: 1-3, the MP cost index */
    u8 path[3];         /* 0x06: the directions; path[0] = the element */
    u8 unk9;
    u16 lifetime;       /* 0x0A */
    u16 power;          /* 0x0C */
    u16 hit_rate;       /* 0x0E: % */
    u16 spell_type;     /* 0x10: SPELL_EFFECT_ (below) */
    u16 projectile_type;/* 0x12 */
    u16 groundMode;
    u16 auto_hit;       /* 0x16 */
    u16 target_type;    /* 0x18: SPELL_TARGET_ (below) */
    u16 unk1A;
    f32 radius;         /* 0x1C: hit radius (collision 0) / effect scale */
    u16 shots;          /* 0x20: > 1 = queued extra shots */
    u16 shot_gap;       /* 0x22: frames between shots */
    u16 delay;          /* 0x24: start delay (object +0x08) */
    u16 originType;
    u16 homing;         /* 0x28: movement 0, index into its steering-strength table (gSpellHomingStrength) */
    u16 gravity;        /* 0x2A: bit 0 = falls 0.3 per frame (movements 0-2) */
    u16 spawn_pattern;  /* 0x2C: start of the offset table 0x800C1878 */
    u16 speed;          /* 0x2E: movement 0, index into its speed table (gSpellSpeeds) */
    s16 spawn_radius;   /* 0x30: index into 0x8004ED54 */
    u8 effects[12];     /* 0x32 */
    char unk3E[6];
} SpellDef;             /* 0x44: the spell record (gSpellDefs[element][index]) */

/* SpellDef.on_hit */
#define SPELL_IN_BATTLE  0x1   /* castable in battle (GetSpellAvailability) */
#define SPELL_IN_FIELD   0x2   /* castable out of battle */
#define SPELL_PIERCES    0x4   /* keeps going after a hit (UpdateSpells); DamageEnemy and DamageBrian receive on_hit, and
                                  read it as HIT_NO_MELEE */
#define SPELL_AIM_ALONG  0x8   /* SpellTarget_Facing aims along the spell's own direction, not the caster's */
/* (0x10 and 0x20 are set on some spells too; no code reads them: they reach DamageEnemy and DamageBrian, which read
   only HIT_NO_MELEE, HIT_CRITICAL and HIT_PHYSICAL) */
/* SpellDef.spell_type: the handler gSpellEffectHandlers runs on a hit */
#define SPELL_EFFECT_DAMAGE        0
#define SPELL_EFFECT_HEAL          1
#define SPELL_EFFECT_DRAIN_MP      2
#define SPELL_EFFECT_ESCAPE        3
#define SPELL_EFFECT_RETURN        4
#define SPELL_EFFECT_STATUS_ONLY   5
#define SPELL_EFFECT_EXIT          6
#define SPELL_EFFECT_INSTANT_KILL  7
#define SPELL_EFFECT_CURE          8
#define SPELL_EFFECT_DAMAGE_CURE   9
/* SpellDef.target_type: the handler gSpellTargetHandlers picks the target with */
#define SPELL_TARGET_FACING       0   /* the enemy nearest the caster's (or the spell's) direction */
#define SPELL_TARGET_FIRST_ENEMY  1
#define SPELL_TARGET_SELF         2

#endif

