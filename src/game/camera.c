#include "common.h"
/* camera state, 0x88 (Quest tempstruct1) */
extern Submap* gSubmapHeader;          /* the current submap */
/* ---- camera.c's .bss (0x80085B80-0x80085F90), in address order. IDO 5.3 lays out the
   variables first declared among a file's first 32 declarations (typedefs and variables, externs too) after all
   later ones; this block is all inside those 32 and the file declares no other .bss, so it keeps its order.
   Declaring any of these variables above it, or adding declarations above it (headers included: recount
   the declarations above this block after a header edit), moves them. The D_ names marked
   unreferenced are space no code addresses (unknown variables). ---- */
typedef struct {                        /* laid out like MonsterBattleData's first 0x6C bytes */
    Vec3f pos;
    char unkC[0x18];
    f32 scale;                          /* 0x24 */
    char unk28[0x3C];
    MonsterBaseData* unk64;
    ActorStats* unk68;
} CameraTarget;                         /* the spell camera's target (SnapSpellCamera copies an enemy into it) */
Camera gDefaultCamera;                 /* default camera (Quest D_80086B00) */
s32 gCameraSetupCount;                         /* camera setups in the map's table */
s32 D_80085C0C;                         /* unreferenced */
Coordinates2D gCameraOffset;
s32 D_80085C18[2];                      /* unreferenced */
CollisionPollingResult gCameraFloorPoll;      /* the camera's floor poll */
u16 gCameraFloorFlags;                         /* its flags */
CameraTarget gSpellCameraTarget;                /* spell camera target (Quest D_80086BC0) */
s32 gCameraShakeTimer;
f32 gCameraShakeX;
f32 gCameraShakeY;
s32 gCameraBob;                         /* set when the map / submap is in gCameraBobRooms */
s32 D_80085CBC;                         /* unreferenced */
u8 D_80085CC0[0x188];                   /* unreferenced */
Camera gCamera;               /* the camera */
MtxF gProjectionMtxF;                        /* UpdateCamera passes it with &gPerspNorm (a perspective's mf, perspNorm?) */
MtxF gViewMtxF;                        /* the view matrix */
f32 gViewConeHalf;                         /* half the view cone, in degrees */
f32 gNearPlane;                         /* near plane distance */
f32 gFarPlane;
f32 gViewTanHalf;
s32 gCameraSetupIndex;                         /* the current camera setup */
CameraSetup* gCameraSetups;                /* the map's camera setups */
CameraTransition gCameraTransition;
f32 gBattleCameraYawOffset;                         /* battle camera yaw offset, +45 deg per press (Quest D_80086EFC) */
u16 gPerspNorm;
void LoadCameraSetup(s32 index);
/* names from vbhayden's Quest 64 scratches */
extern s32 gNpcsDrawn;             /* NPCs drawn this frame (Quest D_8007BCE8) */
extern ActorTransform* gNpcsDrawnList[]; /* the drawn NPCs' transforms (Quest D_8007BCF0) */
float sqrtf(float value);
extern f64 D_8006F270; /* pi */
extern f64 D_8006F278; /* pi / 8 */
void rotateCoordinatesByAngle(f32 angle, Coordinates2D* coordinates);
extern f32 gBattleCenterX;          /* battle centre x (Quest D_8008C59C) */
extern f32 gBattleCenterZ;          /* battle centre z (Quest D_8008C5A0) */
extern u16 gButtonsPressed;          /* buttons pressed this frame (Quest D_80092876) */
f32 Atan2(f32 dx, f32 dz);
void CameraMode_Field(Camera* cam);
/* fixed-position camera view of the camera state (Quest RuntimeCameraData, camera mode 1) */
void func_80015B88(Camera* cam);
void CameraFollowHeight(Camera* cam, MonsterBattleData* target);
f32 cosf(f32 angle);   /* cosf */
f32 sinf(f32 angle);   /* sinf */
extern u16 gBattleState;               /* battle state (gBattleState) */
extern s32 gGameState;               /* game state flags */
extern u16 gButtonsHeld;               /* buttons held */
void CameraTurnToAngle(Camera* cam, Vec3f* target);
f32 CameraAboveNpcs(Camera* cam, f32 height);
void PollWorldCollision(f32 sample_x, f32 sample_y, f32 sample_z, u16 arg3_flags, CollisionPollingResult* result);

