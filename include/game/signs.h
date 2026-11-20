#ifndef GAME_SIGNS_H
#define GAME_SIGNS_H

/* The functions of src/game/signs.c, declared once for every file that calls them. */

#include "common.h"

void LoadRoomSigns(void);
SignSlot* FindSignTarget(ActorTransform* brian);
void LoadSignText(SignRef* sign);

/* The state src/game/signs.c defines (.bss), in its storage order: the order of these declarations is the
 * unit's .bss layout (include/bss_order.h). */
extern Coordinates2D gSignOffset;
extern s32 D_80085AB8;   /* unreferenced: the offset's third word (offsets of this kind -
                                            gSpellOffset, gEnemyStep, gHudVector - are OddVectors whose third
                                            word is a scale; no code reads this one) */
extern s32 gRoomSignCount;
extern SignSlot gRoomSigns[16];   /* the room's signs: LoadRoomSigns writes one per sign, unbounded; 16
                                            fill the space to the next file (15 would too, with padding) */

/* The data src/game/signs.c defines. */
extern SignObject gSigns[38];
extern s32 D_8004EB60;   /* the table's length; no code reads it */

#endif
