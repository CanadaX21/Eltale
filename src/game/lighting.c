#include "common.h"
/* split from collision.c: the original was separate files (each IDO object pads its .rodata to 16;
   the zero words before this file's first literal mark the boundary). Declarations copied from collision.c. */
/* ---- lighting.c's .bss (0x80084330-0x80084B80), in address order. IDO 5.3 lays out the
   variables first declared among a file's first 32 declarations (typedefs and variables, externs too) after
   all later ones; this block is all inside those 32 (one to spare) and the file declares no other .bss, so it
   keeps its order. Adding declarations above it, or declaring these variables there, moves them; so do
   typedefs / externs added to or removed from the headers (they count too: recount the declarations
   above this block after a header edit).
   D_8008434C is space no code addresses (an unknown variable). ---- */
s32 gNextDayPeriod;                         /* the next period */
s32 gFirstDayPeriodEnd;                         /* the first period's end */
u16 gPlayTimeTicks;                         /* ticks toward the next play-time minute (1501 each) */
s32 gPaletteFadeSteps;                         /* palette fade steps left (counted down by the caller) */
s32 gPaletteFadeLength;                         /* steps in the fade */
u16* gPaletteFadeTarget;                        /* the colours it fades towards */
u16* gPaletteFadeWork;                        /* the palette being worked on */
s32 D_8008434C;                         /* unreferenced */
u16 gPaletteFadeSource[256];                    /* its original colours */
MtxF gLightMtx;
Lights3 gFrameLights;                     /* the two frames' lights */
Lights3 gFrameLights1;
Lights3 gLightAnimLights0[8];
Lights3 gLightAnimLights1[8];
s32 gTimeOfDay;                         /* time of day */
s32 gDayLength;
s32 gLightTime;                         /* time of day for the lights */
s32 gFramesPerDay;                         /* frames per day */
u16 gDayCount;
u16 gPlayTime;                         /* play time in minutes: +1 every 1501 gPlayTimeTicks, capped at 59939; saved */
MapLight* gSubmapLight;                   /* the map's light data */
LightAnim gLightAnims[8];                /* the light animators (LightAnim) */
s32 gLightColours[12];                     /* the map's light colours (SetupLights reads 9; ResetTimeOfDay clears 12) */
s32 gLightDirections[6];                      /* and directions */
s32 gSkyColour;                         /* the sky colour (map.c) */
s32 gSkyColourG;
s32 gSkyColourB;
s32 gFogColourR;                         /* (effects.c) */
s32 gFogColourG;
s32 gFogColourB;
FogRange gFogNow;                      /* and fog range */
extern u16 gLightRecordFlags;   /* bit 0: the sun follows the time of day */
extern MapModelData* gMapModelData;
extern NavMeshData* gWallNavmeshActors;
extern NavMeshData* gWallNavmeshCamera;
extern s32 gCurrentSubmap;
s32 PollModelInstance(f32 sample_x, f32 sample_z, u16 face_mask, u16 model_index, CollisionPollingResult* result);
s32 PushOutOfWalls(f32 actor_x, f32 actor_z, NavMeshData* navmesh, CollisionPollingResult* result);
extern ModelTransform* gMapModels;
extern ModelGeometry* gMapGeometry;
extern f64 D_8006F0A0;
s32 PollModelFaces(f32 x, f32 z, f32 scale, u16 face_mask, s32 geometry, CollisionPollingResult* result);
void rotateCoordinatesByAngle(f32 angle, Coordinates2D* coordinates);
extern f32 gCollisionRadius;               /* collision radius */
s32 PushOutOfWallSegment(f32* x, f32* z, f32 radius, f32 ax, f32 az, f32 bx, f32 bz);
extern Submap* gSubmapHeader;
extern u16 gBattleState;
extern Coordinates2D gDoorOffset;
s32 CheckDoorKeyItem(MapDoor* door);
s32 IsAngleWithin22(f32);
void SetDoorWarp(MapDoor* door);
extern u8 gInventory[150];
extern PendingWarp gPendingWarp;
extern u16* gWarpDoorDest;
extern DayPeriod gDayPalettePeriods[];   /* the day's palette periods */
extern u16 gMapFlags;
extern s32 gFieldFreeze;
extern s32 D_8008C164;
extern u16 gSkyPalettes[];             /* the sky palettes, 256 colours each */
extern u16 gFadingPalette[];             /* the palette being faded */
extern s32 gGameState;               /* game state flags */
void UpdateMapLights(s32 time, MapLight* lights);
void StartPaletteFade(s32 steps, u16* target, u16* palette);
void StepPaletteFade(s32 left);
void ResetTimeOfDay(void) {
    s32 i;

    gTimeOfDay = 0x1EC3;
    gDayLength = 0x6978;
    gDayCount = 1;
    D_8008C164 = 7;
    for (i = 0; i < 4; i++) {          /* the four colours, three words each */
        gLightColours[i * 3] = 0;
        gLightColours[i * 3 + 1] = 0;
        gLightColours[i * 3 + 2] = 0;
    }
}