typedef struct {
    s32 map;
    s32 submap;
} MapRef;
void CameraMode_Fixed(Camera* cam);
void AimFixedCamera(Camera* cam);
void CameraMode_Battle(Camera* cam);
void CameraMode_AfterBattle(Camera* cam);
/* the camera setups of a map without its own table (InitRoomCamera) */
CameraSetup gDefaultCameraSetups[] = {
    { 0, 0.0f, 0.0f, 0.0f, 11.0f, 15.0f, 65.0f, 0.0f, 0.0f, 0.0f, 0.0f, { 25.0f, 15.0f, 2500.0f } },
    { 0, 0.0f, 0.0f, 0.0f, 11.0f, 72.0f, 144.0f, 0.0f, 0.0f, 0.0f, 0.0f, { 25.0f, 15.0f, 2500.0f } },
    { 0, 0.0f, 0.0f, 0.0f, 11.0f, 20.0f, 55.0f, 0.0f, 0.0f, 0.0f, 0.0f, { 25.0f, 15.0f, 2500.0f } },
};
/* the camera mode handlers, by mode (UpdateCamera). Declared without a prototype: modes 1 and 3 take the
   FixedCamera view of the camera state. */
void (*gCameraModeHandlers[])() = {
    CameraMode_Field, CameraMode_Fixed, CameraMode_Field, AimFixedCamera, CameraMode_Battle, CameraMode_AfterBattle,
};
/* the submaps whose camera starts with gCameraBob set (InitRoomCamera) */
MapRef gCameraBobRooms[] = {
    { 25, 2 }, { 25, 3 }, { 25, 4 }, { 25, 5 }, { 25, 8 }, { 25, 9 }, { 12, 2 },
};
f32 gCameraShakeOffsets[] = { 0.0f, -0.1f, 0.0f, -0.05f, 0.1f, -0.1f, -0.05f, 0.08f };   /* pairs of floats (StartCameraShake) */
typedef struct {
    CameraSetup* setups;
    u16 count;
} MapCameras;                           /* 8 bytes: a submap's camera table (sMapData +0x24, one per submap) */
extern f32 gBrianSpawnYaw;                  /* Brian's spawn yaw */
extern PlayerState gPlayerState;          /* Brian's state block (player.c) */
extern sMapData* gMapHeader;            /* the current map's header */
extern s32 gCurrentMap;                  /* the current map */
extern s32 gCurrentSubmap;                  /* and submap */
extern PendingWarp gPendingWarp;
extern GfxFrame gFrames[2];       /* the two frames' graphics tasks */
void guOrtho(Mtx* m, f32 l, f32 r, f32 b, f32 t, f32 n, f32 f, f32 scale);   /* guOrtho */

