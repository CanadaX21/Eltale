#include "common.h"

void HermiteBasis(f32 t, f32* h1, f32* h2, f32* h3, f32* h4);   /* Hermite basis */

void SampleSpline(ModelPart* part, ModelKey* key, s32 frame) {
    f32* p1;
    ModelKeyFrame* next;
    s32 i;
    f32 h1;
    f32 h2;
    f32 h3;
    f32 h4;
    ModelKeyFrame* prev;
    f32* dst;
    f32 a;
    f32 b;
    f32 c;
    f32 d;
    f32 inScale;
    f32 outScale;
    f32 delta;
    f32 tanIn;
    f32 tanOut;
    s32 span;

    if (part->keyCount == 1) {
        for (i = 0; i < 9; i++) {
            ((f32*)key)[i] = ((f32*)part->keys)[i];
        }
        return;
    }
    next = part->keys + 1;
    while (next->frame < frame) {
        next++;
    }
    prev = next - 1;
    frame -= prev->frame;
    span = next->frame - prev->frame;
    if (next->linear == 0) {
        HermiteBasis((f32)frame / (f32)span, &h1, &h2, &h3, &h4);
        a = (1.0 - prev->tension) * (1.0 + prev->continuity) * (1.0 + prev->bias);
        b = (1.0 - prev->tension) * (1.0 - prev->continuity) * (1.0 - prev->bias);
        c = (1.0 - next->tension) * (1.0 - next->continuity) * (1.0 + next->bias);
        d = (1.0 - next->tension) * (1.0 + next->continuity) * (1.0 - next->bias);
        if (prev->frame != 0) {
            inScale = (f32)span / (next->frame - prev[-1].frame);
        }
        if (next->frame != part->frames) {
            outScale = (f32)span / (next[1].frame - prev->frame);
        }
    }
    p1 = &next->x;
    dst = (f32*)key;
    for (i = 0; i < 9; i++, dst++) {
        delta = p1[i] - (&prev->x)[i];
        if (next->linear == 0) {
            if (prev->frame == 0) {
                tanIn = (a + b) * 0.5 * delta;
            } else {
                tanIn = (((&prev->x)[i] - (&prev->x)[i - 13]) * a + b * delta) * inScale;
            }
            if (next->frame == part->frames) {
                tanOut = (c + d) * 0.5 * delta;
            } else {
                tanOut = ((p1[i + 13] - p1[i]) * d + c * delta) * outScale;
            }
            *dst = (&prev->x)[i] * h1 + (&next->x)[i] * h2 + tanIn * h3 + tanOut * h4;
        } else {
            *dst = (&prev->x)[i] + ((f32)frame / (f32)span) * delta;
        }
    }
}

void HermiteBasis(f32 t, f32 *h00, f32 *h01, f32 *h10, f32 *h11)
{
  f32 t2;
  f32 p01;
  f32 p11;
  f32 t3;
  f64 t3d;
    
  t2 = t * t;
  t3 = t * t2;
  t3d = t3;
  p11 = t3 - t2;
  p01 = ((( t2 * 3.0) - t3d) - t3d);
  *h00 = 1.0 - p01;
  *h01 = p01;
  p11 = t3 - t2;
  *h10 = (p11 - t2) + t;
  *h11 = p11;
}
