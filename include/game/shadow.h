#ifndef GAME_SHADOW_H
#define GAME_SHADOW_H

/* The functions of src/game/shadow.c, declared once for every file that calls them. */

#include "common.h"

void ResetShadows(void);
void AddShadow(f32 x, f32 y, f32 z, f32 xrot, f32 yrot, f32 zrot, f32 size);   /* the shadow */
void DrawShadows(mkst* dyn);

#endif
