#ifndef GAME_CONTROLLER_H
#define GAME_CONTROLLER_H

/* The functions of src/game/controller.c, declared once for every file that calls them. */

#include "common.h"

s32 InitControllers(void);
void ReadController(s32 channel);   /* read the controller */
s32 ScanControllerPak(s32 channel);
u8 FindNoteExtension(s32 channel);
u8 AllocateSaveNote(s32 channel, u8 ext0);
u8 DeleteSaveNote(s32 channel, u8 ext0);
s32 ReadPakNote(s32 channel, s32 file, s32 offset, s32 size, u8* buf);   /* read a pak note */
s32 WritePakNote(s32 channel, s32 file, s32 offset, s32 size, u8* data);
s32 RepairPak(s32 file, s32 err);
s32 PakErrorCode(s32 pfsErr);
s32 CompareBytes(u8* a, u8* b, s32 n);
void CountPakFiles(OSPfs* pfs, s32* max_files, s32* files_used);
void StartContQuery(OSMesgQueue* mq);
void GetContQuery(OSContStatus* data);
void InitGbPak(OSMesgQueue* mq, OSPfs* pfs, int channel);

/* The state src/game/controller.c defines (.bss), in its storage order: the order of these declarations is the
 * unit's .bss layout (include/bss_order.h). */
extern s8 gControllerChannel;   /* the first usable controller */
extern s8 gStickX;   /* stick x */
extern s8 gStickY;   /* stick y */
extern u16 gButtonsHeld;   /* buttons held */
extern u16 gButtonsPressed;   /* buttons pressed this frame (Quest D_80092876) */
extern u16 gButtonsPrevInv;
extern OSMesgQueue gSiMesgQueue;   /* the SI queue */
extern OSPfs gPfs[4];   /* one per controller port */
extern PakNoteExts gPakNotes[4];
extern u8 gControllerBits;   /* controllers present (bit per port) */
extern OSContStatus gContStatus[4];
extern OSMesg gSiMesgBuf;
extern OSContPad gContPads[4];

typedef struct {
    u8 name[4];
} PfsExtName;                       /* a Controller Pak note's extension */

/* Storage the C does not define yet (assembly or binary), declared here for its type. */
extern PfsExtName gNewNoteExtName;   /* the extension template for a new note */
extern PfsExtName gSaveNoteExtName;   /* the save note's extension template */

#endif
