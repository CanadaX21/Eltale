/*Same as 1BCD0.c in Quest64*/
#include "common.h"


extern s32 gDustCount;
extern Dust gDust[0x100];
extern Gfx* gDisplayListHead;
extern u8 gTexMaskSides[7]; /* texture side lengths 2, 4 .. 128: a side of up to gTexMaskSides[i] texels gets mask i + 1 */

void LoadDustTexture(u8 fmt, u8* texture, s32 width, s32 height, u8 cms, u8 cmt);

//#pragma GLOBAL_ASM("asm/nonmatchings/game/dust/ResetDust.s")
void ResetDust(void) {
    Dust* dust;
    s32 n;

    dust = gDust;
    n = 0x100;
    while (n > 0) {
        dust->frames = 0;
        n--;
        dust++;
    } 
    gDustCount = 0;
}

//#pragma GLOBAL_ASM("asm/nonmatchings/game/dust/SpawnDust.s")
Dust* SpawnDust(Dust* tmpl) {
    Dust* ret = gDust;
    u32 i = 0x100;

    while (i > 0 && ret->frames != 0){
        i--;
        ret++;
    }
    if (i != 0) {
        *ret = *tmpl;
        gDustCount++;
    } else {
        ret = NULL;
    }
    return ret;
}

extern s32 gMatrixHead;
extern Mtx D_2000000[];
extern Camera gCamera;
extern Gfx gDustDefaultGfx[];
void MatrixFromDirection(MtxF* mtx, f32 x, f32 y, f32 z);
void ScaleMatrix(MtxF* m, f32 x, f32 y, f32 z);
void BuildLookAtMatrix(MtxF *mtx, f32 x, f32 y, f32 z, f32 atX, f32 atY, f32 atZ);
void BuildRotationMatrix(MtxF *mtx, f32 xrot, f32 yrot, f32 zrot);
void guMtxCatF(f32 a[4][4], f32 b[4][4], f32 out[4][4]);
void DrawDust(mkst* dyn) {
    s32 n;
    u8* tex;
    Dust* d;
    MtxF mtx;
    MtxF rot;

    tex = NULL;
    d = gDust;
    for (n = gDustCount; n != 0; d++) {
        if (d->frames == 0) {
            continue;
        }
        d->frames--;
        if (tex != d->dustTexture) {
            tex = d->dustTexture;
            LoadDustTexture(d->texFmt, tex, d->texWidth, d->texHeight, d->texCms, d->texCmt);
        }
        if (d->update != NULL) {
            ((void (*)(Dust*))d->update)(d);
        } else {
            gSPDisplayList(gDisplayListHead++, gDustDefaultGfx);
        }
        if (d->flags & 1) {
            MatrixFromDirection(&mtx, d->vx, d->vy, d->vz);
            mtx.mf[3][0] = d->x;
            mtx.mf[3][1] = d->y;
            mtx.mf[3][2] = d->z;
            ScaleMatrix(&mtx, d->scaleX, d->scaleX, d->scaleX);
        } else {
            BuildLookAtMatrix(&mtx, d->x, d->y, d->z, gCamera.cam_x, gCamera.cam_y, gCamera.cam_z);
            if (d->roll != 0.0f) {
                BuildRotationMatrix(&rot, 0.0f, 0.0f, d->roll);
                guMtxCatF(rot.mf, mtx.mf, mtx.mf);
            }
            ScaleMatrix(&mtx, d->scaleX, d->scaleY, 1.0f);
        }
        if (gMatrixHead < 486) {
            guMtxF2L(mtx.mf, &dyn->drawMtx[gMatrixHead]);
            gSPMatrix(gDisplayListHead++, &D_2000000[gMatrixHead] + 3, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
            gMatrixHead++;
            gSPDisplayList(gDisplayListHead++, d->dustDL);
        }
        n--;
        if (d->frames == 0) {
            gDustCount--;
        }
    }
}
void LoadDustTexture(u8 fmt, u8* texture, s32 width, s32 height, u8 cms, u8 cmt) {
    u8 masks;
    u8 maskt;

    for (masks = 0; masks < sizeof(gTexMaskSides); masks++) {
        if (gTexMaskSides[masks] >= width) {
            break;
        }
    }
    masks++;
    for (maskt = 0; maskt < sizeof(gTexMaskSides); maskt++) {
        if (gTexMaskSides[maskt] >= height) {
            break;
        }
    }
    maskt++;
    switch (fmt) {
        case 0:
            gDPLoadTextureBlock(gDisplayListHead++, texture, G_IM_FMT_RGBA, G_IM_SIZ_16b, width, height, 0, cms, cmt,
                                masks, maskt, G_TX_NOLOD, G_TX_NOLOD);
            gDPSetTextureLUT(gDisplayListHead++, G_TT_NONE);
            break;
        case 1:
            gDPLoadTextureBlock(gDisplayListHead++, texture, G_IM_FMT_RGBA, G_IM_SIZ_32b, width, height, 0, cms, cmt,
                                masks, maskt, G_TX_NOLOD, G_TX_NOLOD);
            gDPSetTextureLUT(gDisplayListHead++, G_TT_NONE);
            break;
        case 2:
            gDPLoadTextureBlock(gDisplayListHead++, texture, G_IM_FMT_I, G_IM_SIZ_8b, width, height, 0, cms, cmt,
                                masks, maskt, G_TX_NOLOD, G_TX_NOLOD);
            gDPSetTextureLUT(gDisplayListHead++, G_TT_NONE);
            break;
        case 3:
            gDPLoadTextureBlock(gDisplayListHead++, texture, G_IM_FMT_IA, G_IM_SIZ_8b, width, height, 0, cms, cmt,
                                masks, maskt, G_TX_NOLOD, G_TX_NOLOD);
            gDPSetTextureLUT(gDisplayListHead++, G_TT_NONE);
            break;
        case 4:
            gDPLoadTextureBlock_4b(gDisplayListHead++, texture, G_IM_FMT_I, width, height, 0, cms, cmt, masks, maskt,
                                   G_TX_NOLOD, G_TX_NOLOD);
            gDPSetTextureLUT(gDisplayListHead++, G_TT_NONE);
            break;
        case 5:
            gDPLoadTextureBlock_4b(gDisplayListHead++, texture, G_IM_FMT_IA, width, height, 0, cms, cmt, masks, maskt,
                                   G_TX_NOLOD, G_TX_NOLOD);
            gDPSetTextureLUT(gDisplayListHead++, G_TT_NONE);
            break;
        case 6:
            gDPLoadTextureBlock_4b(gDisplayListHead++, texture, G_IM_FMT_CI, width, height, 0, cms, cmt, masks, maskt,
                                   G_TX_NOLOD, G_TX_NOLOD);
            gDPSetTextureLUT(gDisplayListHead++, G_TT_RGBA16);
            break;
        case 7:
            gDPLoadTextureBlock(gDisplayListHead++, texture, G_IM_FMT_CI, G_IM_SIZ_8b, width, height, 0, cms, cmt,
                                masks, maskt, G_TX_NOLOD, G_TX_NOLOD);
            gDPSetTextureLUT(gDisplayListHead++, G_TT_RGBA16);
            break;
    }
}
