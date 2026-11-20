#include "common.h"
BSS_ORDER_PAD(0)
#include "game/text.h"
#include "game/dma.h"
#include "game/main.h"
#include "segment_symbols.h"

void string_copy(u8 *dst, u8 *src)
{
    s32 i;

    for(i = 0; src[i] != 0xFF; i++) {
        dst[i] = src[i];
    }
    dst[i] = 0xFF;
}
/* ---- text.c's .bss (0x80092C70-0x80092C80): the font state. Defined in their storage order, the order its header declares them in: IDO 5.3 lays the unit's
   .bss out by those declarations (include/bss_order.h). ---- */
s32 gFont;                               /* the current font */
s32 gFontSpacing;                        /* extra spacing */
void loadTextTextureAndPal(void) {
    dma_write(text_palettes_ROM_START, &gTextPalettes, 0x80);    //text palettes
    dma_write(font_sheet_ROM_START, &gFontSheet, 0xE790);  //text gfx
}
void DrawString(s32 x, s32 y, u8* str) {
    s32 cx;
    s32 cy;
    s32 idx;

    cx = x;
    cy = y;
    gDPPipeSync(gDisplayListHead++);
    gDPSetTextureLUT(gDisplayListHead++, G_TT_RGBA16);
    for (; *str != 0xFF; str++) {
        if (*str & 0x80) {
            idx = *str & 0x1F;
            switch (*str & 0x60) {
                case 0x00:
                    gDPLoadTextureTile_4b(gDisplayListHead++, &gFontSheet[idx], G_IM_FMT_CI, 304, 13, 0, 0, 303, 12, 0,
                                          G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK,
                                          G_TX_NOLOD, G_TX_NOLOD);
                    break;
                case 0x20:
                    gDPLoadTLUT_pal16(gDisplayListHead++, 0, &gTextPalettes[idx * 16]);
                    break;
                case 0x40:
                    if (idx < 16) {
                        gFont = idx;
                    } else {
                        gFontSpacing = idx;
                    }
                    break;
                case 0x60:
                    switch (idx) {
                        case 30:
                            gSPDisplayList(gDisplayListHead++, gMenuRenderDL);
                            break;
                        case 0:
                            cx = x;
                            cy += gFontMetrics[gFont].lineHeight;
                            break;
                        case 19:
                        case 20:
                            break;
                    }
                    break;
            }
        } else {
            idx = *str & 0x7F;
            if (*str == 0x7F) {
                cx += gFontMetrics[gFont].advance + gFontSpacing;
            } else {
                gSPTextureRectangle(gDisplayListHead++, cx << 2, cy << 2, (cx + gFontMetrics[gFont].width) << 2,
                                    (cy + gFontMetrics[gFont].height) << 2, G_TX_RENDERTILE,
                                    (gFontMetrics[gFont].width * idx) << 5, 0, 1 << 10, 1 << 10);
                cx += gFontMetrics[gFont].advance + gFontSpacing;
            }
        }
    }
    gDPPipeSync(gDisplayListHead++);
    gDPSetTextureLUT(gDisplayListHead++, G_TT_NONE);
}
