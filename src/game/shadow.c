#include "common.h"
extern Gfx* gDisplayListHead;
extern s16 gFogFar;
extern s16 gFogNear;
extern s32 gMatrixHead;
void MatrixFromDirection(MtxF* mtx, f32 x, f32 y, f32 z);
void ScaleMatrix(MtxF* m, f32 x, f32 y, f32 z);
extern s32 gShadowCount;
extern ShadowRec gShadows[];

void ResetShadows(void) {
    gShadowCount = 0;
}
void AddShadow(f32 x, f32 y, f32 z, f32 xrot, f32 yrot, f32 zrot, f32 size) {
    ShadowRec* shadow;

    if (gShadowCount < 0x40U) {        
        shadow = &gShadows[gShadowCount++];
        shadow->unk0 = x;
        shadow->unk4 = y;
        shadow->unk8 = z;
        shadow->unkC = xrot;
        shadow->unk10 = yrot;
        shadow->unk14 = zrot;
        shadow->unk18 = size;
    }
}
extern Gfx D_1000270[];         /* segment 1: the shadows' render setup */
extern Vtx D_1000230[];         /* segment 1: the shadow quad */
extern Mtx D_2000000[];         /* segment 2: this frame's matrices */
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
        MatrixFromDirection(&mtx, shadow->unkC, shadow->unk10, shadow->unk14);
        mtx.mf[3][0] = shadow->unk0;
        mtx.mf[3][1] = shadow->unk4;
        mtx.mf[3][2] = shadow->unk8;
        ScaleMatrix(&mtx, shadow->unk18, shadow->unk18, shadow->unk18);
        guMtxF2L(mtx.mf, &dyn->drawMtx[gMatrixHead]);
        gSPMatrix(gDisplayListHead++, &D_2000000[gMatrixHead] + 3, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
        gMatrixHead++;
        gSPVertex(gDisplayListHead++, D_1000230, 4, 0);
        gSP2Triangles(gDisplayListHead++, 0, 1, 2, 0, 1, 3, 2, 0);
        shadow++;
        count--;
    }
}
