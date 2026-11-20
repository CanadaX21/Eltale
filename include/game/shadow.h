#ifndef GAME_SHADOW_H
#define GAME_SHADOW_H

/* The functions of src/game/shadow.c, declared once for every file that calls them. */

#include "common.h"

void ResetShadows(void);
void AddShadow(f32 x, f32 y, f32 z, f32 xrot, f32 yrot, f32 zrot, f32 size);   /* the shadow */
void DrawShadows(mkst* dyn);

/* The state src/game/shadow.c defines (.bss), in its storage order: the order of these declarations is the
 * unit's .bss layout (include/bss_order.h). */
extern ShadowRec gShadows[0x40];   /* AddShadow adds one while gShadowCount < 0x40 */
extern s32 gShadowCount;

#endif
