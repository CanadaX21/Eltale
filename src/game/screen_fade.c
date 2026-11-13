#include "common.h"
#include "game/screen_fade.h"
extern u16 gScreenFadeFlags;
extern s32 gScreenFadeFrames;

typedef struct {
    u8 r, g, b, a;
} FadeRGBA;                         /* one fade colour */
extern FadeRGBA* gFadeRampEntry;   /* the fade's current colour ramp entry */
extern u8 gFadeRamp[256][4];       /* RGBA: grey 0xDC, alpha ramping 0 -> 0xFE */
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
extern Mtx D_2000000[];
extern Gfx gScreenFadeGfx[];
extern Gfx gScreenFadeSetupGfx[];
extern Gfx* gDisplayListHead;
extern s32 gMatrixHead;
void DrawScreenFade(mkst* arg0) {
    if (gScreenFadeFlags & 8) {
        gSPDisplayList(gDisplayListHead++, gScreenFadeSetupGfx);
    }
    if (gScreenFadeFlags & 1) {
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
            if (gScreenFadeFlags & 4) {
                gFadeRampEntry--;
            } else {
                gFadeRampEntry++;
            }
            gScreenFadeFrames--;
            if (!gScreenFadeFrames & !(gScreenFadeFlags & 2)) {
                gScreenFadeFlags &= ~1;
            }
        }
    }
}

void StartScreenFade(s32 frames, u16 flags, void* ramp) {
    gScreenFadeFrames = frames;
    gScreenFadeFlags = flags | 1 | (gScreenFadeFlags & 8);
    gFadeRampEntry = ramp;
}
