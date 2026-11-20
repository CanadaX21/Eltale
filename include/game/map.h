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
void StartDoorAnimation(s32 state, s32 frame, s32 flag, void* doorModel);
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

/* The state src/game/map.c defines (.bss), in its storage order: the order of these declarations is the
 * unit's .bss layout (include/bss_order.h). */
extern u16 gLightRecordFlags;   /* the light record's flags */
extern u16 gLoadedSkyRecord;   /* the sky record loaded now */
extern sMapData* gMapHeader;   /* the current map's header (gMapTable[map]) */
extern Submap* gSubmapHeader;   /* the current submap */
extern MapModelData* gMapModelData;   /* its model set */
extern ModelTransform* gMapModels;   /* its placed models */
extern ModelGeometry* gMapGeometry;   /* geometry table */
extern NavMeshData* gWallNavmeshActors;   /* wall navmesh, actors */
extern NavMeshData* gWallNavmeshCamera;   /* wall navmesh, camera */
extern s32 gPropsDrawn;   /* props drawn this frame */
extern s32 gPropsNearCount;   /* props near Brian */
extern ModelTransform* gPropsNear[256];   /* the props near Brian (player.c, enemy.c) */
extern s32 gSpinningPropsLast;
extern s32 gSpinningPropsAdded;   /* entries this frame */
extern u8 gSkyTexture[0x4000];   /* the sky's texture data: LoadMap reads 0x4000 bytes of the sky record into it,
                                           after its 0x800 of palettes (gSkyPalettes); the sky's display lists use it */
extern u16 gFadingPalette[256];   /* the palette being faded (lighting.c) */
extern u16 gSkyPalettes[1024];   /* the sky palettes, 256 colours each */
extern u16 gSkyFadedPalette[512];   /* the faded sky palette (FadeSkyPalette output) */
extern u16 gSkyFadePalettes[768];
extern u16 gRotatingPalette[256];   /* a palette; RotatePalette rotates its first 202 colours by one per step */
extern s32 gDrawListCounts;   /* models in each draw list */
extern s32 gLateDrawCount;
extern s32 gPropDrawCount;
extern s32 gBillboardCount;
extern s32 gFlameCount;
extern s32 gScrollingCount;
extern ModelTransform** gDrawListPtrs;   /* the draw lists' write pointers */
extern ModelTransform** gLateDrawListPtr;
extern ModelTransform** gPropsNearPtr;
extern ModelTransform** gBillboardPtr;
extern ModelTransform** gFlamePtr;
extern ModelTransform** gScrollingPtr;
extern s32 gSaveFloatingRest;
extern ModelTransform gFloatingRests[32];   /* reset from gZeroModelTransform on every map load */
extern ModelTransform* gFloatingRestPositions;   /* the floating models' rest positions */
extern Coordinates2D gDustPuffOffset;   /* the dust puff's offset, rotated by the emitter's angle */
extern s32 D_80081FF8[2];   /* unreferenced */
extern CollisionPollingResult gMapModelPoll;
extern ModelKey gDoorKeySample;   /* the sampled door key, 0x28 (LoadMap's loop over gFloatingRests ends its fourth pointer inside it, the asm's D_80082030) */
extern u16 gFloatingSwayPhases[4][2];
extern ModelTransform* gDrawList[256];   /* this frame's animated models (the first draw list) */
extern ModelTransform* gLateDrawList[256];   /* the second draw list */
/* gBillboardProps to gSpinningProps1 stay consecutive: FadeSkyPalette's statics (src/game/map.c) sit among them in
 * the .bss and tie with their block numbers (docs/DECLARATIONS.md, "Units with .bss statics"). */
extern ModelTransform* gBillboardProps[44];   /* this frame's plain billboard props (gBillboardCount of them) */
extern u8 D_80082910[0x350];   /* unreferenced */
extern ModelTransform* gFlameProps[256];   /* the flames (gFlameCount, DrawFlames) */
extern ModelTransform* gScrollingProps[256];   /* the scrolling ones (gScrollingCount) */
extern SpinningProp gSpinningProps1[8];
extern SpinningProp gSpinningProps0[8];
extern f32 gShadowSizes[256][3];   /* shadow sizes */
extern DoorAnimData gDoorAnims[4];
extern u16 gLavaFrames;
extern f32 gMusicSourceDistance;   /* the nearest music source's distance */
extern s32 gMusicSourceSong;   /* its music */
extern s32 gMusicSourceLastSong;   /* the music playing last frame */
extern u16 gMapFlags;   /* the map's flags (sMapData.flags): MAP_ */
extern s32 gCurrentMap;   /* current map */
extern s32 gCurrentSubmap;   /* current submap */
extern s32 gNextMap;   /* next map */
extern s32 gNextSubmap;   /* next submap */
extern s32 gPreviousMap;   /* previous map */
extern s32 gReturnDest;   /* Return destination map / submap / entrance */
extern s32 gReturnRoom;
extern s32 gReturnEntrance;
extern s32 gExitDest;   /* Exit destination map / submap / entrance */
extern s32 gExitRoom;
extern s32 gExitEntrance;
extern u16 gSubmapFlags;   /* the submap's flags (Submap.flags): SUBMAP_ */

/* The data src/game/map.c defines. */
extern u16 gPropsDrawnFrames;   /* frames the props were drawn (DrawMapBillboards; nothing reads it); IDO pads it to a word */
extern u16 gFloatingSwaySpeeds[4][2];   /* texture scroll speeds, (s, t) pairs */
extern s16 D_8004DD84[2];
extern s32 gMusicSourceSounds[4];   /* the music of each sound source */
extern ModelTransform gZeroModelTransform;   /* the empty entry gFloatingRests[] is reset from */
extern f32 gFloatingSwayDivX[4];   /* the floating models' sway sizes (x, y, z) */
extern f32 gFloatingSwayDivY[4];
extern f32 gFloatingSwayDivZ[4];
extern Dust gMapDustTemplate;   /* the map dust template (map.c's .data, defined after SkyPaletteKey) */
extern Dust gDustEmitterTemplate;   /* the second dust template */
extern u16 gFlameTexWidths[2];   /* the flame kinds' texture widths (DrawFlames) */
extern u8 gFlameAlphas[4];

#endif
