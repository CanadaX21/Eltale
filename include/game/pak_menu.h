#ifndef GAME_PAK_MENU_H
#define GAME_PAK_MENU_H

/* The boot-time Controller Pak manager (src/pak_menu_module/pak_menu.c): the data of its overlay that the C uses. */

#include "common.h"

/* one sprite of a drawn list, 16 bytes: its place, its size in pixels, its image and palette (two pixels a byte) */
typedef struct {
    s16 x;
    s16 y;
    u8 w;
    u8 h;
    u8 pad[2];                                  /* 0 in every list; nothing reads it */
    u8* image;
    u16* palette;
} PakSprite;

/* The functions src/pak_menu_module/pak_menu.c defines. */
void PakMenu_FormatNumber(u16 n, u8* digits);
void PakMenu_DrawGlyph(s32 glyph);
void PakMenu_SetTextPos(s16 x, s16 y);
void func_80100284_DA47F4(void);
void PakMenu_DrawGlyphs(u8* glyphs, u32 count);
void PakMenu_SetPalette(s32 palette);
void PakMenu_DrawImage(s16 x, s16 y, s16 w, s16 h, u8* image, u16* palette, u8 mode);
void PakMenu_DrawSprites(PakSprite* list, s32 count);
void PakMenu_SwapFrame(void);
void PakMenu_DrawNoteRow(s32 row);
void PakMenu_DrawFreePages(u16 pages);
void PakMenu_DrawHand(void);
void PakMenu_DrawDialog(u8* message);
void PakMenu_DrawScreen(void);
void ControllerPakMenu(OSMesgQueue* siQueue, OSMesgQueue* retraceQueue);       /* the boot-time Controller Pak manager, src/pak_menu_module/ */
void PakMenu_DrawText(s32 x, s32 y, u8* str);

/* The overlay's data the C uses, still assembly or binary in its image. */
extern u16 gPakMenuHandPalette[];   /* the hand cursor's palette */
extern u8 gPakMenuHandImage[];   /* its image, 16 x 14 */
extern PakSprite gPakMenuScreenSprites[];   /* the screen: 235 sprites */
extern PakSprite gPakMenuDialogWindow[];   /* a message window: 75 sprites, 15 x 5 tiles */
extern PakSprite gPakMenuDialogChoices[];   /* 22 sprites in two tile rows under the window */
extern PakSprite gPakMenuArrowUp;   /* the note list's scroll arrows */
extern PakSprite gPakMenuArrowDown;   /* assembly */
extern s32 gPakMenuFrameCount;   /* frames shown: its low bit picks the buffer */
extern s32 gPakMenuFrameBuffers[2];   /* the two frame buffers' addresses */
extern u32 gPakMenuGlyphs[];   /* the glyph sheet: 4-bit pixels, eight to a word, twelve rows a glyph */
extern u16 gPakMenuPalettes[];   /* the glyph palettes, four colours each */
extern u16* gPakMenuPalette;   /* the palette glyphs are drawn with */
extern OSPfsState gPakMenuNotes[];   /* the pak's 16 notes, as osPfsFileState reads them */
extern s32 gPakMenuLastGlyph;   /* the highest glyph code PakMenu_DrawGlyph draws */
extern s32 gPakMenuNoteErrors[];   /* osPfsFileState's result for each note: 0 for a note in use */
extern s16 gPakMenuState;   /* the menu's state */
extern s16 gPakMenuNoteRow;   /* the note row the hand is on */
extern u8 gPakMenuScrollAmt;   /* the note list's scroll */
extern s16 gPakMenuHandX;   /* the hand cursor's position */
extern s16 gPakMenuHandY;
extern s32 gPakMenuFreePages;   /* the pak's free bytes (osPfsFreeBlocks), then its free pages */
extern s32 gPakMenuFrameReady;   /* a frame is drawn and waits for the swap */
extern s32 gPakMenuDrawBuffer;   /* the frame being drawn: its byte address */
extern OSMesgQueue* gPakMenuRetraceQueue;   /* the scheduler's retrace queue (ControllerPakMenu's second argument) */
extern OSMesg gPakMenuRetraceMesg;   /* the last message from it */
extern s32 gPakMenuTextCursor;   /* where the next glyph is drawn: a byte address in the frame */
extern s16 gPakMenuTextX;   /* the text position */
extern s16 gPakMenuTextY;

#endif
