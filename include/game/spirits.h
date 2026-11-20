#ifndef GAME_SPIRITS_H
#define GAME_SPIRITS_H

/* The functions of src/game/spirits.c, declared once for every file that calls them. */

#include "common.h"

void ResetSpiritsFound(void);
void SetupMapSpirits(void);
Spirit* CheckSpiritPickup(ActorTransform* brian);
void UpdateSpiritSparkles(void);
void DrawSpiritSparkle(SpiritSparkle* sparkle);
u8 IsSpiritFound(u16 flag);
void SetSpiritFound(u16 flag);

/* The state src/game/spirits.c defines (.bss), in its storage order: the order of these declarations is the
 * unit's .bss layout (include/bss_order.h). */
extern s32 gSpiritCount;   /* spirits on this submap */
extern Spirit gSubmapSpirits[8];   /* this submap's spirits (SetupMapSpirits) */
extern CollisionPollingResult gSpiritFloorPoll;   /* the floor poll for a spirit's height */
extern u8 gSpiritsFound[0x10];   /* spirits found: one bit per spirit flag (IsSpiritFound / SetSpiritFound) */

/* The data src/game/spirits.c defines. */
extern SubmapSpirits gSubmapSpiritTable[43];
extern Vtx gSparkleVertices[4];
extern Gfx gSparkleDisplayList[];
extern u8 gSparkleFrames[4][0x100];   /* src/game/spirits.c's own assembly: the four frames, from the ROM */
extern u8 gSparkleEnvColors[2][4];   /* two RGBA env colours */

#endif
