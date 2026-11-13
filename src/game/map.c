#include "common.h"
#include "game/map.h"
#include "game/audio.h"
#include "game/collision.h"
#include "game/dma.h"
#include "game/dust.h"
#include "game/effects.h"
#include "game/math.h"
#include "game/shadow.h"
#include "game/sound.h"
#include "game/spline.h"
/* float view of the same 0x1C-byte record (AddSpinningProp; Quest func_8000E418) */
typedef struct {
    s32 unk0;
    f32 unk4;
    f32 unk8;
    f32 unkC;
    f32 unk10;
    f32 unk14;  /* angle */
    s32 unk18;  /* id */
} unke3c4f;  /* unused (SpinningProp has these member types); kept for the declaration count */
extern s32 gCurrentFrame;  /* which buffer is current (Quest D_8007B2F4) */
extern Gfx* gDisplayListHead;
extern MapLight* gSubmapLight;         /* the submap's light record */
/* the map loader (LoadMap; Quest func_8000BD10, names from vbhayden's scratch) */
extern sMapData gMapTable[];        /* every map's header */
extern u8* gRegionBlocks[];                /* extra ROM blocks, 3 words each: start, end, RAM (sMapData.unk3C; Quest D_800558A0) */
extern SkyRecord gSkyRecords[];
extern ModelTransform gZeroModelTransform;
extern Dust gMapDustTemplate;              /* the map dust template (map.c's .data, defined after SkyPaletteKey) */
extern Dust gDustEmitterTemplate;              /* the second dust template */
extern PlayerState gPlayerState;      /* Brian's state block */
extern f32 gFloatingSwayDivX[4];            /* the floating models' sway sizes (x, y, z) */
extern f32 gFloatingSwayDivY[4];
extern f32 gFloatingSwayDivZ[4];
extern s8 gTileOffsets0[];               /* tile neighbourhoods: (dx, dz) pairs, one table per tile mode */
extern u16 gLavaPalette[];                /* a palette map 0x1D loads, rotated here */
extern MapPropGfx gFlameModelLists[];     /* the flame kinds' setup / model lists (kinds 0x11, 0x12) */
extern u8** gFlameFrameTables[];            /* their 8 texture frames */
extern u16 gFlameTexWidths[2];
extern u8 gFlameAlphas[4];
extern PendingWarp gPendingWarp;
extern f32 gBrianSpawnX;               /* Brian's spawn x / z / angle */
extern f32 gBrianSpawnZ;
extern f32 gBrianSpawnYaw;
extern s32 gEntranceDoorModel;
extern u8 gameplay_common_ROM_START[];
extern u8 gameplay_common_ROM_END[];               /* the end of the gameplay_common_ROM_START block (undefined_syms.jp.txt; the next block starts here) */
extern u8 interiors_common_ROM_START[];
extern u8 interiors_common_ROM_END[];
extern u8 sky_palettes_ROM_START[];
extern u8 rotating_palette_ROM_START[];
extern u8 field_shared_ROM_START[];
extern u8 field_shared_ROM_END[];
extern u8 D_80100000[];
extern u8 D_8010FE00[];
extern u8 D_801E0000[];

/* other units' data */
extern s8 gTileOffsets1[];
extern s8 gTileOffsets2[];
extern s8 gTileOffsets3[];
extern s8 gTileOffsets4[];
extern ModelPart gMapModelPaths[];        /* the animated models' animations */
extern Gfx D_10000E8[];
extern s16 gFogNear;
extern s16 gFogFar;
extern s32 gMatrixHead;
extern Mtx D_2000000[];
extern Camera gCamera;
extern u8 gPropAlpha[];               /* per map: the props' alpha */
extern s32 gSkyColour;               /* the sky colour */
extern s32 gSkyColourG;
extern s32 gSkyColourB;
extern ModelPart gDoorAnimTable;         /* door animations */
extern ModelPart gDoorAnimAlt;
extern ModelPart gDoorAnim4;
extern u8 gMapDustTextures[];   /* the map dust textures (0x400 each) */
extern Gfx gMapDustGfx[];              /* their display list */
extern u8 gDustTexture2[];              /* the second dust texture */
extern Gfx gDustGfx2[];              /* its display list */

/* ---- map.c's .bss (0x8007C280-0x800842F0). IDO 5.3 orders a file's .bss by where each
   variable is first declared, counted through the file (headers included) modulo 256: these come in address order,
   no extern of them may sit above the block, and FadeSkyPalette's four statics fall between the prop arrays at their
   place in that count (the externs of other units' data above the block keep it). Names inside a block (D_8007C990 / D_8007C998 in the map's loaded
   data) are link names or offsets in the regenerated asm. ---- */
u16 gLightRecordFlags;                  /* the light record's flags */
u16 gLoadedSkyRecord;                   /* the sky record loaded now */
sMapData* gMapHeader;                   /* the current map's header (gMapTable[map]) */
Submap* gSubmapHeader;                  /* the current submap */
MapModelData* gMapModelData;            /* its model set */
ModelTransform* gMapModels;             /* its placed models */
ModelGeometry* gMapGeometry;                       /* geometry table */
NavMeshData* gWallNavmeshActors;                 /* wall navmesh, actors */
NavMeshData* gWallNavmeshCamera;                 /* wall navmesh, camera */
s32 gPropsDrawn;                        /* props drawn this frame */
s32 gPropsNearCount;
ModelTransform* gPropsNear[256];               /* the props near Brian (player.c, enemy.c) */
s32 gSpinningPropsLast;
s32 gSpinningPropsAdded;                /* entries this frame */
u8 gSkyTexture[0x4000];                  /* the sky's texture data: LoadMap reads 0x4000 bytes of the sky record into it,
                                           after its 0x800 of palettes (gSkyPalettes); the sky's display lists use it */
