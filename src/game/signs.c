#include "common.h"
/* split from spirits.c: the original was separate files (each IDO object pads its .rodata to 16;
   the zero words before this file's first literal mark the boundary). Declarations copied from spirits.c. */
extern s32 gSpiritCount;
extern Spirit gSubmapSpirits[];
void rotateCoordinatesByAngle(f32 angle, Coordinates2D* coordinates);
void PlaySound(u8 delay, s8 id, u8 level);
extern u8 gSpiritsFound[];
extern u8 gBitMasks[];
extern u8 gObjectsOpened[];
extern SubmapSpirits gSubmapSpiritTable[43];
extern s32 gCurrentMap;
extern s32 gCurrentSubmap;
extern CollisionPollingResult gSpiritFloorPoll;
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
extern u8 gSparkleEnvColors[][4];
extern Gfx* gDisplayListHead;
f32 sinf(f32 angle);
extern SignObject gSigns[38];
extern s32 gRoomSignCount;
extern SignSlot gRoomSigns[];
extern Coordinates2D gSignOffset;
f32 sqrtf(f32 x);
f32 Atan2(f32 dx, f32 dz);
void LoadRoomSigns(void) {
    SignSlot* entry;
    s32 i;

    gRoomSignCount = 0;
    entry = gRoomSigns;
    for (i = 0; i < 38; i++) {
        if (gSigns[i].map == gCurrentMap && gSigns[i].submap == gCurrentSubmap) {
            entry->npc = &gSigns[i];
            entry->unk8 = 0;
            entry++;
            gRoomSignCount++;
        }
    }
}
SignSlot* TryReadSign(ActorTransform* brian) {
    s32 i;
    SignSlot* entry;
    f32 dx;
    f32 dz;
    f32 angle;

    i = gRoomSignCount;
    entry = gRoomSigns;
    while (i != 0) {
        dx = brian->pos.x - entry->npc->x;
        dz = brian->pos.z - entry->npc->z;
        if (sqrtf(dx * dx + dz * dz) < entry->npc->radius) {
            gSignOffset.x = -dx;
            gSignOffset.y = -dz;
            rotateCoordinatesByAngle(brian->yrot, &gSignOffset);
            angle = Atan2(gSignOffset.x, gSignOffset.y);
            if (angle < 0.7853981633974483 && angle > -0.7853981633974483) {
                entry->angle = brian->yrot - angle;
                if (entry->npc->flags & 1) {
                    break;
                }
                gSignOffset.x = dx;
                gSignOffset.y = dz;
                rotateCoordinatesByAngle(-entry->npc->yaw, &gSignOffset);
                angle = Atan2(gSignOffset.x, gSignOffset.y);
                if (angle < 0.7853981633974483 && angle > -0.7853981633974483) {
                    break;
                }
            }
        }
        i--;
        entry++;
    }
    if (i == 0) {
        entry = NULL;
    }
    return entry;
}
void LoadSignText(SignRef* sign) {
    dma_write(sign->unk0->unk1C + D_D37700, gSpeakerName, 0x20);
    dma_write(sign->unk0->unk1E + D_D37700, gTextBuffer, 0x200);
    if (gSpeakerName[2] == 0xFF) {
        gSpeakerName[0] = 0xFF;
    }
    gPlayerState.savedYRot = sign->unk4;
}
