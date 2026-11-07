#include "common.h"
/* split from map_objects.c: the original was separate files (each IDO object pads its .rodata to 16;
   the zero words before this file's first literal mark the boundary). Declarations copied from map_objects.c. */
/* ---- spirits.c's .bss (0x800859B0-0x80085AB0), in address order. All four are first declared
   inside the file's first 32 declarations (headers counted; IDO 5.3 lays out the .bss variables first declared
   there after all later ones) and the file has no other .bss, so they keep this order. Declaring another .bss
   variable in this file, or adding declarations above these, can move them. ---- */
s32 gSpiritCount;                         /* spirits on this submap */
Spirit gSubmapSpirits[8];                   /* this submap's spirits (SetupMapSpirits) */
CollisionPollingResult gSpiritFloorPoll;      /* the floor poll for a spirit's height */
u8 gSpiritsFound[0x10];                    /* spirits found: one bit per spirit flag (IsSpiritFound / SetSpiritFound) */
void rotateCoordinatesByAngle(f32 angle, Coordinates2D* coordinates);
void PlaySound(u8 delay, s8 id, u8 level);
extern u8 gBitMasks[];
extern u8 gObjectsOpened[];
extern SubmapSpirits gSubmapSpiritTable[43];
extern s32 gCurrentMap;   /* current map */
extern s32 gCurrentSubmap;   /* current submap */
void PollWorldCollision(f32 sample_x, f32 sample_y, f32 sample_z, u16 arg3_flags, CollisionPollingResult* result);
u8 IsSpiritFound(u16 flag);
void SetAnimation(ActorTransform *brian, s32 nextAnim, s32 nextFramerate, s32 afterAnim, s32 afterFramerate, u16 flags);
extern MapObjTable gMapObjectTable[];
extern s32 gMapObjectCount;
extern MapObject gMapObjects[];
extern CollisionPollingResult gMapObjectPoll;
u8 IsMapObjectOpened(u16 id);
extern u8* gItemNamePtrs[];
extern u8 gPickupMessageHead[];
extern u8 gPickupMessageTail[];
extern u8 gTextBuffer[];
extern u8 gSpeakerName[];
extern u8 D_D37700[];
extern PlayerState gPlayerState;
void dma_write(u8* devAddr, void* vaddr, u32 nbytes);
extern AnimEntry gMapObjectAnims[];
void DrawModel(s32 flags, ActorTransform* actor, AnimEntry* anim, s32 mtxList);
void AdvanceAnimation(s32 framerate, ActorTransform* model, AnimEntry* anim);
void AddShadow(f32 x, f32 y, f32 z, f32 xrot, f32 yrot, f32 zrot, f32 size);
extern Coordinates2D gMapObjectOffset;
void SetMapObjectOpened(u16 id);
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
extern Gfx* gDisplayListHead;
f32 sinf(f32 angle);       /* sinf */
extern u8 gSparkleFrames[][0x100];       /* the sparkle frames, 0x100 bytes each */
extern u8 gSparkleDisplayList[];              /* the sparkle display list */
Dust* SpawnDust(Dust* dust);     /* spawn a dust puff from a template (dust.c); returns the new puff */
u32 getRandomNumber(u32 range);
void DrawSpiritSparkle(SpiritSparkle* sparkle);
/* ---- spirits.c's .data (0x8004E650-0x8004E6A0): the unreferenced words, then
   UpdateSpiritSparkles's template (a function-local static) and the env colours after it. ---- */
s32 D_8004E650[2] = { 0, 0 };        /* unreferenced */
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
extern SignObject gSigns[38];
extern s32 gRoomSignCount;              /* entries in gRoomSigns */
extern SignSlot gRoomSigns[];
extern Coordinates2D gSignOffset;
f32 sqrtf(f32 x);           /* sqrtf */
f32 Atan2(f32 dx, f32 dz);
u8 IsSpiritFound(u16 flag) {
    return gSpiritsFound[flag >> 3] & gBitMasks[flag & 7];
}
void SetSpiritFound(u16 flag) {
    gSpiritsFound[flag >> 3] |= gBitMasks[flag & 7];
}
