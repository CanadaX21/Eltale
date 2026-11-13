#ifndef GAME_MAP_H
#define GAME_MAP_H

/* The functions of src/game/map.c, declared once for every file that calls them. */

#include "common.h"

void LoadMap(void);   /* the map loader */
void UpdateLavaPalette(void);
void DrawMapFrame(mkst* arg0);
void DrawMapBillboards(mkst* dyn);
void SortMapModels(s32 count, u16* list);
void DrawSky(Mtx* mtx);
void ResetDoorAnims(void);
s32 FindDoorAnim(void* target);
void StartDoorAnimation(s32 state, s32 frame, s32 flag, void* position);
void UpdateDoorAnimations(void);
void DrawMapProps(mkst* dyn, s32 count, s32 type, ModelTransform** props);
void UpdateDustEmitter(ModelTransform* obj);   /* map dust emitter */
void UpdateFallingParticle(Dust* p);
void TickDustEmitter(ModelTransform* obj);   /* the second dust emitter */
void UpdateArcParticle(Dust* p);
void ResetSpinningProps(void);
void AddSpinningProp(f32 x, f32 y, f32 z, f32 angle, ModelTransform* prop);   /* record a spinning prop */
void DrawSpinningProps(mkst* dyn);
void DrawFlames(mkst* dyn, s32 count, s32 kind, ModelTransform** list);
void FadeSkyPalette(void);
void RotatePalette(void);

#endif
