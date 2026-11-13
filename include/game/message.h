#ifndef GAME_MESSAGE_H
#define GAME_MESSAGE_H

/* The functions of src/game/message.c, declared once for every file that calls them. */

#include "common.h"

void ResetMessageWindows(s32 unused);
void ResetMessageWindow(s32 slot, u8 keepPos);
s32 OpenMessageWindow(s32 type);
void DrawDialogueBox(void);
s32 AnimateMessageWindow(s32 slot);
void DrawMessageText(s32 slot);
void DrawMessageBox(s32 slot);
void PageMessage(s32 slot);
void DrawMoreTextQuill(s32 slot);
void SetMessageWindowSize(s32 slot);
s32 FindMessageWindow(s32 id, s32 type);
void DrawBoxTexture(s32 texture, s32 x, s32 y, s32 w, s32 h);
void DrawMenuBox(s32 x, s32 y, s32 w, s32 h);
void ResetStickHeld(void);
s32 IsStickHeld(void);

#endif
