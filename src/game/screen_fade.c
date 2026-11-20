#include "common.h"
BSS_ORDER_PAD(0)
#include "game/screen_fade.h"
#include "game/main.h"

/* ---- screen_fade.c's .bss (0x8008CA50-0x8008CE60): the screen fade. Defined in their storage order, the order its header declares them in: IDO 5.3 lays the unit's
   .bss out by those declarations (include/bss_order.h), and it 8-aligns every variable of 8 bytes or more. ---- */
u16 gScreenFadeFlags;
u8 gFadeRamp[256][4];                    /* RGBA: grey 0xDC, alpha ramping 0 -> 0xFE */
s32 gScreenFadeFrames;
FadeRGBA* gFadeRampEntry;                /* the fade's current colour ramp entry */
void BuildColourRamp(void) {
    u32 i;

    gScreenFadeFlags = 0;
    gScreenFadeFrames = 0;
    for (i = 0; i < 256; i++) {
        gFadeRamp[i][0] = 0xDC;
        gFadeRamp[i][1] = 0xDC;
        gFadeRamp[i][2] = 0xDC;
        gFadeRamp[i][3] = (i * 0xFF) >> 8;
    }
}
void DrawScreenFade(mkst* arg0) {
    if (gScreenFadeFlags & FADE_SETUP) {
        gSPDisplayList(gDisplayListHead++, gScreenFadeSetupGfx);
    }
    if (gScreenFadeFlags & FADE_ON) {
        gDPPipeSync(gDisplayListHead++);
        gSPMatrix(gDisplayListHead++, &D_2000000[1], G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_PROJECTION);
        guMtxIdent(&arg0->drawMtx[gMatrixHead]);
        gSPMatrix(gDisplayListHead++, &D_2000000[gMatrixHead] + 3, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
        gMatrixHead++;
        gDPSetEnvColor(gDisplayListHead++, gFadeRampEntry->r, gFadeRampEntry->g, gFadeRampEntry->b, gFadeRampEntry->a);
        if (gFadeRampEntry->a == 0xFF) {
            gDPSetRenderMode(gDisplayListHead++, G_RM_OPA_SURF, G_RM_OPA_SURF2);
        } else {
            gDPSetRenderMode(gDisplayListHead++, G_RM_CLD_SURF, G_RM_CLD_SURF2);
        }
        gSPDisplayList(gDisplayListHead++, gScreenFadeGfx);
        if (gScreenFadeFrames != 0) {
            if (gScreenFadeFlags & FADE_BRIGHTEN) {
                gFadeRampEntry--;
            } else {
                gFadeRampEntry++;
            }
            gScreenFadeFrames--;
            if (!gScreenFadeFrames & !(gScreenFadeFlags & FADE_HOLD)) {
                gScreenFadeFlags &= ~FADE_ON;
            }
        }
    }
}

void StartScreenFade(s32 frames, u16 flags, void* ramp) {
    gScreenFadeFrames = frames;
    gScreenFadeFlags = flags | FADE_ON | (gScreenFadeFlags & FADE_SETUP);
    gFadeRampEntry = ramp;
}
