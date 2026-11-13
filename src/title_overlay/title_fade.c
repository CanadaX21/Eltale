#include "common.h"
#include "game/title.h"

/*
 * The title screen's fade, the third of the overlay's three source files (title_screen.c describes the overlay):
 * TitleScreen_UpdateFade starts and runs a fade of the kind the screen sets, TitleScreen_DrawFade draws each of its
 * frames. The .bss is not cleared by the loader (title_screen.c).
 */

typedef struct TitleFade {
    s16 active;     /* -1 while a fade runs */
    u16 framesLeft;
    f32 step;       /* alpha change per frame: 255 / the fade's length */
    f32 alpha;
} TitleFade;

extern Gfx* gDisplayListHead;
extern Gfx gTitleFadeDL[]; /* title_menu.c */

void TitleScreen_DrawFade(TitleFade* fade);

/* ---- .data ---- */
s32 gTitleFadeKind = 0; /* the fade's kind: 0 none, 1 to black, 2 from black, 3 to white, 4 from white */
s32 gTitleFadeFrames = 2; /* the fade's length in frames */

/* ---- .bss ---- */
s32 gTitleFadeColor;

/* Starts and runs the fade the screen asks for in gTitleFadeKind: on the fade's first frame the full-screen rectangle's
   alpha starts at 0 (to black or white) or 255 (from black or white), then moves 255 / gTitleFadeFrames a frame
   (TitleScreen_DrawFade); while no fade is asked for, nothing is drawn. */
void TitleScreen_UpdateFade(void) {
    static TitleFade sTitleFade = { 0, 0, 0.0f, 0.0f }; /* 0x8020F4F8 */

    if (gTitleFadeKind == 0) {
        if (sTitleFade.active != 0) {
            sTitleFade.active = 0;
        }
    } else {
        if (sTitleFade.active == 0) {
            sTitleFade.active = -1;
            sTitleFade.step = 255.0f / gTitleFadeFrames;
            sTitleFade.framesLeft = gTitleFadeFrames;
            switch (gTitleFadeKind) {
                case 1:
                case 3:
                    sTitleFade.alpha = 0.0;
                    break;
                case 2:
                case 4:
                    sTitleFade.alpha = 255.0;
                    break;
                default:
                    sTitleFade.alpha = 128.0;
                    break;
            }
        }
        TitleScreen_DrawFade(&sTitleFade);
    }
}

/* One frame of the fade: the rectangle's alpha moves by the step, up for the odd kinds and down for the even ones;
   the fade ends when the alpha leaves 0-255 or its frames run out; then the full-screen rectangle is drawn at that
   alpha, black for kinds 1 and 2, white for 3 and 4. The colour is also kept in gTitleFadeColor, which nothing
   reads. */
void TitleScreen_DrawFade(TitleFade* fade) {
    s32 alpha;
    s32 color;

    fade->framesLeft--;
    if (gTitleFadeKind & 1) {
        fade->alpha += fade->step;
    } else {
        fade->alpha -= fade->step;
    }
    switch (gTitleFadeKind) {
        case 3:
        case 4:
            color = 255;
            break;
        default:
            color = 0;
            break;
        /* NOTE: the original gives kinds 1 and 2 an arm of their own with the default's colour (merged into the
           default, or left out, the function compiles to another size) */
        case 1:
        case 2:
            color = 0;
            break;
    }
    if (fade->alpha <= 0.0f) {
        gTitleFadeKind = 0;
        fade->alpha = 0.0f;
        fade->active = 0;
    } else if (fade->alpha > 255.0f) {
        gTitleFadeKind = 0;
        fade->alpha = 255.0f;
        fade->active = 0;
    }
    if (fade->framesLeft == 0) {
        if (gTitleFadeKind == 3 || gTitleFadeKind == 1) {
            gTitleFadeKind = 0;
            fade->alpha = 255.0f;
            fade->active = 0;
        } else if (gTitleFadeKind == 4 || gTitleFadeKind == 2) {
            gTitleFadeKind = 0;
            fade->alpha = 0.0f;
            fade->active = 0;
        }
    }
    alpha = fade->alpha;
    gDPPipeSync(gDisplayListHead++);
    gDPSetPrimColor(gDisplayListHead++, 0, 0, color, color, color, alpha);
    gDPSetRenderMode(gDisplayListHead++, G_RM_AA_XLU_SURF, G_RM_AA_XLU_SURF2);
    gSPDisplayList(gDisplayListHead++, gTitleFadeDL);
    gDPFillRectangle(gDisplayListHead++, 0, 0, 336, 960);
    gTitleFadeColor = color;
}
