#ifndef SCENE2D_H
#define SCENE2D_H

#include "common.h"

/*
 * The 2D scene format of the splash-and-intro overlay (game mode 4) and the ending overlay (game mode 5): both hold the
 * same three tables, read by the same code (18 functions the two overlays hold word for word, apart from addresses).
 * The intro's: 108 cues in 36 lists, 459 sprites, the header; the ending's: 24 cues in 8 lists, 97 sprites, the header.
 * The structures and their counts are verified by how the tables chain in the ROM; the fields' meanings are not yet read
 * from the code (still assembly) unless a comment says so.
 */

/* one cue (12 bytes); its three words not yet read from the code */
typedef struct Scene2DCue {
    /* 0x00 */ u32 unk00;
    /* 0x04 */ u32 unk04;
    /* 0x08 */ u32 unk08;
} Scene2DCue; /* size = 0xC */

/* a run of cues */
typedef struct Scene2DCueList {
    /* 0x00 */ Scene2DCue* cues;
    /* 0x04 */ s32 count;
} Scene2DCueList; /* size = 0x8 */

/*
 * One sprite: a rectangle of an image drawn on the screen. Verified: image and palette point at the image's start and
 * its 256-entry palette, and the image's bytes span imageWidth x imageHeight (one byte a pixel where flags has 0x08000000,
 * half a byte where it has 0x04000000). Inferred from the data, not yet from the code: x and y place the rectangle,
 * w and h are its size and s and t its offset in the image (the strips of each image tile it).
 */
typedef struct Scene2DSprite {
    /* 0x00 */ s16 unk00;
    /* 0x02 */ s16 unk02;
    /* 0x04 */ u32 flags;
    /* 0x08 */ s16 x;
    /* 0x0A */ s16 y;
    /* 0x0C */ s16 w;
    /* 0x0E */ s16 h;
    /* 0x10 */ s16 s;
    /* 0x12 */ s16 t;
    /* 0x14 */ s16 imageWidth;
    /* 0x16 */ s16 imageHeight;
    /* 0x18 */ u8* image;
    /* 0x1C */ u16* palette;
    /* 0x20 */ u16 unk20;
    /* 0x22 */ u16 unk22;
    /* 0x24 */ s32 unk24;
} Scene2DSprite; /* size = 0x28 */

/* the sprites and the number of cue lists */
typedef struct Scene2DHeader {
    /* 0x00 */ Scene2DSprite* sprites;
    /* 0x04 */ s32 spriteCount;
    /* 0x08 */ s32 unk08;
    /* 0x0C */ s32 cueListCount;
} Scene2DHeader; /* size = 0x10 */

/* one slot of the drawing state the shared code keeps (an array of 0x50-byte slots; its fields not yet read) */
typedef struct Scene2DSlot {
    /* 0x00 */ char unk00[0x50];
} Scene2DSlot; /* size = 0x50 */

#endif
