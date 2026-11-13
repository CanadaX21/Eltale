#ifndef GAME_DUST_H
#define GAME_DUST_H

/* The functions of src/game/dust.c, declared once for every file that calls them. */

#include "common.h"

void ResetDust(void);
Dust* SpawnDust(Dust* tmpl);   /* spawn a dust puff from a template (dust.c); returns the new puff */
void DrawDust(mkst* dyn);
void LoadDustTexture(u8 fmt, u8* texture, s32 width, s32 height, u8 cms, u8 cmt);

#endif
