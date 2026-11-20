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

/* The state src/game/message.c defines (.bss), in its storage order: the order of these declarations is the
 * unit's .bss layout (include/bss_order.h). */
extern s32 gMessageWindowsOpen;   /* windows opened */
extern s32 gMessageChoice;   /* the message's Yes / No: 0 = Yes (BrianState_TalkEnd) */
extern MessageLines gMessageLines;
extern Message gMessageWindows[4];   /* 4 message windows, 0x9FC each */
extern u8 gControllerConnected;   /* a controller is connected */
extern u16 gStickHoldFrames;   /* frames the stick has been held past radius 60 */

typedef struct {
    s16 x;
    s16 y;
    s16 width;
    s16 height;
} WindowPlacement;
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

/* Storage the C does not define yet (assembly or binary), declared here for its type. */
extern WindowPlacement D_80061668[];   /* window placement by type */
extern WindowPlacement gChoiceBoxSize;   /* the message window being sized */
extern QuillOffsets gQuillLift;   /* link name: _62250_bin + 0x68 */
extern QuillOffsets gQuillWidth;   /* link name: _62250_bin + 0x70 */
extern QuillOffsets gQuillShift;   /* link name: _62250_bin + 0x78 */
extern BoxTextureImages D_800616D0;   /* assembly */
extern BoxTextures D_800616E4;   /* assembly */

#endif
