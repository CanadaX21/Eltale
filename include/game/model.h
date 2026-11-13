#ifndef GAME_MODEL_H
#define GAME_MODEL_H

/* The functions of src/game/model.c, declared once for every file that calls them. */

#include "common.h"

void SetAnimation(ActorTransform* actor, s32 nextAnim, s32 nextFramerate, s32 afterAnim, s32 afterFramerate, u16 flags);
void ApplyRootMotion(ActorTransform* actor, AnimEntry* anim, f32 scale);
void DrawModel(s32 flags, ActorTransform* actor, AnimEntry* anim, Mtx* mtxList);
void DrawModelPart(MtxF* parent, Vec3f scale, s32 flags, s32 index, s32 frame, s32 frames, AnimEntry* anim, ModelPart* parts, Mtx* mtxBuf);   /* draw one part and its children (recursive) */
void AdvanceAnimation(s32 framerate, ActorTransform* model, AnimEntry* anim);
void ApplyPivotOffset(MtxF* mtx, ModelPart* part);

#endif
