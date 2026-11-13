#ifndef GAME_MATH_H
#define GAME_MATH_H

/* The functions of src/game/math.c, declared once for every file that calls them. */

#include "common.h"

u32 getRandomNumber(u32 range);
f32 calc_arctan_in_radians(f32 x);
void Normalize2D(f32* x, f32* z);
f32 Atan2(f32 y, f32 x);
void rotateCoordinatesByAngle(f32 angle, Coordinates2D* coordinates);   /* rotate a 2D point */
void BuildActorMatrix(MtxF *mtx, f32 xrot, f32 yrot, f32 zrot, f32 scale, f32 x, f32 y, f32 z);
void BuildActorMtx(Mtx* mtx, f32 xrot, f32 yrot, f32 zrot, f32 scale, f32 x, f32 y, f32 z);
void BuildRotationMatrix(MtxF *mtx, f32 xrot, f32 yrot, f32 zrot);   /* rotation matrix */
void BuildRotationMtx(Mtx* mtx, f32 xrot, f32 yrot, f32 zrot);
void BuildLookAtMatrix(MtxF *mtx, f32 x, f32 y, f32 z, f32 atX, f32 atY, f32 atZ);
void BuildLookAtMtx(Mtx* mtx, f32 x, f32 y, f32 z, f32 atX, f32 atY, f32 atZ);
void TransformVecByRows(MtxF* m, f32 x, f32 y, f32 z, f32* outX, f32* outY, f32* outZ);
void TransformVecByCols(MtxF* m, f32 x, f32 y, f32 z, f32* outX, f32* outY, f32* outZ);
void MatrixFromDirection(MtxF* mtx, f32 x, f32 y, f32 z);
void MtxFromDirection(Mtx* mtx, f32 x, f32 y, f32 z);
void BuildSRTMatrix(MtxF* mtx, f32 x, f32 y, f32 z, f32 rx, f32 ry, f32 rz, f32 sx, f32 sy, f32 sz);
void ScaleMatrix(MtxF* m, f32 x, f32 y, f32 z);
void MatrixMultiply(MtxF *a, MtxF *b, MtxF *out);
s32 WorldToScreen(f32 x, f32 y, f32 z, s32* screenX, s32* screenY);   /* world to screen; 0 = off screen */

#endif
