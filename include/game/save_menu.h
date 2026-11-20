#ifndef GAME_SAVE_MENU_H
#define GAME_SAVE_MENU_H

/* The functions of src/game/save_menu.c, declared once for every file that calls them. */

#include "common.h"
#include "game/save_format.h"

void EnterSaveMode(s32 saveMode);
void SaveMenu(void);
void DrawSaveMessage(void);
void OpenSaveMenu(void);
void SaveMenuPrompt(void);
void DrawSaveList(s32 arg0);
void SaveMenuWait(void);
void DeleteSaveSlot(void);
void LoadSaveSlot(void);
void SaveToPak(void);
void RepairSaveSlot(void);
void ScanSaveList(void);
void EndMenuDraw(void);
void DrawSaveListFrame(void);
void WriteSaveRecord(u32 slot);
void ReadSaveRecord(s32 slot);


/* The state src/game/save_menu.c defines (.bss), in its storage order: the order of these declarations is the
 * unit's .bss layout (include/bss_order.h). */
extern u16 gSaveMenuState;
extern u8 gSavePrompt;   /* the save menu's message box (index into gSavePromptBoxes /
                                            gSaveModePromptTexts / gTitleModePromptTexts) */
extern u8 gSaveMessage;   /* the save menu's current message (index into gSaveMessageBoxes /
                                            gSaveMessageTexts) */
extern s32 gSavePakError;
extern u8 gSavePromptHand;
extern s8 gSaveInputDelay;
extern s8 gSaveSlotCursor;
extern u8 gSaveListRepeat;   /* the save list's stick repeat counter (DrawSaveList) */
extern u8 gPakRepairCount;
extern s8 gSaveSlots[2 * (SAVE_SLOTS + 1)];   /* per save slot: [0] pak file, [1] its letter; 17 pairs (ScanSaveList
                                            fills 17): the 17th, after gSaveList's 16, ends the list */
extern SaveSlot gSaveList[SAVE_SLOTS];   /* the save list: one record per pak save. A save's pak file holds the record
                                            at 0 (0x80 bytes) and the save data at 0x80 (0xE0 bytes), each written
                                            from a smaller variable (an entry here, 0x68; gSaveData, 0xC8), so each
                                            part also carries the 0x18 bytes of RAM after its variable: the next
                                            entry's start (after the 16th entry, gSaveData's), or after gSaveData,
                                            gSaveRecordBuffer's. The reads copy back only the first 0x68 and 0xC8 */
extern SaveData gSaveData;   /* the save data (WritePakNote writes 0xE0 bytes from it: its 0xC8 and
                                            the first 0x18 of gSaveRecordBuffer) */
extern SaveSlot gSaveRecordBuffer;   /* one save record read from the pak */
extern u8 gSaveRecordBufferTail[SAVE_RECORD_BYTES - sizeof(SaveSlot)];   /* the rest of the 0x80-byte pak record ReadPakNote reads into
                                            gSaveRecordBuffer (the code copies its first 0x68) */
extern u8 gPakReadBuffer[SAVE_DATA_BYTES];   /* the pak read buffer: a note's whole save data (LoadSaveSlot) */

/* The save / load menu (split from sound.c: the original is two files; Quest 64 splits its twin
   code at the same function, and this file's .rodata starts with its own string) */
typedef struct {
    u8 c[14];
} SaveListText;                     /* a short game-encoded text, 0xFF-terminated (DrawSaveList copies one to the stack) */
typedef struct {
    s16 x;
    s16 y;
    s16 w;
    s16 h;
} MenuBox;

/* Storage the C does not define yet (assembly or binary), declared here for its type. */
extern SaveListText D_80061650;   /* link name: _62250_bin + 0x0 */
extern MenuBox gSaveMessageBoxes[];   /* the save menu's message boxes */
extern MenuBox gSavePromptBoxes[];   /* the save menu's message boxes, by message */

#endif
