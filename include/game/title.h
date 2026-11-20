#ifndef GAME_TITLE_H
#define GAME_TITLE_H

/* The functions of the title screen overlay (src/title_overlay/: title_screen.c, title_menu.c, title_fade.c), declared once for every file that calls them. */

#include "common.h"

/* title_screen.c */
/* A fade of the title screen (TitleScreen_UpdateFade, TitleScreen_DrawFade). */
typedef struct TitleFade {
    s16 active;     /* -1 while a fade runs */
    u16 framesLeft;
    f32 step;       /* alpha change per frame: 255 / the fade's length */
    f32 alpha;
} TitleFade;

void TitleScreen_Init(void);
s32 TitleScreen_Update(void);
s32 TitleScreen_Step(void);
void TitleScreen_UpdateMenu(void);
void TitleScreen_FinishSelection(void);

/* title_menu.c */
void TitleScreen_DrawMenu(s32 menuState);   /* still assembly */

/* title_fade.c */
void TitleScreen_UpdateFade(void);
void TitleScreen_DrawFade(TitleFade* fade);

/* The state src/title_overlay/title_screen.c defines (.bss), in its storage order: the order of these declarations is the
 * unit's .bss layout (include/bss_order.h). */
extern s32 gTitleState;
extern s32 gTitleTimer;
extern s32 D_80226D98_DC62B8;
extern s32 gTitlePulseAlpha;
extern s32 gTitlePulseStep;
extern s32 gTitleMenuState;

/* The data src/title_overlay/title_screen.c defines. */
extern s32 gTitleTransition;

/* The state src/title_overlay/title_menu.c defines (.bss), in its storage order: the order of these declarations is the
 * unit's .bss layout (include/bss_order.h). */
extern s8 gTitleOptionCursor;   /* the two-row choice's cursor (0 or 1), moved by the stick; TitleScreen_UpdateMenu reads it */
extern s32 gTitleBlinkAlpha;   /* the text's pulsing alpha (TitleScreen_UpdateMenu sets it each frame): the chosen row blinks */
extern s8 gTitleStickPrev;   /* last frame's stick directions: 1 up, 2 down, 4 left, 8 right */
extern s8 gTitleStickStepY;   /* this frame's newly pushed direction: -1 up, 1 down, else 0 */
extern s8 gTitleStickStepX;   /* -1 left, 1 right, else 0; nothing reads it */
extern u8* gTitleMessageText;   /* the message drawn under the menu (encoded text in the art) */

/* The data src/title_overlay/title_menu.c defines. */
extern s32 D_8020F200_DAE720;   /* TitleScreen_DrawMenu's first-call flag */
extern Gfx gTitleSetupDL[];
extern Gfx gTitleOpaqueModeDL[];
extern Gfx gTitleTranslucentModeDL[];
extern Gfx gTitleMenuWord0DL[];
extern Gfx gTitleMenuWord1DL[];
extern Gfx gTitleMenuWord2DL[];
extern Gfx gTitlePushStartDL[];
extern Gfx gTitleEnvAlphaModeDL[];
extern Gfx gTitleFadeDL[];   /* title_menu.c */
extern s32 D_8020F488_DAE9A8[3][6];
extern s32 D_8020F4D0_DAE9F0;
extern s8 D_8020F4D4_DAE9F4;
extern s8 D_8020F4D8_DAE9F8;
extern s16 D_8020F4DC_DAE9FC[3][2];

/* The state src/title_overlay/title_fade.c defines (.bss), in its storage order: the order of these declarations is the
 * unit's .bss layout (include/bss_order.h). */
extern s32 gTitleFadeColor;

/* The data src/title_overlay/title_fade.c defines. */
extern s32 gTitleFadeKind;   /* the fade's kind: 0 none, 1 to black, 2 from black, 3 to white, 4 from white */
extern s32 gTitleFadeFrames;   /* the fade's length in frames */

/* The overlay's data the C uses, still assembly or binary in its image. */
extern u8 D_8020FE10_DAF330[];   /* the push-start image's palette (256 RGBA16 entries) */
extern u8 D_80210010_DAF530[];   /* the push-start image (CI8, 100 wide) */
extern u8 D_80210D20_DB0240[];   /* the menu words' image (CI4, 176 wide, three words side by side) */

#endif