void InitRoomCamera(void) {
    Camera* cam;
    ActorTransform* brian;
    sMapData* map;
    MapCameras* cams;
    CameraSetup* setup;
    s32 i;

    cam = &gCamera;
    brian = &gPlayerState.brian.transform;
    guOrtho(&gFrames[0].dyn.mtx.unk0[1], -159.0f, 159.0f, 119.0f, -119.0f, 10.0f, 200.0f, 1.0f);
    guOrtho(&gFrames[1].dyn.mtx.unk0[1], -159.0f, 159.0f, 119.0f, -119.0f, 10.0f, 200.0f, 1.0f);
    if (gPendingWarp.entrance < 0) {
        cam->unk84 = 3.1415927f;
    }
    gCameraSetupIndex = (gPlayerState.brian.transform.floorFlags >> 1) & 7;
    map = gMapHeader;
    cams = map->cameras;
    if (cams != NULL) {
        gCameraSetups = cams[gCurrentSubmap].setups;
        gCameraSetupCount = ((MapCameras*)map->cameras)[gCurrentSubmap].count;
    } else {
        gCameraSetups = gDefaultCameraSetups;
        gCameraSetupCount = 1;
    }
    setup = &gCameraSetups[gCameraSetupIndex];
    cam->mode = setup->type;
    cam->flags = 1;
    cam->timer = 0;
    cam->target_transform = brian;
    cam->min_angle_x = setup->min_angle_x;
    cam->max_angle_x = setup->max_angle_x;
    cam->min_angle_y = setup->min_angle_y;
    cam->max_angle_y = setup->max_angle_y;
    cam->ideal_planar_distance = setup->ideal_planar_distance;
    cam->unk50 = 0.0f;
    cam->local_camera_height = setup->local_camera_height;
    cam->local_focus_height = setup->local_focus_height;
    switch (cam->mode) {
        case 0:
        case 2:
            gCameraOffset.x = 0.0f;
            gCameraOffset.y = cam->ideal_planar_distance * 0.8;
            cam->cam_y = brian->pos.y + setup->local_camera_height;
            rotateCoordinatesByAngle(cam->unk84 + gBrianSpawnYaw, &gCameraOffset);
            cam->cam_x = brian->pos.x - gCameraOffset.x;
            cam->cam_z = brian->pos.z - gCameraOffset.y;
            cam->orbit_yaw = 0.0f;
            break;
        case 1:
            cam->cam_x = setup->x;
            cam->cam_y = setup->y;
            cam->cam_z = setup->z;
            cam->orbit_yaw = 0.0f;
            break;
    }
    cam->pitch = 0.0f;
    cam->target_yaw = cam->yaw = 0.0f;
    cam->focus_x = brian->pos.x;
    cam->focus_y = brian->pos.y + cam->local_focus_height;
    cam->focus_z = brian->pos.z;
    cam->unk30 = 0.0f;
    cam->focus_speed_y = 0.0f;
    cam->unk38 = 0.0f;
    cam->cam_speed_x = cam->cam_speed_y = cam->cam_speed_z = 0.0f;
    cam->orbit_speed = 0.0f;
    cam->pull = 0.02f;
    cam->drag = 0.8f;
    cam->orbit_pull = 0.005f;
    cam->orbit_drag = 0.8f;
    gCameraShakeTimer = 0;
    gCameraFloorFlags = 0;
    gViewConeHalf = setup->view.x;
    gNearPlane = setup->view.y;
    gFarPlane = setup->view.z;
    gBattleCameraYawOffset = 0.0f;
    gCameraBob = 0;
    for (i = 0; i < 7; i++) {
        if (gCurrentMap == gCameraBobRooms[i].map && gCurrentSubmap == gCameraBobRooms[i].submap) {
            gCameraBob++;
            break;
        }
    }
}
extern s32 gFieldFreeze;
extern Gfx* gDisplayListHead;
extern s32 gMatrixHead;                 /* matrices used this frame */
extern Mtx D_2000000[];                /* segment 2: the frame's matrices */
extern Mtx D_2000080;
extern Light D_2008A40;                /* segment 2: FrameDynamic.lookat, its x and y halves */
extern Light D_2008A50;
void UpdateCameraBob(Coordinates2D* obj);
void guPerspectiveF(float mf[4][4], u16* perspNorm, f32 fovy, f32 aspect, f32 near, f32 far, f32 scale);   /* guPerspectiveF */
void guLookAtReflectF(float mf[4][4], LookAt* l, f32 xEye, f32 yEye, f32 zEye, f32 xAt, f32 yAt, f32 zAt, f32 xUp, f32 yUp, f32 zUp);   /* guLookAtReflectF */
void UpdateCamera(FrameDynamic* dyn) {
    /* The camera's frame: Brian's camera preset on maps with flag 2, the mode's handler, L cycles the map's camera
       setups, the bob, the projection (gViewConeHalf degrees, 4:3) and view into the frame's matrices, the screen
       shake (gCameraShakeTimer frames of a translate that flips and shrinks by 0.8), the look-at, perspNorm, and
       gViewTanHalf = tan(half the view cone) */
    Camera* cam;
    f32 half;
    s32 next;
    LookAt* look;

    cam = &gCamera;
    if ((gSubmapHeader->flags & 2) && !(gBattleState & 1) && cam->mode != 3) {
        next = (gPlayerState.brian.transform.floorFlags >> 1) & 7;
        if (next != gCameraSetupIndex) {
            LoadCameraSetup(next);
        }
    }
    gCameraModeHandlers[cam->mode](cam);
    if (!(gGameState & 0x82) && gFieldFreeze == 0 && !(gBattleState & 1) && !(gSubmapHeader->flags & 2) &&
        (gButtonsPressed & L_TRIG) && gCameraSetupCount >= 2) {
        gCameraSetupIndex++;
        if (gCameraSetupIndex >= gCameraSetupCount) {
            gCameraSetupIndex = 0;
        }
        LoadCameraSetup(gCameraSetupIndex);
    }
    gCameraOffset.y = 0.0f;
    if (gCameraBob != 0) {
        UpdateCameraBob(&gCameraOffset);
    }
    guPerspectiveF(gProjectionMtxF.mf, &gPerspNorm, gViewConeHalf, 4.0f / 3.0f, gNearPlane, gFarPlane, 1.0f);
    look = &dyn->lookat;
    guLookAtReflectF(gViewMtxF.mf, look, cam->cam_x, cam->cam_y + gCameraOffset.y, cam->cam_z, cam->focus_x, cam->focus_y,
                  cam->focus_z, 0.0f, 1.0f, 0.0f);
    guMtxF2L(gProjectionMtxF.mf, &dyn->mtx.unk0[0]);
    guMtxF2L(gViewMtxF.mf, &dyn->mtx.unk0[2]);
    if (gCameraShakeTimer != 0) {
        guTranslate(&dyn->mtx.unkC0[gMatrixHead], gCameraShakeX, gCameraShakeY, 0.0f);
        gCameraShakeTimer--;
        gCameraShakeX *= -0.8;
        gCameraShakeY *= -0.8;
        gSPMatrix(gDisplayListHead++, &D_2000000[gMatrixHead] + 3, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_PROJECTION);
        gMatrixHead++;
        gSPMatrix(gDisplayListHead++, D_2000000, G_MTX_NOPUSH | G_MTX_MUL | G_MTX_PROJECTION);
    } else {
        gSPMatrix(gDisplayListHead++, D_2000000, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_PROJECTION);
    }
    gSPMatrix(gDisplayListHead++, &D_2000080, G_MTX_NOPUSH | G_MTX_MUL | G_MTX_PROJECTION);
    gSPLookAtX(gDisplayListHead++, &D_2008A40);
    gSPLookAtY(gDisplayListHead++, &D_2008A50);
    gSPPerspNormalize(gDisplayListHead++, gPerspNorm);
    half = gViewConeHalf * 0.017453292519943295 / 2;
    gViewTanHalf = sinf(half) / cosf(half);
}
/* The default camera (Quest func_80013008): trail Brian at arg0->distance, swing behind him when he is close
   (or while flag 0x20 is set), clamp to the floor poll and the camera height, then aim at the focus. */
