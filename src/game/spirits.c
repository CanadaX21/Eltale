#include "common.h"
BSS_ORDER_PAD(0)
#include "game/spirits.h"
#include "game/collision.h"
#include "game/dust.h"
#include "game/math.h"
#include "game/main.h"
#include "game/map.h"
#include "maps.h"
/* split from map_objects.c: the original was separate files (each IDO object pads its .rodata to 16;
   the zero words before this file's first literal mark the boundary). */
/* ---- spirits.c's .bss (0x800859B0-0x80085AB0), in address order, the order game/spirits.h declares
   them in: IDO 5.3 lays the unit's .bss out by those declarations (include/bss_order.h). ---- */
s32 gSpiritCount;                         /* spirits on this submap */
Spirit gSubmapSpirits[8];                   /* this submap's spirits (SetupMapSpirits) */
CollisionPollingResult gSpiritFloorPoll;      /* the floor poll for a spirit's height */
u8 gSpiritsFound[0x10];                    /* spirits found: one bit per spirit flag (IsSpiritFound / SetSpiritFound) */
/* ---- spirits.c's .data (0x8004DFC0-0x8004E6A0), in address order: the spirit table,
   then (below, before UpdateSpiritSparkles) the sparkle's quad, display list and frames, the sparkle template and its
   colours. SetupMapSpirits finds the row of gCurrentMap and gCurrentSubmap; its spots lie in the map as loaded, and
   every row names its map's record of them. ---- */
