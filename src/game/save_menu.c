#include "common.h"
/* The save / load menu (split from sound.c: the original is two files; Quest 64 splits its twin
   code at the same function, and this file's .rodata starts with its own string) */
extern Gfx* gDisplayListHead;
extern Gfx gMenuRenderDL[];
extern s8 D_8008FB81[];
typedef struct {
    BrianData1 stats;               /* 0x00: gBrianStats */
    u16 playTime;                   /* 0x38: minutes (gPlayTime) */
    u16 dayCount;                   /* 0x3A: gDayCount (counts up to 9999) */
    s32 map;                        /* 0x3C: where the save was made (the save list names it, DrawSaveList) */
    s32 room;                       /* 0x40 */
    s32 returnMap;                  /* 0x44: the Return destination (gReturnDest-D8) */
    s32 returnRoom;
    s32 returnEntrance;
    s32 exitMap;                    /* 0x50: the Exit destination (gExitDest-E4) */
    s32 exitRoom;
    s32 exitEntrance;
    u16 respawnMap;                 /* 0x5C: gSavedMap-B0 */
    u16 respawnRoom;
    u16 respawnEntrance;
    s8 messageSpeed;                /* gMessageSpeed */
    s8 musicVolume;                 /* gMusicVolumeSetting */
    u16 cleared;                    /* gGameState bit 0x20000: the game has been finished */
} SaveSlot;                         /* 0x68 */
typedef struct {
    u8 c[14];
} SaveListText;                     /* a short game-encoded text, 0xFF-terminated (DrawSaveList copies one to the stack) */
extern s8 gSaveSlots[];   /* per save slot: [0] pak file, [1] its letter */
extern s8 gControllerChannel;
u8 DeleteSaveNote(s32 channel, u8 ext0);
extern s32 gSavePakError;
s32 PakErrorCode(s32);
extern u16 gMenuState;
extern u16 gGameMode;
extern u16 gSaveMenuState;
extern u8 gSavePromptHand;
extern s8 gSaveSlotCursor;
extern s8 gSaveInputDelay;
extern u8 gSavePrompt;                /* the save menu's message box (index into gSavePromptBoxes / gLoadScreenTexts / gSaveScreenTexts) */
extern u8 gSaveListRepeat;                /* the save list's stick repeat counter (DrawSaveList) */
extern u8 gPakRepairCount;
/* the save menu's step, its low three bits (the macro is why the target has a write of the state to itself on step 0) */
#define SET_SAVE_STATE(n) gSaveMenuState &= ~7; gSaveMenuState |= (n)
void EnterSaveMode(s32 returnMode) {
    gGameMode = MODE_PAUSED;
    gMenuState = gMenuState | returnMode | 0x4000;
    gSaveMenuState = 0x1000;
    gSavePromptHand = 0xFF;
    gSaveSlotCursor = 0;
    gSaveInputDelay = 0;
    gSavePrompt = 0;
    gSaveListRepeat = 0;
    gPakRepairCount = 0;
}
extern u8 gSaveMessage;                /* the save menu's current message (index into gSaveMessageBoxes / gSaveMessageTexts) */
typedef struct {
    s16 x;
    s16 y;
    s16 w;
    s16 h;
} MenuBox;
extern MenuBox gSaveMessageBoxes[];         /* the save menu's message boxes */
extern char* gSaveMessageTexts[];           /* their texts */
extern SaveSlot gSaveList[];          /* the save list: one record per pak save */
extern BrianData1 gBrianStats;
extern u16 gPlayTime;
extern u16 gDayCount;
extern s32 gCurrentMap;
extern s32 gCurrentSubmap;
extern s32 gReturnDest;
extern s32 gReturnRoom;
extern s32 gReturnEntrance;
extern s32 gExitDest;
extern s32 gExitRoom;
extern s32 gExitEntrance;
extern u16 gSavedMap;
extern u16 gSavedSubmap;
extern u16 gSavedEntrance;
extern s8 gMessageSpeed;
extern s8 gMusicVolumeSetting;
extern s32 gGameState;
extern u8 gObjectsOpened[];
extern u8 gSpiritsFound[];
extern u8 gInventory[];
extern u8 gStoryFlags[];
void DrawMenuSprite(s32 sprite, s32 x, s32 y, s32 w, s32 h);
void DrawString(s32 x, s32 y, u8* text);
void DrawSaveListFrame(void);
void OpenSaveMenu(void);
void SaveMenuPrompt(void);
void DrawSaveList(s32 arg0);
void DeleteSaveSlot(void);
void LoadSaveSlot(void);
void SaveToPak(void);
void RepairSaveSlot(void);
void SaveMenuWait(void);
void DrawSaveMessage(void);
void SaveMenu(void) {
    DrawSaveListFrame();
    if (!(gSaveMenuState & 0x4000)) {
        if (gSaveMenuState & 0x2000) {
            OpenSaveMenu();
            gSaveMenuState |= 0x4000;
            gSaveMenuState &= ~0x2000;
            gPakRepairCount = 0;
        } else {
            gSaveMenuState |= 0x2000;
            gSaveMessage = 0;
        }
    }
    if (gSaveMenuState & 0x4000) {
        switch (gSaveMenuState & 7) {
            case 5:
                SaveMenuPrompt();
                break;
            case 0:
                DrawSaveList(0);
                break;
            case 4:
                DeleteSaveSlot();
                break;
            case 2:
                LoadSaveSlot();
                break;
            case 3:
                SaveToPak();
                break;
            case 1:
                RepairSaveSlot();
                break;
            case 6:
                SaveMenuWait();
                break;
        }
    }
    if (gSaveMenuState & 0x2000) {
        DrawSaveMessage();
    }
    if (gSaveMenuState & 0x8000) {
        gMenuState &= ~0x8000;
    }
}
void EndMenuDraw(void);
void DrawSaveMessage(void) {
    s32 x;
    s32 y;
    s32 w;
    s32 h;

    x = gSaveMessageBoxes[gSaveMessage].x;
    y = gSaveMessageBoxes[gSaveMessage].y;
    w = gSaveMessageBoxes[gSaveMessage].w;
    h = gSaveMessageBoxes[gSaveMessage].h;
    DrawMenuSprite(0, x, y, w, h);
    DrawMenuSprite(0x37, x + 3, y + h, w, 3);
    DrawMenuSprite(0x37, x + w + 1, y + 6, 2, h - 5);
    DrawMenuSprite(0x37, x - 1, y - 1, w + 2, 1);
    DrawMenuSprite(0x37, x - 1, y + h, w + 2, 1);
    DrawMenuSprite(0x37, x - 1, y, 1, h);
    DrawMenuSprite(0x37, x + w, y, 1, h);
    DrawMenuSprite(0x38, x + 1, y + 1, w - 3, 1);
    DrawMenuSprite(0x38, x + 1, y + 2, 1, h - 4);
    DrawMenuSprite(0x39, x + 1, y + h - 2, w - 2, 1);
    DrawMenuSprite(0x39, x + w - 2, y + 1, 1, h - 3);
    DrawString(x + 6, y + 6, gSaveMessageTexts[gSaveMessage]);
    EndMenuDraw();
}
extern OSMesgQueue gSiMesgQueue;
extern OSPfs gPfs[];
extern PakNoteExts gPakNotes[];
s32 func_8003D554(OSMesgQueue *mq, OSPfs *pfs, s32 channel);
s32 func_8003D6B0(OSMesgQueue *mq, OSPfs *pfs, s32 channel);
s32 ScanControllerPak(s32 channel);
void ScanSaveList(void);
void OpenSaveMenu(void) {
    s32 mode;
    s32 err;
    PakNoteExts* pak;

    mode = gMenuState & 0xC;
    if (gSaveMenuState & 0x1000) {
        err = osPfsInitPak(&gSiMesgQueue, &gPfs[gControllerChannel], gControllerChannel);
        if (err == 2) {
            err = 0;
        } else if (err == 10) {
            if (func_8003D554(&gSiMesgQueue, &gPfs[gControllerChannel], gControllerChannel) == 0) {
                err = 1;
            } else if (func_8003D6B0(&gSiMesgQueue, &gPfs[gControllerChannel], gControllerChannel) == 0) {
                err = 1;
            } else {
                err = 10;
            }
        }
        gSaveMenuState &= ~0x1000;
    }
    if (err == 0) {
        err = ScanControllerPak(gControllerChannel);
    }
    if (err != 0) {
        gSavePakError = err;
        if (-PakErrorCode(err) == 1) {
            if (mode == 4) {
                gSavePrompt = 0xD;
                gSaveInputDelay = 10;
            } else {
                gSavePrompt = 8;
                gSaveInputDelay = 10;
            }
        } else {
            gSavePrompt = 0x11;
            gSaveInputDelay = 10;
        }
        SET_SAVE_STATE(5);
        return;
    }
    if (mode == 4) {
        pak = &gPakNotes[gControllerChannel];
        if (pak->freePages < 2) {
            if (pak->flags & 0x4000) {
                gSavePrompt = 0x15;
            } else {
                gSavePrompt = 0x14;
            }
            gSaveInputDelay = 10;
            SET_SAVE_STATE(5);
            return;
        }
        if (pak->flags & 0x2000) {
            if (pak->flags & 0x4000) {
                gSavePrompt = 0x15;
                gSaveSlotCursor = 0;
                gSaveMenuState &= ~0x20;
            } else {
                gSavePrompt = 0x14;
            }
            gSaveInputDelay = 10;
            SET_SAVE_STATE(5);
            return;
        }
    } else {
        if (gPakRepairCount != 0) {
            pak = &gPakNotes[gControllerChannel];
            if (pak->freePages < 2 || (pak->flags & 0x2000)) {
                gSavePrompt = 0x18;
                gSaveInputDelay = 10;
                ScanSaveList();
                SET_SAVE_STATE(5);
                gSaveSlotCursor = 0;
                gSaveMenuState &= ~0x20;
                return;
            }
        }
        if (!(gPakNotes[gControllerChannel].flags & 0x4000)) {
            if (mode == 0) {
                gSavePrompt = 9;
                gSaveInputDelay = 10;
            } else {
                gSavePrompt = 10;
                gSaveInputDelay = 10;
            }
            SET_SAVE_STATE(5);
            return;
        }
    }
    ScanSaveList();
    SET_SAVE_STATE(0);
    gSaveSlotCursor = 0;
    gSaveMenuState &= ~0x20;
}
extern MenuBox gSavePromptBoxes[];         /* the save menu's message boxes, by message */
extern char* gLoadScreenTexts[];           /* message texts on the load screen (gMenuState & 4); [0..3] also the Yes / No texts */
extern char* gSaveScreenTexts[];           /* message texts on the save screen */
extern s16 gYesNoHandPositions[][2];          /* the Yes / No hand positions: [0..1] one box, [2..3] the other */
extern s8 gStickY;                /* stick y */
extern u16 gButtonsPressed;               /* buttons pressed */
void SaveMenuPrompt(void) {
    s32 x;
    s32 y;
    s32 w;
    s32 h;
    char* text;

    if (gSavePrompt == 4 || gSavePrompt == 5 || gSavePrompt == 0x16 || gSavePrompt == 0x17) {
        DrawSaveList(1);
    }
    x = gSavePromptBoxes[gSavePrompt].x;
    y = gSavePromptBoxes[gSavePrompt].y;
    w = gSavePromptBoxes[gSavePrompt].w;
    h = gSavePromptBoxes[gSavePrompt].h;
    if (gMenuState & 4) {
        text = gLoadScreenTexts[gSavePrompt];
    } else {
        text = gSaveScreenTexts[gSavePrompt];
    }
    DrawMenuSprite(0, x, y, w, h);
    DrawMenuSprite(0x37, x + 3, y + h, w, 3);
    DrawMenuSprite(0x37, x + w + 1, y + 6, 2, h - 5);
    DrawMenuSprite(0x37, x - 1, y - 1, w + 2, 1);
    DrawMenuSprite(0x37, x - 1, y + h, w + 2, 1);
    DrawMenuSprite(0x37, x - 1, y, 1, h);
    DrawMenuSprite(0x37, x + w, y, 1, h);
    DrawMenuSprite(0x38, x + 1, y + 1, w - 3, 1);
    DrawMenuSprite(0x38, x + 1, y + 2, 1, h - 4);
    DrawMenuSprite(0x39, x + 1, y + h - 2, w - 2, 1);
    DrawMenuSprite(0x39, x + w - 2, y + 1, 1, h - 3);
    DrawString(x + 6, y + 6, text);
    EndMenuDraw();
    if (gSaveInputDelay >= 0) {
        gSaveInputDelay--;
    }
    if (gSaveInputDelay < 0 && !(gSaveMenuState & 0x800)) {
        if (gButtonsPressed & A_BUTTON) {
            switch (gSavePrompt) {
                case 8:
                case 9:
                case 10:
                case 11:
                case 12:
                case 16:
                case 20:
                    gSaveMenuState |= 0x8000;
                    PlaySound(0, 1, 0xFF);
                    break;
                case 24:
                    gSaveMenuState = 0x1000;
                    break;
            }
        }
        if (gButtonsPressed & (A_BUTTON | B_BUTTON)) {
            switch (gSavePrompt) {
                case 19:
                    gSaveMenuState &= ~0x2000;
                    gSaveMenuState &= ~7;
                    gSaveMenuState &= ~0x4000;
                    gSaveMenuState &= ~0x20;
                    gSaveSlotCursor = 0;
                    gSaveMenuState |= 0x1000;
                    PlaySound(0, 1, 0xFF);
                    break;
                case 17:
                    gSaveMenuState |= 0x10;
                    gSaveMenuState |= 0x800;
                    gSavePromptHand = 0;
                    gSaveInputDelay = 10;
                    PlaySound(0, 1, 0xFF);
                    break;
                case 4:
                case 5:
                case 6:
                case 7:
                case 13:
                case 14:
                case 15:
                case 21:
                case 22:
                case 23:
                    gSaveMenuState &= ~0x10;
                    gSaveMenuState |= 0x800;
                    if (gSavePrompt == 4 || gSavePrompt == 5 || gSavePrompt == 6) {
                        gSavePromptHand = 0;
                    } else {
                        gSavePromptHand = 1;
                    }
                    gSaveInputDelay = 10;
                    PlaySound(0, 1, 0xFF);
                    break;
            }
        } else {
            switch (gSavePrompt) {
                case 17:
                    gSaveMenuState |= 0x10;
                    gSaveMenuState |= 0x800;
                    gSavePromptHand = 0;
                    gSaveInputDelay = 10;
                    break;
                case 4:
                case 5:
                case 6:
                case 7:
                case 13:
                case 14:
                case 15:
                case 18:
                case 21:
                case 22:
                case 23:
                    gSaveMenuState &= ~0x10;
                    gSaveMenuState |= 0x800;
                    if (gSavePrompt == 4 || gSavePrompt == 5 || gSavePrompt == 6 || gSavePrompt == 18) {
                        gSavePromptHand = 0;
                    } else {
                        gSavePromptHand = 1;
                    }
                    gSaveInputDelay = 10;
                    break;
            }
        }
    }
    if (gSaveMenuState & 0x800) {
        if (gSaveMenuState & 0x10) {
            x = gSavePromptBoxes[0].x;
            y = gSavePromptBoxes[0].y;
            w = gSavePromptBoxes[0].w;
            h = gSavePromptBoxes[0].h;
        } else {
            x = gSavePromptBoxes[2].x;
            y = gSavePromptBoxes[2].y;
            w = gSavePromptBoxes[2].w;
            h = gSavePromptBoxes[2].h;
        }
        DrawMenuSprite(0, x, y, w, h);
        DrawMenuSprite(0x37, x + 3, y + h, w, 3);
        DrawMenuSprite(0x37, x + w + 1, y + 6, 2, h - 5);
        DrawMenuSprite(0x37, x - 1, y - 1, w + 2, 1);
        DrawMenuSprite(0x37, x - 1, y + h, w + 2, 1);
        DrawMenuSprite(0x37, x - 1, y, 1, h);
        DrawMenuSprite(0x37, x + w, y, 1, h);
        DrawMenuSprite(0x38, x + 1, y + 1, w - 3, 1);
        DrawMenuSprite(0x38, x + 1, y + 2, 1, h - 4);
        DrawMenuSprite(0x39, x + 1, y + h - 2, w - 2, 1);
        DrawMenuSprite(0x39, x + w - 2, y + 1, 1, h - 3);
        if (gSaveMenuState & 0x10) {
            text = gLoadScreenTexts[1];
            DrawString(x + 6, y + 6, text);
            text = gLoadScreenTexts[0];
            DrawString(x + 6, y + 0x13, text);
        } else {
            text = gLoadScreenTexts[2];
            DrawString(x + 6, y + 6, text);
            text = gLoadScreenTexts[3];
            DrawString(x + 6, y + 0x13, text);
        }
        EndMenuDraw();
        if (gSaveInputDelay < 0) {
            if (gStickY < -20 || gStickY > 20) {
                if (!(gSaveMenuState & 8)) {
                    gSavePromptHand ^= 1;
                    gSaveMenuState |= 8;
                    PlaySound(0, 0, 0xFF);
                }
            } else {
                gSaveMenuState &= ~8;
            }
            EndMenuDraw();
            if (gSaveMenuState & 0x10) {
                DrawMenuSprite(4, gYesNoHandPositions[gSavePromptHand + 2][0], gYesNoHandPositions[gSavePromptHand + 2][1], 0xC, 0xC);
            } else {
                DrawMenuSprite(4, gYesNoHandPositions[gSavePromptHand][0], gYesNoHandPositions[gSavePromptHand][1], 0xC, 0xC);
            }
            if (gButtonsPressed & A_BUTTON) {
                if (gSavePromptHand == 0) {
                    PlaySound(0, 1, 0xFF);
                    switch (gSavePrompt) {
                        case 17:
                            gSavePrompt = 0x12;
                            gSaveInputDelay = 10;
                            break;
                        case 5:
                            SET_SAVE_STATE(4);
                            gSaveMenuState |= 0x2000;
                            gSaveMessage = 3;
                            break;
                        case 4:
                            gSaveMenuState |= 0x2000;
                            gSaveMessage = 2;
                            SET_SAVE_STATE(3);
                            break;
                        case 6:
                            gSaveMenuState |= 0x2000;
                            gSaveMessage = 4;
                            SET_SAVE_STATE(1);
                            break;
                        case 21:
                            ScanSaveList();
                            SET_SAVE_STATE(0);
                            break;
                        case 14:
                        case 15:
                        case 18:
                        case 22:
                        case 23:
                            gSaveMenuState &= ~0x2000;
                            gSaveMenuState &= ~7;
                            gSaveMenuState &= ~0x4000;
                            gSaveMenuState |= 0x1000;
                            break;
                        case 13:
                            gSaveMenuState |= 0x8000;
                            break;
                        case 7:
                            gSaveMenuState |= 0x8000;
                            break;
                    }
                } else {
                    PlaySound(0, 2, 0xFF);
                    switch (gSavePrompt) {
                        case 17:
                            gSavePrompt = 6;
                            break;
                        case 15:
                        case 18:
                        case 22:
                        case 23:
                            gSaveMenuState |= 0x8000;
                            break;
                        case 13:
                            gSaveMenuState &= ~0x2000;
                            gSaveMenuState &= ~7;
                            gSaveMenuState &= ~0x4000;
                            gSaveMenuState |= 0x1000;
                            break;
                        case 14:
                        case 21:
                            gSavePrompt = 0xF;
                            gSaveInputDelay = 10;
                            break;
                        case 4:
                        case 5:
                        case 7:
                            gSaveMenuState &= ~7;
                            if (gSaveMenuState & 0x1000) {
                                gSaveMenuState &= ~0x4000;
                                gSaveMenuState |= 0x2000;
                            } else {
                                gSaveMenuState |= 0;
                            }
                            break;
                        case 6:
                            gSaveMenuState &= ~0x2000;
                            SET_SAVE_STATE(0x1005);
                            gSavePrompt = 7;
                            break;
                    }
                }
                gSaveMenuState &= ~0x800;
            } else if (gButtonsPressed & B_BUTTON) {
                switch (gSavePrompt) {
                    case 6:
                    case 7:
                    case 13:
                    case 14:
                    case 15:
                    case 17:
                    case 18:
                    case 21:
                    case 22:
                    case 23:
                        break;
                    default:
                        gSaveMenuState &= ~0x800;
                        gSaveInputDelay = 2;
                        SET_SAVE_STATE(0);
                        break;
                }
            }
        }
    }
}
void DrawMenuDigits(s32 digits, s32 x, s32 y, u32 value);
void DrawMenuNumber(s32 digits, s32 x, s32 y, u32 value, s32 font);
void DrawMenuNumberZeros(s32 digits, s32 x, s32 y, u32 value, s32 font);
extern s32 gMenuFrame;
extern s8 gArrowBobOffsets[];
extern SaveListText D_80061650;
extern u8* D_8006CC28[];
void DrawSaveList(s32 arg0) {
    s32 slot;               /* the first row's slot; later the menu state's mode bits (RULE-EXCEPTION R30: one stack word for both; a separate local moves the frame) */
    s32 n;                  /* set when the first row is the new-save line; later the arrows' bob offset (RULE-EXCEPTION R30: one stack word for both; a separate local moves the frame) */
    s32 minutes;
    s32 k;                  /* the place-name index; later the hours (RULE-EXCEPTION R30: one stack word for both; a separate local moves the frame) */
    s32 stop;
    s32 font;               /* the font of a row's numbers; between the rows, the second row's slot word (RULE-EXCEPTION R30: with no other value between the rows the second row's font stores move above its calls) */
    SaveListText text;

    n = 0;
    text = D_80061650;
    if (gSaveMenuState & 0x20) {
        if (gSaveSlotCursor == 0) {
            slot = gSaveSlotCursor;
            gSaveMenuState &= ~0x20;
        } else {
            slot = gSaveSlotCursor - 1;
        }
    } else {
        slot = gSaveSlotCursor;
    }
    if (gSaveSlots[slot * 2] != -2) {
        if (gSaveSlots[slot * 2 + 1] != -1) {
            if (slot == gSaveSlotCursor) {
                DrawMenuNumber(2, 0x6F, 0x36, slot, 0xA);
                DrawMenuSprite(0x36, 0x53, 0x35, 0x1C, 0xD);
                DrawMenuSprite(0x19, 0x92, 0x36, 8, 0xA);
                DrawMenuSprite(0x43, 0xD9, 0x35, 0x1C, 0xE);
                font = 0xA;
            } else {
                DrawMenuNumber(2, 0x6F, 0x36, slot, 0x48);
                DrawMenuSprite(0x45, 0x53, 0x35, 0x1C, 0xD);
                DrawMenuSprite(0x52, 0x92, 0x36, 8, 0xA);
                DrawMenuSprite(0x44, 0xD9, 0x35, 0x1C, 0xE);
                font = 0x48;
            }
            DrawMenuNumber(2, 0x65, 0x6C, gSaveList[slot].stats.spirits[0], 0xA);
            DrawMenuNumber(2, 0x8A, 0x6C, gSaveList[slot].stats.spirits[1], 0xA);
            DrawMenuNumber(2, 0xAF, 0x6C, gSaveList[slot].stats.spirits[2], 0xA);
            DrawMenuNumber(2, 0xD4, 0x6C, gSaveList[slot].stats.spirits[3], 0xA);
            switch (gSaveList[slot].map) {
                case 15:
                    if (gSaveList[slot].room == 5) {
                        k = 0;
                    } else {
                        k = 16;
                    }
                    break;
                case 16:
                    if (gSaveList[slot].room == 7) {
                        k = 1;
                    } else {
                        k = 2;
                    }
                    break;
                case 17:
                    k = 4;
                    break;
                case 18:
                    if (gSaveList[slot].room == 6) {
                        k = 3;
                    } else {
                        k = 5;
                    }
                    break;
                case 20:
                    k = 6;
                    break;
                case 22:
                    if (gSaveList[slot].room == 22) {
                        k = 7;
                    } else {
                        k = 8;
                    }
                    break;
                case 23:
                    switch (gSaveList[slot].room) {
                        case 0:
                            k = 9;
                            break;
                        case 8:
                            k = 12;
                            break;
                        case 19:
                            k = 10;
                            break;
                        case 22:
                            k = 11;
                            break;
                    }
                    break;
                case 25:
                    k = 14;
                    break;
                case 26:
                    k = 13;
                    break;
                case 30:
                    k = 15;
                    break;
            }
            DrawString(0x51, 0x48, D_8006CC28[k]);
            EndMenuDraw();
            minutes = gSaveList[slot].playTime;
            k = 0;
            while (minutes >= 60) {
                minutes -= 60;
                k++;
            }
            DrawMenuNumber(3, 0x7E, 0x36, k, font);
            DrawMenuNumberZeros(2, 0x9A, 0x36, minutes, font);
            DrawMenuNumber(3, 0xC0, 0x36, gSaveList[slot].dayCount, font);
            DrawMenuDigits(3, 0x70, 0x58, gSaveList[slot].stats.currHP);
            DrawMenuDigits(3, 0xC0, 0x58, gSaveList[slot].stats.currMP);
        }
    } else {
        n = 1;
        DrawString(0x82, 0x33, text.c);
    }
    EndMenuDraw();
    slot++;
    font = gSaveSlots[slot * 2];
    if (font != -1 && n == 0) {
        if (font != -2) {
            if (gSaveSlots[slot * 2 + 1] != -1) {
                if (slot == gSaveSlotCursor) {
                    DrawMenuNumber(2, 0x6F, 0x83, slot, 0xA);
                    DrawMenuSprite(0x36, 0x53, 0x82, 0x1C, 0xD);
                    DrawMenuSprite(0x19, 0x92, 0x83, 8, 0xA);
                    DrawMenuSprite(0x43, 0xD9, 0x82, 0x1C, 0xE);
                    font = 0xA;
                } else {
                    DrawMenuNumber(2, 0x6F, 0x83, slot, 0x48);
                    DrawMenuSprite(0x45, 0x53, 0x82, 0x1C, 0xD);
                    DrawMenuSprite(0x52, 0x92, 0x83, 8, 0xA);
                    DrawMenuSprite(0x44, 0xD9, 0x82, 0x1C, 0xE);
                    font = 0x48;
                }
                DrawMenuNumber(2, 0x65, 0xBA, gSaveList[slot].stats.spirits[0], 0xA);
                DrawMenuNumber(2, 0x8A, 0xBA, gSaveList[slot].stats.spirits[1], 0xA);
                DrawMenuNumber(2, 0xAF, 0xBA, gSaveList[slot].stats.spirits[2], 0xA);
                DrawMenuNumber(2, 0xD4, 0xBA, gSaveList[slot].stats.spirits[3], 0xA);
                switch (gSaveList[slot].map) {
                    case 15:
                        if (gSaveList[slot].room == 5) {
                            k = 0;
                        } else {
                            k = 16;
                        }
                        break;
                    case 16:
                        if (gSaveList[slot].room == 7) {
                            k = 1;
                        } else {
                            k = 2;
                        }
                        break;
                    case 17:
                        k = 4;
                        break;
                    case 18:
                        if (gSaveList[slot].room == 6) {
                            k = 3;
                        } else {
                            k = 5;
                        }
                        break;
                    case 20:
                        k = 6;
                        break;
                    case 22:
                        if (gSaveList[slot].room == 22) {
                            k = 7;
                        } else {
                            k = 8;
                        }
                        break;
                    case 23:
                        switch (gSaveList[slot].room) {
                            case 0:
                                k = 9;
                                break;
                            case 8:
                                k = 12;
                                break;
                            case 19:
                                k = 10;
                                break;
                            case 22:
                                k = 11;
                                break;
                        }
                        break;
                    case 25:
                        k = 14;
                        break;
                    case 26:
                        k = 13;
                        break;
                    case 30:
                        k = 15;
                        break;
                }
                DrawString(0x51, 0x96, D_8006CC28[k]);
                EndMenuDraw();
                minutes = gSaveList[slot].playTime;
                k = 0;
                while (minutes >= 60) {
                    minutes -= 60;
                    k++;
                }
                DrawMenuNumber(3, 0x7E, 0x83, k, font);
                DrawMenuNumberZeros(2, 0x9A, 0x83, minutes, font);
                DrawMenuNumber(3, 0xC0, 0x83, gSaveList[slot].dayCount, font);
                DrawMenuDigits(3, 0x70, 0xA6, gSaveList[slot].stats.currHP);
                DrawMenuDigits(3, 0xC0, 0xA6, gSaveList[slot].stats.currMP);
            }
        } else {
            DrawString(0x82, 0x7F, text.c);
        }
    }
    EndMenuDraw();
    n = gMenuFrame;
    n >>= 2;
    n &= 0xF;
    n = gArrowBobOffsets[n];
    if (gSaveSlotCursor >= 2) {
        DrawMenuSprite(5, 0x38, 0x4D, 7, 7);
        DrawMenuSprite(0x53, 0x36, n + 0x3F, 0xC, 0x10);
    } else if (gSaveSlotCursor == 1 && !(gSaveMenuState & 0x20)) {
        DrawMenuSprite(5, 0x38, 0x4D, 7, 7);
        DrawMenuSprite(0x53, 0x36, n + 0x3F, 0xC, 0x10);
    }
    if (slot < 16 && gSaveSlots[(slot + 1) * 2] != -1) {
        DrawMenuSprite(5, 0x38, 0xA4, 7, 7);
        DrawMenuSprite(0x54, 0x36, 0xAC - n, 0xE, 0xB);
    }
    if (gSaveMenuState & 0x20) {
        DrawMenuSprite(1, n + 0x42, 0x80, 0x10, 0xD);
    } else {
        DrawMenuSprite(1, n + 0x42, 0x37, 0x10, 0xD);
    }
    if (arg0 != 1) {
        stop = 0;
        if (gStickY < -20) {
            if (gSaveListRepeat == 0) {
                PlaySound(0, 0, 0xFF);
                gSaveSlotCursor++;
                if (gSaveSlots[gSaveSlotCursor * 2] == -1) {
                    gSaveSlotCursor--;
                    stop = 1;
                }
                if (!(gSaveMenuState & 0x20) && stop == 0) {
                    gSaveMenuState |= 0x20;
                } else if (stop == 0) {
                    SET_SAVE_STATE(6);
                    gSaveInputDelay = 4;
                }
            }
            gSaveListRepeat++;
            gSaveListRepeat &= 0xF;
        } else if (gStickY > 20) {
            if (gSaveListRepeat == 0) {
                PlaySound(0, 0, 0xFF);
                gSaveSlotCursor--;
                if (gSaveSlotCursor < 0) {
                    gSaveSlotCursor = 0;
                    stop = 1;
                }
                if (gSaveMenuState & 0x20) {
                    gSaveMenuState &= ~0x20;
                } else if (stop == 0) {
                    SET_SAVE_STATE(6);
                    gSaveInputDelay = 4;
                }
            }
            gSaveListRepeat++;
            gSaveListRepeat &= 0xF;
        } else {
            gSaveListRepeat = 0;
        }
        if (gButtonsPressed & 0x8000) {
            slot = gMenuState & 0xC;
            PlaySound(0, 1, 0xFF);
            switch (slot) {
                case 0:
                    gSaveMenuState |= 0x2000;
                    gSaveMessage = 1;
                    SET_SAVE_STATE(2);
                    break;
                case 8:
                    gSavePrompt = 5;
                    gSaveInputDelay = 10;
                    SET_SAVE_STATE(5);
                    break;
                case 4:
                    if (gSaveSlots[gSaveSlotCursor * 2] == -2) {
                        gSaveMenuState |= 0x2000;
                        gSaveMessage = 2;
                        SET_SAVE_STATE(3);
                    } else {
                        gSavePrompt = 4;
                        gSaveInputDelay = 10;
                        SET_SAVE_STATE(5);
                    }
                    break;
            }
        } else if (gButtonsPressed & 0x5000) {
            gSavePrompt = 7;
            gSaveInputDelay = 10;
            SET_SAVE_STATE(5);
            PlaySound(0, 2, 0xFF);
        }
    }
}

