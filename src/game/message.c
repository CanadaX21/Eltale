#include "common.h"
#include "game/message.h"
#include "game/sound.h"
#include "game/text.h"
extern Message gMessageWindows[];         /* 4 message windows, 0x9FC each */
typedef struct {
    s16 x;
    s16 y;
    s16 width;
    s16 height;
} WindowPlacement;
extern WindowPlacement gChoiceBoxSize;     /* the message window being sized */
typedef struct {
    u8 v[8];
} QuillOffsets;                     /* per animation frame (DrawMoreTextQuill) */
typedef struct {
    s32 width;
    s32 height;
    s32 cms;
    s32 cmt;
    s32 masks;
    s32 maskt;
} BoxTexture;                       /* one window texture (DrawBoxTexture) */
typedef struct {
    BoxTexture e[5];
} BoxTextures;                      /* D_800616E4, copied to the stack */
typedef struct {
    u8* tex[5];
} BoxTextureImages;                 /* D_800616D0, copied to the stack */
extern Gfx* gDisplayListHead;

void ResetMessageWindows(s32 unused) {
    s32 i;

    for (i = 0; i != 4; i++) {
        ResetMessageWindow(i, 0);
    }
}
void ResetMessageWindow(s32 slot, u8 keepPos) {
    s32 i;
    s32 j;

    if (keepPos == 0) {
        gMessageWindows[slot].xpos = gMessageWindows[slot].ypos = 0;
        gMessageWindows[slot].flags = 0;
        gMessageWindows[slot].width = gMessageWindows[slot].height = 0;
    }
    gMessageWindows[slot].quillTimer = 1;
    gMessageWindows[slot].id = 0;
    gMessageWindows[slot].charDelay = 0;
    gMessageWindows[slot].charTimer = 0;
    gMessageWindows[slot].textPos = 0;
    gMessageWindows[slot].heldChar = 0xFF;
    gMessageWindows[slot].choice = 0;
    gMessageWindows[slot].stickHeld = 0;
    for (i = 0; i < 0x20; i++) {
        gMessageWindows[slot].name[i] = 0xFF;
    }
    for (i = 0; i < 0x4B0; i++) {
        gMessageWindows[slot].text[i] = 0xFF;
        gMessageWindows[slot].page[i] = 0xFF;
    }
    gMessageWindows[slot].lineCount = 0;
    gMessageWindows[slot].unk3A = 0;
    for (i = 0; i < 0x18; i++) {
        for (j = 0; j < 4; j++) {
            gMessageWindows[slot].lines[j][i] = 0xFF;
        }
    }
}
/* opening a window (OpenMessageWindow) */
extern s32 gMessageWindowsOpen;              /* windows opened */
extern MessageLines gMessageLines;
extern s8 D_80061660[8];             /* the character delay for each speed code, by its low three bits */
extern s8 gMessageSpeed;
extern WindowPlacement D_80061668[];   /* window placement by type */
extern u8 gSpeakerName[0x20];         /* the speaker name being set up */
extern u8 gTextBuffer[0x4B0];        /* the text being set up */
extern u8 gControllerConnected;   /* a controller is connected */
extern s32 gGameState;              /* game state flags */
s32 OpenMessageWindow(s32 type) {
    s32 found;
    s32 i;
    s32 j;                  /* the byte counter; first the decremented place (RULE-EXCEPTION R30: a local of its own, or k, takes other registers) */
    s32 k;                  /* the line */
    u8* line;

    found = -1;
    for (i = 0; i < 4; i++) {
        if (gMessageWindows[i].flags & 0x20000000) {
            j = (gMessageWindows[i].flags & 7) - 1;
            if (j > 0) {
                gMessageWindows[i].flags &= ~7;
                gMessageWindows[i].flags |= j;
                gMessageWindows[i].flags &= 0x7FFFFFFF;
            }
            gMessageWindows[i].flags |= 0x400000;
        } else if (found < 0) {
            gMessageWindows[i].flags |= 0x10000000;
            if (type < 0) {
                gMessageWindows[i].flags |= 0x40000000;
                if (type == -2) {
                    gMessageWindows[i].flags |= 0x80000;
                    gMessageWindows[i].flags &= ~0x40000000;
                }
                if (type == -3) {
                    gMessageWindows[i].flags |= 0x80000;
                }
            }
            gMessageWindows[i].flags &= ~7;
            gMessageWindows[i].flags |= 4;
            if (!(gMessageWindows[i].flags & 0x80000)) {
                for (j = 0; j < 0x20; j++) {
                    gMessageWindows[i].name[j] = gSpeakerName[j];
                }
                for (j = 0; j < 0x4B0; j++) {
                    gMessageWindows[i].text[j] = gTextBuffer[j];
                }
                for (k = 0; k < gMessageLines.lineCount; k++) {
                    line = gMessageLines.lines[k];
                    for (j = 0; j < 0x18; j++) {
                        gMessageWindows[i].lines[k][j] = line[j];
                    }
                }
                gMessageWindows[i].lineCount = gMessageLines.lineCount;
                gMessageWindows[i].unk3A = gMessageLines.unk4;
                if (type >= 0) {
                    gMessageWindows[i].id = type;
                    gMessageWindows[i].xpos = D_80061668[type].x;
                    gMessageWindows[i].ypos = D_80061668[type].y;
                    gMessageWindows[i].width = D_80061668[type].width;
                    gMessageWindows[i].height = D_80061668[type].height;
                } else {
                    gMessageWindows[i].xpos = gChoiceBoxSize.x;
                    gMessageWindows[i].ypos = gChoiceBoxSize.y;
                    gMessageWindows[i].width = gChoiceBoxSize.width;
                    gMessageWindows[i].height = gChoiceBoxSize.height;
                    gMessageWindows[i].id = 0x80;
                }
                gMessageWindows[i].charDelay = D_80061660[0] + gMessageSpeed;
                gMessageWindows[i].charTimer = D_80061660[0] + gMessageSpeed;
            }
            gMessageWindows[i].textPos = 1;
            gMessageWindows[i].heldChar = gMessageWindows[i].text[1];
            gMessageWindows[i].text[1] = 0xFF;
            found = i;
            gMessageWindowsOpen++;
        }
    }
    gGameState |= 2;
    gControllerConnected = 0;
    return found;
}

