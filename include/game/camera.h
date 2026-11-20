#ifndef GAME_CAMERA_H
#define GAME_CAMERA_H

/* The functions of src/game/camera.c, declared once for every file that calls them. */

#include "common.h"

void InitRoomCamera(void);
void UpdateCamera(FrameDynamic* dyn);
void CameraMode_Field(Camera* cam);
void CameraMode_Fixed(Camera* cam);
void AimFixedCamera(Camera* cam);
void CameraMode_Battle(Camera* cam);
void CameraMode_AfterBattle(Camera* cam);
void func_80015B88(Camera* cam);
void CameraFollowHeight(Camera* cam, MonsterBattleData* target);
void CameraTurnToAngle(Camera* cam, Vec3f* target);
void StartCameraShake(s32 kind);
void LoadCameraSetup(s32 index);
void SnapSpellCamera(s32 duration, MonsterBattleData* caster);
void StartAfterBattleCamera(s32 index);
void ApplyCameraSetup(CameraValues* setup, MonsterBattleData* target);
void ClampCameraToTarget(void);
f32 CameraAboveNpcs(Camera* cam, f32 height);
void UpdateCameraBob(Coordinates2D* obj);

/* ---- camera.c's .bss (0x80085B80-0x80085F90), in address order. IDO 5.3 lays a unit's .bss
   out by its variables' block numbers, so a declaration added or removed before them, in a header too, can change
   their order; BSS_ORDER_PAD keeps the original order and `python3 tools/bss_order.py fix` recomputes it
   (docs/DECLARATIONS.md). The D_ names marked unreferenced are space no code addresses (unknown
   variables). ---- */
typedef struct {                        /* laid out like MonsterBattleData's first 0x6C bytes */
    Vec3f pos;
    char unkC[0x18];
    f32 scale;                          /* 0x24 */
    char unk28[0x3C];
    MonsterBaseData* monster;
    ActorStats* stats;
} CameraTarget;                         /* the spell camera's target (SnapSpellCamera copies an enemy into it) */
typedef struct {
    s32 map;
    s32 submap;
} MapRef;

/* The state src/game/camera.c defines (.bss), in its storage order: the order of these declarations is the
 * unit's .bss layout (include/bss_order.h). */
extern Camera gDefaultCamera;   /* default camera (Quest D_80086B00) */
extern s32 gCameraSetupCount;   /* camera setups in the map's table */
extern s32 D_80085C0C;   /* unreferenced */
extern Coordinates2D gCameraOffset;
extern s32 D_80085C18[2];   /* unreferenced */
extern CollisionPollingResult gCameraFloorPoll;   /* the camera's floor poll */
extern u16 gCameraFloorFlags;   /* its flags */
extern CameraTarget gSpellCameraTarget;   /* spell camera target (Quest D_80086BC0) */
extern s32 gCameraShakeTimer;
extern f32 gCameraShakeX;
extern f32 gCameraShakeY;
extern s32 gCameraBob;   /* set when the map / submap is in gCameraBobRooms */
extern s32 D_80085CBC;   /* unreferenced */
extern u8 D_80085CC0[0x188];   /* unreferenced */
extern Camera gCamera;   /* the camera */
extern MtxF gProjectionMtxF;   /* the projection matrix: UpdateCamera builds it with guPerspectiveF, which writes gPerspNorm too */
extern MtxF gViewMtxF;   /* the view matrix */
extern f32 gViewConeHalf;   /* half the view cone, in degrees */
extern f32 gNearPlane;   /* near plane distance */
extern f32 gFarPlane;
extern f32 gViewTanHalf;
extern s32 gCameraSetupIndex;   /* the current camera setup */
extern CameraSetup* gCameraSetups;   /* the map's camera setups */
extern CameraTransition gCameraTransition;
extern f32 gBattleCameraYawOffset;   /* battle camera yaw offset, +45 deg per press (Quest D_80086EFC) */
extern u16 gPerspNorm;

/* The data src/game/camera.c defines. */
extern CameraSetup gDefaultCameraSetups[];
extern void (*gCameraModeHandlers[])();
extern MapRef gCameraBobRooms[];
extern f32 gCameraShakeOffsets[];   /* pairs of floats (StartCameraShake) */

#endif
