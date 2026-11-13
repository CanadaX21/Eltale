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

#endif