extern s8 gStickY;   /* stick y */
extern u8 gSavePromptHand;
extern s32 gMessageChoice;
extern u16 gButtonsPressed;
extern Gfx gMenuRenderDL[];
extern u16 gQuillPalette[];
void DrawDialogueBox(void) {
    typedef struct {
        s8 slot[4];
    } WindowOrder;     /* the draw order: a window index per place, -1 = none */
    extern WindowOrder D_800616B4;  /* all -1 */
    s32 i;                  /* the page-full routine's index; then the skipping run's line limit (RULE-EXCEPTION R30: one local, as the compiled code requires; a local per job makes the frame a word too big) */
    s32 place;
    u8 c;                   /* a window's kind; then the page-full routine's character; then the typed character (RULE-EXCEPTION R30: one local, as the compiled code requires; a local per job takes other registers) */
    s32 slot;
    Message* win;
    s32 limit;
    s32 x;
    s32 y;
    s32 n;                  /* the window loops' counter, the page-full and typing positions, the page clear's count and the skipping position (RULE-EXCEPTION R30: one local, as the compiled code requires; a local per job makes the frame a word too big) */
    s32 seen;               /* the page-full routine's code flags; then the skipping run's width sum (RULE-EXCEPTION R30: one local, as the compiled code requires; a local per job makes the frame a word too big) */
    WindowOrder order;
    s32 out;                /* the page-full copy's output index; then the typing speed (RULE-EXCEPTION R30: one local, as the compiled code requires; a local per job makes the frame a word too big) */
    s32 col;
    s32 quill;
    s32 resized;
    s32 open;
    s32 pauses;

    quill = 0;
    resized = 0;
    open = 0;
    pauses = 0;
    order = D_800616B4;
    gSPDisplayList(gDisplayListHead++, gMenuRenderDL);
    gDPSetBlendColor(gDisplayListHead++, 0, 0, 0, 1);
    for (n = 0; n < 4; n++) {
        c = gMessageWindows[n].flags & 7;
        switch (c) {        /* one line per case: a break on a line of its own changes how the assembler orders case 1 */
            case 1: order.slot[0] = n; break;
            case 2: order.slot[1] = n; break;
            case 3: order.slot[2] = n; break;
            case 4: order.slot[3] = n; break;
        }
        if (c != 0) {
            open++;
        }
    }
    for (place = 0; place < 4; place++) {
        gDPPipeSync(gDisplayListHead++);
        gDPSetTextureLUT(gDisplayListHead++, G_TT_RGBA16);
        gDPPipeSync(gDisplayListHead++);
        gDPLoadTLUT_pal256(gDisplayListHead++, gQuillPalette);
        slot = order.slot[place];
        if (slot < 0) {
            continue;
        }
        win = &gMessageWindows[slot];
        if (win->flags & 0x10000000) {
            /* opening */
            if (AnimateMessageWindow(slot) < 0) {
                win->flags &= ~0x10000000;
                win->flags |= 0xA8000000;
            }
        } else if (win->flags & 0x01000000) {
            /* closing: the windows above move down one place */
            if (AnimateMessageWindow(slot) < 0) {
                win->flags = 0;
                for (n = 0; n != 4; n++) {
                    c = gMessageWindows[n].flags & 7;
                    if (c != 0) {
                        c++;
                        gMessageWindows[n].flags &= ~7;
                        gMessageWindows[n].flags |= c;
                        if (c >= 4) {
                            gMessageWindows[n].flags &= ~0x00400000;
                            gMessageWindows[n].flags |= 0x80000000;
                            if (win->flags & 0x80000) {
                                ResetMessageWindow(n, 1);
                                gMessageWindows[n].flags |= 0x01000000;
                                gMessageWindows[n].flags &= ~0x80000000;
                            }
                        }
                    }
                }
                ResetMessageWindow(slot, 0);
                gMessageWindowsOpen--;
                if (open <= 0) {
                    gGameState &= ~2;
                }
            }
        } else {
            if (win->flags & 0x20000000) {
            DrawMessageBox(slot);
            if (win->flags & 0x40000000) {
                /* a choice box: one n per option, the hand on the chosen one */
                gDPPipeSync(gDisplayListHead++);
                gDPSetEnvColor(gDisplayListHead++, 0xFF, 0xFF, 0xFF, 0xCC);
                x = win->xpos + 12;
                y = win->ypos + 6;
                for (limit = 0; limit < gMessageWindows[slot].lineCount; limit++) {
                    DrawString(x, y, gMessageWindows[slot].lines[limit]);
                    y += 13;
                }
                if (win->flags & 0x80000000) {
                    if (gButtonsPressed & 0xA000) {
                        PlaySound(0, 1, 0xFF);
                        if (win->flags & 0x80000) {
                            gSavePromptHand = win->choice;
                        } else {
                            gMessageChoice = win->choice;
                        }
                        ResetMessageWindow(slot, 1);
                        win->flags |= 0x01000000;
                    } else {
                        if (gStickY > 20) {
                            if (win->stickHeld == 0) {
                                win->choice--;
                                win->stickHeld = 1;
                                PlaySound(0, 0, 0xFF);
                            }
                        } else if (gStickY < -20) {
                            if (win->stickHeld == 0) {
                                win->choice++;
                                win->stickHeld = 1;
                                PlaySound(0, 0, 0xFF);
                            }
                        } else {
                            win->stickHeld = 0;
                        }
                        if (win->choice >= win->lineCount) {
                            win->choice = 0;
                        } else if (win->choice < 0) {
                            win->choice = win->lineCount - 1;
                        }
                    }
                    if (!(win->flags & 0x01000000)) {
                        gDPPipeSync(gDisplayListHead++);
                        gDPSetEnvColor(gDisplayListHead++, 0xFF, 0xFF, 0xFF, 0xFF);
                        x = win->xpos - 2;
                        y = win->choice * 13 + win->ypos + 2;
                        gDPSetTextureLUT(gDisplayListHead++, G_TT_RGBA16);
                        gDPLoadTLUT_pal256(gDisplayListHead++, gQuillPalette);
                        DrawBoxTexture(4, x, y, 12, 12);
                    }
                }
            } else if (gMessageWindows[slot].flags & 0x80000000) {
                if (gMessageWindows[slot].flags & 0x08000000) {
                    if (gMessageWindows[slot].flags & 0x04000000) {
                        /* the page is full: wait for the button, then keep the open colour / font codes for the next page */
                        quill = 1;
                        if (gButtonsPressed & 0xA000) {
                            n = gMessageWindows[slot].textPos;
                            out = -1;
                            seen = 0;
                            gMessageWindows[slot].text[n] = gMessageWindows[slot].heldChar;
                            i = n;
                            do {
                                c = gMessageWindows[slot].text[i--];
                                if (c >= 0xC0 && c < 0xE0 && !(seen & 1)) {
                                    gMessageWindows[slot].text[++out] = c;
                                    seen |= 1;
                                }
                                if (c >= 0x80 && c < 0xA0 && !(seen & 2)) {
                                    gMessageWindows[slot].text[++out] = c;
                                    seen |= 2;
                                }
                                if (c >= 0xA0 && c < 0xB0 && !(seen & 4)) {
                                    gMessageWindows[slot].text[++out] = c;
                                    seen |= 4;
                                }
                            } while (i >= 0);
                            do {
                                gMessageWindows[slot].text[++out] = gMessageWindows[slot].text[n++];
                            } while (gMessageWindows[slot].text[out] != 0xFF);
                            gMessageWindows[slot].textPos = 1;
                            gMessageWindows[slot].heldChar = gMessageWindows[slot].text[1];
                            gMessageWindows[slot].text[1] = 0xFF;
                            gMessageWindows[slot].flags &= ~0x04000000;
                            for (n = 0; n != 0x4B0; n++) {
                                gMessageWindows[slot].page[n] = 0xFF;
                            }
                        }
                    } else {
                        /* typing: one character per unkF frames; gMessageSpeed 0 = the whole page at once */
                        out = gMessageSpeed;
                        if (out != 0) {
                            if (!(gMessageWindows[slot].flags & 0x100000) && (gButtonsPressed & 0xA000)) {
                                gMessageWindows[slot].flags |= 0x100000;
                            }
                        } else {
                            gMessageWindows[slot].flags |= 0x100000;
                        }
                        if (!(gMessageWindows[slot].flags & 0x100000)) {
                            gMessageWindows[slot].charTimer--;
                            if (gMessageWindows[slot].charTimer <= 0) {
                                gMessageWindows[slot].charTimer = gMessageWindows[slot].charDelay;
                                n = gMessageWindows[slot].textPos++;
                                gMessageWindows[slot].text[n] = gMessageWindows[slot].heldChar;
                                c = gMessageWindows[slot].heldChar;
                                gMessageWindows[slot].heldChar = gMessageWindows[slot].text[++n];
                                gMessageWindows[slot].text[n] = 0xFF;
                                switch (c) {
                                    case 0xF2:
                                        gMessageWindows[slot].flags |= 0x400000;
                                        SetMessageWindowSize(slot);
                                        resized = 1;
                                        gMessageWindows[slot].flags |= 0x40000;
                                        break;
                                    case 0xF0:
                                        if (pauses != 0) {
                                            gMessageWindows[slot].flags |= 0x04000000;
                                        }
                                        break;
                                    case 0xFF:
                                        gMessageWindows[slot].flags |= 0x02000000;
                                        gMessageWindows[slot].flags &= ~0x08000000;
                                        ResetStickHeld();
                                        break;
                                    case 0xE1:
                                    case 0xE2:
                                    case 0xE3:
                                    case 0xE4:
                                    case 0xE5:
                                    case 0xE6:
                                    case 0xE7:
                                    case 0xE8:
                                        gMessageWindows[slot].charDelay = D_80061660[c & 7] * out;
                                        gMessageWindows[slot].charTimer = D_80061660[c & 7] * out;
                                    default:
                                        pauses++;
                                        PlaySound(0, 3, 0xFF);
                                        if (c >= 0x80) {
                                            do {
                                                n = gMessageWindows[slot].textPos++;
                                                gMessageWindows[slot].text[n] = gMessageWindows[slot].heldChar;
                                                c = gMessageWindows[slot].heldChar;
                                                gMessageWindows[slot].heldChar = gMessageWindows[slot].text[++n];
                                                gMessageWindows[slot].text[n] = 0xFF;
                                            } while (c >= 0x80);
                                        }
                                        break;
                                }
                            }
                        } else {
                            /* skipping: run to the end of the page */
                            col = 0;
                            n = win->textPos;
                            gMessageWindows[slot].text[n] = gMessageWindows[slot].heldChar;
                            seen = 0;
                            i = ((win->width - 20) / 10) * 4;
                            for (;;) {
                                if (gMessageWindows[slot].text[n] & 0x80) {
                                    if (gMessageWindows[slot].text[n] == 0xE0) {
                                        seen += (win->width - 20) / 10;
                                        if (col > seen) {
                                            col -= seen;
                                            col += seen;
                                        } else {
                                            col = 0;
                                            col += seen;
                                        }
                                        if (col >= i) {
                                            win->flags |= 0x04000000;
                                            win->heldChar = win->text[++n];
                                            win->text[n] = 0xFF;
                                            win->textPos++;
                                            break;
                                        }
                                    }
                                } else {
                                    col++;
                                    if (col >= i) {
                                        win->flags |= 0x04000000;
                                        win->heldChar = win->text[++n];
                                        win->text[n] = 0xFF;
                                        win->textPos++;
                                        break;
                                    }
                                    if (col > i) {
                                        win->heldChar = win->text[++n];
                                        win->text[n] = 0xFF;
                                        win->textPos++;
                                        win->flags |= 0x04000000;
                                        break;
                                    }
                                }
                                if (gMessageWindows[slot].text[n] == 0xF2) {
                                    win->heldChar = win->text[++n];
                                    win->text[n] = 0xFF;
                                    win->textPos++;
                                    break;
                                }
                                if (gMessageWindows[slot].text[n] == 0xF0) {
                                    win->heldChar = win->text[++n];
                                    win->text[n] = 0xFF;
                                    win->textPos++;
                                    win->flags |= 0x04000000;
                                    break;
                                }
                                if (gMessageWindows[slot].text[n] == 0xFF) {
                                    win->heldChar = win->text[++n];
                                    win->text[n] = 0xFF;
                                    win->text[n + 1] = 0xFF;
                                    break;
                                }
                                n++;
                                win->textPos++;
                            }
                            switch (gMessageWindows[slot].text[n - 1]) {
                                case 0xF2:
                                    win->flags |= 0x400000;
                                    SetMessageWindowSize(slot);
                                    resized = 1;
                                    win->flags |= 0x40000;
                                    break;
                                case 0xF0:
                                    win->flags |= 0x04000000;
                                    break;
                                case 0xFF:
                                    win->flags |= 0x02000000;
                                    win->flags &= ~0x08000000;
                                    ResetStickHeld();
                                    break;
                            }
                            win->flags &= ~0x100000;
                        }
                    }
                    PageMessage(slot);
                    DrawMessageText(slot);
                } else if (win->flags & 0x02000000) {
                    /* the last page: close on the button or a held stick */
                    if ((gButtonsPressed & 0xA000) || IsStickHeld() != 0) {
                        ResetMessageWindow(slot, 1);
                        win->flags |= 0x01000000;
                    } else {
                        if (win->flags & 0x40000) {
                            win->flags &= ~0x40000;
                            ResetMessageWindow(slot, 1);
                            win->flags |= 0x01000000;
                        }
                        DrawMessageText(slot);
                    }
                }
            } else {
                DrawMessageText(slot);
            }
            }
            if (quill != 0) {
                DrawMoreTextQuill(slot);
            }
        }
    }
    if (resized != 0) {
        if (gMessageWindows[slot].flags & 0x80000) {
            OpenMessageWindow(-3);
        } else {
            OpenMessageWindow(-1);
        }
    }
    gDPSetTextureLUT(gDisplayListHead++, G_TT_NONE);
    gDPPipeSync(gDisplayListHead++);
    gDPSetBlendColor(gDisplayListHead++, 0, 0, 0, 1);
    gDPPipeSync(gDisplayListHead++);
}

