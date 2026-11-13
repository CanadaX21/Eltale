#include "common.h"

/* Part of the cutscene bank (ROM 0xD74AC0, resident at 0x8039ADB0 from boot): the boss files' ROM bounds, by boss
 * number. LoadMapBoss DMAs gBossFiles[bossNumber] to the boss buffer when its room loads; each pair is that boss
 * segment's own bounds, so a boss file is found wherever the ROM's layout puts it. */
extern u8 boss_solvaring_ROM_START[], boss_solvaring_ROM_END[];
extern u8 boss_zelse_ROM_START[], boss_zelse_ROM_END[];
extern u8 boss_nepty_ROM_START[], boss_nepty_ROM_END[];
extern u8 boss_shilf_ROM_START[], boss_shilf_ROM_END[];
extern u8 boss_fargo_ROM_START[], boss_fargo_ROM_END[];
extern u8 boss_guilty_ROM_START[], boss_guilty_ROM_END[];
extern u8 boss_beigis_ROM_START[], boss_beigis_ROM_END[];
extern u8 boss_mammon_ROM_START[], boss_mammon_ROM_END[];

bossFile gBossFiles[8] = {
    { (s32)boss_solvaring_ROM_START, (s32)boss_solvaring_ROM_END },/* 0 */
    { (s32)boss_zelse_ROM_START, (s32)boss_zelse_ROM_END },    /* 1 */
    { (s32)boss_nepty_ROM_START, (s32)boss_nepty_ROM_END },    /* 2 */
    { (s32)boss_shilf_ROM_START, (s32)boss_shilf_ROM_END },    /* 3 */
    { (s32)boss_fargo_ROM_START, (s32)boss_fargo_ROM_END },    /* 4 */
    { (s32)boss_guilty_ROM_START, (s32)boss_guilty_ROM_END },   /* 5 */
    { (s32)boss_beigis_ROM_START, (s32)boss_beigis_ROM_END },   /* 6 */
    { (s32)boss_mammon_ROM_START, (s32)boss_mammon_ROM_END },   /* 7 */
};

extern AnimEntry gSolvaringAnims[];   /* Solvaring's animation table (boss_solvaring) */
extern AnimEntry gZelseAnims[];   /* Zelse's animation table (boss_zelse) */
extern AnimEntry gNeptyAnims[];   /* Nepty's animation table (boss_nepty) */
extern AnimEntry gShilfAnims[];   /* Shilf's animation table (boss_shilf) */
extern AnimEntry gFargoAnims[];   /* Fargo's animation table (boss_fargo) */
extern AnimEntry gGuiltyAnims[];   /* Guilty's animation table (boss_guilty) */
extern AnimEntry gBeigisAnims[];   /* Beigis's animation table (boss_beigis) */
extern AnimEntry gMammonAnims[];   /* Mammon's animation table (boss_mammon) */

/* Each boss placement: its room (map, submap), its boss file (gBossFiles[]), its model's animation table (the
 * same as its gBossAnimTables entry; no code reads this copy) and where it stands. */
BossPlacement gBossPlacements[8] = {
    { 0x1F, 0x01, 0, 0, gSolvaringAnims, -3.0f, -17.5f, 0.0f },   /* 0: Solvaring */
    { 0x21, 0x00, 1, 0, gZelseAnims, 375.0f, -260.0f, -2.8f },   /* 1: Zelse */
    { 0x23, 0x00, 2, 0, gNeptyAnims, 420.0f, -173.0f, -1.77f },   /* 2: Nepty */
    { 0x1C, 0x00, 3, 0, gShilfAnims, 3974.0f, -1800.0f, -1.5706f },   /* 3: Shilf */
    { 0x1D, 0x00, 4, 0, gFargoAnims, 7044.0f, 1158.0f, -1.79f },   /* 4: Fargo */
    { 0x1E, 0x0A, 5, 0, gGuiltyAnims, 0.0f, 0.0f, 3.14f },   /* 5: Guilty */
    { 0x1E, 0x0E, 6, 0, gBeigisAnims, 1.5f, 42.8f, 3.1f },   /* 6: Beigis */
    { 0x22, 0x0A, 7, 0, gMammonAnims, -20.0f, -350.0f, -0.2f },   /* 7: Mammon */
};
