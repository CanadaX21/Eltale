#ifndef GAME_START_MENU_H
#define GAME_START_MENU_H

/* The functions of src/game/start_menu.c, declared once for every file that calls them. */

#include "common.h"

void InitStartMenu(void);
void CloseStartMenu(void);
void StartMenu(mkst* dyn);
void TurnMenuPage(void);
void DrawStatusPage(s32 arg0);
void DrawEnvironmentPage(s32 arg0);
void DrawMapPage(s32 arg0);
void DrawElementsPage(s32 x);
void DrawMenuSprite(s32 sprite, s32 x, s32 y, s32 w, s32 h);
void DrawElementGauge(s32 percent, s32 x);
void DrawMenuBackground(void);
void DrawMenuNumber(s32 digits, s32 x, s32 y, u32 value, s32 font);   /* draw a number with the sheet's digits (font = kind of '0') */
void DrawMenuNumberZeros(s32 digits, s32 x, s32 y, u32 value, s32 font);
void DrawMenuDigits(s32 digits, s32 x, s32 y, u32 value);
void MapCursorToWorld(void);

#endif