s32 AnimateMessageWindow(s32 slot) {
    Message* msg;
    s32 ret;
    s16 cx;
    s16 cy;
    s32 w;
    s32 h;
    s32 x;
    s32 y;
    u32 steps;

    msg = &gMessageWindows[slot];
    ret = 0;
    cx = msg->xpos + (msg->width >> 1);
    cy = msg->ypos + (msg->height >> 1);
    if (msg->flags & 0x10000000) {
        steps = 10;
        w = msg->width / steps;
        w = msg->quillTimer * w;
        h = msg->height / steps;
        h = msg->quillTimer * h;
    } else {
        steps = 8;
        w = msg->width / 8U;
        w = (9 - msg->quillTimer) * w;
        h = msg->height / 8U;
        h = (9 - msg->quillTimer) * h;
    }
    x = cx - (w >> 1);
    y = cy - (h >> 1);
    DrawBoxTexture(0, x, y, w, h);
    DrawBoxTexture(3, x + w, y + 3, 3, h);
    DrawBoxTexture(3, x + 3, y + h, w, 3);
    DrawBoxTexture(3, x, y, w, 1);
    DrawBoxTexture(3, x, y + h - 1, w, 1);
    DrawBoxTexture(3, x, y, 1, h);
    DrawBoxTexture(3, x + w - 1, y, 1, h);
    msg->quillTimer++;
    if (steps < msg->quillTimer) {
        ret = -1;
    }
    return ret;
}
void DrawMessageText(s32 slot) {
    s32 x;
    s32 y;

    gDPPipeSync(gDisplayListHead++);
    gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, 204);
    if (gMessageWindows[slot].name[0] != 0xFF) {
        x = gMessageWindows[slot].xpos + 27;
        y = gMessageWindows[slot].ypos + 4;
        DrawBoxTexture(1, x, y, 40, 13);
        x = gMessageWindows[slot].xpos + 73;
        y = gMessageWindows[slot].ypos + 4;
        DrawString(x, y, gMessageWindows[slot].name);
        x = gMessageWindows[slot].xpos + 10;
        y = gMessageWindows[slot].ypos + 20;
        DrawString(x, y, gMessageWindows[slot].page);
    } else {
        x = gMessageWindows[slot].xpos + 10;
        y = gMessageWindows[slot].ypos + 12;
        DrawString(x, y, gMessageWindows[slot].page);
    }
}

