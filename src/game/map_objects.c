#include "common.h"
BSS_ORDER_PAD(0)
#include "game/map_objects.h"
#include "game/collision.h"
#include "game/math.h"
#include "game/model.h"
#include "game/shadow.h"
#include "game/sound.h"
#include "game/enemy.h"
#include "game/map.h"
#include "maps.h"
/* ---- map_objects.c's .bss (0x80085290-0x800859B0): the map objects. Defined in their storage order, the order its header declares them in: IDO 5.3 lays the unit's
   .bss out by those declarations (include/bss_order.h), and it 8-aligns every variable of 8 bytes or more. The D_ name
   marked unreferenced is space no code addresses (an unknown variable). ---- */
MapObject gMapObjects[16];               /* 16 fill the space to gMapObjectCount (no code bound states it) */
s32 gMapObjectCount;                     /* objects in gMapObjects */
Coordinates2D gMapObjectOffset;
s32 D_80085960;                          /* unreferenced: the offset's third word (offsets of this kind -
                                            gSpellOffset, gEnemyStep, gHudVector - are OddVectors whose third
                                            word is a scale; no code reads this one) */
CollisionPollingResult gMapObjectPoll;
u8 gObjectsOpened[32];                   /* a bit per map object id (ResetMapObjectFlags clears 32 bytes) */

/* ---- map_objects.c's .data (0x8004DF00-0x8004DFC0), in address order. The pickup message's two
   halves, in the dialogue font's encoding: BuildPickupMessage copies each up to its 0xFF (the byte after it is never
   read). Then the maps that place objects: SpawnMapObjects finds gCurrentMap and takes its submap's list from the
   map's own table, which lies in the map as loaded. Every map is built from typed sources, so each row names its
   map's table. ---- */
u8 gPickupMessageHead[8] = { 0xA0, 0xC0, 0x7F, 0xE0, 0x80, 0x0E, 0xFF, 0x00 };
u8 gPickupMessageTail[20] = { 0xA0, 0xC0, 0x80, 0x0F, 0x84, 0x0E, 0x8F, 0x0C, 0x83, 0x15, 0x95, 0x19, 0x84, 0x0B, 0x83, 0x0F, 0x80, 0x0A, 0xFF, 0x00 };
MapObjTable gMapObjectTable[19] = {
    { 13, gMap13MapObjects },   /* Melrode Monastery: built from typed sources */
    { 14, gMap14MapObjects },   /* Dondoran Castle: built from typed sources */
    { 15, gMap15MapObjects },   /* Melrode buildings: built from typed sources */
    { 17, gMap17MapObjects },   /* Larapool inn: built from typed sources */
    { 19, gMap19MapObjects },   /* rooms off Normoon: built from typed sources */
    { 21, gMap21MapObjects },   /* Limelin Castle: built from typed sources */
    { 22, gMap22MapObjects },   /* Limelin buildings: built from typed sources */
    { 23, gMap23MapObjects },   /* Highland buildings: built from typed sources */
    { 24, gMap24MapObjects },   /* Shamwood: built from typed sources */
    { 25, gMap25MapObjects },   /* ship and cabin rooms: built from typed sources */
    { 26, gMap26MapObjects },   /* Blue Cave: built from typed sources */
    { 27, gMap27MapObjects },   /* Cull Hazard: built from typed sources */
    { 28, gMap28MapObjects },   /* Baragoon Tunnel: built from typed sources */
    { 29, gMap29MapObjects },   /* Boil Hole: built from typed sources */
    { 30, gMap30MapObjects },   /* Brannoch Castle: built from typed sources */
    { 31, gMap31MapObjects },   /* Connor Forest: built from typed sources */
    { 32, gMap32MapObjects },   /* Glencoe Forest: built from typed sources */
    { 33, gMap33MapObjects },   /* Windward Forest: built from typed sources */
    { 9, gMap09MapObjects },   /* Dindom Dries: built from typed sources */
};

