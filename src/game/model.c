#include "common.h"
typedef struct {
    char unk0[8];
    Vec3f unk8;                     /* a point ApplyPivotOffset moves a matrix's origin by */
} unk20A28s;
extern s32 gPartPositionCount;
void ApplyPivotOffset(MtxF* mtx, ModelPart* part);
void SampleSpline(ModelPart* part, ModelKey* key, s32 frame);
void BuildActorMatrix(MtxF* mf, f32 xrot, f32 yrot, f32 zrot, f32 scale, f32 x, f32 y, f32 z);
void DrawModelPart(MtxF* mf, Vec3f scale, s32 flags, s32 part, s32 frame, s32 frames, AnimEntry* anim,
                   ModelPart* parts, Mtx* mtxBuf);   /* draw one part and its children (recursive) */

void SetAnimation(ActorTransform* actor, s32 nextAnim, s32 nextFramerate, s32 afterAnim, s32 afterFramerate, u16 flags) {
    if (nextAnim != -1) {
        if (nextAnim != actor->animID) {
            actor->animID = nextAnim;
            actor->unk60 |= 0x400;
            actor->unk60 &= ~0x200;
        }
        actor->animFramerate = nextFramerate;
        if (flags & 1) {
            actor->animFrame = actor->totalAnimFrames = 0;
        }
    }
    if (afterAnim != -1) {
        if (afterAnim != actor->afterAnim) {
            actor->afterAnim = afterAnim;
        }
        actor->afterFramerate = afterFramerate;
    }
}
void ApplyRootMotion(ActorTransform* actor, AnimEntry* anim, f32 scale) {
    f32 rx;
    f32 ry;
    f32 rz;
    s32 i;
    ModelKeyFrame* rec;
    s32 n;
    ModelKey key;
    ModelPart* part;
    s16 count;
    u16 frame;

    count = anim->count;
    frame = actor->animFrame;
    part = anim->parts;
    i = 0;
    while (i < count && part->parent != 0) {
        i++;
        part++;
    }
    n = part->frames;
    if (n != 0) {
        n = frame % n;
    } else {
        n = 0;
    }
    SampleSpline(part, &key, n);
    if (actor->unk60 & 0x200) {
        rec = part->keys;
        n = part->frames;
        while (rec->frame != n) {
            rec++;
        }
        rx = rec->x;
        ry = rec->y;
        rz = rec->z;
        actor->unkDoor44 = (key.x + rx) * scale - actor->unkDoor38;
        actor->unkDoor48 = (key.y + ry) * scale - actor->unkDoor3C;
        actor->unkDoor4C = (key.z + rz) * scale - actor->unkDoor40;
        actor->unkDoor38 = key.x * scale;
        actor->unkDoor3C = key.y * scale;
        actor->unkDoor40 = key.z * scale;
    } else if (actor->unk60 & 0x400) {
        actor->unkDoor44 = 0.0f;
        actor->unkDoor48 = 0.0f;
        actor->unkDoor4C = 0.0f;
        actor->unkDoor38 = key.x * scale;
        actor->unkDoor3C = key.y * scale;
        actor->unkDoor40 = key.z * scale;
    } else {
        actor->unkDoor44 = key.x * scale - actor->unkDoor38;
        actor->unkDoor48 = key.y * scale - actor->unkDoor3C;
        actor->unkDoor4C = key.z * scale - actor->unkDoor40;
        actor->unkDoor38 = key.x * scale;
        actor->unkDoor3C = key.y * scale;
        actor->unkDoor40 = key.z * scale;
    }
}
void DrawModel(s32 flags, ActorTransform* actor, AnimEntry* anim, Mtx* mtxList) {
    s32 i;
    ModelPart* parts;
    s32 frame;
    s32 frames;
    MtxF mtx;
    Vec3f scale;

    gPartPositionCount = 0;
    BuildActorMatrix(&mtx, actor->xrot, actor->yrot, actor->zrot, actor->scale, actor->pos.x, actor->pos.y, actor->pos.z);
    scale.x = actor->scale;
    scale.y = actor->scale;
    scale.z = actor->scale;
    parts = anim->parts;
    frame = actor->animFrame;
    frames = actor->totalAnimFrames;
    for (i = 0; i < anim->count; i++) {
        if (parts[i].parent == 0) {
            DrawModelPart(&mtx, scale, flags, i, frame, frames, anim, parts, mtxList);
        }
    }
}
extern Camera gCamera;     /* the camera */
extern MtxF gBillboardMtx;
extern Vec3f gPartPositions[];           /* recorded part positions */
extern s32 gMatrixHead;               /* matrices used this frame */
extern Gfx* gDisplayListHead;
extern Mtx D_2000000[];              /* segment 2: the matrix buffer */
void BuildSRTMatrix(MtxF* mf, f32 x, f32 y, f32 z, f32 a, f32 b, f32 c, f32 sx, f32 sy, f32 sz);
void MatrixMultiply(MtxF* a, MtxF* b, MtxF* out);
void BuildLookAtMatrix(MtxF *mtx, f32 x, f32 y, f32 z, f32 atX, f32 atY, f32 atZ);
void ScaleMatrix(MtxF* m, f32 x, f32 y, f32 z);
void DrawModelPart(MtxF* parent, Vec3f scale, s32 flags, s32 index, s32 frame, s32 frames, AnimEntry* anim,
                   ModelPart* parts, Mtx* mtxBuf) {
    s32* dlistFrames;
    Gfx** dlists;
    s32 i;
    ModelPart* part;
    MtxF mtx;
    ModelKey key;

    part = &parts[index];
    {
        s32 f = part->frames;

        if (f != 0) {
            f = frame % f;
        } else {
            f = 0;
        }
        SampleSpline(part, &key, f);
    }
    if (part->parent == 0) {
        if (flags & 1) {
            key.z = key.x = 0.0f;
        }
        if (flags & 2) {
            key.y = 0.0f;
        }
    }
    BuildSRTMatrix(&mtx, key.x, key.y, key.z, key.unk10, key.unkC, key.unk14, key.scaleX, key.scaleY, key.scaleZ);
    ApplyPivotOffset(&mtx, part);
    scale.x *= key.scaleX;
    scale.y *= key.scaleY;
    scale.z *= key.scaleZ;
    if (part->flags & 1) {
        MatrixMultiply(parent, &mtx, &mtx);
        BuildLookAtMatrix(&gBillboardMtx, mtx.mf[3][0], mtx.mf[3][1], mtx.mf[3][2], gCamera.cam_x, gCamera.cam_y,
                      gCamera.cam_z);
        ScaleMatrix(&gBillboardMtx, scale.x, scale.y, scale.z);
        guMtxF2L(gBillboardMtx.mf, mtxBuf + 3 + gMatrixHead);
    } else {
        MatrixMultiply(parent, &mtx, &mtx);
        guMtxF2L(mtx.mf, mtxBuf + 3 + gMatrixHead);
    }
    gSPMatrix(gDisplayListHead++, D_2000000 + 3 + gMatrixHead++, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
    if (part->flags & 2) {
        gPartPositions[gPartPositionCount].x = mtx.mf[3][0];
        gPartPositions[gPartPositionCount].y = mtx.mf[3][1];
        gPartPositions[gPartPositionCount].z = mtx.mf[3][2];
        gPartPositionCount++;
    }
    dlists = anim->dlists;
    dlistFrames = part->dlistFrames;
    if (dlistFrames == NULL) {
        i = part->dlist;
        if (i != -1) {
            if (dlists[i] != NULL) {
                gSPDisplayList(gDisplayListHead++, dlists[i]);
            }
        }
    } else {
        gSPDisplayList(gDisplayListHead++, dlists[dlistFrames[frames % part->dlistFrameCount]]);
    }
    for (i = 0; i < anim->count; i++) {
        if (index + 1 == parts[i].parent) {
            DrawModelPart(&mtx, scale, flags, i, frame, frames, anim, parts, mtxBuf);
        }
    }
}
void AdvanceAnimation(s32 framerate, ActorTransform* model, AnimEntry* anim) {
    s32 frame = model->animFrame += framerate;

    if (frame >= anim->frames) {
        if (model->animID != model->afterAnim) {
            model->animID = model->afterAnim;
            model->animFrame = 0;
            model->totalAnimFrames = 0;
            model->animFramerate = model->afterFramerate;
            model->unk60 |= 0x400;
        } else {
            model->animFrame = frame - anim->frames;
            model->unk60 |= 0x200;
        }
    } else {
        model->unk60 &= ~0x600;
    }
    model->totalAnimFrames++;
}
void ApplyPivotOffset(MtxF* mtx, ModelPart* part) {
    f32 a, b, c;

    a = mtx->mf[0][0];
    b = mtx->mf[1][0];
    c = mtx->mf[2][0];
    mtx->mf[3][0] -= a * part->pivot.x + b * part->pivot.y + c * part->pivot.z;
    a = mtx->mf[0][1];
    b = mtx->mf[1][1];
    c = mtx->mf[2][1];
    mtx->mf[3][1] -= a * part->pivot.x + b * part->pivot.y + c * part->pivot.z;
    a = mtx->mf[0][2];
    b = mtx->mf[1][2];
    c = mtx->mf[2][2];
    mtx->mf[3][2] -= a * part->pivot.x + b * part->pivot.y + c * part->pivot.z;
}
