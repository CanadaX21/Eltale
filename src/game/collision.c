#include "common.h"
extern u16 gLightRecordFlags;              /* bit 0: the sun follows the time of day; bit 1: the day cycle runs */
extern s32 gLightTime;   /* time of day for the lights */
extern s32 gFramesPerDay;              /* frames per day */
extern MapLight* gSubmapLight;        /* the map's light data */
extern MapModelData* gMapModelData;    /* (Quest D_80084F20) */
extern NavMeshData* gWallNavmeshActors;              /* wall navmesh, actors (Quest D_80084F2C) */
extern NavMeshData* gWallNavmeshCamera;              /* wall navmesh, camera (Quest D_80084F30) */
extern s32 gCurrentSubmap;              /* current submap (Quest gNextSubmap) */
s32 PollModelInstance(f32 sample_x, f32 sample_z, u16 face_mask, u16 model_index, CollisionPollingResult* result);
s32 PushOutOfWalls(f32 actor_x, f32 actor_z, NavMeshData* navmesh, CollisionPollingResult* result);
extern ModelTransform* gMapModels;  /* (Quest D_80084F24) */
extern ModelGeometry* gMapGeometry;              /* geometry table, 0x20 per entry (Quest D_80084F28) */
extern f64 D_8006F0A0;              /* pi / 180 */
s32 PollModelFaces(f32 x, f32 z, f32 scale, u16 face_mask, ModelGeometry* geometry, CollisionPollingResult* result);
void rotateCoordinatesByAngle(f32 angle, Coordinates2D* coordinates);
extern f32 gCollisionRadius; /* collision radius (Quest D_80085340) */
s32 PushOutOfWallSegment(f32* x, f32* z, f32 radius, f32 ax, f32 az, f32 bx, f32 bz);
extern LightAnim gLightAnims[];
extern Submap* gSubmapHeader;
extern u16 gBattleState;
extern Coordinates2D gDoorOffset;
s32 CheckDoorKeyItem(MapDoor* door);
s32 IsAngleWithin22(f32);
void SetDoorWarp(MapDoor* door);
extern s32 gTimeOfDay;
extern s32 gDayLength;
extern u16 gDayCount;