void ResetLightAnims(void);
void InitDayPalette(void) {
    s32 i;
    s32 last;
    DayPeriod* period;
    u16* dst;
    u16* src;
    LightKey* key;
    DayPeriod* top;
    s32 t;

    top = &gDayPalettePeriods[3];
    period = top;
    i = 3;
    if (period != 0) {                  /* always passes: period holds a constant address */
        t = gFirstDayPeriodEnd = period->time;
    }
    if (gTimeOfDay < t) {
        do {
            period--;
            last = i;
            i--;
            if (last == 0) {
                break;
            }
        } while (gTimeOfDay < period->time);
    }
    if (i < 0) {
        period = top;
        gNextDayPeriod = 3;
    } else {
        gNextDayPeriod = i;
    }
    src = &gSkyPalettes[period->palette * 256];
    dst = gFadingPalette;
    for (i = 0x100; i != 0; i--) {
        *dst++ = *src++;
    }
    gPaletteFadeSteps = 0;
    if (++gNextDayPeriod >= 4) {
        gNextDayPeriod = 0;
    }
    if (gLightRecordFlags & 2) {
        gLightTime = gTimeOfDay;
        gFramesPerDay = gDayLength;
    } else {
        key = &gSubmapLight->keys[gSubmapLight->count - 1];
        gLightTime = 0;
        gFramesPerDay = key->time;
    }
    ResetLightAnims();
    gPlayTimeTicks = 0;
}

void UpdateTimeOfDay(void) {
    DayPeriod* period;

    UpdateMapLights(gLightTime, gSubmapLight);
    if (gPaletteFadeSteps != 0) {
        gPaletteFadeSteps--;
        if (gPaletteFadeSteps < 0) {
            gPaletteFadeSteps = 0;
        }
        StepPaletteFade(gPaletteFadeSteps);
    } else {
        period = &gDayPalettePeriods[gNextDayPeriod];
        if (gLightTime >= period->time && (gNextDayPeriod != 0 || gLightTime < gFirstDayPeriodEnd)) {
            StartPaletteFade(period->fadeFrames, &gSkyPalettes[period->palette * 256], gFadingPalette);
            gNextDayPeriod++;
            if (gNextDayPeriod >= 4) {
                gNextDayPeriod = 0;
            }
        }
    }
    if (gMapFlags & 5) {
        if (!(gBattleState & 1) && !(gGameState & 3) && gFieldFreeze == 0) {
            gTimeOfDay++;
            if (gTimeOfDay >= gDayLength) {
                gTimeOfDay -= gDayLength;
                if (gDayCount < 999) {
                    gDayCount++;
                }
            }
            gLightTime = gTimeOfDay;
        }
    } else if ((gLightRecordFlags & 1) && gFieldFreeze == 0) {
        gLightTime++;
        if (gLightTime >= gFramesPerDay) {
            gLightTime -= gFramesPerDay;
        }
    }
    gPlayTimeTicks++;
    if (gPlayTimeTicks >= 1501) {
        gPlayTimeTicks = 0;
        if (gPlayTime < 59939) {
            gPlayTime++;
        }
    }
}
void SetTimeOfDay(s32 time) {

    gTimeOfDay = time;
    if (gDayLength <= gTimeOfDay) {
        gTimeOfDay = gTimeOfDay - gDayLength;
        if (gDayCount < 9999) {
            gDayCount++;
            }
    }
    gLightTime = gTimeOfDay;
}
void UpdateMapLights(s32 time, MapLight* lights) {
    Rgb32* dst;
    LightKey* key;
    LightKey* prev;
    Rgb32* from;
    Rgb32* to;
    s32 span;
    s32 into;
    s32 rest;
    s32 i;
    s32 count;

    count = lights->count;
    key = lights->keys;
    dst = (Rgb32*)gLightColours;
    if (count == 1) {
        from = key->colours;
        for (i = 4; i != 0; i--) {
            *dst++ = *from++;
        }
        gFogNow = key->fog;
        return;
    }
    while (time >= key->time) {
        key++;
    }
    prev = key - 1;
    span = key->time - prev->time;
    into = time - prev->time;
    rest = span - into;
    from = prev->colours;
    to = key->colours;
    for (i = 4; i != 0; i--) {
        dst->r = (from->r * rest + to->r * into) / span;
        dst->g = (from->g * rest + to->g * into) / span;
        dst->b = (from->b * rest + to->b * into) / span;
        dst++;
        from++;
        to++;
    }
    gFogNow.near = (prev->fog.near * rest + key->fog.near * into) / span;
    gFogNow.far = (prev->fog.far * rest + key->fog.far * into) / span;
}