void CameraMode_Field(Camera* cam) {
    u8 behind;
    f32 dx;
    f32 dy;
    f32 dz;                     /* also the floor height below (RULE-EXCEPTION R30: its own local takes another register) */
    f32 dist;
    f32 ratio;
    f32 pushX;
    f32 diff;
    f32 pushZ;
    ActorTransform* target;

    target = cam->target_transform;
    func_80015B88(cam);
    if (!(gGameState & 0x82) && !(gBattleState & 0x200) && !(cam->flags & 0x40)) {
        if ((gButtonsHeld & B_BUTTON) || cam->mode == 2) {
            cam->flags |= 0x24;
        } else {
            cam->flags &= ~0x24;
        }
    } else {
        cam->flags &= ~0x24;
    }
    dx = target->pos.x - cam->cam_x;
    dz = target->pos.z - cam->cam_z;
    dist = sqrtf(dx * dx + dz * dz);
    if (dist != 0.0f) {
        if (cam->ideal_planar_distance < dist) {
            ratio = cam->ideal_planar_distance / dist;
            pushX = (dx - dx * ratio) * cam->pull;
            pushZ = (dz - dz * ratio) * cam->pull;
        } else if (dist <= cam->ideal_planar_distance) {
            ratio = cam->ideal_planar_distance / dist;
            pushX = (dx - dx * ratio) * cam->pull * 0.5;
            pushZ = (dz - dz * ratio) * cam->pull * 0.5;
        }
    } else {
        pushZ = 0.0f;
        pushX = 0.0f;
    }
    cam->cam_speed_x += pushX;
    cam->cam_speed_z += pushZ;
    cam->cam_x += cam->cam_speed_x;
    cam->cam_z += cam->cam_speed_z;
    cam->cam_speed_x *= cam->drag;
    cam->cam_speed_z *= cam->drag;
    behind = 0;
    if (dist < 25.0f && !(gGameState & 0x80)) {
        diff = target->yrot - cam->target_yaw;
        if (diff > 3.141592653589793) {
            diff -= 6.283185307179586;
        } else if (diff < -3.141592653589793) {
            diff += 6.283185307179586;
        }
        if (diff < 1.5707963267948966 && diff > -1.5707963267948966) {
            cam->orbit_yaw = ((diff < 0.0f) ? 1.5707963267948966 : -1.5707963267948966) + target->yrot;
        }
        cam->flags |= 4;
        behind = 1;
    } else if (cam->flags & 0x20) {
        cam->orbit_yaw = target->yrot + 3.141592653589793;
        if (cam->orbit_yaw > 3.141592653589793) {
            cam->orbit_yaw -= 6.283185307179586;
        } else if (cam->orbit_yaw < -3.141592653589793) {
            cam->orbit_yaw += 6.283185307179586;
        }
    }
    CameraTurnToAngle(cam, &target->pos);
    gCameraFloorPoll.x = 4.0f;
    PollWorldCollision(cam->cam_x, cam->cam_y, cam->cam_z, 0x602, &gCameraFloorPoll);
    cam->cam_x = gCameraFloorPoll.x;
    cam->cam_z = gCameraFloorPoll.z;
    if (gCameraFloorPoll.flags & 0x8000) {
        dy = target->pos.y;
    } else {
        dy = gCameraFloorPoll.y;
    }
    gCameraFloorFlags = gCameraFloorPoll.flags;
    if (dy < target->pos.y + cam->local_camera_height) {
        dy = target->pos.y + cam->local_camera_height;
    }
    dz = CameraAboveNpcs(cam, dy);
    if ((gBattleState & 1) && dz < gPlayerState.brian.transform.pos.y) {
        dz = gPlayerState.brian.transform.pos.y;
    }
    dy = dz - cam->cam_y;
    dy *= cam->pull * ((dy < 0.0f) ? 0.2 : 0.8);
    cam->cam_speed_y += dy;
    cam->cam_y += cam->cam_speed_y;
    cam->cam_speed_y *= cam->drag;
    CameraFollowHeight(cam, (MonsterBattleData*)target);
    dx = cam->cam_x - cam->focus_x;
    dy = cam->cam_y - cam->focus_y;
    dz = cam->cam_z - cam->focus_z;
    if (dx > -0.5 && dx < 0.5 && dz > -0.5 && dz < 0.5) {
        cam->cam_x += 0.5;
        cam->cam_z += 0.5;
    }
    dist = sqrtf(dx * dx + dz * dz);
    if (dy != 0.0f || dist != 0.0f) {
        cam->pitch = Atan2(dy, dist);
    }
    if (dx != 0.0f || dz != 0.0f) {
        cam->yaw = Atan2(dx, dz);
        if (!behind) {
            cam->target_yaw = cam->yaw;
        }
    }
}
void CameraMode_Fixed(Camera* cam) {
    f32 dx;
    f32 dy;
    f32 dz;
    f32 planar_camera_distance;
    MonsterBattleData* followed;

    followed = (MonsterBattleData*)cam->target_transform;
    func_80015B88(cam);
    CameraFollowHeight(cam, followed);

    dx = cam->cam_x - cam->focus_x;
    dy = cam->cam_y - cam->focus_y;
    dz = cam->cam_z - cam->focus_z;

    planar_camera_distance = sqrtf((dx * dx) + (dz * dz));

    if ((dy != 0.0f) || (planar_camera_distance != 0.0f)) {
        cam->pitch = Atan2(dy, planar_camera_distance);
    }

    if ((dx != 0.0f) || (dz != 0.0f)) {
        cam->target_yaw = Atan2(dx, dz);
    }

    if ((cam->min_angle_x != 0.0f) || (cam->max_angle_x != 0.0f)) {
        if (cam->pitch < cam->min_angle_x) {
            cam->pitch = cam->min_angle_x;
            dx = planar_camera_distance / cosf(cam->min_angle_x);
            cam->focus_y = cam->cam_y - (sinf(cam->min_angle_x) * dx);
        } else if (cam->max_angle_x < cam->pitch) {
            cam->pitch = cam->max_angle_x;
            dx = planar_camera_distance / cosf(cam->max_angle_x);
            cam->focus_y = cam->cam_y - (sinf(cam->max_angle_x) * dx);
        }
    }

    if (cam->min_angle_y < cam->max_angle_y) {
        if (cam->target_yaw < cam->min_angle_y) {
            cam->target_yaw = cam->min_angle_y;
            gCameraOffset.x = 0.0f;
            gCameraOffset.y = -planar_camera_distance;
            rotateCoordinatesByAngle(cam->target_yaw, &gCameraOffset);
            cam->focus_x = cam->cam_x + gCameraOffset.x;
            cam->focus_z = cam->cam_z + gCameraOffset.y;
        } else if (cam->max_angle_y < cam->target_yaw) {
            cam->target_yaw = cam->max_angle_y;
            gCameraOffset.x = 0.0f;
            gCameraOffset.y = -planar_camera_distance;
            rotateCoordinatesByAngle(cam->target_yaw, &gCameraOffset);
            cam->focus_x = cam->cam_x + gCameraOffset.x;
            cam->focus_z = cam->cam_z + gCameraOffset.y;
        }
    } else if (cam->max_angle_y < cam->min_angle_y) {
        if (cam->target_yaw < 0.0) {
            if (cam->max_angle_y < cam->target_yaw) {
                cam->target_yaw = cam->max_angle_y;
                gCameraOffset.x = 0.0f;
                gCameraOffset.y = -planar_camera_distance;
                rotateCoordinatesByAngle(cam->target_yaw, &gCameraOffset);
                cam->focus_x = cam->cam_x + gCameraOffset.x;
                cam->focus_z = cam->cam_z + gCameraOffset.y;
            }
        } else if (cam->target_yaw < cam->min_angle_y) {
            cam->target_yaw = cam->min_angle_y;
            gCameraOffset.x = 0.0f;
            gCameraOffset.y = -planar_camera_distance;
            rotateCoordinatesByAngle(cam->target_yaw, &gCameraOffset);
            cam->focus_x = cam->cam_x + gCameraOffset.x;
            cam->focus_z = cam->cam_z + gCameraOffset.y;
        }
    }

    cam->yaw = cam->target_yaw;
}
void AimFixedCamera(Camera* cam) {
    f32 dx = cam->cam_x - cam->focus_x;
    f32 dz = cam->cam_z - cam->focus_z;

    if ((dx != 0.0f) || (dz != 0.0f)) {
        cam->target_yaw = cam->yaw = Atan2(dx, dz);
    }
}
void CameraMode_Battle(Camera* cam) {
    f32 angle;
    ActorTransform* subject;

    if (cam->flags & 0x40) {
        cam->timer--;
        if (cam->timer == 0) {
            cam->flags &= ~0x40;
            *cam = gDefaultCamera;
        }
    } else {
        subject = cam->target_transform;
        angle = Atan2(gBattleCenterX - subject->pos.x, gBattleCenterZ - subject->pos.z) + 3.141592653589793;

        if (angle > 3.141592653589793) {
            angle -= 6.283185307179586;
        } else if (angle < -3.141592653589793) {
            angle += 6.283185307179586;
        }

        angle = (s32)(angle / 0.39269908169872414) * 0.39269908169872414 - 0.19634954084936207;

        if (angle > 3.141592653589793) {
            angle -= 6.283185307179586;
        } else if (angle < -3.141592653589793) {
            angle += 6.283185307179586;
        }
        angle += gBattleCameraYawOffset;
        if (angle > 3.141592653589793) {
            angle -= 6.283185307179586;
        } else if (angle < -3.141592653589793) {
            angle += 6.283185307179586;
        }
        cam->orbit_yaw = angle;
        if (gButtonsPressed & L_TRIG) {
            gBattleCameraYawOffset += 0.7853981633974483;

            if (gBattleCameraYawOffset > 3.141592653589793) {
                gBattleCameraYawOffset -= 6.283185307179586;
            } else if (gBattleCameraYawOffset < -3.141592653589793) {
                gBattleCameraYawOffset += 6.283185307179586;
            }
        }
    }
    CameraMode_Field(cam);
}
void CameraMode_AfterBattle(Camera* cam) {
    switch (gCameraTransition.phase) {
    case 0:
        if (--gCameraTransition.timer == 0) {
            gCameraTransition.phase++;
            gCameraTransition.timer = 0x4B;
            cam->flags |= 0x10;
        }
        break;
    case 1:
        if (--gCameraTransition.timer == 0) {
            cam->mode = gCameraTransition.savedMode;
            cam->flags = gCameraTransition.savedFlags;
            gCamera.orbit_pull = gCameraTransition.orbitPull;
            gCamera.orbit_drag = gCameraTransition.orbitDrag;
        }
        break;
    }
    CameraMode_Field(cam);
}
void func_80015B88(Camera* cam) {
}
void CameraFollowHeight(Camera* cam, MonsterBattleData* target) {
    f32 y;

    cam->focus_x = target->transform.pos.x;
    cam->focus_z = target->transform.pos.z;
    if (target->transform.base.monster->monsterType == FLYING) {
        y = target->stats->flyY;
    } else {
        y = target->transform.pos.y;
    }
    y += cam->local_focus_height - cam->focus_y;
    cam->focus_speed_y = cam->focus_speed_y + y * cam->pull * (1.0f + 1.0f);
    cam->focus_y += cam->focus_speed_y;
    cam->focus_speed_y = cam->focus_speed_y * (cam->drag * 0.8);
}
void CameraTurnToAngle(Camera* cam, Vec3f* target) {
    f32 dx;
    f32 dz;
    f32 dtheta;
    f32 turn;

    dx = cam->cam_x - target->x;
    dz = cam->cam_z - target->z;
    if (cam->flags & 0x14) {
        dtheta = Atan2(dx, dz) - cam->orbit_yaw;
        if (3.141592653589793 < dtheta) {
            dtheta = dtheta - 6.283185307179586;
        } else if (dtheta < -3.141592653589793) {
            dtheta = dtheta + 6.283185307179586;
        }
        turn = (cam->orbit_pull * dtheta) + cam->orbit_speed;
    } else {
        turn = cam->orbit_speed;
    }
    cam->orbit_speed = cam->orbit_drag * turn;
    gCameraOffset.x = dx;
    gCameraOffset.y = dz;
    rotateCoordinatesByAngle(turn, &gCameraOffset);
    cam->cam_x = target->x - gCameraOffset.x;
    cam->cam_z = target->z + gCameraOffset.y;
}
void StartCameraShake(s32 kind) {
    gCameraShakeTimer = 0xF;
    gCameraShakeX = gCameraShakeOffsets[kind * 2];
    gCameraShakeY = gCameraShakeOffsets[kind * 2 + 1];
}
void LoadCameraSetup(s32 index) {
    Camera* cam = &gCamera;

    gCameraSetupIndex = index;
    if (cam->flags & 0x40) {
        cam->target_transform = gDefaultCamera.target_transform;
    }
    cam->mode = gCameraSetups[index].type;
    if (cam->mode != 0 && cam->mode != 2) {
        cam->cam_x = gCameraSetups[index].x;
        cam->cam_y = gCameraSetups[index].y;
        cam->cam_z = gCameraSetups[index].z;
    }
    cam->flags &= ~0x50;
    cam->ideal_planar_distance = gCameraSetups[index].ideal_planar_distance;
    cam->local_camera_height = gCameraSetups[index].local_camera_height;
    cam->local_focus_height = gCameraSetups[index].local_focus_height;
    cam->min_angle_y = gCameraSetups[index].min_angle_y;
    cam->max_angle_y = gCameraSetups[index].max_angle_y;
    cam->min_angle_x = gCameraSetups[index].min_angle_x;
    cam->max_angle_x = gCameraSetups[index].max_angle_x;
    gViewConeHalf = gCameraSetups[index].view.x;
    gNearPlane = gCameraSetups[index].view.y;
    gFarPlane = gCameraSetups[index].view.z;
    cam->cam_speed_x = cam->cam_speed_y = cam->cam_speed_z = 0.0f;
    cam->orbit_speed = 0.0f;
}