void DrawMessageBox(s32 slot) {
    Message* msg;
    s32 x;
    s32 y;

    gDPPipeSync(gDisplayListHead++);
    gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, 204);
    msg = &gMessageWindows[slot];
    DrawBoxTexture(0, msg->xpos, msg->ypos, msg->width, msg->height);
    DrawBoxTexture(3, msg->width + msg->xpos, msg->ypos + 2, 2, msg->height);
    DrawBoxTexture(3, msg->xpos + 2, msg->height + msg->ypos, msg->width, 2);
    gDPPipeSync(gDisplayListHead++);
    gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, 255);
    x = msg->xpos;
    y = msg->ypos;
    DrawBoxTexture(3, x, y, msg->width, 1);
    DrawBoxTexture(3, x, msg->height + y - 1, msg->width, 1);
    DrawBoxTexture(3, x, y, 1, msg->height);
    DrawBoxTexture(3, msg->width + x - 1, y, 1, msg->height);
}
/* Fills the window's page buffer from its text: up to `rows` rows of `cols` glyphs. In the text, bytes below 0x80
 * are glyphs; 0xE0 ends a row, 0xFF ends the text, and 0xF0 ends the page once a glyph is on it. A row that runs
 * out of columns takes the control codes after it through an 0xE0 among them (and the byte after that), or gets
 * an 0xE0 of its own. A page that ends inside the text is marked (0x04000000) and terminated; when a character
 * is held from the previous page, it is put back and the character after this page is held instead. */
