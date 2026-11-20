#include "common.h"
BSS_ORDER_PAD(0)
#include "game/signs.h"
#include "game/dma.h"
#include "game/math.h"
#include "game/enemy.h"
#include "game/map.h"
#include "game/player.h"
#include "dialogue.h"
/* split from spirits.c: the original was separate files (each IDO object pads its .rodata to 16;
   the zero words before this file's first literal mark the boundary). */
/* the signs, doors and shelves Brian can read, by map and submap (LoadRoomSigns): map, submap, x, z, yaw,
   radius, flags, and the strings LoadSignText reads, named in the dialogue bank's sign part */
SignObject gSigns[38] = {
    { 0, 3, 58.0f, -324.0f, -1.5707964f, 10.0f, 0x0, DLG_SIGN_0070, DLG_SIGN_0078 },
    { 0, 3, 168.0f, -295.0f, 1.2217305f, 10.0f, 0x0, DLG_SIGN_0070, DLG_SIGN_0078 },
    { 0, 3, -384.0f, -146.0f, -2.3561945f, 10.0f, 0x0, DLG_SIGN_0070, DLG_SIGN_0078 },
    { 0, 3, -153.0f, -165.0f, 3.1415927f, 10.0f, 0x0, DLG_SIGN_0070, DLG_SIGN_0078 },
    { 0, 3, 49.0f, -179.0f, 3.1415927f, 10.0f, 0x0, DLG_SIGN_0070, DLG_SIGN_0078 },
    { 0, 3, 186.0f, -181.0f, 1.3962634f, 10.0f, 0x0, DLG_SIGN_0070, DLG_SIGN_0078 },
    { 0, 3, 48.0f, 95.0f, -2.9146998f, 10.0f, 0x0, DLG_SIGN_0070, DLG_SIGN_0078 },
    { 0, 3, 183.0f, -51.0f, 1.5707964f, 10.0f, 0x0, DLG_SIGN_0070, DLG_SIGN_0078 },
    { 0, 3, 12.0f, 654.0f, 3.1415927f, 10.0f, 0x0, DLG_SIGN_0070, DLG_SIGN_0078 },
    { 2, 0, -1039.5f, 1459.5f, -1.2217305f, 10.0f, 0x0, DLG_SIGN_0090, DLG_SIGN_0098 },
    { 2, 0, -1010.0f, 1980.0f, 2.443461f, 10.0f, 0x0, DLG_SIGN_00D0, DLG_SIGN_00D8 },
    { 3, 0, -102.5f, -1557.5f, 0.2617994f, 10.0f, 0x0, DLG_SIGN_00F8, DLG_SIGN_0100 },
    { 3, 0, -582.0f, -720.0f, -1.7453293f, 10.0f, 0x0, DLG_SIGN_0120, DLG_SIGN_0128 },
    { 5, 0, 90.0f, -420.0f, -1.9198622f, 10.0f, 0x0, DLG_SIGN_01B8, DLG_SIGN_01C0 },
    { 5, 0, 285.0f, 1385.0f, 3.1415927f, 10.0f, 0x0, DLG_SIGN_0200, DLG_SIGN_0208 },
    { 5, 0, -555.0f, 885.0f, 3.1415927f, 10.0f, 0x0, DLG_SIGN_0230, DLG_SIGN_0238 },
    { 6, 0, 397.5f, 480.0f, 0.0f, 10.0f, 0x0, DLG_SIGN_0258, DLG_SIGN_0260 },
    { 6, 0, 10.0f, -532.0f, 0.0f, 10.0f, 0x0, DLG_SIGN_0280, DLG_SIGN_0288 },
    { 7, 0, 1285.0f, 1135.0f, -2.443461f, 10.0f, 0x0, DLG_SIGN_02B0, DLG_SIGN_02B8 },
    { 11, 0, 475.0f, -104.0f, 0.0f, 10.0f, 0x0, DLG_SIGN_0358, DLG_SIGN_0360 },
    { 11, 0, 186.0f, -168.0f, 0.17453292f, 10.0f, 0x0, DLG_SIGN_0358, DLG_SIGN_0360 },
    { 11, 0, 117.0f, -63.0f, -2.7925267f, 10.0f, 0x0, DLG_SIGN_0358, DLG_SIGN_0360 },
    { 11, 1, -1717.0f, -160.0f, -1.5707964f, 10.0f, 0x0, DLG_SIGN_0380, DLG_SIGN_0388 },
    { 13, 11, 1.0f, 50.0f, -1.5707964f, 10.0f, 0x0, DLG_SIGN_0600, DLG_SIGN_0608 },
    { 13, 15, -16.0f, -33.0f, 0.0f, 10.0f, 0x0, DLG_SIGN_0620, DLG_SIGN_0628 },
    { 13, 15, 18.0f, -33.0f, 0.0f, 10.0f, 0x0, DLG_SIGN_0670, DLG_SIGN_0678 },
    { 13, 15, 54.0f, -5.0f, 1.5707964f, 10.0f, 0x0, DLG_SIGN_06B0, DLG_SIGN_06B8 },
    { 14, 9, -4.0f, -41.0f, 0.0f, 10.0f, 0x0, DLG_SIGN_07D0, DLG_SIGN_07D8 },
    { 14, 9, 28.0f, 31.0f, 1.5707964f, 10.0f, 0x0, DLG_SIGN_0778, DLG_SIGN_0780 },
    { 14, 9, -28.0f, 31.0f, -1.5707964f, 10.0f, 0x0, DLG_SIGN_0730, DLG_SIGN_0738 },
    { 21, 6, -38.0f, -12.0f, -1.5707964f, 10.0f, 0x0, DLG_SIGN_0AF8, DLG_SIGN_0B00 },
    { 21, 6, -38.0f, 12.0f, -1.5707964f, 10.0f, 0x0, DLG_SIGN_0AA8, DLG_SIGN_0AB0 },
    { 21, 6, -38.0f, 36.0f, -1.5707964f, 10.0f, 0x0, DLG_SIGN_0A60, DLG_SIGN_0A68 },
    { 28, 0, -2902.5f, 432.5f, 2.3561945f, 10.0f, SIGN_NO_POSE, DLG_SIGN_0C60, DLG_SIGN_0C68 },
    { 31, 0, 42.0f, -524.0f, 0.5235988f, 10.0f, 0x0, DLG_SIGN_0CF8, DLG_SIGN_0D00 },
    { 31, 0, -646.0f, 414.0f, 3.1415927f, 10.0f, 0x0, DLG_SIGN_0D20, DLG_SIGN_0D28 },
    { 31, 0, -42.0f, -328.0f, 2.9670596f, 10.0f, 0x0, DLG_SIGN_0D48, DLG_SIGN_0D50 },
    { 33, 0, 858.5f, -438.0f, 0.0f, 10.0f, 0x0, DLG_SIGN_0D88, DLG_SIGN_0D90 },
};
s32 D_8004EB60 = 38;                    /* the table's length; no code reads it */
/* ---- signs.c's .bss (0x80085AB0-0x80085B80): the current room's signs. Defined in their storage order, the order its header declares them in: IDO 5.3 lays the unit's
   .bss out by those declarations (include/bss_order.h), and it 8-aligns every variable of 8 bytes or more. The D_
   name marked unreferenced is space no code addresses (an unknown variable). ---- */