SubmapSpirits gSubmapSpiritTable[43] = {
    { 0, 0, 1, gMap00Room00Spirits },   /* Melrode: built from typed sources */
    { 0, 1, 1, gMap00Room01Spirits },   /* Melrode: built from typed sources */
    { 1, 0, 1, gMap01Room00Spirits },   /* Dondoran: built from typed sources */
    { 2, 0, 2, gMap02Room00Spirits },   /* Holy Plains: built from typed sources */
    { 3, 0, 3, gMap03Room00Spirits },   /* Dondoran Flats: built from typed sources */
    { 4, 2, 3, gMap04Room02Spirits },   /* Larapool: built from typed sources */
    { 5, 0, 3, gMap05Room00Spirits },   /* West Carmagh: built from typed sources */
    { 6, 0, 3, gMap06Room00Spirits },   /* Normoon: built from typed sources */
    { 7, 0, 4, gMap07Room00Spirits },   /* East Limelin: built from typed sources */
    { 8, 0, 4, gMap08Room00Spirits },   /* Limelin: built from typed sources */
    { 8, 1, 1, gMap08Room01Spirits },   /* Limelin: built from typed sources */
    { 9, 0, 6, gMap09Room00Spirits },   /* Dindom Dries: built from typed sources */
    { 10, 0, 2, gMap10Room00Spirits },   /* Shamwood: built from typed sources */
    { 10, 1, 2, gMap10Room01Spirits },   /* Shamwood: built from typed sources */
    { 11, 1, 1, gMap11Room01Spirits },   /* Baragoon Moor: built from typed sources */
    { 11, 2, 4, gMap11Room02Spirits },   /* Baragoon Moor: built from typed sources */
    { 12, 0, 4, gMap12Room00Spirits },   /* Isle of Skye: built from typed sources */
    { 26, 0, 6, gMap26Room00Spirits },   /* Blue Cave: built from typed sources */
    { 26, 2, 1, gMap26Room02Spirits },   /* Blue Cave: built from typed sources */
    { 27, 0, 2, gMap27Room00Spirits },   /* Cull Hazard: built from typed sources */
    { 28, 0, 5, gMap28Room00Spirits },   /* Baragoon Tunnel: built from typed sources */
    { 29, 0, 2, gMap29Room00Spirits },   /* Boil Hole: built from typed sources */
    { 30, 8, 1, gMap30Room08Spirits },   /* Brannoch Castle: built from typed sources */
    { 30, 15, 1, gMap30Room15Spirits },   /* Brannoch Castle: built from typed sources */
    { 31, 0, 4, gMap31Room00Spirits },   /* Connor Forest: built from typed sources */
    { 31, 2, 1, gMap31Room02Spirits },   /* Connor Forest: built from typed sources */
    { 32, 0, 6, gMap32Room00Spirits },   /* Glencoe Forest: built from typed sources */
    { 33, 0, 3, gMap33Room00Spirits },   /* Windward Forest: built from typed sources */
    { 33, 1, 1, gMap33Room01Spirits },   /* Windward Forest: built from typed sources */
    { 14, 15, 1, gMap14Room15Spirits },   /* Dondoran Castle: built from typed sources */
    { 14, 12, 1, gMap14Room12Spirits },   /* Dondoran Castle: built from typed sources */
    { 16, 21, 1, gMap16Room21Spirits },   /* Dondoran buildings: built from typed sources */
    { 21, 0, 2, gMap21Room00Spirits },   /* Limelin Castle: built from typed sources */
    { 21, 6, 1, gMap21Room06Spirits },   /* Limelin Castle: built from typed sources */
    { 22, 1, 1, gMap22Room01Spirits },   /* Limelin buildings: built from typed sources */
    { 22, 23, 2, gMap22Room23Spirits },   /* Limelin buildings: built from typed sources */
    { 23, 19, 1, gMap23Room19Spirits },   /* Highland buildings: built from typed sources */
    { 23, 23, 1, gMap23Room23Spirits },   /* Highland buildings: built from typed sources */
    { 24, 0, 4, gMap24Room00Spirits },   /* Shamwood: built from typed sources */
    { 24, 2, 2, gMap24Room02Spirits },   /* Shamwood: built from typed sources */
    { 25, 1, 1, gMap25Room01Spirits },   /* ship and cabin rooms: the Pirate Ship at Skye: built from typed sources */
    { 25, 4, 1, gMap25Room04Spirits },   /* ship and cabin rooms: the Pirate Ship at Skye: built from typed sources */
    { 25, 9, 1, gMap25Room09Spirits },   /* ship and cabin rooms: the Pirate Ship at Skye: built from typed sources */
};
void ResetSpiritsFound(void) {

    s32 i;

    for(i=0; i<16; i++){
        gSpiritsFound[i] = 0;
    }
}
void SetupMapSpirits(void) {
    u32 i;
    SubmapSpirits* map;
    SpiritSpot* spot;
    Spirit* spirit;

    for (i = 0; i < 43; i++) {
        if (gCurrentMap == gSubmapSpiritTable[i].map && gCurrentSubmap == gSubmapSpiritTable[i].submap) {
            break;
        }
    }
    if (i < 43) {
        map = &gSubmapSpiritTable[i];
        spot = map->spots;
        spirit = gSubmapSpirits;
        gSpiritCount = map->count;
        i = gSpiritCount;            /* RULE-EXCEPTION R30: i turns from the search index into the countdown; a separate counter loses the match */
        while (i != 0) {
            spirit->animTimer = -1;
            spirit->pickedUp = IsSpiritFound(spot->flag);
            spirit->x = spot->x;
            spirit->z = -spot->z;
            PollWorldCollision(spirit->x, 0.0f, spirit->z, 0xA00, &gSpiritFloorPoll);
            spirit->y = gSpiritFloorPoll.y;
            spirit->randomOffset = 0.0f;
            i--;
            spirit->flag = spot->flag;
            spot++;
            spirit++;
        }
    } else {
        gSpiritCount = 0;
    }
}

Spirit* CheckSpiritPickup(ActorTransform* brian) {
    
    Spirit* spirit;
    f32 dx;
    f32 dz;
    f32 dist;
    Spirit* found;
    s32 n;
    Coordinates2D coordinates;
    

    n = gSpiritCount;
    found = NULL;
    spirit = gSubmapSpirits;
    while (n != 0) {
        if ((spirit->pickedUp == 0)){
            dx = spirit->x - brian->pos.x;
            dz = spirit->z - brian->pos.z;
            dist = (dx * dx) + (dz * dz);
            if (dist < 400.0f) {
                coordinates.x = dx;
                coordinates.y = dz;
                rotateCoordinatesByAngle(brian->yrot, &coordinates);
                if((coordinates.y  >= 0.0f) 
                    && ((coordinates.x > -5.0)) && (coordinates.x < 5.0)
                ) {
                    found = spirit;
                    break;
                } 
            }
        }
        n--;
        spirit++;

    }
    return found;
}
/* ---- the sparkle (UpdateSpiritSparkles' template draws it): a unit quad, the list that draws it, and four 16x16 I8
   frames (the template's texture format 2 at 16 by 16; LoadDustTexture loads one as I 8-bit). A spirit's sparkle shows
   frame animTimer >> 1 for animTimer 1, 3, 5 and 7, so all four frames are read. The frames
   themselves are included from the ROM at build time (GLOBAL_ASM .incbin), like the other textures. ---- */