u16 gFadingPalette[256];                /* the palette being faded (lighting.c) */
u16 gSkyPalettes[1024];                 /* the sky palettes, 256 colours each */
u16 gSkyFadedPalette[512];              /* the faded sky palette (FadeSkyPalette output) */
u16 gSkyFadePalettes[768];
u16 gRotatingPalette[256];              /* a palette; RotatePalette rotates its first 202 colours by one per step */
s32 gDrawListCounts;                    /* models in each draw list */
s32 gLateDrawCount;
s32 gPropDrawCount;
s32 gBillboardCount;
s32 gFlameCount;
s32 gScrollingCount;
ModelTransform** gDrawListPtrs;         /* the draw lists' write pointers */
ModelTransform** gLateDrawListPtr;
ModelTransform** gPropsNearPtr;
ModelTransform** gBillboardPtr;
ModelTransform** gFlamePtr;
ModelTransform** gScrollingPtr;
s32 gSaveFloatingRest;
ModelTransform gFloatingRests[32];               /* reset from gZeroModelTransform on every map load */
ModelTransform* gFloatingRestPositions; /* the floating models' rest positions */
Coordinates2D gDustPuffOffset;          /* the dust puff's offset, rotated by the emitter's angle */
s32 D_80081FF8[2];                      /* unreferenced */
CollisionPollingResult gMapModelPoll;
ModelKey gDoorKeySample;                /* the sampled door key, 0x28 (LoadMap's loop over gFloatingRests ends its fourth pointer inside it, the asm's D_80082030) */
u16 gFloatingSwayPhases[4][2];
ModelTransform* gDrawList[256];        /* this frame's animated models (the first draw list) */
ModelTransform* gLateDrawList[256];        /* the second draw list */
ModelTransform* gBillboardProps[44];    /* this frame's plain billboard props (gBillboardCount of them) */
u8 D_80082910[0x350];                   /* unreferenced */
ModelTransform* gFlameProps[256];       /* the flames (gFlameCount, DrawFlames) */
ModelTransform* gScrollingProps[256];   /* the scrolling ones (gScrollingCount) */
SpinningProp gSpinningProps1[8];
SpinningProp gSpinningProps0[8];
f32 gShadowSizes[256][3];               /* shadow sizes */
DoorAnimData gDoorAnims[4];
u16 gLavaFrames;
f32 gMusicSourceDistance;               /* the nearest music source's distance */
s32 gMusicSourceSong;                   /* its music */
s32 gMusicSourceLastSong;                         /* the music playing last frame */
u16 gMapFlags;                          /* the map's flags (sMapData.unk0) */
s32 gCurrentMap;                        /* current map */
s32 gCurrentSubmap;                     /* current submap */
s32 gNextMap;                           /* next map */
s32 gNextSubmap;                        /* next submap */
s32 gPreviousMap;                       /* previous map */
s32 gReturnDest;                        /* Return destination map / submap / entrance */
s32 gReturnRoom;
s32 gReturnEntrance;
s32 gExitDest;                          /* Exit destination map / submap / entrance */
s32 gExitRoom;
s32 gExitEntrance;
u16 gSubmapFlags;                       /* the submap's flags (Submap.unk14) */

