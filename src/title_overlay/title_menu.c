#include "common.h"
#include "game/title.h"

/*
 * The title screen's drawing, the second of the overlay's three source files (title_screen.c describes the overlay):
 * TitleScreen_DrawMenu (the background, the logo, the menu and its cursor, the messages of each menu state) and its
 * data: the display lists, the menu words' rectangles, the cursor and the drawing's state. The .bss is not cleared
 * by the loader (title_screen.c).
 */

extern u8 D_8020FE10_DAF330[];  /* the push-start image's palette (256 RGBA16 entries) */
extern u8 D_80210010_DAF530[];  /* the push-start image (CI8, 100 wide) */
extern u8 D_80210D20_DB0240[];  /* the menu words' image (CI4, 176 wide, three words side by side) */

/* ---- .data ---- */
s32 D_8020F200_DAE720 = 0; /* TitleScreen_DrawMenu's first-call flag */

/* the screen's set-up */
Gfx gTitleSetupDL[] = {
    gsDPPipeSync(),
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsSPClearGeometryMode(0xFFFFFFFF),
    gsSPSetGeometryMode(G_SHADE | G_SHADING_SMOOTH),
    gsSPTexture(0x8000, 0x8000, 0, G_TX_RENDERTILE, G_ON),
    gsDPSetTextureConvert(G_TC_FILT),
    gsDPSetTextureDetail(G_TD_SHARPEN),
    /* render-mode flags given to the alpha compare: only its two low bits (none) take effect */
    gsDPSetAlphaCompare(AA_EN | CVG_X_ALPHA | ALPHA_CVG_SEL | FORCE_BL | G_RM_PASS),
    gsDPSetTexturePersp(G_TP_NONE),
    gsDPSetTextureFilter(G_TF_POINT),
    gsSPEndDisplayList(),
};

Gfx gTitleOpaqueModeDL[] = {
    gsDPPipeSync(),
    gsDPSetRenderMode(G_RM_OPA_SURF, G_RM_OPA_SURF2),
    gsDPSetCombineMode(G_CC_DECALRGBA, G_CC_DECALRGBA),
    gsSPEndDisplayList(),
};

Gfx gTitleTranslucentModeDL[] = {
    gsDPPipeSync(),
    gsDPSetRenderMode(G_RM_XLU_SURF, G_RM_XLU_SURF2),
    gsDPSetCombineMode(G_CC_DECALRGBA, G_CC_DECALRGBA),
    gsDPSetTextureLUT(G_TT_RGBA16),
    gsSPEndDisplayList(),
};

/* the three menu words, one slice of the image each */
Gfx gTitleMenuWord0DL[] = {
    gsDPLoadTextureTile_4b(D_80210D20_DB0240, G_IM_FMT_CI, 176, 0, 0, 0, 53, 11, 0, G_TX_NOMIRROR | G_TX_WRAP,
                           G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD),
    gsSPTextureRectangle(0x00D4, 0x0280, 0x01AC, 0x02B0, G_TX_RENDERTILE, 0, 0, 0x0400, 0x0400),
    gsSPEndDisplayList(),
};

Gfx gTitleMenuWord1DL[] = {
    gsDPLoadTextureTile_4b(D_80210D20_DB0240, G_IM_FMT_CI, 176, 0, 54, 0, 108, 11, 0, G_TX_NOMIRROR | G_TX_WRAP,
                           G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD),
    gsSPTextureRectangle(0x0204, 0x0280, 0x02E0, 0x02B0, G_TX_RENDERTILE, 0x06C0, 0, 0x0400, 0x0400),
    gsSPEndDisplayList(),
};

Gfx gTitleMenuWord2DL[] = {
    gsDPLoadTextureTile_4b(D_80210D20_DB0240, G_IM_FMT_CI, 176, 0, 109, 0, 174, 11, 0, G_TX_NOMIRROR | G_TX_WRAP,
                           G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD),
    gsSPTextureRectangle(0x032C, 0x0280, 0x0434, 0x02B0, G_TX_RENDERTILE, 0x0DA0, 0, 0x0400, 0x0400),
    gsSPEndDisplayList(),
};

/* the push-start image */
Gfx gTitlePushStartDL[] = {
    gsDPPipeSync(),
    gsDPSetRenderMode(G_RM_XLU_SURF, G_RM_XLU_SURF2),
    gsDPSetCombineLERP(0, 0, 0, TEXEL0, TEXEL0, 0, ENVIRONMENT, 0, 0, 0, 0, TEXEL0, TEXEL0, 0, ENVIRONMENT, 0),
    gsDPSetTextureLUT(G_TT_RGBA16),
    gsDPLoadTLUT_pal256(D_8020FE10_DAF330),
    gsDPLoadTextureTile(D_80210010_DAF530, G_IM_FMT_CI, G_IM_SIZ_8b, 100, 0, 0, 0, 90, 14, 0, G_TX_NOMIRROR | G_TX_WRAP,
                        G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD),
    gsSPTextureRectangle(0x01C8, 0x0280, 0x0334, 0x02BC, G_TX_RENDERTILE, 0, 0, 0x0400, 0x0400),
    gsSPEndDisplayList(),
};

Gfx gTitleEnvAlphaModeDL[] = {
    gsDPPipeSync(),
    gsDPSetCombineLERP(0, 0, 0, TEXEL0, TEXEL0, 0, ENVIRONMENT, 0, 0, 0, 0, TEXEL0, TEXEL0, 0, ENVIRONMENT, 0),
    gsSPEndDisplayList(),
};

/* the fade's rectangle: primitive colour and alpha */
Gfx gTitleFadeDL[] = {
    gsDPPipeSync(),
    gsDPSetCombineLERP(PRIMITIVE, TEXEL0, ENVIRONMENT, 0, 0, 0, 0, PRIMITIVE, 0, 0, 0, PRIMITIVE, 0, 0, 0, PRIMITIVE),
    gsSPEndDisplayList(),
};

/* the menu words' screen rectangles (x, y, the image's column, row, width, height); no code reads it */
s32 D_8020F488_DAE9A8[3][6] = {
    { 0x35, 0xA0, 0x00, 0, 0x36, 0xC },
    { 0x81, 0xA0, 0x36, 0, 0x37, 0xC },
    { 0xCB, 0xA0, 0x6D, 0, 0x42, 0xC },
};
s32 D_8020F4D0_DAE9F0 = 0;
s8 D_8020F4D4_DAE9F4 = 0;
s8 D_8020F4D8_DAE9F8 = 0;
s16 D_8020F4DC_DAE9FC[3][2] = { { 0x2A, 0xA0 }, { 0x77, 0xA1 }, { 0xBF, 0xA0 } };

/* ---- .bss ---- */
s8 gTitleOptionCursor;  /* the two-row choice's cursor (0 or 1), moved by the stick; TitleScreen_UpdateMenu reads it */
s32 gTitleBlinkAlpha;   /* the text's pulsing alpha (TitleScreen_UpdateMenu sets it each frame): the chosen row blinks */
s8 gTitleStickPrev;     /* last frame's stick directions: 1 up, 2 down, 4 left, 8 right */
s8 gTitleStickStepY;    /* this frame's newly pushed direction: -1 up, 1 down, else 0 */
s8 gTitleStickStepX;    /* -1 left, 1 right, else 0; nothing reads it */
u8* gTitleMessageText;  /* the message drawn under the menu (encoded text in the art) */

#pragma GLOBAL_ASM("asm/nonmatchings/title_overlay/title_menu/TitleScreen_DrawMenu.s")