extern s32 gCurrentFrame;               /* which frame is current */
extern Gfx* gDisplayListHead;
extern s16 gFogNear;               /* fog near */
extern s16 gFogFar;               /* fog far */
void UpdateLightDirections();               /* takes no arguments (lights stays in a0 for SetupLights) */
void SetupLights(Lights3* lights, s32* colors, s32* dirs);
void UpdateFrameLights(void) {
    Lights3* lights;

    if (gCurrentFrame != 0) {
        lights = &gFrameLights1;
    } else {
        lights = &gFrameLights;
    }
    UpdateLightDirections(lights);
    SetupLights(lights, gLightColours, gLightDirections);
    gFogNear = gFogNow.near;
    gFogFar = gFogNow.far;
    gSPFogPosition(gDisplayListHead++, gFogNear, gFogFar);
    gDPSetEnvColor(gDisplayListHead++, 0, 0, 0, 0xFF);
}
extern MtxF gViewMtxF;              /* the view matrix */
void BuildRotationMatrix(MtxF *mtx, f32 xrot, f32 yrot, f32 zrot);   /* rotation matrix */
void TransformVecByRows(MtxF* mf, f32 x, f32 y, f32 z, f32* ox, f32* oy, f32* oz);
void UpdateLightDirections(void) {
    f32 angle;
    f32 ox;
    f32 oy;
    f32 oz;

    if (gLightRecordFlags & 1) {
        angle = (f32)gLightTime * 3.141592653589793 * (1.0 + 1.0) / (f32)gFramesPerDay - 3.141592653589793;
        BuildRotationMatrix(&gLightMtx, 0.0f, 0.0f, angle);
        guMtxXFMF(gLightMtx.mf, 0.0f, 90.0f, 50.0f, &ox, &oy, &oz);
        gLightDirections[0] = ox;
        gLightDirections[1] = oy;
        gLightDirections[2] = oz;
    } else if (gSubmapLight->hasDirection != 0) {
        gLightDirections[0] = gSubmapLight->direction[0];
        gLightDirections[1] = gSubmapLight->direction[1];
        gLightDirections[2] = gSubmapLight->direction[2];
    } else {
        gLightDirections[0] = -89;
        gLightDirections[1] = 89;
        gLightDirections[2] = 20;
    }
    TransformVecByRows(&gViewMtxF, -89.0f, 89.0f, 20.0f, &ox, &oy, &oz);
    gLightDirections[3] = ox;
    gLightDirections[4] = oy;
    gLightDirections[5] = oz;
}
/* A palette fade (CI8, 256 RGBA5551 colours): StartPaletteFade starts it, StepPaletteFade sets each step. */
void StartPaletteFade(s32 steps, u16* target, u16* palette) {
    u16* copy;
    s32 i;

    gPaletteFadeLength = steps;
    gPaletteFadeSteps = gPaletteFadeLength;
    gPaletteFadeTarget = target;
    gPaletteFadeWork = palette;
    copy = gPaletteFadeSource;
    i = 256;
    while (i--) {
        *copy++ = *palette++;
    }
}
void StepPaletteFade(s32 left) {
    u16* target = gPaletteFadeTarget;
    u16* original = gPaletteFadeSource;
    u16* out = gPaletteFadeWork;
    s32 i = 256;
    u16 r, g, b;
    u16 or, og, ob;

    if (left != 0) {
        while (i--) {
            r = (*target & 0xF800) >> 11;
            g = (*target & 0x7C0) >> 6;
            b = (*target & 0x3E) >> 1;
            or = (*original & 0xF800) >> 11;
            og = (*original & 0x7C0) >> 6;
            ob = (*original & 0x3E) >> 1;
            r = (r * (gPaletteFadeLength - left) + or * left) / gPaletteFadeLength;
            g = (g * (gPaletteFadeLength - left) + og * left) / gPaletteFadeLength;
            b = (b * (gPaletteFadeLength - left) + ob * left) / gPaletteFadeLength;
            *out = (r << 11) | (g << 6) | (b << 1);
            target++;
            original++;
            out++;
        }
    } else {
        while (i--) {
            *out = *target;
            target++;
            out++;
        }
    }
}
void ResetLightAnims(void) {
    LightAnim* anim;
    s32 i;

    anim = gLightAnims;
    i = 8;
    while (i > 0) {
        anim->mode = 0;
        anim++;
        i--;
    }
}
s32 AllocLightAnim(void) {
    s32 i;
    LightAnim* anim;

    anim = &gLightAnims[7];
    for (i = 7; i != 0 && anim->mode != 0; i--) {
        anim--;
    }
    anim->mode = 1;
    anim->colourMode = 0;
    anim->lightMode = 0;
    anim->dirs = NULL;
    return i;
}
void FreeLightAnim(s32 n) {
    gLightAnims[n].mode = 0;
}
void SetLightAnimColours(s32 n, s32 mode, s32 count, s32* keys) {
    LightAnim* anim;

    anim = &gLightAnims[n];
    anim->colourMode = mode;
    anim->colourLoopCount = count;
    anim->colourCount = count;
    anim->colourLoop = keys;
    anim->colourKey = keys;
}
void SetLightAnimLights(s32 n, s32 mode, s32 count, s32* keys) {
    LightAnim* anim;

    anim = &gLightAnims[n];
    anim->lightMode = mode;
    anim->lightLoopCount = count;
    anim->lightCount = count;
    anim->lightLoop = keys;
    anim->lightKey = keys;
}
void SetLightAnimDirections(s32 n, s32* dirs) {
    gLightAnims[n].dirs = dirs;
}
extern s32 gSkyFillR;
extern s32 gSkyFillG;
extern u32 gSkyFillB;
void StepLightAnim(s32 index) {
    LightAnim* anim;
    s32* key;
    s32 r;
    s32 g;
    s32 b;
    Lights3* lights;
    s32* colours;
    s32 i;
    s32* p;
    s32* dirs;
    s32 buf[12];

    anim = &gLightAnims[index];
    if (gCurrentFrame != 0) {
        lights = &gLightAnimLights1[index];
    } else {
        lights = &gLightAnimLights0[index];
    }
    if (anim->colourMode != 0) {
        colours = anim->colourKey;
        r = colours[0];
        g = colours[1];
        b = colours[2];
        switch (anim->colourMode) {
            case 1:
                anim->colourKey += 3;
                if (--anim->colourCount == 0) {
                    anim->colourMode = 0;
                }
                break;
            case 2:
                if (anim->colourCount != 0) {
                    if (--anim->colourCount != 0) {
                        anim->colourKey += 3;
                    }
                }
                break;
            case 3:
                if (--anim->colourCount == 0) {
                    anim->colourCount = anim->colourLoopCount;
                    anim->colourKey = anim->colourLoop;
                } else {
                    anim->colourKey += 3;
                }
                break;
        }
    } else {
        r = g = b = 0;
    }
    gDPSetEnvColor(gDisplayListHead++, r, g, b, 0xB4);
    if (anim->lightMode != 0) {
        key = anim->lightKey;
        buf[0] = (key[0] * gLightColours[0]) >> 8;
        buf[1] = (key[1] * gLightColours[1]) >> 8;
        buf[2] = (key[2] * gLightColours[2]) >> 8;
        key += 3;
        p = &gLightColours[3];
        colours = &buf[3];
        for (i = 3; i != 0; i--) {
            colours[0] = (key[0] * p[0]) >> 8;
            colours[1] = (key[1] * p[1]) >> 8;
            colours[2] = (key[2] * p[2]) >> 8;
            p += 3;
            colours += 3;
        }
        gSkyColour = key[0];
        gSkyColourG = key[1];
        gSkyColourB = key[2];
        colours = buf;
        switch (anim->lightMode) {
            case 1:
                anim->lightKey += 6;
                if (--anim->lightCount == 0) {
                    anim->lightMode = 0;
                }
                break;
            case 2:
                if (anim->lightCount != 0) {
                    if (--anim->lightCount != 0) {
                        anim->lightKey += 6;
                    }
                }
                break;
            case 3:
                if (--anim->lightCount == 0) {
                    anim->lightCount = anim->lightLoopCount;
                    anim->lightKey = anim->lightLoop;
                } else {
                    anim->lightKey += 6;
                }
                break;
        }
    } else {
        gSkyColour = gSkyColourG = gSkyColourB = 0xFF;
        colours = gLightColours;
    }
    dirs = anim->dirs;
    if (dirs == NULL) {
        dirs = gLightDirections;
    }
    if (anim->mode == 2) {
        p = colours;
        colours = buf;
        for (i = 3; i != 0; i--) {
            colours[0] = p[0] - (p[0] >> 2) + 0x3F;
            colours[1] = p[1] - (p[1] >> 2) + 0x3F;
            colours[2] = p[2] - (p[2] >> 2) + 0x3F;
            colours += 3;
            p += 3;
        }
        colours[0] = p[0];
        colours[1] = p[1];
        colours[2] = p[2];
        colours = buf;
    }
    SetupLights(lights, colours, dirs);
    gSPSetLights3(gDisplayListHead++, (*lights));
    gDPSetFogColor(gDisplayListHead++, colours[9], colours[10], colours[11], 0xFF);
    if (gMapFlags & 0x80) {
        gSkyFillR = colours[9];
        gSkyFillG = colours[10];
        gSkyFillB = colours[11];
    } else {
        gSkyFillR = gSkyFillG = gSkyFillB = 0;
    }
    gFogColourR = colours[9];
    gFogColourG = colours[10];
    gFogColourB = colours[11];
}