void PageMessage(s32 slot) {
    Message* msg;
    s32 cols;
    s32 n;
    s32 out;
    s32 in;
    s32 row;
    s32 col;
    s32 rows;
    s32 j;
    s32 wrap;
    s32 k;

    msg = &gMessageWindows[slot];
    cols = (msg->width - 20) / 10 + 1;
    n = 0;
    rows = (msg->height - 20) / 13;
    out = -1;
    in = -1;
    row = 0;
    while (row++ < rows) {
        for (col = 0; col < cols;) {
            gMessageWindows[slot].page[++out] = gMessageWindows[slot].text[++in];
            if (gMessageWindows[slot].page[out] & 0x80) {
                if (gMessageWindows[slot].page[out] == 0xE0 || gMessageWindows[slot].page[out] == 0xFF) {
                    break;
                }
                if (gMessageWindows[slot].page[out] == 0xF0 && n != 0) {
                    break;
                }
            } else {
                col++;
                n++;
            }
        }
        if (gMessageWindows[slot].page[out] == 0xFF) {
            break;
        }
        if (gMessageWindows[slot].page[out] == 0xF0) {
            msg->flags |= 0x04000000;
            break;
        }
        if (gMessageWindows[slot].page[out] == 0xE0) {
            continue;
        }
        if (gMessageWindows[slot].page[out] != 0xE0) { /* always true here: the original tests it again */
            wrap = -1;
            j = in;
            while (gMessageWindows[slot].text[++j] >= 0x80) {
                if (gMessageWindows[slot].text[j] == 0xE0) {
                    for (k = j - in; k > 0; k--) {
                        gMessageWindows[slot].page[++out] = gMessageWindows[slot].text[++in];
                    }
                    gMessageWindows[slot].page[++out] = gMessageWindows[slot].text[++in];
                    wrap = 0;
                    break;
                }
            }
            if (wrap != 0) {
                gMessageWindows[slot].page[++out] = 0xE0;
            }
        }
    }
    if (gMessageWindows[slot].page[out] != 0xFF && gMessageWindows[slot].page[out] != 0xF0) {
        msg->flags |= 0x04000000;
        msg->page[out + 1] = 0xFF;
        if (msg->heldChar != 0xFF) {
            /* RULE-EXCEPTION R30: the page index is reused for the saved text position; a separate local takes
             * another register and moves the held character and the 0xFF store with it */
            out = msg->textPos;
            msg->text[out] = msg->heldChar;
            msg->textPos = in + 1;
            msg->heldChar = msg->text[in + 1];
            msg->text[in + 1] = 0xFF;
        } else {
            msg->flags |= 0x02000000;
            msg->flags &= ~0x08000000;
            ResetStickHeld();
        }
    }
}

