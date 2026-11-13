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
void func_80036124(OSPfs* pfs, s32* max_files, s32* files_used);
void func_80036144(OSMesgQueue* mq);
void func_80036164(OSContStatus* data);
void func_80036184(OSMesgQueue* mq, OSPfs* pfs, int channel);

#endif
