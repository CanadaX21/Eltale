#include "common.h"

/* Part of the cutscene bank (resident at 0x8039ADB0 from boot), after gBossBaseData: the boss models' animation
 * tables (twice) and the bosses' names (with a second copy of their table). */

extern AnimEntry gSolvaringAnims[];   /* Solvaring's animation table (boss_solvaring) */
extern AnimEntry gZelseAnims[];   /* Zelse's animation table (boss_zelse) */
extern AnimEntry gNeptyAnims[];   /* Nepty's animation table (boss_nepty) */
extern AnimEntry gShilfAnims[];   /* Shilf's animation table (boss_shilf) */
extern AnimEntry gFargoAnims[];   /* Fargo's animation table (boss_fargo) */
extern AnimEntry gGuiltyAnims[];   /* Guilty's animation table (boss_guilty) */
extern AnimEntry gBeigisAnims[];   /* Beigis's animation table (boss_beigis) */
extern AnimEntry gMammonAnims[];   /* Mammon's animation table (boss_mammon) */

/* The same 8 animation tables as gBossAnimTables; no code reference to this copy was found. */
AnimEntry* D_803AB9E0[8] = {
    gSolvaringAnims,   /* Solvaring */
    gZelseAnims,   /* Zelse */
    gNeptyAnims,   /* Nepty */
    gShilfAnims,   /* Shilf */
    gFargoAnims,   /* Fargo */
    gGuiltyAnims,   /* Guilty */
    gBeigisAnims,   /* Beigis */
    gMammonAnims,   /* Mammon */
};

/* The bosses' names, which gBossNames points at (the game's text encoding, each ended by 0xFF; the zero
 * bytes after one pad to the next), read from the ROM at build time. */
GLOBAL_ASM(
.section .data
glabel D_803ABA00
.incbin "baserom.jp.z64", 0xD85710, 0x8
glabel D_803ABA08
.incbin "baserom.jp.z64", 0xD85718, 0x8
glabel D_803ABA10
.incbin "baserom.jp.z64", 0xD85720, 0x8
glabel D_803ABA18
.incbin "baserom.jp.z64", 0xD85728, 0x4
glabel D_803ABA1C
.incbin "baserom.jp.z64", 0xD8572C, 0x8
glabel D_803ABA24
.incbin "baserom.jp.z64", 0xD85734, 0x8
glabel D_803ABA2C
.incbin "baserom.jp.z64", 0xD8573C, 0x8
glabel D_803ABA34
.incbin "baserom.jp.z64", 0xD85744, 0x4
)
extern u8 D_803ABA00[], D_803ABA08[], D_803ABA10[], D_803ABA18[], D_803ABA1C[], D_803ABA24[], D_803ABA2C[], D_803ABA34[];

/* The same 8 pointers as gBossNames; no code reference to this copy was found. */
u8* D_803ABA38[8] = { D_803ABA00, D_803ABA08, D_803ABA10, D_803ABA18, D_803ABA1C, D_803ABA24, D_803ABA2C, D_803ABA34 };

/* The boss models' animation tables by model ID (DrawBoss; gMonsterAnims during a boss battle). */
AnimEntry* gBossAnimTables[8] = {
    gSolvaringAnims,   /* Solvaring */
    gZelseAnims,   /* Zelse */
    gNeptyAnims,   /* Nepty */
    gShilfAnims,   /* Shilf */
    gFargoAnims,   /* Fargo */
    gGuiltyAnims,   /* Guilty */
    gBeigisAnims,   /* Beigis */
    gMammonAnims,   /* Mammon */
};

/* The first 8 bytes of the byte data that continues after this unit, read from the ROM at build time. */
GLOBAL_ASM(
.section .data
glabel D_803ABA78
.incbin "baserom.jp.z64", 0xD85788, 0x8
)