void SaveMenuWait(void) {

    gSaveInputDelay--;
    if (gSaveInputDelay < 0) {
        SET_SAVE_STATE(0);
    }
}
void DeleteSaveSlot(void) {
    s32 err;
    s32 code;

    err = DeleteSaveNote(gControllerChannel, gSaveSlots[gSaveSlotCursor * 2 + 1]);
    if (err != 0) {
        gSavePakError = err;
        code = -PakErrorCode(err);
        switch (code) {
            case 1:
                gSavePrompt = 8;
                gSaveInputDelay = 0xA;
                break;
            case 4:
                gSavePrompt = 0xC;
                gSaveInputDelay = 0xA;
                break;
            default:
                gSavePrompt = 0x11;
                gSaveInputDelay = 0xA;
                break;
        }
        SET_SAVE_STATE(5);
        gSaveMenuState &= ~0x2000;
    } else {
        SET_SAVE_STATE(5);
        gSaveMenuState &= ~0x2000;
        gSavePrompt = 0x16;
        gSaveInputDelay = 0xA;
        gSaveSlots[gSaveSlotCursor * 2 + 1] = -1;
    }
}
extern SaveData gSaveData;      /* the save data */
extern u8 gPakReadBuffer[0xE0];          /* the pak read buffer */
s32 ReadPakNote(s32 channel, s32 file, s32 offset, s32 size, u8* buf);   /* read a pak note */
void ReadSaveRecord(s32 slot);
void LoadSaveSlot(void) {
    s32 err;

    err = ReadPakNote(gControllerChannel, gSaveSlots[gSaveSlotCursor * 2], 0x80, 0xE0, gPakReadBuffer);
    gSaveData = *(SaveData*)gPakReadBuffer;
    if (err != 0) {
        gSavePakError = err;
        switch (-PakErrorCode(err)) {
            case 1:
                gSavePrompt = 8;
                gSaveInputDelay = 0xA;
                break;
            case 4:
                gSavePrompt = 0xB;
                gSaveInputDelay = 0xA;
                break;
            default:
                gSavePrompt = 0x11;
                gSaveInputDelay = 0xA;
                break;
        }
        SET_SAVE_STATE(5);
        gSaveMenuState &= ~0x2000;
    } else {
        ReadSaveRecord(gSaveSlotCursor);
        gSaveMenuState |= 0x80;
        gSaveMenuState |= 0x8000;
    }
}
u8 AllocateSaveNote(s32 channel, u8 ext0);
s32 WritePakNote(s32 channel, s32 file, s32 offset, s32 size, u8* data);
void WriteSaveRecord(u32 slot);
void SaveToPak(void) {
    s32 i;
    s32 ret;
    s32 letter;
    s32 n;

    if (gSaveSlots[gSaveSlotCursor * 2] == -2) {
        n = gPakNotes[gControllerChannel].flags & 0xF;
        for (letter = 0xF; letter >= 0;) {
            i = 0;
            letter++;
            do {
                if (letter == gSaveSlots[i * 2 + 1]) {
                    i = 0x7F;
                    break;
                }
                i++;
            } while (i < n);
            if (i != 0x7F) {
                n = letter;
                letter = -1;
            }
        }
        gSaveSlots[gSaveSlotCursor * 2 + 1] = n;
        if (gSaveSlotCursor + 1 < 16) {
            gSaveSlots[(gSaveSlotCursor + 1) * 2 + 1] = -2;
        }
        ret = AllocateSaveNote(gControllerChannel, n);
        if (ret >= 0) {
            gSaveSlots[gSaveSlotCursor * 2] = ret;
        } else { /* the original leaves this branch empty */
        }
        WriteSaveRecord(gSaveSlotCursor);
    } else {
        WriteSaveRecord(gSaveSlotCursor);
    }
    ret = WritePakNote(gControllerChannel, gSaveSlots[gSaveSlotCursor * 2], 0, 0x80, (u8*) &gSaveList[gSaveSlotCursor]);
    if (ret != 0) {
        gSavePakError = ret;
        switch (-PakErrorCode(ret)) {
            default:
                gSavePrompt = 0x11;
                gSaveInputDelay = 10;
                break;
            case 1:
                gSavePrompt = 0xD;
                gSaveInputDelay = 10;
                break;
            case 4:
                gSavePrompt = 0xE;
                gSaveInputDelay = 10;
                break;
        }
        SET_SAVE_STATE(5);
        gSaveMenuState &= ~0x2000;
        return;
    }
    ret = WritePakNote(gControllerChannel, gSaveSlots[gSaveSlotCursor * 2], 0x80, 0xE0, (u8*) &gSaveData);
    if (ret != 0) {
        gSavePakError = ret;
        switch (-PakErrorCode(ret)) {
            default:
                gSavePrompt = 0x11;
                gSaveInputDelay = 10;
                break;
            case 1:
                gSavePrompt = 0xD;
                gSaveInputDelay = 10;
                break;
            case 4:
                gSavePrompt = 0xE;
                gSaveInputDelay = 10;
                break;
        }
        SET_SAVE_STATE(5);
        gSaveMenuState &= ~0x2000;
        return;
    }
    SET_SAVE_STATE(5);
    gSaveMenuState &= ~0x2000;
    gSavePrompt = 0x17;
    gSaveInputDelay = 10;
}

