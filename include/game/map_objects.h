#ifndef GAME_MAP_OBJECTS_H
#define GAME_MAP_OBJECTS_H

/* The functions of src/game/map_objects.c, declared once for every file that calls them. */

#include "common.h"

void ResetMapObjectFlags(void);
void SpawnMapObjects(void);
void DrawMapObjects(Mtx* mtxList);
MapObject* FindMapObjectInFront(ActorTransform* brian);
void OpenMapObject(ActorTransform* obj);
void BuildPickupMessage(s32 itemID);
u8 IsMapObjectOpened(u16 id);
void SetMapObjectOpened(u16 id);

/* The state src/game/map_objects.c defines (.bss), in its storage order: the order of these declarations is the
 * unit's .bss layout (include/bss_order.h). */
extern MapObject gMapObjects[16];   /* 16 fill the space to gMapObjectCount (no code bound states it) */
extern s32 gMapObjectCount;   /* objects in gMapObjects */
extern Coordinates2D gMapObjectOffset;
extern s32 D_80085960;   /* unreferenced: the offset's third word (offsets of this kind -
                                            gSpellOffset, gEnemyStep, gHudVector - are OddVectors whose third
                                            word is a scale; no code reads this one) */
extern CollisionPollingResult gMapObjectPoll;
extern u8 gObjectsOpened[32];   /* a bit per map object id (ResetMapObjectFlags clears 32 bytes) */

/* The data src/game/map_objects.c defines. */
extern u8 gPickupMessageHead[8];
extern u8 gPickupMessageTail[20];
extern MapObjTable gMapObjectTable[19];

#endif
