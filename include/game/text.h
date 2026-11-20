#ifndef GAME_TEXT_H
#define GAME_TEXT_H

/* The functions of src/game/text.c, declared once for every file that calls them. */

#include "common.h"

void string_copy(u8 *dst, u8 *src);
void loadTextTextureAndPal(void);
void DrawString(s32 x, s32 y, u8* str);

/* The state src/game/text.c defines (.bss), in its storage order: the order of these declarations is the
 * unit's .bss layout (include/bss_order.h). */
extern s32 gFont;   /* the current font */
extern s32 gFontSpacing;   /* extra spacing */

typedef struct {
    s32 width;                      /* a glyph's cell width (texels) */
    s32 height;
    s32 advance;                    /* pen advance after a glyph */
    s32 lineHeight;
} FontMetrics;                      /* gFontMetrics[16] */

/* Storage the C does not define yet (assembly or binary), declared here for its type. */
extern FontMetrics gFontMetrics[];   /* link name: _622F0_bin + 0x70 */

#endif
