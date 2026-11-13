#include "common.h"
#include "game/signs.h"
#include "game/dma.h"
#include "game/math.h"
/* split from spirits.c: the original was separate files (each IDO object pads its .rodata to 16;
   the zero words before this file's first literal mark the boundary). Declarations copied from spirits.c. */
extern s32 gCurrentMap;
extern s32 gCurrentSubmap;
extern u8 gTextBuffer[];
extern u8 gSpeakerName[];
extern u8 D_D37700[];
extern PlayerState gPlayerState;
extern SignObject gSigns[38];
extern s32 gRoomSignCount;
extern SignSlot gRoomSigns[];
extern Coordinates2D gSignOffset;
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
