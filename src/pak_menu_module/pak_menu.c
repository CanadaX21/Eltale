#include "common.h"
#include "game/pak_menu.h"
/* The boot-time Controller Pak manager (the Controller Pak menu): mainproc copies this module to 0x80100000 (the RAM
   gameplay_common takes later; the module's .bss is neither copied nor cleared) and calls ControllerPakMenu before the
   scheduler starts. With no controller it returns at once; otherwise it reads the pads itself, and returns when the
   game's controller (gControllerChannel) has no pak, holds a Rumble Pak or a Transfer Pak (told apart after
   osPfsInitPak's PFS_ERR_ID_FATAL), or gives another pak error. With Start held at power-on, or with a memory pak whose
   ID area is damaged, it runs its own menu (eleven states: docs/SAVE.md), drawn by the CPU into two framebuffers (0x80200000 and
   0x80225800, cleared first; the scheduler is not running): the pak's notes of every game with their names,
   extensions and pages, the free pages, a note's deletion, and the repair of a damaged pak (osPfsRepairId); then it
   returns to mainproc, which does not read the result (0 at once or on a pak error, 2 a good pak without Start, 3
   left by the player).
   Its twin in Quest 64 lives in Quest's main code (Mallos31/Quest, src/328F0.c): the number, glyph, string, image,
   sprite-list and note-row drawing and the frame swap match it function for function. Eltale's own: the reset check in
   PakMenu_SwapFrame and PakMenu_DrawText (the game's text codes, drawn by the CPU), which the dialogs use. C: every
   function but the entry and PakMenu_DrawText, which stay the original assembly. The module is built -O2 -g3: Quest's
   matched C for its twins matches only there. */





/* N as three glyphs of the pak's character set (999 at most): a leading zero is a blank (glyph 15), digits are 16-25 */
void PakMenu_FormatNumber(u16 n, u8* digits) {
    if (n >= 1000) {
        n = 999;
    }
    digits[0] = (n / 100) + (n >= 100) + 15;
    digits[1] = ((n % 100) / 10) + (n >= 10) + 15;
    digits[2] = (n % 10) + 16;
}

/* one glyph at the text cursor in the current palette; colour 0 is not drawn. Code 1 is a narrow space, 0xA a new line
   (13 pixels down); a code under 15 or above gPakMenuLastGlyph draws glyph 0x3B. The sheet keeps eight glyphs of 8 x 12
   pixels to a row. */
void PakMenu_DrawGlyph(s32 glyph) {
    u8 pixel;
    u16* dst;
    s32 i;
    s32 j;
    u32* src;

    if (glyph == 1) {
        gPakMenuTextCursor += 6;
        return;
    }
    if (glyph == 0xA) {
        gPakMenuTextY += 0xD;
        PakMenu_SetTextPos(gPakMenuTextX, gPakMenuTextY);
        return;
    }
    if (glyph < 0xF || gPakMenuLastGlyph < glyph) {
        glyph = 0x3B;
    }
    glyph -= 0xF;
    src = (u32*)((((glyph & 0xF8) * 0x60) / 2) + (((glyph & 7) * 8) / 2) + (s32)gPakMenuGlyphs);
    dst = (u16*)gPakMenuTextCursor;
    for (i = 0; i < 0xC; i++) {
        for (j = 28; j >= 0; j -= 4, dst++) {
            pixel = ((*src) >> j) % 0x10;
            if (pixel != 0) {
                *dst = gPakMenuPalette[pixel];
            }
        }
        dst += 0x138;
        src += 0x8;
    }
    gPakMenuTextCursor += 0x12;
}

/* the frame is 320 pixels wide, two bytes a pixel */
void PakMenu_SetTextPos(s16 x, s16 y) {
    gPakMenuTextX = x;
    gPakMenuTextY = y;
    gPakMenuTextCursor = (x + y * 0x140) * 2 + gPakMenuDrawBuffer;
}

/* empty, and nothing calls it; Quest 64's module has its string drawing here (func_80031F74) */
void func_80100284_DA47F4(void) {
}

void PakMenu_DrawGlyphs(u8* glyphs, u32 count) {
    while (count-- && *glyphs) {
        PakMenu_DrawGlyph(*glyphs++);
    }
}

/* 1 the list's colours, 3 the highlighted note row's */
void PakMenu_SetPalette(s32 palette) {
    gPakMenuPalette = &gPakMenuPalettes[palette * 4];
}

/* mode 5: one byte a pixel, every pixel drawn; mode 2: two pixels a byte (high nibble first), colour 0 not drawn */
void PakMenu_DrawImage(s16 x, s16 y, s16 w, s16 h, u8* image, u16* palette, u8 mode) {
    u16* dst;
    u8* src;
    s32 i;
    s32 j;
    u8 c;
    u16 hi;
    u16 lo;

    dst = (u16*)((x + y * 0x140) * 2 + gPakMenuDrawBuffer);
    src = image;
    if (mode == 5) {
        for (i = 0; i < h; i++) {
            for (j = 0; j < w; j++) {
                *dst++ = palette[*src++];
            }
            dst += 0x140 - w;
        }
    } else if (mode == 2) {
        for (i = 0; i < h; i++) {
            for (j = 0; j < w; j += 2) {
                c = *src++;
                hi = c >> 4;
                lo = c & 0xF;
                if (hi != 0) {
                    *dst++ = palette[hi];
                } else {
                    dst++;
                }
                if (lo != 0) {
                    *dst++ = palette[lo];
                } else {
                    dst++;
                }
            }
            dst += 0x140 - w;
        }
    }
}