Vtx gSparkleVertices[4] = {
    { { { 1, 1, 1 }, 0, { 0x3C0, 0x0 }, { 0, 0, 0, 0 } } },
    { { { -1, 1, 1 }, 0, { 0x0, 0x0 }, { 0, 0, 0, 0 } } },
    { { { 1, -1, 1 }, 0, { 0x3C0, 0x3C0 }, { 0, 0, 0, 0 } } },
    { { { -1, -1, 1 }, 0, { 0x0, 0x3C0 }, { 0, 0, 0, 0 } } },
};
Gfx gSparkleDisplayList[] = {
    gsDPPipeSync(),
    gsDPSetCycleType(G_CYC_2CYCLE),
    gsDPSetRenderMode(G_RM_PASS, G_RM_ZB_XLU_SURF2),
    gsSPClearGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_SHADING_SMOOTH),
    gsSPSetGeometryMode(G_ZBUFFER | G_SHADE),
    gsDPSetCombineLERP(1, TEXEL0, ENVIRONMENT, TEXEL0, TEXEL0, 0, ENVIRONMENT, 0, 0, 0, 0, COMBINED, 0, 0, 0, COMBINED),
    gsSPTexture(0x8000, 0x8000, 0, G_TX_RENDERTILE, G_ON),
    gsSPVertex(gSparkleVertices, 4, 0),
    gsSP2Triangles(0, 1, 2, 0, 1, 3, 2, 0),
    gsSPEndDisplayList(),
};
GLOBAL_ASM(
.section .data
glabel gSparkleFrames
.incbin "baserom.jp.z64", 0x4EE58, 0x400
)
/* then UpdateSpiritSparkles's template (a function-local static) and the env colours after it */
void UpdateSpiritSparkles(void) {
    static Dust sFx = {                 /* the sparkle template (DrawSpiritSparkle draws it) */
        0x1E, 0, 0, DrawSpiritSparkle, 2, 0x10, 0x10, 0, 0, { 0 }, gSparkleFrames[0], gSparkleDisplayList,
        0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.5f, 0.0f, 0.3f, 0.0f, 0, { 0 },
    };
    Dust* fx = &sFx;
    s32 i;
    Spirit* spirit;
    Coordinates2D offset;

    i = gSpiritCount;
    spirit = gSubmapSpirits;
    while (i != 0) {
        if (spirit->pickedUp == 0) {
            spirit->animTimer++;
            if (spirit->animTimer >= 0 && (spirit->animTimer & 1)) {
                offset.x = 0.0f;
                offset.y = 1.0f;
                rotateCoordinatesByAngle(spirit->randomOffset, &offset);
                fx->x = spirit->x + offset.x;
                fx->y = spirit->y;
                fx->z = spirit->z + offset.y;
                fx->dustTexture = gSparkleFrames[spirit->animTimer >> 1];
                SpawnDust(fx);
                if (spirit->animTimer >= 6) {
                    spirit->animTimer = getRandomNumber(30) - 60;
                    spirit->randomOffset += 0.39269908169872414;
                }
            }
        }
        i--;
        spirit++;
    }
}
u8 gSparkleEnvColors[2][4] = { { 0x8C, 0x00, 0xFF, 0xDC }, { 0x8C, 0x00, 0xFF, 0x78 } };   /* two RGBA env colours */

void DrawSpiritSparkle(SpiritSparkle* sparkle) {
    f32 wobble;
    u8* colour;

    wobble = sinf((f32)sparkle->phase * 0.09817477042468103) * 0.1;
    sparkle->phase += 4;
    colour = gSparkleEnvColors[sparkle->flags & 1];
    sparkle->x += sparkle->velX + wobble;
    sparkle->y += sparkle->velY;
    sparkle->z += sparkle->velZ + wobble;
    gDPSetEnvColor(gDisplayListHead++, colour[0], colour[1], colour[2], colour[3]);
}
u8 IsSpiritFound(u16 flag) {
    return gSpiritsFound[flag >> 3] & gBitMasks[flag & 7];
}
void SetSpiritFound(u16 flag) {
    gSpiritsFound[flag >> 3] |= gBitMasks[flag & 7];
}