void osSyncPrintf(const char* fmt, ...);   /* debug print (a stub in retail) */
s32 RepairPak(s32 file, s32 err);
void RepairSaveSlot(void) {
    s32 err;

    osSyncPrintf("Repair Info %d %d\n", gSaveSlotCursor, gSavePakError);
    err = RepairPak(gSaveSlots[gSaveSlotCursor * 2], gSavePakError);
    if (err != 0) {
        gSavePakError = err;
        switch (-PakErrorCode(err)) {
            case 1:
                gSavePrompt = 8;
                gSaveInputDelay = 0xA;
                break;
            case 3:
                gSavePrompt = 0x10;
                gSaveInputDelay = 0xA;
                break;
            default:
                gSavePrompt = 0x11;
                gSaveInputDelay = 0xA;
                break;
        }
        gSaveMenuState &= ~7;
        gSaveMenuState &= ~0x4000;
        gSaveMenuState |= 0x1000;
    } else {
        gSaveMenuState &= ~0x2000;
        gSaveMenuState &= ~7;
        gSaveMenuState &= ~0x4000;
        gSaveMenuState |= 0x1000;
        gPakRepairCount++;
    }
}
extern SaveSlot gSaveRecordBuffer;          /* one save record read from the pak */
/* Build the save list for the pak in controller gControllerChannel (Quest func_8002DD18; spelling from Rainchus's
   decomp.me scratch Fus0c): clear the 17 slot records, list this game's notes (pak file, letter), add a
   "new save" slot (-2) when the pak has room, then read every listed note's record into gSaveList. */
