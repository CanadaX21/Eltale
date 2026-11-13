#ifndef GAME_TEXT_H
#define GAME_TEXT_H

/* The functions of src/game/text.c, declared once for every file that calls them. */

#include "common.h"

void string_copy(u8 *dst, u8 *src);
void loadTextTextureAndPal(void);
void DrawString(s32 x, s32 y, u8* str);

#endif