void ResetMapObjectFlags(void) {

    s32 i;

    for(i=0; i<32; i++){
        gObjectsOpened[i] = 0;
    }
}
void SpawnMapObjects(void) {
    MapObjTable* entry;
    MapObjList* list;
    MapObjRec* rec;
    MapObject* obj;
    u32 i;
    s32 anim;

    for (i = 0; i < 19; i++) {
        if (gCurrentMap == gMapObjectTable[i].map) {
            break;
        }
    }
    if (i != 19) {
        entry = &gMapObjectTable[i];
        list = &entry->submaps[gCurrentSubmap];
        gMapObjectCount = list->count;
        rec = list->recs;
        obj = gMapObjects;
        /* RULE-EXCEPTION R30: i turns from the search index into the countdown; a separate counter loses the match */
        for (i = gMapObjectCount; i != 0; i--, obj++, rec++) {
            obj->transform.pos.x = rec->x;
            obj->transform.pos.z = rec->z;
            obj->transform.xrot = 0.0f;
            obj->transform.zrot = 0.0f;
            obj->transform.yrot = rec->yrot;
            PollWorldCollision(obj->transform.pos.x, 0.0f, obj->transform.pos.z, 0xA00, &gMapObjectPoll);
            obj->transform.pos.y = gMapObjectPoll.y;
            obj->transform.shadowXRot = gMapObjectPoll.sn_x;
            obj->transform.shadowYRot = gMapObjectPoll.sn_y;
            obj->transform.shadowZRot = gMapObjectPoll.sn_z;
            obj->transform.yspeed = rec->useYaw;
            obj->transform.xspeed = rec->useHalfWidth;
            obj->transform.zspeed = rec->useHalfDepth;
            obj->transform.rootMoveX = rec->useX;
            obj->transform.rootMoveZ = rec->useZ;
            obj->transform.scale = 0.07f;
            obj->transform.floorFlags = rec->openedFlagId;
            obj->transform.modelID = rec->item;
            obj->transform.stateFlags = 0;
            anim = IsMapObjectOpened(obj->transform.floorFlags) ? 3 : 0;
            SetAnimation(&obj->transform, anim, 1, anim, 1, 1);
        }
    } else {
        gMapObjectCount = 0;
    }
}

void DrawMapObjects(Mtx* mtxList) {
    s32 i;
    MapObject* obj;
    u16 timer;
    u16 t;

    i = gMapObjectCount;
    obj = gMapObjects;
    while (i != 0) {
        timer = obj->transform.stateFlags;
        if (obj->transform.stateFlags & MAPOBJ_OPENING) {
            t = timer & ~MAPOBJ_OPENING;
            t--;
            if (t == 0) {
                obj->transform.stateFlags = 0;
                SetAnimation(&obj->transform, 3, 1, 3, 1, 1);
            } else {
                obj->transform.stateFlags = t | MAPOBJ_OPENING;
            }
        }
        DrawModel(0, &obj->transform, &gMapObjectAnims[obj->transform.animID], mtxList);
        AdvanceAnimation(obj->transform.animFramerate, &obj->transform, &gMapObjectAnims[obj->transform.animID]);
        AddShadow(obj->transform.pos.x, obj->transform.pos.y, obj->transform.pos.z, obj->transform.shadowXRot,
                      obj->transform.shadowYRot, obj->transform.shadowZRot, 6.0f);
        i--;
        obj++;
    }
}
MapObject* FindMapObjectInFront(ActorTransform* brian) {
    MapObject* obj;
    s32 n;
    f32 d;
    f32 dx;
    f32 dz;

    n = gMapObjectCount;
    obj = gMapObjects;
    if (n != 0) {
        do {
            if (IsMapObjectOpened(obj->transform.floorFlags) == 0) {
                d = brian->yrot - obj->transform.yspeed;
                if (d > 3.141592653589793) {
                    d -= 3.141592653589793;
                } else if (d < -3.141592653589793) {
                    d += 3.141592653589793;
                }
                if (d < 0.7853981633974483 && d > -0.7853981633974483) {
                    dx = brian->pos.x - obj->transform.rootMoveX;
                    dz = brian->pos.z - obj->transform.rootMoveZ;
                    if ((dx * dx) + (dz * dz) < 300.0f) {
                        gMapObjectOffset.x = dx;
                        gMapObjectOffset.y = dz;
                        rotateCoordinatesByAngle(obj->transform.yspeed, &gMapObjectOffset);
                        dx = gMapObjectOffset.x;
                        dz = gMapObjectOffset.y;
                        if (dx < 0.0f) {
                            dx = -dx;
                        }
                        if (dz < 0.0f) {
                            dz = -dz;
                        }
                        if (dx < obj->transform.xspeed && dz < obj->transform.zspeed) {
                            break;
                        }
                    }
                }
            }
            n--;
            obj++;
        } while (n != 0);
    }
    if (n == 0) {
        obj = NULL;
    }
    return obj;
}
void OpenMapObject(ActorTransform* obj) {
    SetMapObjectOpened(obj->floorFlags);
    obj->stateFlags = MAPOBJ_OPENING | 91;
    SetAnimation(obj, 1, 1, 2, 1, 1);
    PlaySound(0x1A, 8, 0xFF);
}
void BuildPickupMessage(s32 itemID) {
    u8* src;
    u8* dst;

    src = gPickupMessageHead;
    dst = gTextBuffer;
    while (*src != 0xFF) {
        *dst++ = *src++;
    }
    src = gItemNamePtrs[itemID];
    while (*src != 0xFF) {
        *dst++ = *src++;
    }
    src = gPickupMessageTail;
    while (*src != 0xFF) {
        *dst++ = *src++;
    }
    *dst = *src;
    gSpeakerName[0] = 0xFF;
}
u8 IsMapObjectOpened(u16 id) {
    return gObjectsOpened[id >> 3] & gBitMasks[id & 7];
}
void SetMapObjectOpened(u16 id) {
    gObjectsOpened[id >> 3] |= gBitMasks[id & 7];
}