void LoadMap(void) {
    s32 count;
    s32 n;
    s32 i;
    u8* start;
    u8* end;
    u16* src;
    u16* dst;
    f32 (*shadow)[3];
    u8** block;
    MapEntrance* entrance;
    ModelTransform* model;
    s32 kind;

    gMapHeader = &gMapTable[gNextMap];
    if (gCurrentMap != -1) {
        if (gNextMap == 0xB) {
            if ((gCurrentSubmap == 2) && (gNextSubmap == 0)) {
                gReturnDest = gNextMap;
                gReturnRoom = gNextSubmap;
                gReturnEntrance = gPendingWarp.entrance;
            }
        } else if (gNextMap == 6) {
            gReturnDest = gNextMap;
            gReturnRoom = gNextSubmap;
            gReturnEntrance = gPendingWarp.entrance;
        } else if ((gMapFlags & 1) && (gMapHeader->flags & 4)) {
            gReturnDest = gNextMap;
            gReturnRoom = gNextSubmap;
            gReturnEntrance = gPendingWarp.entrance;
        } else if ((gNextMap != gCurrentMap) && (gMapHeader->flags & 8)) {
            gExitDest = gNextMap;
            gExitRoom = gNextSubmap;
            gExitEntrance = gPendingWarp.entrance;
            if ((gExitDest == 0x1A) && (gExitRoom == 4)) {
                gExitRoom = 2;
                gExitEntrance = 1;
            }
        }
    }
    if (gCurrentMap == -1) {
        dma_write(gameplay_common_ROM_START, D_80100000, gameplay_common_ROM_END - gameplay_common_ROM_START);
        dma_write(sky_palettes_ROM_START, gSkyFadePalettes, 0x600);
        dma_write(rotating_palette_ROM_START, gRotatingPalette, 0x200);
    }
    if (gCurrentMap != gNextMap) {
        gPreviousMap = gCurrentMap;
        gCurrentMap = gNextMap;
        gMapFlags = gMapHeader->flags;
        if (gMapFlags & 7) {
            if (gMapFlags & 2) {
                dma_write(interiors_common_ROM_START, D_8010FE00, interiors_common_ROM_END - interiors_common_ROM_START);
            }
            block = &gRegionBlocks[gMapHeader->regionBlock * 3];
            start = block[0];
            end = block[1];
            block += 2;
            dma_write(start, block[0], end - start);
        }
        start = gMapHeader->romStart;
        dma_write(start, gMapHeader->ram, gMapHeader->romEnd - start);
        switch (gCurrentMap) {
            case 3:
            case 5:
            case 7:
            case 0xC:
                dma_write(field_shared_ROM_START, D_801E0000, field_shared_ROM_END - field_shared_ROM_START);
                break;
        }
    }
    count = 0x65;
    src = &gRotatingPalette[100];
    dst = &gRotatingPalette[101];
    do {
        *dst++ = *src--;
    } while (--count != 0);
    gCurrentSubmap = gNextSubmap;
    gSubmapHeader = &gMapHeader->submaps[gCurrentSubmap];
    gMapGeometry = gMapHeader->geometry;
    gMapModelData = &gMapHeader->modelSets[gSubmapHeader->modelSet];
    gSubmapLight = &gMapHeader->lights[gSubmapHeader->light];
    gMapModels = gMapModelData->transforms;
    gWallNavmeshActors = gMapHeader->actorNavmesh;
    gWallNavmeshCamera = gMapHeader->cameraNavmesh;
    gSubmapFlags = gSubmapHeader->flags;
    gLightRecordFlags = gSubmapLight->flags;
    n = gSubmapLight->skyRecord;        /* the same local becomes the shadow counter below */
    if (gLoadedSkyRecord != n) {
        gLoadedSkyRecord = gSubmapLight->skyRecord;
        dma_write(gSkyRecords[gLoadedSkyRecord].rom, gSkyPalettes, 0x800);
        dma_write(gSkyRecords[gLoadedSkyRecord].rom + 0x800, gSkyTexture, 0x4000);
    }
    if (gPendingWarp.entrance != -1) {
        entrance = &gSubmapHeader->entrances[gPendingWarp.entrance];
        gBrianSpawnX = entrance->x;
        gBrianSpawnZ = entrance->z;
        gBrianSpawnYaw = entrance->yrot;
        gEntranceDoorModel = entrance->unkE;
    }
    count = gMapModelData->transform_count;
    model = gMapModels;
    gPropsNearCount = 0;
    shadow = gShadowSizes;
    for (n = 0; count != 0; count--, model++) {
        if (model->flags & 0x8000) {
            PollWorldCollision(model->x, model->y, model->z, 0xA00, &gMapModelPoll);
            model->y = gMapModelPoll.y;
        }
        kind = model->flags & 0xFF;
        if (kind == 0x10) {
            (*shadow)[0] = gMapModelPoll.sn_x;
            (*shadow)[1] = gMapModelPoll.sn_y;
            (*shadow)[2] = gMapModelPoll.sn_z;
            model->angle = n;
            n++;
            shadow++;
        } else if (kind == 0x83) {
            model->index = 0;
        } else if (kind == 0x84) {
            model->index = getRandomNumber(0x96) + 0x96;
        } else if ((model->flags >= 0x11) && (model->flags < 0x15)) {
            model->index = getRandomNumber(0x10);
        }
    }
    for (i = 0; i < 4; i++) {
        gFloatingSwayPhases[i][0] = 0;
        gFloatingSwayPhases[i][1] = 0;
    }
    for (i = 0; i < 32; i++) {
        gFloatingRests[i] = gZeroModelTransform;
    }
    gSaveFloatingRest = 1;
    ResetDoorAnims();
    ResetSpinningProps();
    gMusicSourceLastSong = -1;
}

void UpdateLavaPalette(void) {
    s32 i;
    u16* colour;
    u16 last;

    gSpinningPropsAdded = 0;
    if (gCurrentMap == 0x1D) {
        i = 18;
        colour = gLavaPalette + i;
        last = *colour;
        while (i != 0) {
            colour[0] = colour[-1];
            colour--;
            i--;
        }
        *colour = last;
        gLavaFrames++;
    }
}