extern QuillOffsets gQuillLift;
extern QuillOffsets gQuillWidth;
extern QuillOffsets gQuillShift;
void DrawMoreTextQuill(s32 slot) {
    s32 x;
    s32 y;
    s32 frame;
    QuillOffsets lift;
    QuillOffsets width;
    QuillOffsets shift;

    lift = gQuillLift;
    width = gQuillWidth;
    shift = gQuillShift;
    gDPPipeSync(gDisplayListHead++);
    gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, 255);
    gDPPipeSync(gDisplayListHead++);
    gDPSetTextureLUT(gDisplayListHead++, G_TT_RGBA16);
    gDPPipeSync(gDisplayListHead++);
    gDPLoadTLUT_pal256(gDisplayListHead++, gQuillPalette);
    gMessageWindows[slot].quillTimer++;
    frame = (gMessageWindows[slot].quillTimer >> 1) & 7;
    x = gMessageWindows[slot].xpos + gMessageWindows[slot].width - 11;
    y = gMessageWindows[slot].ypos + gMessageWindows[slot].height - 3;
    x = x - shift.v[frame];
    DrawBoxTexture(3, x, y, width.v[frame], 1);
    x = gMessageWindows[slot].xpos + gMessageWindows[slot].width - 12;
    y = gMessageWindows[slot].ypos + gMessageWindows[slot].height - 15;
    x = lift.v[frame] + x;
    y = y - lift.v[frame];
    DrawBoxTexture(2, x, y, 12, 12);
}