Coordinates2D gSignOffset;
s32 D_80085AB8;                          /* unreferenced: the offset's third word (offsets of this kind -
                                            gSpellOffset, gEnemyStep, gHudVector - are OddVectors whose third
                                            word is a scale; no code reads this one) */
s32 gRoomSignCount;
SignSlot gRoomSigns[16];                 /* the room's signs: LoadRoomSigns writes one per sign, unbounded; 16
                                            fill the space to the next file (15 would too, with padding) */
void LoadRoomSigns(void) {
    SignSlot* entry;
    s32 i;

    gRoomSignCount = 0;
    entry = gRoomSigns;
    for (i = 0; i < 38; i++) {
        if (gSigns[i].map == gCurrentMap && gSigns[i].submap == gCurrentSubmap) {
            entry->sign = &gSigns[i];
            entry->unk8 = 0;
            entry++;
            gRoomSignCount++;
        }
    }
}
SignSlot* FindSignTarget(ActorTransform* brian) {
    s32 i;
    SignSlot* entry;
    f32 dx;
    f32 dz;
    f32 angle;

    i = gRoomSignCount;
    entry = gRoomSigns;
    while (i != 0) {
        dx = brian->pos.x - entry->sign->x;
        dz = brian->pos.z - entry->sign->z;
        if (sqrtf(dx * dx + dz * dz) < entry->sign->radius) {
            gSignOffset.x = -dx;
            gSignOffset.y = -dz;
            rotateCoordinatesByAngle(brian->yrot, &gSignOffset);
            angle = Atan2(gSignOffset.x, gSignOffset.y);
            if (angle < 0.7853981633974483 && angle > -0.7853981633974483) {
                entry->angle = brian->yrot - angle;
                if (entry->sign->flags & SIGN_ANY_SIDE) {
                    break;
                }
                gSignOffset.x = dx;
                gSignOffset.y = dz;
                rotateCoordinatesByAngle(-entry->sign->yaw, &gSignOffset);
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
    dma_write(sign->text->nameOffset + D_D37700, gSpeakerName, 0x20);
    dma_write(sign->text->textOffset + D_D37700, gTextBuffer, 0x200);
    if (gSpeakerName[2] == 0xFF) {
        gSpeakerName[0] = 0xFF;
    }
    gPlayerState.savedYRot = sign->readYaw;
}