void SnapSpellCamera(s32 duration, MonsterBattleData* caster) {
    f32 cam_dist;
    MonsterBaseData* caster_attrs;

    if (gBattleState & 1) {
        if (!(gCamera.flags & 0x40)) {
            gDefaultCamera = gCamera;
        }

        gSpellCameraTarget.pos.x = caster->transform.pos.x;
        gSpellCameraTarget.pos.z = caster->transform.pos.z;

        if (caster->transform.actorFlags & 4) {
            gSpellCameraTarget.pos.y = gPlayerState.brian.transform.pos.y;
        } else if (caster->transform.base.monster->monsterType == FLYING) {
            gSpellCameraTarget.pos.y = caster->stats->flyY;
        } else {
            gSpellCameraTarget.pos.y = caster->transform.pos.y;
        }

        gSpellCameraTarget.scale = caster->transform.scale;
        gSpellCameraTarget.unk64 = caster->transform.base.monster;
        gSpellCameraTarget.unk68 = caster->stats;

        if (caster->transform.actorFlags & 4) {
            cam_dist = caster->transform.base.monster->height * caster->transform.scale + 40.0;
        } else {
            caster_attrs = caster->transform.base.monster;

            if (caster_attrs->height < caster_attrs->radius) {
                cam_dist = 2.0 * (caster_attrs->radius * caster->transform.scale) + 40.0;
            } else {
                cam_dist = 2.0 * (caster_attrs->height * caster->transform.scale) + 40.0;
            }
        }

        gCamera.mode = 4;
        gCamera.flags |= 0x40;
        gCamera.target_transform = &gSpellCameraTarget;

        gCamera.local_camera_height = (caster->transform.actorFlags & 4) ? 20.0 : 15.0;
        gCamera.cam_speed_x = gCamera.cam_speed_y = gCamera.cam_speed_z = 0.0f;
        gCamera.ideal_planar_distance = cam_dist;
        gCamera.timer = duration;
        gCamera.orbit_speed = 0.0f;

        gCameraOffset.x = 0.0f;
        gCameraOffset.y = cam_dist + 10.0;

        rotateCoordinatesByAngle(gCamera.orbit_yaw, &gCameraOffset);

        gCamera.cam_x = caster->transform.pos.x + gCameraOffset.x;
        gCamera.cam_y = gSpellCameraTarget.pos.y + (gCamera.local_camera_height * 1.1);
        gCamera.cam_z = caster->transform.pos.z + gCameraOffset.y;

        if (gCamera.cam_y < gPlayerState.brian.transform.pos.y) {
            gCamera.cam_y = gPlayerState.brian.transform.pos.y;
        }
    }
}
void StartAfterBattleCamera(s32 index) {
    LoadCameraSetup(index);
    gCameraTransition.phase = 0;
    gCameraTransition.timer = 0xF;
    gCameraTransition.savedMode = gCamera.mode;
    gCameraTransition.savedFlags = gCamera.flags;
    gCameraTransition.orbitPull = 0.005f;
    gCameraTransition.orbitDrag = 0.9f;
    gCamera.mode = 5;
    gCamera.orbit_yaw = gCameraTransition.orbitYaw;
    
}
void ApplyCameraSetup(CameraValues* setup, MonsterBattleData* target) {
    gCamera.mode = setup->type;
    gCamera.flags = 0x40;

    gCamera.ideal_planar_distance = setup->ideal_planar_distance;
    gCamera.local_camera_height = setup->local_camera_height;
    gCamera.local_focus_height = setup->local_focus_height;

    gCamera.max_angle_y = 0.0f;
    gCamera.max_angle_x = 0.0f;
    gCamera.min_angle_y = gCamera.max_angle_y;
    gCamera.min_angle_x = gCamera.max_angle_x;

    gCamera.target_transform = &target->transform;

    switch (setup->type) {
    case 0:
        gCameraOffset.x = 0.0f;
        gCameraOffset.y = setup->ideal_planar_distance + 20.0;
        rotateCoordinatesByAngle(target->transform.yrot - 3.141592653589793 + 0.39269908169872414, &gCameraOffset);
        gCamera.cam_x = target->transform.pos.x - gCameraOffset.x;
        gCamera.cam_z = target->transform.pos.z - gCameraOffset.y;
        gCamera.cam_y = target->transform.pos.y + setup->local_camera_height;
        break;
    case 1:
        gCamera.cam_x = setup->x;
        gCamera.cam_y = setup->y;
        gCamera.cam_z = setup->z;
        break;
    }

    gCamera.cam_speed_z = 0.0f;
    gCamera.focus_speed_y = 0.0f;
    gCamera.orbit_speed = 0.0f;
    gCamera.cam_speed_y = gCamera.cam_speed_z;
    gCamera.cam_speed_x = gCamera.cam_speed_y;
}
void ClampCameraToTarget(void) {
    f32 yaw;

    yaw = gCamera.target_yaw - gPlayerState.brian.transform.yrot;
    if (yaw < -3.141592653589793) {
        yaw += 6.283185307179586;
    } else if (yaw > 3.141592653589793) {
        yaw -= 6.283185307179586;
    }
    if (yaw > 0.7853981633974483) {
        yaw = 0.7853982f;
    }
    if (yaw < -0.7853981633974483) {
        yaw = -0.7853982f;
    }
    yaw += 3.141592653589793;
    if (yaw < -3.141592653589793) {
        yaw += 6.283185307179586;
    } else if (yaw > 3.141592653589793) {
        yaw -= 6.283185307179586;
    }
    gCamera.unk84 = yaw;
    gCamera.orbit_yaw = gPendingWarp.yaw + 3.141592653589793;
    if (gCamera.orbit_yaw > 3.141592653589793) {
        gCamera.orbit_yaw -= 6.283185307179586;
    } else if (gCamera.orbit_yaw < -3.141592653589793) {
        gCamera.orbit_yaw += 6.283185307179586;
    }
    gCamera.flags |= 0x10;
}
f32 CameraAboveNpcs(Camera* cam, f32 height) {
    s32 n;
    f32 dx;
    f32 dz;
    ActorTransform* npc;
    ActorTransform** list;

    if (gNpcsDrawn != 0) {
        for (n = gNpcsDrawn, list = gNpcsDrawnList; n != 0; n--) {
            npc = *list++;
            dx = npc->pos.y - height;

            if (dx < 20.0) {
                dx = npc->pos.x - cam->cam_x;
                dz = npc->pos.z - cam->cam_z;
                if (sqrtf((dx * dx) + (dz * dz)) < 30.0) {
                    height = npc->pos.y + 30.0;
                    break;
                }
            }
        }
    }
    return height;
}
/* the camera's bob (gCameraOffset.y, added to the eye height by UpdateCamera) */
void UpdateCameraBob(Coordinates2D* obj) {
    static f32 sPhase1 = 0.0f;          /* two sine phases; their sum sets the bob */
    static f32 sPhase2 = 0.0f;

    obj->y = sinf(sPhase1 + sPhase2);
    sPhase1 += 0.02617993877991494;     /* pi / 120 per frame */
    sPhase2 += 0.015707963267948967;    /* pi / 200 */
    if (sPhase1 > 6.283185307179586) {
        sPhase1 -= 6.283185307179586;
    }
    if (sPhase2 > 6.283185307179586) {
        sPhase2 -= 6.283185307179586;
    }
}
