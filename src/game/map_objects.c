#include "common.h"
extern s32 gSpiritCount;
extern Spirit gSubmapSpirits[];
void rotateCoordinatesByAngle(f32 angle, Coordinates2D* coordinates);
void PlaySound(u8 delay, s8 id, u8 level);
extern u8 gSpiritsFound[];
extern u8 gBitMasks[];
extern u8 gObjectsOpened[];
extern SubmapSpirits gSubmapSpiritTable[43];
extern s32 gCurrentMap;              /* current map */
extern s32 gCurrentSubmap;              /* current submap */
extern CollisionPollingResult gSpiritFloorPoll;
void PollWorldCollision(f32 sample_x, f32 sample_y, f32 sample_z, u16 arg3_flags, CollisionPollingResult* result);   /* floor poll (collision.c) */
u8 IsSpiritFound(u16 flag);
void SetAnimation(ActorTransform *brian, s32 nextAnim, s32 nextFramerate, s32 afterAnim, s32 afterFramerate, u16 flags);
extern MapObjTable gMapObjectTable[];
extern s32 gMapObjectCount;   /* objects in gMapObjects */
extern MapObject gMapObjects[];
extern CollisionPollingResult gMapObjectPoll;
u8 IsMapObjectOpened(u16 id);

extern u8* gItemNamePtrs[];
extern u8 gPickupMessageHead[];
extern u8 gPickupMessageTail[];
extern u8 gTextBuffer[];             /* 0x4B0-byte buffer (enemy.c and ending_text.c fill it too) */
extern u8 gSpeakerName[];             /* 0x20-byte buffer */
extern u8 D_D37700[];
extern PlayerState gPlayerState;
void dma_write(u8* devAddr, void* vaddr, u32 nbytes);

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
            obj->transform.yspeed = rec->unk14;
            obj->transform.xspeed = rec->unk18;
            obj->transform.zspeed = rec->unk1C;
            obj->transform.rootMoveX = rec->unkC;
            obj->transform.rootMoveZ = rec->unk10;
            obj->transform.scale = 0.07f;
            obj->transform.floorFlags = rec->flags;
            obj->transform.modelID = rec->unk21;
            obj->transform.stateFlags = 0;
            anim = IsMapObjectOpened(obj->transform.floorFlags) ? 3 : 0;
            SetAnimation(&obj->transform, anim, 1, anim, 1, 1);
        }
    } else {
        gMapObjectCount = 0;
    }
}

extern AnimEntry gMapObjectAnims[];        /* their animations */
void DrawModel(s32 flags, ActorTransform* actor, AnimEntry* anim, s32 mtxList);
void AdvanceAnimation(s32 framerate, ActorTransform* model, AnimEntry* anim);
void AddShadow(f32 x, f32 y, f32 z, f32 xrot, f32 yrot, f32 zrot, f32 size);   /* the shadow */
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
extern Coordinates2D gMapObjectOffset;
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
void SetMapObjectOpened(u16 id);
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