void SetMessageWindowSize(s32 slot) {
    s32 longest;
    s32 i;
    s32 len;
    s32 j;
    s16 lines;

    longest = 0;
    lines = gMessageWindows[slot].lineCount;
    gChoiceBoxSize.height = lines * 13 + 12;
    for (i = 0; i < lines; i++) {
        j = 0;
        len = 0;
        while (gMessageWindows[slot].lines[i][j] != 0xFF) {
            if (gMessageWindows[slot].lines[i][j] < 0x80) {
                len++;
            }
            j++;
        }
        if (longest < len) {
            longest = len;
        }
    }
    gChoiceBoxSize.width = longest * 10 + 0x18;
}
s32 FindMessageWindow(s32 id, s32 type) {
    Message* win;
    s32 i;
    s32 ret = -1;

    if (id >= 0) {
        for (i = 0, win = gMessageWindows; i < 4; i++, win++) {
            if (id == win->id) {
                ret = i;
                break;
            }
        }
    } else {
        for (i = 0, win = gMessageWindows; i < 4; i++, win++) {
            if ((win->flags & 7) == type) {
                ret = i;
                break;
            }
        }
    }
    return ret;
}
extern BoxTextureImages D_800616D0;
extern BoxTextures D_800616E4;
void DrawBoxTexture(s32 texture, s32 x, s32 y, s32 w, s32 h) {
    s32 i;
    BoxTextureImages images = D_800616D0;
    BoxTextures textures = D_800616E4;

    i = texture;
    gDPPipeSync(gDisplayListHead++);
    gDPLoadTextureTile(gDisplayListHead++, images.tex[i], G_IM_FMT_CI, G_IM_SIZ_8b, textures.e[i].width,
                       textures.e[i].height, 0, 0, textures.e[i].width - 1, textures.e[i].height - 1, 0,
                       textures.e[i].cms, textures.e[i].cmt, textures.e[i].masks, textures.e[i].maskt, G_TX_NOLOD, G_TX_NOLOD);
    gDPPipeSync(gDisplayListHead++);
    if (texture == 0) {
        gSPTextureRectangle(gDisplayListHead++, x << 2, y << 2, (x + w) << 2, (y + h) << 2, G_TX_RENDERTILE,
                            (x & 0xF) << 5, (y & 0xF) << 5, 1 << 10, 1 << 10);
    } else {
        gSPTextureRectangle(gDisplayListHead++, x << 2, y << 2, (x + w) << 2, (y + h) << 2, G_TX_RENDERTILE, 0, 0,
                            1 << 10, 1 << 10);
    }
    if (texture >= 3) { /* a clamp to the last of four textures; nothing reads i after it */
        i = 3;
    } else {
        i = texture;
    }
    gDPPipeSync(gDisplayListHead++);
}

