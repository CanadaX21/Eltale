#include "common.h"
#include "game/math.h"
extern MtxF gScratchMtxF;
/* This file's six doubles (+-pi/2 twice, pi twice) stay in assembly (the splat config's rodata piece at ROM 0x70170):
 * Atan2 reads two of them from memory and converts them at run time, which no literal reproduces (IDO folds the
 * conversion of a constant), and IDO puts a const double object in .data, not .rodata. So the code names them. */
extern f64 D_8006F588;
extern f64 D_8006F580;
extern f64 D_8006F590;
extern f64 D_8006F598;

u8 gBitMasks[8] = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 }; /* bit n of a flag byte (story and map object flags) */
u32 gRandomSeed = 0x9B0;   /* the random seed (Quest D_8004D748) */
extern f64 D_8006F570;
extern f64 D_8006F578;

u32 getRandomNumber(u32 range) {
    if (range != 0) {
        return ((u32) ((gRandomSeed = (gRandomSeed * 0x41C64E6D) + 0x3039) >> 0x10)) % range;
    }
    return 0;
}
f32 calc_arctan_in_radians(f32 x) {
    f32 frac = 0.0f;
    s32 kk;
    s32 range;
    s32 k;

    if (x > 1.0f) {
        range = 1;
        x = 1.0f / x;
    } else if (x < -1.0f) {
        range = -1;
        x = 1.0f / x;
    } else {
        range = 0;
    }

    for (k = 12; k > 0; k--) {
        kk = k * k;
        frac = ((kk * x * x) / (k * 2 + 1 + frac));
    }

    if (range > 0) {
        return D_8006F570 - (x / (1.0f + frac));
    } else if (range < 0) {
        return D_8006F578 - (x / (1.0f + frac));
    } else {
        return x / (1.0f + frac);
    }
}
void Normalize2D(f32* x, f32* z) {
    
    f32 inv;
   
    inv = 1.0f / sqrtf((*x * *x) + (*z * *z));
    *x *= inv;
    *z *= inv;
}
f32 Atan2(f32 y, f32 x)
{
  f32 angle;
  f32 t;
  if (x == 0.0f)
  {
    if (y >= 0.0f)
    {
      angle = (f32) D_8006F580;
    }
    else
    {
      t = (f32) D_8006F588;
      angle = (f32) t;
    }
  }
  else
    if (x > 0.0f)
  {
    angle = calc_arctan_in_radians(y / x);
  }
  else
    if ((x < 0.0f) && (y <= 0.0f))
  {
    angle = calc_arctan_in_radians(y / x) - D_8006F590;
  }
  else
  {
    t = ( calc_arctan_in_radians( (y / x))) + D_8006F598;
    angle = (f32) t;

  }
  return angle;
}
void rotateCoordinatesByAngle(f32 angle, Coordinates2D* coordinates) {
    f32 sinAngle;
    f32 cosAngle;
    f32 xcoord;

    sinAngle = sinf(angle);
    cosAngle = cosf(angle);
    xcoord = coordinates->x;
    coordinates->x = (coordinates->y * sinAngle) - (xcoord * cosAngle);
    coordinates->y = (coordinates->y * cosAngle) + (xcoord * sinAngle);
}
void BuildActorMatrix(MtxF *mtx, f32 xrot, f32 yrot, f32 zrot, f32 scale, f32 x, f32 y, f32 z)
{
    
  f32 sx;    
  f32 sy;
  f32 sz;
  f32 cx;    
  f32 cy;    
  f32 cz;
    
  sx = sinf(xrot);
  cx = cosf(xrot);
  sy = sinf(yrot);
  cy = cosf(yrot);
  sz = sinf(zrot);
  cz = cosf(zrot);
  mtx->mf[0][0] = (f32) (((cz * cy) + ((sz * sx) * sy)) * scale);
  mtx->mf[0][1] = (f32) ((sz * cx) * scale);
  mtx->mf[0][2] = (f32) ((((-sy) * cz) + ((sz * sx) * cy)) * scale);
  mtx->mf[0][3] = 0.0f;
  mtx->mf[1][0] = (f32) ((((-sz) * cy) + ((cz * sx) * sy)) * scale);
  mtx->mf[1][1] = (f32) ((cz * cx) * scale);
  mtx->mf[1][2] = (f32) ((((-sz) * (-sy)) + ((cz * sx) * cy)) * scale);
  mtx->mf[1][3] = 0.0f;
  mtx->mf[2][0] = (f32) ((cx * sy) * scale);
  mtx->mf[2][1] = (f32) ((-sx) * scale);
  mtx->mf[2][2] = (f32) ((cx * cy) * scale);
  mtx->mf[2][3] = 0.0f;
  mtx->mf[3][0] = x;
  mtx->mf[3][1] = y;
  mtx->mf[3][2] = z;
  mtx->mf[3][3] = 1.0f; 
}
void BuildActorMtx(Mtx* mtx, f32 xrot, f32 yrot, f32 zrot, f32 scale, f32 x, f32 y, f32 z) {
    BuildActorMatrix(&gScratchMtxF, xrot, yrot, zrot, scale, x, y, z);
    guMtxF2L(gScratchMtxF.mf, mtx);
}
void BuildRotationMatrix(MtxF *mtx, f32 xrot, f32 yrot, f32 zrot)
{
    f32 sin1;
    f32 sin2;
    f32 sin3;
    f32 cos1;
    f32 cos2;
    f32 cos3;
  
  
    sin1 = sinf(xrot);
    cos1 = cosf(xrot);
    sin2 = sinf(yrot);
    cos2 = cosf(yrot);
    sin3 = sinf(zrot);
    cos3 = cosf(zrot);

    mtx->mf[0][0] = (cos3 * cos2) + (sin3 * sin1 * sin2);
    mtx->mf[0][1] = sin3 * cos1;
    mtx->mf[0][2] = (-sin2 * cos3) + (sin3 * sin1 * cos2);
    mtx->mf[0][3] = 0.0f;

    mtx->mf[1][0] = (-sin3 * cos2) + (cos3 * sin1 * sin2);
    mtx->mf[1][1] = cos3 * cos1;
    mtx->mf[1][2] = (-sin3 * -sin2) + (cos3 * sin1 * cos2);
    mtx->mf[1][3] = 0.0f;

    mtx->mf[2][0] = cos1 * sin2;
    mtx->mf[2][1] = -sin1;
    mtx->mf[2][2] = cos1 * cos2;
    mtx->mf[2][3] = 0.0f;

    mtx->mf[3][0] = 0.0f;
    mtx->mf[3][1] = 0.0f;
    mtx->mf[3][2] = 0.0f;
    mtx->mf[3][3] = 1.0f;
}
void BuildRotationMtx(Mtx* mtx, f32 xrot, f32 yrot, f32 zrot) {
    BuildRotationMatrix(&gScratchMtxF, xrot, yrot, zrot);
    guMtxF2L(gScratchMtxF.mf, mtx);
}
void BuildLookAtMatrix(MtxF *mtx, f32 x, f32 y, f32 z, f32 atX, f32 atY, f32 atZ)
{
  
  f32 len;
  f32 dz;
  f32 dy;
  f32 planar;
  f32 dx;
  f32 cosPitch;
  f32 sinYaw;
  f32 cosYaw;
  f32 sinPitch;
    
  dx = atX - x;
  dz = atZ - z;
  planar = sqrtf((dx * dx) + (dz * dz));
  if (planar == 0.0f)
  {
    sinYaw = 0.0f;
    cosYaw = 1.0f;
  }
  else
  {
    sinYaw = dx / planar;
    cosYaw = dz / planar;
  }
  dy = atY - y;
  len = sqrtf((planar * planar) + (dy * dy));
  if (len == 0.0f)
  {
    sinPitch = 0.0f;
    cosPitch = 1.0f;
  }
  else
  {
    sinPitch = (-dy) / len;
    cosPitch = planar / len;
  }
  mtx->mf[0][0] = cosYaw;
  mtx->mf[0][1] = 0.0f;
  mtx->mf[0][2] = -sinYaw;
  mtx->mf[0][3] = 0.0f;
    
  mtx->mf[1][0] = sinPitch * sinYaw;
  mtx->mf[1][1] = cosPitch;
  mtx->mf[1][2] = sinPitch * cosYaw;
  mtx->mf[1][3] = 0.0f;
    
  mtx->mf[2][0] = cosPitch * sinYaw;
  mtx->mf[2][1] = -sinPitch;
  mtx->mf[2][2] = cosPitch * cosYaw;
  mtx->mf[2][3] = 0.0f;
    
  mtx->mf[3][0] = x;
  mtx->mf[3][1] = y;
  mtx->mf[3][2] = z;
  mtx->mf[3][3] = 1.0f;
}
void BuildLookAtMtx(Mtx* mtx, f32 x, f32 y, f32 z, f32 atX, f32 atY, f32 atZ) {
    BuildLookAtMatrix(&gScratchMtxF, x, y, z, atX, atY, atZ);
    guMtxF2L(gScratchMtxF.mf, mtx);
}
void TransformVecByRows(MtxF* m, f32 x, f32 y, f32 z, f32* outX, f32* outY, f32* outZ) {
    *outX = (m->zx * z) + ((m->xx * x) + (m->yx * y));
    *outY = (m->zy * z) + ((m->xy * x) + (m->yy * y));
    *outZ = (m->zz * z) + ((m->xz * x) + (m->yz * y));
}
void TransformVecByCols(MtxF* m, f32 x, f32 y, f32 z, f32* outX, f32* outY, f32* outZ) {
    *outX = (m->xz * z) + ((m->xx * x) + (m->xy * y));
    *outY = (m->yz * z) + ((m->yx * x) + (m->yy * y));
    *outZ = -((m->zz * z) + ((m->zx * x) + (m->zy * y)));
}
void MatrixFromDirection(MtxF* mtx, f32 x, f32 y, f32 z) {
    f32 s;
    f32 len2;
    f32 sp;
    f32 ns;
    f32 c;
    f32 zc;
    f32 len;
    f32 cp;

    len = sqrtf((x * x) + (z * z));
    if (len != 0.0f) {
        s = x / len;
        c = z / len;
    } else {
        s = 0.0f;
        c = (y >= 0.0f) ? 1.0 : -1.0;
    }
    len2 = sqrtf((len * len) + (y * y));
    cp = len / len2;
    sp = y / len2;
    zc = 0 * cp;
    ns = -s;
    mtx->mf[0][0] = (1 * c) + (zc * s);
    mtx->mf[0][1] = 0 * sp;
    mtx->mf[0][2] = (ns * 1) + (zc * c);    /* a written-out product: the original really multiplies by 1.0 and 0 */
    mtx->mf[0][3] = 0;
    mtx->mf[1][0] = (-0.0f * c) + ((1 * cp) * s);
    mtx->mf[1][1] = 1 * sp;
    mtx->mf[1][2] = (-0.0f * ns) + ((1 * cp) * c);
    mtx->mf[1][3] = 0;
    mtx->mf[2][0] = sp * s;
    mtx->mf[2][1] = -cp;
    mtx->mf[2][2] = sp * c;
    mtx->mf[2][3] = 0;
    mtx->mf[3][0] = 0;
    mtx->mf[3][1] = 0;
    mtx->mf[3][2] = 0;
    mtx->mf[3][3] = 1;
}
void MtxFromDirection(Mtx* mtx, f32 x, f32 y, f32 z) {
    MatrixFromDirection(&gScratchMtxF, x, y, z);
    guMtxF2L(gScratchMtxF.mf, mtx);
}
void BuildSRTMatrix(MtxF* mtx, f32 x, f32 y, f32 z, f32 rx, f32 ry, f32 rz, f32 sx, f32 sy, f32 sz) {
    f32 sinx;
    f32 siny;
    f32 sinz;
    f32 cosx;
    f32 cosy;
    f32 cosz;
    static f32 sDegToRad = 0.017453292f; /* pi / 180: the angles come in degrees */

    rx *= sDegToRad;
    ry *= sDegToRad;
    rz *= sDegToRad;
    sinx = sinf(rx);
    cosx = cosf(rx);
    siny = sinf(ry);
    cosy = cosf(ry);
    sinz = sinf(rz);
    cosz = cosf(rz);
    mtx->mf[0][0] = ((cosz * cosy) + ((sinz * sinx) * siny)) * sx;
    mtx->mf[0][1] = (sinz * cosx) * sx;
    mtx->mf[0][2] = (((-siny) * cosz) + ((sinz * sinx) * cosy)) * sx;
    mtx->mf[0][3] = 0.0f;
    mtx->mf[1][0] = (((-sinz) * cosy) + ((cosz * sinx) * siny)) * sy;
    mtx->mf[1][1] = (cosz * cosx) * sy;
    mtx->mf[1][2] = (((-sinz) * (-siny)) + ((cosz * sinx) * cosy)) * sy;
    mtx->mf[1][3] = 0.0f;
    mtx->mf[2][0] = (cosx * siny) * sz;
    mtx->mf[2][1] = (-sinx) * sz;
    mtx->mf[2][2] = (cosx * cosy) * sz;
    mtx->mf[2][3] = 0.0f;
    mtx->mf[3][0] = x;
    mtx->mf[3][1] = y;
    mtx->mf[3][2] = z;
    mtx->mf[3][3] = 1.0f;
}

