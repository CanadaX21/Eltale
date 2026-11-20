#ifndef GAME_DUST_H
#define GAME_DUST_H

/* The functions of src/game/dust.c, declared once for every file that calls them. */

#include "common.h"

void ResetDust(void);
Dust* SpawnDust(Dust* tmpl);   /* spawn a dust puff from a template (dust.c); returns the new puff */
void DrawDust(mkst* dyn);
void LoadDustTexture(u8 fmt, u8* texture, s32 width, s32 height, u8 cms, u8 cmt);

/* The state src/game/dust.c defines (.bss), in its storage order: the order of these declarations is the
 * unit's .bss layout (include/bss_order.h). */
extern s32 gDustCount;
extern Dust gDust[0x100];   /* ResetDust and SpawnDust walk 0x100 */

#endif