void PollWorldCollision(f32 sample_x, f32 sample_y, f32 sample_z, u16 flags, CollisionPollingResult* result) {
    s32 tile_x;
    s32 tile_z;
    s32 tile_index;
    s32 iter_model_count;
    u16* area_wall_defs;
    u16* iter_model_index_ptr;
    MapModelData* map_model_data;

    gCollisionRadius = result->x;

    result->x = sample_x;
    result->y = sample_y;
    result->z = sample_z;
    result->flags = 0x8000;
    result->sn_x = 0.0f;
    result->sn_y = 1.0f;
    result->sn_z = 0.0f;

    if (flags & 1) {
        if ((gWallNavmeshActors != NULL) &&
            (PushOutOfWalls(sample_x, sample_z, &gWallNavmeshActors[gCurrentSubmap], result) != 0)) {
            sample_x = result->x;
            sample_z = result->z;
        }
    } else if ((flags & 2) && (gWallNavmeshCamera != NULL) &&
               (PushOutOfWalls(sample_x, sample_z, &gWallNavmeshCamera[gCurrentSubmap], result) != 0)) {
        sample_x = result->x;
        sample_z = result->z;
    }

    map_model_data = gMapModelData;

    tile_x = 0;
    tile_z = 0;

    if (map_model_data->tile_flags == 0) {
        area_wall_defs = map_model_data->wall_defs;
    } else {
        tile_x = (sample_x - map_model_data->min_x) / map_model_data->tile_w;
        tile_z = (sample_z - map_model_data->min_z) / map_model_data->tile_d;

        tile_index = (map_model_data->tile_columns * tile_z) + tile_x;
        area_wall_defs = map_model_data->tile_offsets[tile_index] + map_model_data->wall_defs;
    }

    if ((tile_x >= 0) && (tile_x < map_model_data->tile_columns) && (tile_z >= 0) &&
        (tile_z < map_model_data->tile_rows)) {
        iter_model_count = *area_wall_defs++;

        while (iter_model_count != 0) {
            if (PollModelInstance(sample_x, sample_z, flags, *area_wall_defs++, result) != 0) {
                return;
            }
            iter_model_count--;
        }
    }

    map_model_data = gMapModelData;

    iter_model_count = map_model_data->model_count;
    iter_model_index_ptr = map_model_data->non_tiled;

    while (iter_model_count != 0) {
        if (PollModelInstance(sample_x, sample_z, flags, *iter_model_index_ptr++, result) != 0) {
            return;
        }
        iter_model_count--;
    }
}
s32 PollModelInstance(f32 sample_x, f32 sample_z, u16 face_mask, u16 model_index, CollisionPollingResult* result) {
    s32 hit_terrain;
#ifdef MATCHING_HACK_IDO
    s16 pad;                         /* unused: the -g3 stack word above geometry */
#endif
    ModelGeometry* geometry;
    ModelTransform* transform;
    f32 local_angle;
    Coordinates2D local_coord;

    transform = &gMapModels[model_index];

    sample_x -= transform->x;
    sample_z -= transform->z;

    local_angle = 0.017453292519943295 * transform->angle;
    hit_terrain = 0;

    geometry = &gMapGeometry[transform->index];

    if (local_angle != 0.0f) {
        local_coord.x = sample_x;
        local_coord.y = sample_z;
        rotateCoordinatesByAngle(local_angle, &local_coord);
        sample_x = -local_coord.x;
        sample_z = local_coord.y;
    }

    if ((transform->flags & 0xFF) < 0x10) {
        hit_terrain = PollModelFaces(sample_x, sample_z, transform->model_scale, face_mask, geometry, result);
        if (hit_terrain) {
            result->y += transform->y;
            if (local_angle != 0.0f) {
                local_coord.x = result->sn_x;
                local_coord.y = result->sn_z;
                rotateCoordinatesByAngle(-local_angle, &local_coord);
                result->sn_x = -local_coord.x;
                result->sn_z = local_coord.y;
            }
        }
    }
    return hit_terrain;
}
s32 EdgeCrossesRight(f32 arg0, f32 arg1, f32 arg2, f32 arg3, f32 arg4, f32 arg5);
s32 PollModelFaces(f32 x, f32 z, f32 scale, u16 face_mask, ModelGeometry* geometry, CollisionPollingResult* result) {
    GeometryVertex* vertices;
    s32 ret;
    f32 cx;
    f32 cz;
    f32 bx;
    f32 bz;
    f32 ax;
    f32 az;
    f32 ay;
    s32 ia;
    s32 ib;
    s32 ic;
    f32 nx;
    f32 ny;
    f32 nz;
    s32 expected;
    s32 crossings;
    s32 remaining;
    GeometryFace* face;
    GeometryVertex* va;
    GeometryVertex* vb;
    GeometryVertex* vc;

    expected = 1;
    ret = 0;
    face = geometry->faces;
    vertices = geometry->vertices;
    if (face != NULL) {
        ax = scale * geometry->x_min;
        bx = scale * geometry->x_max;
        az = scale * geometry->z_min;
        bz = scale * geometry->z_max;
        if (x < ax || bx < x || z < az || bz < z) {
            return 0;
        }
        for (remaining = geometry->face_count; remaining != 0; remaining--, face++) {
            if (face->flags & face_mask) {
                ia = face->a;
                ib = face->b;
                ic = face->c;
                va = &vertices[0] + ia;
                vb = &vertices[0] + ib;
                vc = &vertices[0] + ic;
                ax = scale * va->x;
                az = scale * va->z;
                bx = scale * vb->x;
                bz = scale * vb->z;
                cx = scale * vc->x;
                cz = scale * vc->z;
                if ((x == ax) && (z == az)) {
                    crossings = 1;
                } else if (1) {                 /* always passes: only a block in the original, whose test is lost */
                    if ((x == bx) && (z == bz)) {
                        crossings = 1;
                    } else if ((x == cx) && (z == cz)) {
                        crossings = 1;
                    } else {
                        crossings = 0;
                        crossings += EdgeCrossesRight(x, z, ax, az, bx, bz);
                        crossings += EdgeCrossesRight(x, z, bx, bz, cx, cz);
                        crossings += EdgeCrossesRight(x, z, ax, az, cx, cz);
                    }
                }
                if (crossings == expected) {
                    ny = face->ny;
                    nx = face->nx;
                    nz = face->nz;
                    ay = scale * va->y;
                    if (ny != 0.0f) {
                        result->y = -(nx * x + nz * z - (nx * ax + ny * ay + nz * az)) / ny;
                        result->flags = face->flags;
                    } else {
                        result->flags = face->flags;
                    }
                    result->sn_x = nx;
                    result->sn_y = ny;
                    result->sn_z = nz;
                    ret = 1;
                    break;
                }
            }
        }
    }
    return ret;
}

