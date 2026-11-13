#ifndef GAME_TITLE_H
#define GAME_TITLE_H

/* The functions of the title screen overlay (src/title_overlay/: title_screen.c, title_menu.c, title_fade.c), declared once for every file that calls them. */

#include "common.h"

/* title_screen.c */
void TitleScreen_Init(void);
s32 TitleScreen_Update(void);
s32 TitleScreen_Step(void);
void TitleScreen_UpdateMenu(void);
void TitleScreen_FinishSelection(void);

/* title_menu.c */
void TitleScreen_DrawMenu(s32 menuState);   /* still assembly */

/* title_fade.c */
void TitleScreen_UpdateFade(void);

#endif