void PakMenu_DrawSprites(PakSprite* list, s32 count) {
    s32 i;
    PakSprite* s;

    i = 0;
    if (count > 0) {
        s = list;
        do {
            PakMenu_DrawImage(s->x, s->y, s->w, s->h, s->image, s->palette, 2);
            i++;
            s++;
        } while (i != count);
    }
}

/* waits for the next retrace; the pre-NMI message (0x29D, the reset button) blanks the screen, else a drawn frame is
   shown and the other buffer becomes the one drawn */
void PakMenu_SwapFrame(void) {
    osWritebackDCacheAll();
    osRecvMesg(gPakMenuRetraceQueue, &gPakMenuRetraceMesg, OS_MESG_BLOCK);
    if ((s32)gPakMenuRetraceMesg == 0x29D) {
        osViBlack(TRUE);
    } else if (gPakMenuFrameReady != 0) {
        osViSwapBuffer((void*)gPakMenuFrameBuffers[gPakMenuFrameCount & 1]);
        gPakMenuDrawBuffer = gPakMenuFrameBuffers[++gPakMenuFrameCount & 1];
        gPakMenuFrameReady = 0;
    }
}

/* one row of the note list, 13 pixels apart from y 88: the row's number, and for a note in use its name, its extension
   after a dot and its pages; the row the hand is on is highlighted while a note is chosen */
void PakMenu_DrawNoteRow(s32 row) {
    u8 digits[3];
    s16 y;

    gPakMenuLastGlyph = 0x94;
    if (row == gPakMenuNoteRow && gPakMenuState >= 2 && gPakMenuState < 5) {
        PakMenu_SetPalette(3);
    } else {
        PakMenu_SetPalette(1);
    }
    if (gPakMenuNoteErrors[row + gPakMenuScrollAmt] == 0) {
        y = row * 13 + 0x58;
        PakMenu_SetTextPos(0x52, y);
        PakMenu_DrawGlyphs((u8*)gPakMenuNotes[row + gPakMenuScrollAmt].game_name, 16);
        /* RULE-EXCEPTION R19: the SDK's ext_name is char; the pak's character codes are read unsigned */
        if ((u8)gPakMenuNotes[row + gPakMenuScrollAmt].ext_name[0] != 0) {
            PakMenu_DrawGlyph(0x3C);
            /* RULE-EXCEPTION R36: the cursor is loaded and stored back unchanged, as in Quest 64's func_8003232C (a
               shift of 0 in the original; the bytes need the load and the store) */
            gPakMenuTextCursor = gPakMenuTextCursor;
            PakMenu_DrawGlyph((u8)gPakMenuNotes[row + gPakMenuScrollAmt].ext_name[0]);
        }
        PakMenu_SetTextPos(0x100, y);
        PakMenu_FormatNumber((gPakMenuNotes[row + gPakMenuScrollAmt].file_size + 0xFF) >> 8, digits);
        PakMenu_DrawGlyphs(digits, 3);
    }
    y = row * 13 + 0x58;
    if (gPakMenuScrollAmt + row < 9) {
        PakMenu_SetTextPos(0x3C, y);
        PakMenu_DrawGlyph(gPakMenuScrollAmt + row + 0x11);
    } else {
        PakMenu_SetTextPos(0x37, y);
        PakMenu_DrawGlyph(0x11);
        PakMenu_SetTextPos(0x3D, y);
        PakMenu_DrawGlyph(gPakMenuScrollAmt + row + 7);
    }
    PakMenu_SetTextPos(0x43, y);
    PakMenu_DrawGlyph(0x3E);
    gPakMenuLastGlyph = 0x62;
}

void PakMenu_DrawFreePages(u16 pages) {
    u8 digits[3];

    PakMenu_SetPalette(1);
    PakMenu_FormatNumber(pages, digits);
    PakMenu_SetTextPos(0xC8, 0xAD);
    PakMenu_DrawGlyphs(digits, 3);
}

void PakMenu_DrawHand(void) {
    PakMenu_DrawImage(gPakMenuHandX, gPakMenuHandY, 0x10, 0xE, gPakMenuHandImage, gPakMenuHandPalette, 2);
}

/* the screen, then a message window over it with MESSAGE in the game's font */
void PakMenu_DrawDialog(u8* message) {
    PakMenu_DrawScreen();
    PakMenu_DrawSprites(gPakMenuDialogWindow, 0x4B);
    PakMenu_DrawSprites(gPakMenuDialogChoices, 0x16);
    PakMenu_DrawText(0x34, 0x4E, message);
}

/* one frame: the screen, the free pages, the scroll arrows the list needs (16 notes, six rows), the six note rows */
void PakMenu_DrawScreen(void) {
    s32 row;

    PakMenu_DrawSprites(gPakMenuScreenSprites, 0xEB);
    PakMenu_DrawFreePages(gPakMenuFreePages);
    if (gPakMenuScrollAmt != 0) {
        PakMenu_DrawSprites(&gPakMenuArrowUp, 1);
    }
    if (gPakMenuScrollAmt < 0xA) {
        PakMenu_DrawSprites(&gPakMenuArrowDown, 1);
    }
    row = 0;
    do {
        PakMenu_DrawNoteRow(row);
        row++;
    } while (row != 6);
    gPakMenuFrameReady = 1;
}

#pragma GLOBAL_ASM("asm/nonmatchings/pak_menu_module/pak_menu/ControllerPakMenu.s")

#pragma GLOBAL_ASM("asm/nonmatchings/pak_menu_module/pak_menu/PakMenu_DrawText.s")