void DrawMenuBox(s32 x, s32 y, s32 w, s32 h) {
    gDPPipeSync(gDisplayListHead++);
    gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, 204);
    DrawBoxTexture(0, x, y, w, h);
    DrawBoxTexture(3, x + w, y + 2, 2, h);
    DrawBoxTexture(3, x + 2, y + h, w, 2);
    gDPPipeSync(gDisplayListHead++);
    gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, 255);
    DrawBoxTexture(3, x, y, w, 1);
    DrawBoxTexture(3, x, y + h - 1, w, 1);
    DrawBoxTexture(3, x, y, 1, h);
    DrawBoxTexture(3, x + w - 1, y, 1, h);
}
extern u16 gStickHoldFrames;  /* frames the stick has been held past radius 60 */
extern s8 gStickX;   /* stick x (src/game/controller.c) */
void ResetStickHeld(void) {
    gStickHoldFrames = 0;
}
s32 IsStickHeld(void) {
    s32 held = 0;
    u32 dist;

    if (gControllerConnected != 0) {
        dist = gStickX * gStickX + gStickY * gStickY;
        if (dist > 60 * 60) {
            gStickHoldFrames++;
            if (gStickHoldFrames >= 5) {
                held = 1;
            }
        } else {
            gStickHoldFrames = 0;
        }
    }
    return held;
}
