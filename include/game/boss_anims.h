#ifndef GAME_BOSS_ANIMS_H
#define GAME_BOSS_ANIMS_H

/* The bosses' animation tables src/game/boss_anims.c defines (in the cutscene bank). */

#include "common.h"

/* The data src/game/boss_anims.c defines. */
extern AnimEntry* D_803AB9E0[8];
extern u8* D_803ABA38[8];
extern AnimEntry* gBossAnimTables[8];   /* the boss models' animation tables by model ID (DrawBoss; gMonsterAnims during a boss battle) */

/* Labels of the unit's own assembly (its data still written as assembly in the .c) that its C uses. */
extern u8 D_803ABA00[];   /* src/game/boss_anims.c's own assembly */
extern u8 D_803ABA08[];   /* src/game/boss_anims.c's own assembly */
extern u8 D_803ABA10[];   /* src/game/boss_anims.c's own assembly */
extern u8 D_803ABA18[];   /* src/game/boss_anims.c's own assembly */
extern u8 D_803ABA1C[];   /* src/game/boss_anims.c's own assembly */
extern u8 D_803ABA24[];   /* src/game/boss_anims.c's own assembly */
extern u8 D_803ABA2C[];   /* src/game/boss_anims.c's own assembly */
extern u8 D_803ABA34[];   /* src/game/boss_anims.c's own assembly */

#endif