void ScaleMatrix(MtxF* m, f32 x, f32 y, f32 z) {
    m->mf[0][0] *= x;
    m->mf[0][1] *= x;
    m->mf[0][2] *= x;
    m->mf[1][0] *= y;
    m->mf[1][1] *= y;
    m->mf[1][2] *= y;
    m->mf[2][0] *= z;
    m->mf[2][1] *= z;
    m->mf[2][2] *= z;
}
void MatrixMultiply(MtxF *a, MtxF *b, MtxF *out)
{
  f32 a01;
  f32 a20;
  f32 a10;
  f32 a02;
  f32 a00;
  f32 a11;
  f32 a21;
  f32 a22;
  f32 a12;
  f32 a30;
  f32 a31;
  f32 a32;
  f32 b00;
  f32 b01;
  f32 b02;
  f32 b10;
  f32 b11;
  f32 b12;  
  f32 b20;
  f32 b21;
  f32 b22;
  f32 b30;
  f32 b31;  
  f32 b32;
  
    
  a00 = a->mf[0][0];
  a01 = a->mf[0][1];
  a02 = a->mf[0][2];
    
  a10 = a->mf[1][0];
  a11 = a->mf[1][1];
  a12 = a->mf[1][2];
    
  a20 = a->mf[2][0];
  a21 = a->mf[2][1];
  a22 = a->mf[2][2];
    
  a30 = a->mf[3][0];
  a31 = a->mf[3][1];
  a32 = a->mf[3][2];
    
  b00 = b->mf[0][0];
  b01 = b->mf[0][1];
  b02 = b->mf[0][2];
    
  b10 = b->mf[1][0];
  b11 = b->mf[1][1];
  b12 = b->mf[1][2];
    
  b20 = b->mf[2][0];
  b21 = b->mf[2][1];
  b22 = b->mf[2][2];
    
  b30 = b->mf[3][0];
  b31 = b->mf[3][1];
  b32 = b->mf[3][2];
    
  out->mf[0][0] = ((a00 * b00) + (a10 * b01)) + (a20 * b02);
  out->mf[1][0] = ((a00 * b10) + (a10 * b11)) + (a20 * b12);
  out->mf[2][0] = ((a00 * b20) + (a10 * b21)) + (a20 * b22);
  out->mf[3][0] = (((a00 * b30) + (a10 * b31)) + (a20 * b32)) + a30;
    
  out->mf[0][1] = ((a01 * b00) + (a11 * b01)) + (a21 * b02);
  out->mf[1][1] = ((a01 * b10) + (a11 * b11)) + (a21 * b12);
  out->mf[2][1] = ((a01 * b20) + (a11 * b21)) + (a21 * b22);
  out->mf[3][1] = (((a01 * b30) + (a11 * b31)) + (a21 * b32)) + a31;
    
  out->mf[0][2] = ((a02 * b00) + (a12 * b01)) + (a22 * b02);
  out->mf[1][2] = ((a02 * b10) + (a12 * b11)) + (a22 * b12);
  out->mf[2][2] = ((a02 * b20) + (a12 * b21)) + (a22 * b22);
  out->mf[3][2] = (((a02 * b30) + (a12 * b31)) + (a22 * b32)) + a32;
    
  out->mf[0][3] = 0.0f; 
  out->mf[1][3] = 0.0f;
  out->mf[2][3] = 0.0f;
  out->mf[3][3] = 1.f;
}
extern MtxF gViewMtxF;              /* the view-projection matrix */
extern f32 gNearPlane;               /* near plane distance */
extern f32 gViewTanHalf;
s32 WorldToScreen(f32 x, f32 y, f32 z, s32* screenX, s32* screenY) {
    f32 w;
    f32 ox;
    f32 oy;
    f32 oz;

    guMtxXFMF(&gViewMtxF, x, y, z, &ox, &oy, &oz);
    if (oz < -gNearPlane) {
        w = oz * gViewTanHalf;
        *screenX = 160 - (s32)(160.0 * ox / w * 0.75);
        *screenY = (s32)(120.0f * oy / w) + 120;
    }
    return oz < -gNearPlane;
}