void SetupLights(Lights3* lights, s32* colors, s32* dirs) {
    lights->l[1].l.dir[0] = *dirs++;
    lights->l[1].l.dir[1] = *dirs++;
    lights->l[1].l.dir[2] = *dirs++;
    lights->l[2].l.dir[0] = lights->l[0].l.dir[0] = dirs[0];
    lights->l[2].l.dir[1] = lights->l[0].l.dir[1] = dirs[1];
    lights->l[2].l.dir[2] = lights->l[0].l.dir[2] = dirs[2];
    lights->a.l.col[0] = lights->a.l.colc[0] = *colors++;
    lights->a.l.col[1] = lights->a.l.colc[1] = *colors++;
    lights->a.l.col[2] = lights->a.l.colc[2] = *colors++;
    lights->l[1].l.col[0] = lights->l[1].l.colc[0] = *colors++;
    lights->l[1].l.col[1] = lights->l[1].l.colc[1] = *colors++;
    lights->l[1].l.col[2] = lights->l[1].l.colc[2] = *colors++;
    lights->l[0].l.col[0] = lights->l[0].l.colc[0] = colors[0];
    lights->l[0].l.col[1] = lights->l[0].l.colc[1] = colors[1];
    lights->l[0].l.col[2] = lights->l[0].l.colc[2] = colors[2];
    lights->l[2].l.col[0] = lights->l[2].l.colc[0] = colors[0];
    lights->l[2].l.col[1] = lights->l[2].l.colc[1] = colors[1];
    lights->l[2].l.col[2] = lights->l[2].l.colc[2] = colors[2];
}