s32 EdgeCrossesRight(f32 x, f32 z, f32 ax, f32 az, f32 bx, f32 bz) {
    if (((az <= z) && (z < bz)) || ((bz <= z) && (z < az))) {
        if ((ax <= x) && (bx < x)) {
            return 1;
        }
        if ((x <= ax) && (x < bx)) {
            return 0;
        }
        if ((az != bz) && ((((z - az) * ((ax - bx) / (az - bz))) + ax) < x)) {
            return 1;
        }
    }
    return 0;
}
f32 sqrtf(f32 x);           /* sqrtf */
/* Push a circle (centre *x, *z) out of a wall segment (ax, az)-(bx, bz) (Quest func_8000F6A4): 0 if it is
   clear, else move it off the line, or off an end when the closest point lies past it; returns 1 if it moved. */
s32 PushOutOfWallSegment(f32* x, f32* z, f32 radius, f32 ax, f32 az, f32 bx, f32 bz) {
    s32 ret;
    f32 a;
    f32 b;
    f32 c;
    f32 side;               /* also the wall's max z (RULE-EXCEPTION R30: a separate local grows the frame) */
    f32 ap;
    f32 bp;
    f32 cp;
    f32 dist;
    f32 scale;
    f32 cx;
    f32 cz;
    f32 dx;
    f32 dz;

    ret = 0;
    a = (ax <= bx) ? ax : bx;
    b = (bx < ax) ? ax : bx;
    c = (az <= bz) ? az : bz;
    side = (bz < az) ? az : bz;
    if (*x < a - radius || b + radius < *x || *z < c - radius || side + radius < *z) {
        return 0;
    }
    if (ax == bx) {
        a = 1.0f;
        b = 0.0f;
        c = -ax;
    } else if (az == bz) {
        a = 0.0f;
        b = 1.0f;
        c = -az;
    } else {
        b = 1.0f;
        a = -(bz - az) / (bx - ax);
        c = -a * ax - az;
    }
    side = a * *x + b * *z + c;
    if (side < 0.0f) {
        side = -side;
    }
    dist = side / sqrtf(a * a + b * b);
    if (dist < radius) {
        if (a == 0.0f) {
            ap = 1.0f;
            bp = 0.0f;
            cp = -*x;
        } else if (b == 0.0f) {
            ap = 0.0f;
            bp = 1.0f;
            cp = -*z;
        } else {
            ap = -1.0f / a;
            bp = 1.0f;
            cp = -ap * *x - *z;
        }
        cx = (cp * b - c * bp) / (a * bp - ap * b);
        cz = (ap * c - a * cp) / (a * bp - ap * b);
        a = (ax <= bx) ? ax : bx;
        b = (bx < ax) ? ax : bx;
        c = (az <= bz) ? az : bz;
        side = (bz < az) ? az : bz;
        if (cx < a || b < cx || cz < c || side < cz) {
            a = ax - *x;
            b = az - *z;
            c = sqrtf(a * a + b * b);
            if (c < radius) {
                dx = ax - *x;
                dz = az - *z;
                scale = ((radius + 0.1) - c) / sqrtf(dx * dx + dz * dz);
                *x -= dx * scale;
                *z -= dz * scale;
                return 1;
            }
            a = bx - *x;
            b = bz - *z;
            c = sqrtf(a * a + b * b);
            if (c < radius) {
                dx = ax - *x;   /* BUG: pushes away from the first end (ax, az), not the one it is near (bx, bz) */
                dz = az - *z;
                scale = ((radius + 0.1) - c) / sqrtf(dx * dx + dz * dz);
                *x -= dx * scale;
                *z -= dz * scale;
                return 1;
            }
            return 0;
        }
        dx = cx - *x;
        dz = cz - *z;
        if (dist != 0.0f) {
            scale = ((radius + 0.1) - dist) / sqrtf(dx * dx + dz * dz);
            *x -= dx * scale;
            *z -= dz * scale;
        }
        ret = 1;
    }
    return ret;
}
s32 PushOutOfWalls(f32 actor_x, f32 actor_z, NavMeshData* navmesh, CollisionPollingResult* result) {
    f32 ax;
    f32 bx;
    f32 az;
    f32 bz;
    s32 collided_with_wall;
    VertexData* vert_addr_root;
    s32 group_count;
    s32 wall_count;
    VertexData* vertex;
    s32* wall_addr;
    VertexGroup* current_wall_def;

    group_count = navmesh->group_count;
    current_wall_def = navmesh->groups;
    collided_with_wall = 0;

    while (group_count != 0) {
        wall_addr = (navmesh->indices + current_wall_def->index);
        wall_count = current_wall_def->count;

        while (wall_count != 0) {
            vert_addr_root = navmesh->vertices;

            vertex = &vert_addr_root[wall_addr[0]];
            ax = vertex->x;
            az = vertex->z;

            vertex = &vert_addr_root[wall_addr[1]];
            bx = vertex->x;
            bz = vertex->z;

            wall_addr++;
            collided_with_wall |= PushOutOfWallSegment(&actor_x, &actor_z, gCollisionRadius, ax, az, bx, bz);

            wall_count--;
        }
        group_count--;
        current_wall_def++;
    }
    result->x = actor_x;
    result->z = actor_z;

    return collided_with_wall;
}
s32 CheckDoorTriggers(f32 x, f32 z, f32 yaw) {
    f32 dx;
    s32 ret;
    Submap* submap;
    f32 dz;
    s32 n;
    MapDoor* door;

    submap = gSubmapHeader;
    ret = 0;
    if (submap->doorCount == 0 || (gBattleState & 1)) {
        return 0;
    }
    door = submap->doors;
    for (n = submap->doorCount; n != 0; n--, door++) {
        if (CheckDoorKeyItem(door)) {
            gDoorOffset.x = x - door->x;
            gDoorOffset.y = z - door->z;
            rotateCoordinatesByAngle(door->angle, &gDoorOffset);
            dx = gDoorOffset.x;
            if (dx < 0.0f) {
                dx = -dx;
            }
            dz = gDoorOffset.y;
            if (dz < 0.0f) {
                dz = -dz;
            }
            if (dx < door->halfWidth && dz < door->halfDepth) {
                /* inside the door's rectangle: warp if the door needs no facing or Brian faces it; either way
                   no other door is checked */
                if ((door->flags & 2) || IsAngleWithin22(yaw - door->angle)) {
                    SetDoorWarp(door);
                    ret = 1;
                }
                break;
            }
        }
    }
    return ret;
}
s32 IsAngleWithin22(f32 angle) {
    if (3.141592653589793 < angle) {
        angle = angle - 3.141592653589793 * 2;
    } else if (angle < -3.141592653589793) {
        angle = angle + 3.141592653589793 * 2;
    }
    if (angle < 0.0f) {
        angle = -angle;
    }
    return angle < 0.39269908169872414;
}
extern u8 gInventory[150];          /* the inventory: item ids, 0xFF = empty */
s32 CheckDoorKeyItem(MapDoor* door) {
    s32 i;
    s32 open;

    if (door->flags & 0x30) {
        for (i = 0; i < 150; i++) {
            if (door->keyItem == gInventory[i]) {
                break;
            }
        }
        if (door->flags & 0x10) {
            if (i < 150) {
                open = 1;
            } else {
                open = 0;
            }
        } else if (i < 150) {
            open = 0;
        } else {
            open = 1;
        }
    } else {
        open = 1;
    }
    return open;
}
extern PendingWarp gPendingWarp;     /* the pending warp: map, submap, entrance, ... */
extern u16* gWarpDoorDest;              /* the destination triple of the door last used */
void SetDoorWarp(MapDoor* door) {
    gWarpDoorDest = door->dest;
    gPendingWarp.map = gWarpDoorDest[0];
    gPendingWarp.submap = gWarpDoorDest[1];
    gPendingWarp.entrance = gWarpDoorDest[2];
    gPendingWarp.doorFlags = door->flags;
    gPendingWarp.door = door->model;
    gPendingWarp.yaw = door->angle;
}
