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

#endif
