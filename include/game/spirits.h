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

#endif
