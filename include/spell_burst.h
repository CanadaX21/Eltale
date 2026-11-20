#ifndef SPELL_BURST_H
#define SPELL_BURST_H

/* SpellBurstDef: spells.c's type, in a header of its own so the spell bank's generated records are written through it
 * (tools/splat_ext/anim.py lays records out from the project's headers). spells.c includes it. */
typedef struct {
    u16 kind;       /* 1 / 2: which burst StartSpellVisual starts (on the caster, at spawn) */
    u16 arg;
    u16 land_kind;  /* 0x04: on the target when the object lands (StartSpellLandVisual) */
    u16 land_arg;
    u16 hit_kind;   /* 0x08: on the target when a hit resolves (StartSpellHitVisual) */
    u16 hit_arg;
} SpellBurstDef;    /* gSpellBurstDefs[school][index], 0xC each: the per-slot effect entry */

#endif
