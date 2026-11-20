#ifndef GAME_OVERLAY_H
#define GAME_OVERLAY_H

/* The functions of src/game/overlay.c, declared once for every file that calls them. */

#include "common.h"

void LoadOvl_TitleScreen(void);
void LoadOvl_Splash_and_Intro(void);
void LoadOvl_Epilogue(void);   /* the epilogue overlay; its segment keeps the historical name credits_overlay */

#endif
