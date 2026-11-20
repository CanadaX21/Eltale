#include "common.h"
BSS_ORDER_PAD(0)
#include "game/shadow.h"
#include "game/math.h"
#include "game/main.h"
/* ---- shadow.c's .bss (0x80084B80-0x80085290): the shadows. Defined in their storage order, the order its header declares them in: IDO 5.3 lays the unit's
   .bss out by those declarations (include/bss_order.h). ---- */
ShadowRec gShadows[0x40];                /* AddShadow adds one while gShadowCount < 0x40 */
s32 gShadowCount;

void ResetShadows(void) {
    gShadowCount = 0;
}
void AddShadow(f32 x, f32 y, f32 z, f32 xrot, f32 yrot, f32 zrot, f32 size) {
    ShadowRec* shadow;

    if (gShadowCount < 0x40U) {        
        shadow = &gShadows[gShadowCount++];
        shadow->x = x;
        shadow->y = y;
        shadow->z = z;
        shadow->xrot = xrot;
        shadow->yrot = yrot;
        shadow->zrot = zrot;
        shadow->size = size;
    }
}
void DrawShadows(mkst* dyn) {
    s32 count;
    ShadowRec* shadow;
    MtxF mtx;

    count = gShadowCount;
    shadow = gShadows;
    if (count != 0) {
        gSPDisplayList(gDisplayListHead++, D_1000270);
        gSPFogPosition(gDisplayListHead++, gFogFar, gFogNear);
    }
    while (count != 0) {
        MatrixFromDirection(&mtx, shadow->xrot, shadow->yrot, shadow->zrot);
        mtx.mf[3][0] = shadow->x;
        mtx.mf[3][1] = shadow->y;
        mtx.mf[3][2] = shadow->z;
        ScaleMatrix(&mtx, shadow->size, shadow->size, shadow->size);
        guMtxF2L(mtx.mf, &dyn->drawMtx[gMatrixHead]);
        gSPMatrix(gDisplayListHead++, &D_2000000[gMatrixHead] + 3, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
        gMatrixHead++;
        gSPVertex(gDisplayListHead++, D_1000230, 4, 0);
        gSP2Triangles(gDisplayListHead++, 0, 1, 2, 0, 1, 3, 2, 0);
        shadow++;
        count--;
    }
}
