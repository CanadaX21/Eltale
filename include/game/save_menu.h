#ifndef GAME_SAVE_MENU_H
#define GAME_SAVE_MENU_H

/* The functions of src/game/save_menu.c, declared once for every file that calls them. */

#include "common.h"

void EnterSaveMode(s32 returnMode);
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

#endif