extern ModelKey gDoorKeySample;
extern Coordinates2D gDustPuffOffset;
extern ModelTransform* gDrawList[];  /* this frame's animated models */
extern ModelTransform* gLateDrawList[];  /* the plain ones */
extern ModelTransform* gBillboardProps[];
extern ModelTransform* gFlameProps[];
extern ModelTransform* gScrollingProps[];
extern s32 gPropsDrawn;
extern u16 gFloatingSwaySpeeds[][2];   /* texture scroll speeds, (s, t) pairs */
extern s32 gMusicSourceSounds[];              /* the music of each sound source */
void DrawMapFrame(mkst* arg0) {
    Camera* cam = &gCamera;
    s8* offs;
    s32 n;
    s32 tx;
    s32 tz;
    s32 x;
    s32 z;
    u16* list;
    u16 count;
    ModelTransform* t;
    s32 song;
    f32 px;
    f32 py;
    f32 pz;
    f32 angle;
    ModelPart* anim;
    s32 tile;
    s32 door;
    DoorAnimData* rec;
    f32 camX;
    f32 camZ;

    gLateDrawCount = 0;
    gDrawListCounts = 0;
    gPropDrawCount = 0;
    gBillboardCount = 0;
    gFlameCount = 0;
    gScrollingCount = 0;
    gPropsNearCount = 0;
    gDrawListPtrs = gDrawList;
    gLateDrawListPtr = gLateDrawList;
    gPropsNearPtr = gPropsNear;
    gBillboardPtr = gBillboardProps;
    gFlamePtr = gFlameProps;
    gScrollingPtr = gScrollingProps;
    gFloatingRestPositions = gFloatingRests;
    gMusicSourceDistance = 600.0f;
    gMusicSourceSong = -1;
    camX = cam->cam_x;
    camZ = cam->cam_z;
    switch (gMapModelData->tile_flags) {
    case 1:
        offs = gTileOffsets1;
        n = 157;
        tx = (camX - gMapModelData->min_x) / gMapModelData->tile_w;
        tz = (camZ - gMapModelData->min_z) / gMapModelData->tile_d;
        break;
    case 2:
        offs = gTileOffsets2;
        n = 89;
        tx = (camX - gMapModelData->min_x) / gMapModelData->tile_w;
        tz = (camZ - gMapModelData->min_z) / gMapModelData->tile_d;
        break;
    case 3:
        offs = gTileOffsets3;
        n = 9;
        tx = (camX - gMapModelData->min_x) / gMapModelData->tile_w;
        tz = (camZ - gMapModelData->min_z) / gMapModelData->tile_d;
        break;
    case 4:
        offs = gTileOffsets4;
        n = 21;
        tx = (camX - gMapModelData->min_x) / gMapModelData->tile_w;
        tz = (camZ - gMapModelData->min_z) / gMapModelData->tile_d;
        break;
    default:
        tx = 0;
        offs = gTileOffsets0;
        tz = 0;
        n = 1;
        break;
    }
    for (; n != 0; n--) {
        x = *offs++ + tx;
        z = *offs++ + tz;
        if (x >= 0 && z >= 0 && x < gMapModelData->tile_columns && z < gMapModelData->tile_rows) {
            tile = gMapModelData->tile_columns * z + x;
            list = &gMapModelData->wall_defs[gMapModelData->tile_offsets[tile]];
            count = *list++;
            SortMapModels(count, list);
        }
    }
    if (gMapModelData->model_count != 0) {
        list = gMapModelData->non_tiled;
        count = gMapModelData->model_count;
        SortMapModels(count, list);
    }
    gSPDisplayList(gDisplayListHead++, D_10000E8);
    gSPFogPosition(gDisplayListHead++, gFogNear, gFogFar);
    gPropsDrawn = 0;
    gDrawListPtrs = gDrawList;
    for (n = gDrawListCounts; n != 0; n--) {
        t = *gDrawListPtrs++;
        px = t->x;
        py = t->y;
        pz = t->z;
        angle = t->angle;
        if (t->flags & 0x2000) {
            door = (t->flags & 0xF00) >> 8;
            anim = &gMapModelPaths[door];
            count = t->flags;
            count &= 0xFF;
            SampleSpline(anim, &gDoorKeySample, count++);
            px += gDoorKeySample.x * t->model_scale;
            py += gDoorKeySample.y * t->model_scale;
            pz += gDoorKeySample.z * t->model_scale;
            if (count >= anim->frames) {
                count = 0;
            }
            t->flags = (t->flags & 0xFF00) | count;
        } else if (t->flags & 0x100) {
            door = FindDoorAnim(t);
            if (door != 0) {
                rec = &gDoorAnims[door - 1];
                gDustPuffOffset.x = rec->xSlideOffset;
                gDustPuffOffset.y = 0.0f;
                rotateCoordinatesByAngle(t->angle * 0.017453292519943295, &gDustPuffOffset);
                px -= gDustPuffOffset.x;
                pz -= gDustPuffOffset.y;
                angle += rec->yRotation;
            }
        }
        guPosition(&arg0->drawMtx[gMatrixHead], 0.0f, angle, 0.0f, t->model_scale, px, py, pz);
        gSPMatrix(gDisplayListHead++, &D_2000000[gMatrixHead] + 3, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
        gMatrixHead++;
        gSPDisplayList(gDisplayListHead++, gMapGeometry[t->index].dl);
        gPropsDrawn++;
    }
    gDrawListPtrs = gLateDrawList;
    for (n = gLateDrawCount; n != 0; n--) {
        t = *gDrawListPtrs++;
        guPosition(&arg0->drawMtx[gMatrixHead], 0.0f, t->angle, 0.0f, t->model_scale, t->x, t->y, t->z);
        gSPMatrix(gDisplayListHead++, &D_2000000[gMatrixHead] + 3, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
        gMatrixHead++;
        gSPDisplayList(gDisplayListHead++, gMapGeometry[t->index].dl);
        gPropsDrawn++;
    }
    DrawSpinningProps(arg0);
    if (gPropDrawCount != 0) {
        gDPPipeSync(gDisplayListHead++);
        gDPSetRenderMode(gDisplayListHead++, G_RM_FOG_SHADE_A, G_RM_AA_ZB_TEX_EDGE2);
        gPropsNearPtr = gPropsNear;
        DrawMapProps(arg0, gPropDrawCount, 0, gPropsNearPtr);
        DrawMapProps(arg0, gPropDrawCount, 1, gPropsNearPtr);
        DrawMapProps(arg0, gPropDrawCount, 2, gPropsNearPtr);
        DrawMapProps(arg0, gPropDrawCount, 3, gPropsNearPtr);
    }
    gDPSetTextureFilter(gDisplayListHead++, G_TF_BILERP);
    gDPSetTextureLUT(gDisplayListHead++, G_TT_NONE);
    for (n = 0; n < 4; n++) {
        gFloatingSwayPhases[n][0] += gFloatingSwaySpeeds[n][0];
        gFloatingSwayPhases[n][1] += gFloatingSwaySpeeds[n][1];
    }
    gSaveFloatingRest = 0;
    FadeSkyPalette();
    RotatePalette();
    song = gMusicSourceSong;
    if (song == -1) {
        if (song != gMusicSourceLastSong) {
            StopLoopingSoundSlot4();
        }
    } else {
        if (gMusicSourceDistance > 200.0) {
            n = (600.0 - gMusicSourceDistance) * 0.6375;
        } else {
            n = 255;
        }
        if (n >= 2) {
            PlaySoundNow(gMusicSourceSounds[song], n);
        } else {
            StopLoopingSoundSlot4();
            gMusicSourceSong = gMusicSourceLastSong = -1;
        }
    }
    song = gMusicSourceSong;
    gMusicSourceLastSong = song;
}

/* ---- the start of map.c's .data (0x8004DD70-0x8004DDAF): defined here because DrawMapBillboards's statics
   (0x8004DDB0-0x8004DDD3) follow them, and IDO writes .data (statics included) in definition order. The rest of
   the .data is defined further down. ---- */
u16 gPropsDrawnFrames = 0;                  /* frames the props were drawn (DrawMapBillboards; nothing reads it); IDO pads it to a word */
u16 gFloatingSwaySpeeds[4][2] = { { 0, 0 }, { 0x160, 0x160 }, { 0x160, 0x160 }, { 0x160, 0x100 } };
s16 D_8004DD84[2] = { 0x50, 0x50 };
s32 gMusicSourceSounds[4] = { 25, 27, 30, 32 };
ModelTransform gZeroModelTransform = { { 0 } };    /* the empty entry gFloatingRests[] is reset from */
void DrawMapBillboards(mkst* dyn) {
    static u16 sScroll = 0;          /* the scrolling props' texture offset, in rows */
    static u8* sTextures[5] = {      /* the scrolling props' textures by kind - 0x40 (RGBA16, 32 wide) */
        (u8*)0x801093B0, (u8*)0x8010A3B0, (u8*)0x80129D10, (u8*)0x80160B30, (u8*)0x8014DA40,
    };
    static u16 sHeights[6] = { 0x40, 0x40, 0x20, 0x40, 0x20, 0 };   /* their heights */
    ModelTransform* t;
    s32 n;
    s16 type;
    s32 kind;
    f32 angle;

    gBillboardPtr = gBillboardProps;
    n = gBillboardCount;
    if (n != 0) {
        gDPPipeSync(gDisplayListHead++);
        gDPSetRenderMode(gDisplayListHead++, G_RM_PASS, G_RM_AA_ZB_XLU_SURF2);
        gDPSetEnvColor(gDisplayListHead++, 0, 0, 0, gPropAlpha[gCurrentMap]);
        gDPSetTextureFilter(gDisplayListHead++, G_TF_BILERP);
    }
    for (; n != 0; n--) {
        t = *gBillboardPtr++;
        type = t->index;
        guPosition(&dyn->drawMtx[gMatrixHead], 0.0f, t->angle, 0.0f, t->model_scale, t->x, t->y, t->z);
        gSPMatrix(gDisplayListHead++, &D_2000000[gMatrixHead] + 3, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
        gMatrixHead++;
        gSPDisplayList(gDisplayListHead++, gMapGeometry[type].dl);
        gPropsDrawn++;
    }
    if (gScrollingCount != 0) {
        gDPSetEnvColor(gDisplayListHead++, 0, 0, 0, gPropAlpha[gCurrentMap]);
        gDPSetRenderMode(gDisplayListHead++, G_RM_FOG_SHADE_A, G_RM_AA_ZB_XLU_SURF2);
        gDPSetCombineLERP(gDisplayListHead++, TEXEL0, 0, SHADE, 0, TEXEL0, 0, ENVIRONMENT, 0, COMBINED, 0, SHADE, 0, 0, 0, 0,
                          COMBINED);
    }
    gScrollingPtr = gScrollingProps;
    for (n = gScrollingCount; n != 0; n--) {
        t = *gScrollingPtr++;
        angle = t->angle;
        type = t->index;
        kind = (t->flags & 0xFF) - 0x40;
        guPosition(&dyn->drawMtx[gMatrixHead], 0.0f, angle, 0.0f, t->model_scale, t->x, t->y, t->z);
        gSPMatrix(gDisplayListHead++, &D_2000000[gMatrixHead] + 3, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
        gMatrixHead++;
        gDPLoadTextureTile(gDisplayListHead++, sTextures[kind], G_IM_FMT_RGBA, G_IM_SIZ_16b, 32, sHeights[kind], 0, 0, 31,
                           sHeights[kind] - 1, 0, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, 5,
                           (sHeights[kind] == 32) ? 5 : 6, G_TX_NOLOD, G_TX_NOLOD);
        gDPSetTileSize(gDisplayListHead++, G_TX_RENDERTILE, 0, (sScroll % 256) << 2, 32 << 2,
                       (sHeights[kind] + sScroll % 256) << 2);
        gSPDisplayList(gDisplayListHead++, gMapGeometry[type].dl);
        gPropsDrawn++;
    }
    if (gFlameCount != 0) {
        gDPPipeSync(gDisplayListHead++);
        gDPSetRenderMode(gDisplayListHead++, G_RM_PASS, G_RM_AA_ZB_XLU_SURF2);
        gDPSetTextureFilter(gDisplayListHead++, G_TF_BILERP);
        gFlamePtr = gFlameProps;
        DrawFlames(dyn, gFlameCount, 0x11, gFlamePtr);
        DrawFlames(dyn, gFlameCount, 0x12, gFlamePtr);
    }
    gPropsDrawnFrames++;
    gDPPipeSync(gDisplayListHead++);
    gDPSetTextureFilter(gDisplayListHead++, G_TF_BILERP);
    gDPSetTextureLUT(gDisplayListHead++, G_TT_NONE);
    sScroll += 2;
}

void SortMapModels(s32 count, u16* list) {
    ModelTransform* t;
    f32 dx;
    f32 dy;
    f32 dz;
    f32 dist;
    u16 kind;
    u16 flags;

    for (; count != 0; count--) {
        t = &gMapModels[*list++];
        if (t->flags & 0x1000) {
            dx = t->x - gPlayerState.brian.transform.pos.x;
            dy = t->y - gPlayerState.brian.transform.pos.y;
            dz = t->z - gPlayerState.brian.transform.pos.z;
            dist = sqrtf(dx * dx + dy * dy + dz * dz);
            if (dist < gMusicSourceDistance) {
                gMusicSourceDistance = dist;
                gMusicSourceSong = (t->flags & 0xF00) >> 8;
            }
        }
        flags = t->flags;
        if (flags & 0x2000) {
            kind = 0;
        } else {
            kind = flags & 0xFF;
        }
        if (!kind || kind == 8) {
            if (flags & 0x4000) {
                *gLateDrawListPtr = t;
                gLateDrawListPtr++;
                gLateDrawCount++;
            } else {
                *gDrawListPtrs = t;
                gDrawListPtrs++;
                gDrawListCounts++;
            }
        } else if (kind < 4) {
            if (gSaveFloatingRest != 0) {
                *gFloatingRestPositions = *t;
            }
            t->x = (f32)sins(gFloatingSwayPhases[kind][0] + 0xA000) / gFloatingSwayDivX[kind] + gFloatingRestPositions->x;
            t->z = (f32)sins(gFloatingSwayPhases[kind][0] + 0x2000) / gFloatingSwayDivZ[kind] + gFloatingRestPositions->z;
            t->y = (f32)sins(gFloatingSwayPhases[kind][1]) / gFloatingSwayDivY[kind] + gFloatingRestPositions->y;
            *gBillboardPtr = t;
            gBillboardPtr++;
            gBillboardCount++;
            gFloatingRestPositions++;
        } else if (kind == 0x11 || kind == 0x12) {
            *gFlamePtr = t;
            gFlamePtr++;
            gFlameCount++;
        } else if (kind == 0x10) {
            *gPropsNearPtr = t;
            gPropsNearPtr++;
            gPropDrawCount++;
            gPropsNearCount++;
        } else if (kind == 0x7F || kind == 0x80) {
            *gBillboardPtr = t;
            gBillboardPtr++;
            gBillboardCount++;
        } else if (kind == 0x81) {
            UpdateEnvironmentObjects(t->x, t->y, t->z, t);
        } else if (kind == 0x83) {
            UpdateDustEmitter(t);
        } else if (kind == 0x84) {
            TickDustEmitter(t);
        } else if (kind == 0x82) {
            AddSpinningProp(t->x, t->y, t->z, t->angle, t);
        } else if (kind >= 0x40 && kind < 0x46) {
            *gScrollingPtr = t;
            gScrollingPtr++;
            gScrollingCount++;
        }
    }
}

void DrawSky(Mtx* mtx) {
    BuildActorMtx(&mtx[3] + gMatrixHead, 0.0f, 0.0f, 0.0f, 1.0f, gCamera.cam_x, gCamera.cam_y - 10.0f,
                  gCamera.cam_z);
    gSPMatrix(gDisplayListHead++, &D_2000000[gMatrixHead] + 3, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
    gMatrixHead++;
    gDPSetEnvColor(gDisplayListHead++, gSkyColour, gSkyColourG, gSkyColourB, 255);
    gSPDisplayList(gDisplayListHead++, gSkyRecords[gSubmapLight->skyRecord].dl);
}
void ResetDoorAnims(void)
{
    DoorAnimData *door = gDoorAnims;
    s32 n = 4;

  while (n--)
  {
      door->timer = 0;
      door++;
  }
}
s32 FindDoorAnim(void* target) {
    s32 i = 4;
    DoorAnimData* door = gDoorAnims;
    
    while (i != 0) {
        if ((door->timer != 0) && (target == door->position)) {
            i = 5 - i;
            break;
        }
        door++;
        i--;
    }
    return i;
}
void StartDoorAnimation(s32 state, s32 frame, s32 flag, void* position) {
    s32 i;
    DoorAnimData* door;
    ModelPart* anim;

    i = 3;
    door = gDoorAnims;
    while (i != 0 && door->timer != 0) {
        i--;
        door++;
    }
    door->unkDoorState = state;
    door->unkCurrDoorAnimFrame = frame;
    door->unkDoorFlag = flag;
    door->position = position;
    door->yRotation = 0.0f;
    door->xSlideOffset = 0.0f;
    if (state != 0) {
        switch ((flag >> 8) & 0xF) {
            case 2:
            case 3:
                if (door->unkDoorFlag & 1) {
                    anim = &gDoorAnimAlt;
                } else {
                    anim = &gDoorAnimTable;
                }
                break;
            case 4:
                anim = &gDoorAnim4;
                break;
        }
        door->timer = anim->frames - frame + 1;
    } else {
        door->timer = 7;
    }
}
void UpdateDoorAnimations(void) {
    s32 i;
    DoorAnimData* door;
    ModelPart* anim;

    i = 4;
    door = gDoorAnims;
    while (i != 0) {
        if (door->timer != 0) {
            switch (door->unkDoorState) {
                case 0:
                    door->timer--;
                    if (door->timer == 0) {
                        door->timer = 0x50;
                        door->unkDoorState++;
                    }
                    break;
                case 1:
                    door->timer--;
                    switch ((door->unkDoorFlag >> 8) & 0xF) {
                        case 2:
                        case 3:
                            if (door->unkDoorFlag & 1) {
                                anim = &gDoorAnimAlt;
                            } else {
                                anim = &gDoorAnimTable;
                            }
                            break;
                        case 4:
                            anim = &gDoorAnim4;
                            break;
                    }
                    SampleSpline(anim, &gDoorKeySample, door->unkCurrDoorAnimFrame++);
                    door->xSlideOffset = gDoorKeySample.x;
                    door->yRotation = gDoorKeySample.unkC;
                    break;
            }
        }
        i--;
        door++;
    }
}
extern f32 gViewConeHalf;              /* half the view cone, in degrees */
void DrawMapProps(mkst* dyn, s32 count, s32 type, ModelTransform** props) {
    ModelTransform* prop;
    s32 drawn;
    MapPropGfx* gfx;
    f32* size;
    MtxF mtx;
    f32 dx;
    f32 dz;
    Gfx* setup;
    Gfx* model;
    f32 angle;

    gfx = gMapHeader->propGfx;
    drawn = 0;
    setup = gfx[type].setup;
    model = gfx[type].model;
    while (count != 0) {
        prop = *props++;
        if (prop->index == type) {
            dx = prop->x - gCamera.cam_x;
            dz = prop->z - gCamera.cam_z;
            if (sqrtf(dx * dx + dz * dz) < 1500.0f) {
                angle = Atan2(dx, dz) - gCamera.yaw + 3.141592653589793;
                if (angle < -3.141592653589793) {
                    angle += 6.283185307179586;
                } else if (angle > 3.141592653589793) {
                    angle -= 6.283185307179586;
                }
                if (angle < 0.0f) {
                    angle = -angle;
                }
                angle = angle * 57.29577951308232;
                if (angle < gViewConeHalf) {
                    if (drawn == 0) {
                        gSPDisplayList(gDisplayListHead++, setup);
                        drawn++;
                        gDPSetTextureFilter(gDisplayListHead++, G_TF_BILERP);
                    }
                    BuildActorMatrix(&mtx, 0.0f, gCamera.yaw, 0.0f, prop->model_scale, prop->x, prop->y, prop->z);
                    guMtxF2L(mtx.mf, &dyn->drawMtx[gMatrixHead]);
                    gSPMatrix(gDisplayListHead++, &D_2000000[gMatrixHead] + 3, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
                    gMatrixHead++;
                    gSPDisplayList(gDisplayListHead++, model);
                    size = gShadowSizes[(s32)prop->angle];
                    AddShadow(prop->x, prop->y, prop->z, size[0], size[1], size[2], 8.0f);
                    gPropsDrawn++;
                }
            }
        }
        count--;
    }
}

void UpdateDustEmitter(ModelTransform* obj) {
    Dust* dust = &gMapDustTemplate;

    obj->index++;
    if (obj->index >= 0) {
        dust->x = obj->x;
        dust->y = obj->y;
        dust->z = obj->z;
        dust->scaleY = obj->model_scale * 10.0;
        dust->scaleX = dust->scaleY;
        gDustPuffOffset.x = 2.0 * obj->model_scale;
        gDustPuffOffset.y = 0.0f;
        rotateCoordinatesByAngle(obj->angle, &gDustPuffOffset);
        dust->vx = gDustPuffOffset.x;
        dust->vz = gDustPuffOffset.y;
        dust->dustTexture = gMapDustTextures + (obj->index << 10);
        SpawnDust(dust);
        if (obj->index >= 7) {
            obj->index = getRandomNumber(60) - 120;
        }
    }
}
void UpdateFallingParticle(Dust* p) {

    p->x += p->vx;
    p->y += p->vy;
    p->z += p->vz;
    p->vy -= 0.9;

    gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, (gLavaFrames & 1) ? 220 : 120);
}
void TickDustEmitter(ModelTransform* obj) {
    Dust* dust = &gDustEmitterTemplate;

    if (--obj->index == 0) {
        dust->x = obj->x;
        dust->y = obj->y;
        dust->z = obj->z;
        dust->scaleY = obj->model_scale;
        dust->scaleX = dust->scaleY;
        SpawnDust(dust);
        obj->index = getRandomNumber(0x96) + 0x96;
    }
}
void UpdateArcParticle(Dust* p)
{
  p->y += p->vy / 2;
  p->vy -= 0.2;
  gDPSetEnvColor(gDisplayListHead++, 0xFF, 0xFF, 0xFF, p->frames * 5);
}
void ResetSpinningProps(void) {
    SpinningProp* recB;
    SpinningProp* recA;
    s32 n;

    gSpinningPropsLast = 0;
    n = 8;
    recA = gSpinningProps1;
    recB = gSpinningProps0;

    for (; n != 0; recA++, recB++, n--) {
        recB->active = 0;
        recA->active = 0;
    }
}
void AddSpinningProp(f32 x, f32 y, f32 z, f32 angle, ModelTransform* prop) {
    s32 n;
    SpinningProp* rec;
    SpinningProp* prev;

    if (gCurrentFrame != 0) {
        rec = gSpinningProps1;
    } else {
        rec = gSpinningProps0;
    }

    if (gCurrentFrame != 0) {
        prev = gSpinningProps0;
    } else {
        prev = gSpinningProps1;
    }
    if (gSpinningPropsAdded < 8) {
        n = gSpinningPropsLast;
        rec += gSpinningPropsAdded;

        while (n != 0 && prop != prev->prop) {
            n--;
            prev++;
        }
        if (n != 0) {
            rec->active = prev->active;
            rec->spin = prev->spin;
        } else {
            rec->active = 1;
            rec->spin = (f32)getRandomNumber(8) * 3.141592653589793 / 4;
        }
        rec->x = x;
        rec->y = y;
        rec->z = z;
        rec->angle = angle;
        rec->prop = prop;
        gSpinningPropsAdded++;
    }
}
typedef struct {
    s32 time;                       /* from this time of day (gTimeOfDay) */
    u16 steps;                      /* fade length in frames */
    u16 palette;                    /* index into gSkyFadePalettes; 0xFF ends the table */
} SkyPaletteKey;                    /* gSkyPaletteKeys */

/* ---- map.c's .data (0x8004DD70-0x8004DED0, ROM 0x4E970). In address order: IDO writes
   .data in definition order. ---- */
/* 0x8004DD70-0x8004DDAF are defined above DrawMapBillboards, 0x8004DDB0-0x8004DDD3 are its statics */
f32 gFloatingSwayDivX[4] = { 0.0f, 5000.0f, 1000.0f, 3000.0f };
f32 gFloatingSwayDivY[4] = { 0.0f, 30000.0f, 30000.0f, 48000.0f };
f32 gFloatingSwayDivZ[4] = { 0.0f, 1000.0f, 5000.0f, 3000.0f };
Dust gMapDustTemplate = {                  /* the map dust template (UpdateDustEmitter fills and spawns it) */
    0x23, 0, 0, UpdateFallingParticle, 7, 0x20, 0x20, 0, 0, { 0 }, gMapDustTextures, gMapDustGfx,
    0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 15.0f, 0.0f, 0, { 0 },
};
Dust gDustEmitterTemplate = {                  /* the second dust template (TickDustEmitter) */
    0xF, 0, 0, UpdateArcParticle, 0, 8, 0x10, 0, 0, { 0 }, gDustTexture2, gDustGfx2,
    0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0, { 0 },
};
u16 gFlameTexWidths[2] = { 0x10, 0x10 };     /* the flame kinds' texture widths (DrawFlames) */
u8 gFlameAlphas[4] = { 0x82, 0xDC, 0x78, 0xF0 };
/* 0x8004DE8C-0x8004DEC7 are FadeSkyPalette's statics */
/* 0x8004DEC8 is RotatePalette's function-local static (its delay counter), the last item of this .data */
void DrawSpinningProps(mkst* dyn) {
    SpinningProp* rec;
    ModelTransform* prop;
    s32 n;

    gSpinningPropsLast = gSpinningPropsAdded;
    if (gCurrentFrame != 0) {
        rec = gSpinningProps1;
    } else {
        rec = gSpinningProps0;
    }
    n = gSpinningPropsLast;
    while (n--) {
        if (rec->active != 0) {
            prop = rec->prop;
            rec->spin -= 0.015707963267948967;
            BuildActorMtx(&dyn->drawMtx[gMatrixHead], 0.0f, rec->angle * 0.017453292519943295, rec->spin, prop->model_scale,
                          prop->x, prop->y, prop->z);
            gSPMatrix(gDisplayListHead++, &D_2000000[gMatrixHead] + 3, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
            gMatrixHead++;
            gSPDisplayList(gDisplayListHead++, gMapGeometry[prop->index].dl);
            gPropsDrawn++;
        }
        rec++;
    }
}
void DrawFlames(mkst* dyn, s32 count, s32 kind, ModelTransform** list) {
    ModelTransform* t;
    s32 idx;
    s32 started;
    u8** frames;
    Gfx* model;

    idx = kind - 0x11;
    started = 0;
    model = gFlameModelLists[kind - 0x11].model;
    frames = gFlameFrameTables[kind - 0x11];
    for (; count != 0; count--) {
        t = *list++;
        if ((t->flags & 0xFF) == kind) {
            if (started == 0) {
                gSPDisplayList(gDisplayListHead++, gFlameModelLists[idx].setup);
                started++;
            }
            gDPLoadTextureBlock(gDisplayListHead++, frames[(t->index >> 1) & 7], G_IM_FMT_CI, G_IM_SIZ_8b,
                                gFlameTexWidths[kind - 0x11], 32, 0, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP,
                                G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
            BuildActorMtx(&dyn->drawMtx[gMatrixHead], 0.0f, gCamera.yaw, 0.0f, t->model_scale, t->x, t->y, t->z);
            gSPMatrix(gDisplayListHead++, &D_2000000[gMatrixHead] + 3, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
            gMatrixHead++;
            gDPSetEnvColor(gDisplayListHead++, 0, 0, 0, gFlameAlphas[t->index & 3]);
            gSPDisplayList(gDisplayListHead++, model);
            t->index++;
            gPropsDrawn++;
        }
    }
}
extern s32 gTimeOfDay;
void FadeSkyPalette(void) {
    static s32 gSkyPaletteWanted = 0;   /* the palette the time of day asks for */
    static s32 gSkyFadeLeft = 1;        /* fade steps left */
    static s32 gSkyFadeTotal;           /* the fade's length in steps */
    static s32 gSkyPaletteCurrent = 1;  /* the palette shown */
    static SkyPaletteKey gSkyPaletteKeys[6] = {
        { 0, 0x1C, 2 }, { 0x15F9, 0x80, 0 }, { 0x4F1A, 0x1C, 1 }, { 0x6513, 0x1C, 2 }, { 0x6977, 0x1C, 2 }, { 0x6979, 0xFF, 0xFF },
    };
    static u16* gSkyFadeFrom;           /* the palette faded from */
    static u16* gSkyFadeTo;             /* the palette faded to */
    static u16* gSkyFadeOut;            /* where the faded palette goes */
    SkyPaletteKey* key;
    u16* out;
    u16* from;
    u16* to;
    s32 i;
    s32 done;
    s32 c1;
    s32 n;
    u16 r1;
    u16 g1;
    u16 b1;
    u16 r2;
    u16 g2;
    u16 b2;

    for (key = gSkyPaletteKeys; key->palette != 0xFF; key++) {
        if (gTimeOfDay >= key[0].time && gTimeOfDay < key[1].time) {
            gSkyPaletteWanted = key->palette;
            break;
        }
    }
    if (gSkyPaletteWanted != gSkyPaletteCurrent) {
        gSkyFadeFrom = &gSkyFadePalettes[gSkyPaletteCurrent * 256];
        gSkyFadeLeft = gSkyFadeTotal = key->steps;
        gSkyFadeTo = &gSkyFadePalettes[gSkyPaletteWanted * 256];
        gSkyFadeOut = gSkyFadedPalette;
    }
    if (gSkyFadeLeft != 0) {
        out = gSkyFadeOut;
        from = gSkyFadeFrom;
        to = gSkyFadeTo;
        done = gSkyFadeTotal - gSkyFadeLeft;
        i = n = 0xFF;
        while (n != 0) {
            c1 = *from;
            r1 = (*from & 0xF800) >> 11;
            g1 = (c1 & 0x7C0) >> 6;
            b1 = (c1 & 0x3E) >> 1;
            r2 = (*to & 0xF800) >> 11;
            g2 = (*to & 0x7C0) >> 6;
            b2 = (*to & 0x3E) >> 1;
            from++;
            to++;
            *out++ = (((r1 * gSkyFadeLeft + r2 * done) / gSkyFadeTotal) << 11) | (((g1 * gSkyFadeLeft + g2 * done) / gSkyFadeTotal) << 6) |
                     (((b1 * gSkyFadeLeft + b2 * done) / gSkyFadeTotal) << 1);
            n = i--;
        }
        gSkyFadeLeft--;
    }
    gSkyPaletteCurrent = gSkyPaletteWanted;
}

void RotatePalette(void) {
    static s32 sTimer = 0;
    s32 i;
    u16 last;
    u16* p;

    if (sTimer >= 0) {
        sTimer = 0;
        last = gRotatingPalette[201];
        i = 201;
        p = &gRotatingPalette[200];
        while (i != 0) {
            p[1] = p[0];
            p--;
            i--;
        }
        gRotatingPalette[0] = last;
    } else {
        sTimer++;
    }
}
