#ifndef GAME_SPLINE_H
#define GAME_SPLINE_H

/* The functions of src/game/spline.c, declared once for every file that calls them. */

#include "common.h"

void SampleSpline(ModelPart* part, ModelKey* key, s32 frame);   /* key: x, y, z, then three angles in degrees */
void HermiteBasis(f32 t, f32 *h00, f32 *h01, f32 *h10, f32 *h11);

#endif
