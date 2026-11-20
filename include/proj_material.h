#ifndef PROJ_MATERIAL_H
#define PROJ_MATERIAL_H

/* ProjModelDef: effects.c's type, in a header of its own so the spell bank's generated records are written through it
 * (tools/splat_ext/anim.py lays records out from the project's headers). game/effects.h includes it. */
typedef struct {
    u16 flags;              /* 1: 16 x 16 (else 32 x 32), 2: intensity (else CI with a gProjPalettes palette),
                               4 / 8: animated frames, 0x10: mirrored, 0x20: texgen (environment mapped), 0x40 */
    u16 frameMask;          /* 0x02: with flag 8 */
    u16 frameShift;         /* 0x04: the slot's age >> this picks the frame (SetProjectileTexture) */
    u16 palette;            /* 0x06: index into gProjPalettes */
    u8* texture;            /* 0x08: NULL = untextured */
    s16 s;                  /* 0x0C: texture scroll position, 10.2 */
    s16 t;
    s16 ds;                 /* 0x10: scroll per frame */
    s16 dt;
} ProjModelDef;         /* a projectile's material: gProjModelDefs[], 0x14 each (Quest D_800DA828); SetProjectileTexture */

#endif
