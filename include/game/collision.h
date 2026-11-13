#ifndef GAME_COLLISION_H
#define GAME_COLLISION_H

/* The functions of src/game/collision.c, declared once for every file that calls them. */

#include "common.h"

void PollWorldCollision(f32 sample_x, f32 sample_y, f32 sample_z, u16 flags, CollisionPollingResult* result);   /* floor poll (collision.c) */
s32 PollModelInstance(f32 sample_x, f32 sample_z, u16 face_mask, u16 model_index, CollisionPollingResult* result);
s32 PollModelFaces(f32 x, f32 z, f32 scale, u16 face_mask, ModelGeometry* geometry, CollisionPollingResult* result);
s32 EdgeCrossesRight(f32 x, f32 z, f32 ax, f32 az, f32 bx, f32 bz);
s32 PushOutOfWallSegment(f32* x, f32* z, f32 radius, f32 ax, f32 az, f32 bx, f32 bz);
s32 PushOutOfWalls(f32 actor_x, f32 actor_z, NavMeshData* navmesh, CollisionPollingResult* result);
s32 CheckDoorTriggers(f32 x, f32 z, f32 yaw);   /* walked into a door (collision.c) */
s32 IsAngleWithin22(f32 angle);
s32 CheckDoorKeyItem(MapDoor* door);
void SetDoorWarp(MapDoor* door);

#endif