void ScanSaveList(void) {
    s32 i;
    s32 n;
    s32 ret;
    s32 err;

    err = 0;
    for (i = 0; i < 17; i++) {
        gSaveSlots[i * 2] = -1;
        gSaveSlots[i * 2 + 1] = -1;
    }
    n = 0;
    for (i = 0; i < 16; i++) {
        if (gPakNotes[gControllerChannel].ext[i] != 0x7F && gPakNotes[gControllerChannel].ext[i] >= 0) {
            gSaveSlots[n * 2] = i;
            gSaveSlots[n * 2 + 1] = gPakNotes[gControllerChannel].ext[i];
            n++;
        }
    }
    if (gMenuState & 4) {
        if (gPakNotes[gControllerChannel].freePages >= 2 && !(gPakNotes[gControllerChannel].flags & 0x2000)) {
            gSaveSlots[n * 2] = -2;
        }
    }
    gSaveSlotCursor = 0;
    for (i = 0; i < 16; i++) {
        if (gSaveSlots[i * 2] >= 0) {
            ret = ReadPakNote(gControllerChannel, gSaveSlots[i * 2], 0, 0x80, (u8*)&gSaveRecordBuffer);
            gSaveList[i] = gSaveRecordBuffer;
            if (ret != 0) {
                gSavePakError = err;                  /* BUG: stores and decodes err (0 the first time), not ret */
                err = -PakErrorCode(err);
                if (err == 1) {
                    gSavePrompt = 8;
                    gSaveInputDelay = 10;
                } else {
                    gSavePrompt = 0x11;
                    gSaveInputDelay = 10;
                }
                SET_SAVE_STATE(5);
            } else if (gSaveList[i].cleared != 0) {
                gGameState |= 0x20000;
            }
        }
    }
}
void EndMenuDraw(void) {
    gSPDisplayList(gDisplayListHead++, gMenuRenderDL);
    gDPSetTextureLUT(gDisplayListHead++, G_TT_RGBA16);
    gMenuState &= ~0x10;
}
void DrawSaveListFrame(void) {
    DrawMenuSprite(0, 0x3C, 0x22, 0xC8, 0xAB);
    DrawMenuSprite(0x37, 0x3F, 0xCE, 0xC8, 3);
    DrawMenuSprite(0x37, 0x105, 0x28, 2, 0xA9);
    DrawMenuSprite(0x37, 0x3B, 0x21, 0xCA, 1);
    DrawMenuSprite(0x37, 0x3B, 0xCD, 0xCA, 1);
    DrawMenuSprite(0x37, 0x3B, 0x21, 1, 0xAD);
    DrawMenuSprite(0x37, 0x104, 0x21, 1, 0xAD);
    DrawMenuSprite(0x38, 0x3D, 0x23, 0xC5, 1);
    DrawMenuSprite(0x38, 0x3D, 0x23, 1, 0xA8);
    DrawMenuSprite(0x39, 0x3D, 0xCB, 0xC6, 1);
    DrawMenuSprite(0x39, 0x102, 0x23, 1, 0xA9);
    DrawMenuSprite(0x3F, 0x5A, 0x2E, 0x96, 2);
    DrawMenuSprite(0x3F, 0x5A, 0x44, 0x96, 2);
    DrawMenuSprite(0x3F, 0x5A, 0x7B, 0x96, 2);
    DrawMenuSprite(0x3F, 0x5A, 0x91, 0x96, 2);
    DrawMenuSprite(0x42, 0x5E, 0x67, 0x7C, 0xC);
    DrawMenuSprite(0x42, 0x5E, 0xB5, 0x7C, 0xC);
    DrawMenuSprite(0x46, 0x55, 0x58, 0x14, 0xE);
    DrawMenuSprite(0x47, 0xA2, 0x58, 0x18, 0xE);
    DrawMenuSprite(0x46, 0x55, 0xA6, 0x14, 0xE);
    DrawMenuSprite(0x47, 0xA2, 0xA6, 0x18, 0xE);
}
void WriteSaveRecord(u32 slot) {
    SaveSlot* s;
    u8* dst;
    u8* src;
    s32 i;

    s = &gSaveList[slot];
    s->stats = gBrianStats;
    s->playTime = gPlayTime;
    s->dayCount = gDayCount;
    s->map = gCurrentMap;
    s->room = gCurrentSubmap;
    s->returnMap = gReturnDest;
    s->returnRoom = gReturnRoom;
    s->returnEntrance = gReturnEntrance;
    s->exitMap = gExitDest;
    s->exitRoom = gExitRoom;
    s->exitEntrance = gExitEntrance;
    s->respawnMap = gSavedMap;
    s->respawnRoom = gSavedSubmap;
    s->respawnEntrance = gSavedEntrance;
    s->messageSpeed = gMessageSpeed;
    s->musicVolume = gMusicVolumeSetting;
    if (gGameState & 0x20000) {
        s->cleared = 1;
    } else {
        s->cleared = 0;
    }
    dst = gSaveData.spirits;
    src = gObjectsOpened;
    i = 0x20;
    while (i != 0) {
        *dst++ = *src++;
        i--;
    }
    dst = gSaveData.flags;
    src = gSpiritsFound;
    i = 0x10;
    while (i != 0) {
        *dst++ = *src++;
        i--;
    }
    dst = gSaveData.items;
    src = gInventory;
    i = 0x97;
    while (i != 0) {
        *dst++ = *src++;
        i--;
    }
    dst = &gSaveData.story;
    src = gStoryFlags;
    i = 1;
    while (i != 0) {
        *dst++ = *src++;
        i--;
    }
}
void ReadSaveRecord(s32 slot) {
    SaveSlot* s;
    u8* src;
    u8* dst;
    s32 i;

    s = &gSaveList[slot];
    gBrianStats = s->stats;
    gPlayTime = s->playTime;
    gDayCount = s->dayCount;
    gCurrentMap = s->map;
    gCurrentSubmap = s->room;
    gReturnDest = s->returnMap;
    gReturnRoom = s->returnRoom;
    gReturnEntrance = s->returnEntrance;
    gExitDest = s->exitMap;
    gExitRoom = s->exitRoom;
    gExitEntrance = s->exitEntrance;
    gSavedMap = s->respawnMap;
    gSavedSubmap = s->respawnRoom;
    gSavedEntrance = s->respawnEntrance;
    gMessageSpeed = s->messageSpeed;
    gMusicVolumeSetting = s->musicVolume;
    if (s->cleared != 0) {
        gGameState |= 0x20000;
    }
    src = gSaveData.spirits;
    dst = gObjectsOpened;
    i = 0x20;
    while (i != 0) {
        *dst++ = *src++;
        i--;
    }
    src = gSaveData.flags;
    dst = gSpiritsFound;
    i = 0x10;
    while (i != 0) {
        *dst++ = *src++;
        i--;
    }
    src = gSaveData.items;
    dst = gInventory;
    i = 0x97;
    while (i != 0) {
        *dst++ = *src++;
        i--;
    }
    src = &gSaveData.story;
    dst = gStoryFlags;
    i = 1;
    while (i != 0) {
        *dst++ = *src++;
        i--;
    }
}
