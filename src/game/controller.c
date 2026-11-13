#include "common.h"
#include "game/controller.h"
extern OSPfs gPfs[];
typedef struct {
    u8 name[4];
} PfsExtName;                       /* a Controller Pak note's extension */
extern PfsExtName gSaveNoteExtName;       /* the save note's extension template */
extern u8 gSaveNoteName[];             /* the save note's name */
extern PakNoteExts gPakNotes[];
extern OSMesgQueue gSiMesgQueue;
s32 osGbpakInit(OSMesgQueue *mq, OSPfs *pfs, int channel);
extern OSContPad gContPads[4];
extern u16 gButtonsHeld;
extern u16 gButtonsPrevInv;
extern s8 gStickX;
extern s8 gStickY;
extern u16 gButtonsPressed;

extern s8 gControllerChannel;   /* the first usable controller */
extern OSMesg gSiMesgBuf;
extern u8 gControllerBits;                /* controllers present (bit per port) */
extern OSContStatus gContStatus[];
s32 InitControllers(void) {
    s32 count;
    s32 i;
    s32 ret;

    ret = 0;
    gButtonsPrevInv = 0;
    for (i = 0; i < 4; i++) {
        gPakNotes[i].status = -0x80;
    }
    for (i = 0; i < 4; i++) {
        gContPads[i].button = 0;
        gContPads[i].stick_x = 0;
        gContPads[i].stick_y = 0;
    }
    osCreateMesgQueue(&gSiMesgQueue, &gSiMesgBuf, 1);
    osSetEventMesg(OS_EVENT_SI, &gSiMesgQueue, &gSiMesgBuf);
    osContInit(&gSiMesgQueue, &gControllerBits, gContStatus);
    osContSetCh(4);
    gControllerChannel = -1;
    count = 0;
    for (i = 0; i != 4; i++) {
        if (gControllerBits & (1 << i)) {
            if (gContStatus[i].errno & 4) {
                gPakNotes[i].status = -1;
            } else if (gContStatus[i].type & 2) {
                gPakNotes[i].status = -2;
            } else {
                gPakNotes[i].status = 0;
                count++;
                if (gControllerChannel == -1) {
                    gControllerChannel = i;
                }
                if (gContStatus[i].status & 1) {
                    gPakNotes[i].flags |= 0x8000;
                } else {
                    gPakNotes[i].flags = 0;
                    if (i == gControllerChannel) {
                        ret = 1;
                    }
                }
            }
        } else {
            gPakNotes[i].status = -3;
        }
    }
    if (count == 0) {
        return -1;
    }
    return ret;
}
void ReadController(s32 channel) {
    OSContPad* pad;
    u16 buttons;

    osContSetCh(4U);
    osContStartReadData(&gSiMesgQueue);
    if (osRecvMesg(&gSiMesgQueue, NULL, OS_MESG_BLOCK) != -1) {
        osContGetReadData(gContPads);
    }
    pad = &gContPads[channel];
    buttons = pad->button;
    gButtonsHeld = buttons;
    gStickX = pad->stick_x;
    gStickY = pad->stick_y;
    gButtonsPressed = buttons & gButtonsPrevInv;
    gButtonsPrevInv = ~buttons;
}
s32 ScanControllerPak(s32 channel) {
    s32 i;                          /* also receives the free byte count */
    s32 ret;
    s32 count;
    s32 used;
    OSPfs* pfs;
#ifdef MATCHING_HACK_IDO
    s32 pad[4]; /* unused: the four stack words between the other locals and state */
#endif
    OSPfsState state;

    count = 0;
    used = 0;
    gPakNotes[channel].freePages = 0xFF;
    gPakNotes[channel].flags = 0;
    for (i = 0; i < 16; i++) {
        gPakNotes[channel].ext[i] = -1;
    }
    pfs = &gPfs[channel];
    ret = osPfsInitPak(&gSiMesgQueue, pfs, channel);
    if (ret != 0) {
        if (ret == PFS_ERR_ID_FATAL) {
            if (osMotorInit(&gSiMesgQueue, pfs, channel) == 0) {
                return 1;
            }
            if (osGbpakInit(&gSiMesgQueue, pfs, channel) == 0) {
                return 1;
            }
            ret = PFS_ERR_ID_FATAL;
        }
        return ret;
    }
    for (i = 0; i < 16; i++) {
        ret = osPfsFileState(pfs, i, &state);
        if (ret == 0) {
            if (state.game_code == 0x4E45544A && state.company_code == 0x3943) {
                if (CompareBytes(gSaveNoteName, state.game_name, 16) == 0) {
                    count++;
                    gPakNotes[channel].ext[i] = ((u8*)state.ext_name)[0];   /* RULE-EXCEPTION R19: the SDK's ext_name is char; read unsigned */
                    gPakNotes[channel].flags |= 0x4000;
                } else {
                    gPakNotes[channel].ext[i] = 0x7F;
                }
            } else {
                gPakNotes[channel].ext[i] = 0x7F;
            }
            used++;
        } else if (ret == PFS_ERR_INVALID) {
            gPakNotes[channel].ext[i] = -1;
            ret = 0;
        } else {
            break;
        }
    }
    if (ret == 0) {
        gPakNotes[channel].flags &= ~0xF;
        gPakNotes[channel].flags |= count;
        if (count >= 16) {
            gPakNotes[channel].flags |= 0x2000;
        }
        if (used >= 16) {
            gPakNotes[channel].flags |= 0x2000;
        }
        osPfsFreeBlocks(pfs, &i);
        i /= 256;
        gPakNotes[channel].freePages = i;
        if (i < 2) {
            gPakNotes[channel].flags |= 0x2000;
        }
    }
    return ret;
}
/* Meant to pick a note extension letter (0x1A + n) that none of the pak's 16 notes uses (Quest func_80031800;
   spelling from Rainchus's decomp.me scratch CeG4H). BUG: the loop runs while the candidate is free and stops at
   the first letter in use, so it always returns -1 (0xFF). */
u8 FindNoteExtension(s32 channel) {
    s32 letter;
    s32 found;
    s32 i;

    letter = i = -1;            /* guess: i's -1 is dead (reset below), but without it letter takes another register (R36) */
    do {
        letter++;
        i = 0;
        found = 0;
        for (; i < 16; i++) {
            if (gPakNotes[channel].ext[i] == letter + 0x1A) {
                found = -1;
                break;
            }
        }
        if (letter >= 16 && found != 0) {
            found = -1;
            break;
        }
    } while (found == 0);
    if (found == 0) {
        found = letter + 0x1A;
    }
    return found;
}
extern PfsExtName gNewNoteExtName;       /* the extension template for a new note */
u8 AllocateSaveNote(s32 channel, u8 ext0) {
    s32 ret;
    s32 fileNo = -1;
    PfsExtName ext = gNewNoteExtName;
    s32 err;

    ext.name[0] = ext0;
    err = osPfsAllocateFile(&gPfs[channel], 0x3943, 0x4E45544A, gSaveNoteName, ext.name, 0x200, &fileNo);
    if (err == 0) {
        ret = fileNo;
        gPakNotes[channel].ext[ret] = ext0;
    } else {
        ret = -err;
    }
    return ret;
}
u8 DeleteSaveNote(s32 channel, u8 ext0) {
    s32 err;
    PfsExtName ext = gSaveNoteExtName;

    ext.name[0] = ext0;
    err = osPfsDeleteFile(&gPfs[channel], 0x3943, 0x4E45544A, gSaveNoteName, ext.name);
    return err;
}
s32 ReadPakNote(s32 channel, s32 file, s32 offset, s32 size, u8* buf) {
    return osPfsReadWriteFile(&gPfs[channel], file, 0, offset, size, buf);
}
s32 WritePakNote(s32 channel, s32 file, s32 offset, s32 size, u8* data) {
    s32 err;
    s32 freeBytes;

    /* the free-space query fails first when the pak is gone or damaged; its error is returned unwritten */
    err = osPfsFreeBlocks(&gPfs[channel], &freeBytes);
    if (err != 0) {
        return err;
    }
    return osPfsReadWriteFile(&gPfs[channel], file, 1, offset, size, data);
}
s32 RepairPak(s32 file, s32 err) {
    s32 ret;
    s32 channel;

    channel = gControllerChannel;
    if (err == PFS_ERR_ID_FATAL) {
        if (osMotorInit(&gSiMesgQueue, &gPfs[channel], channel) != 0) {
            if (osGbpakInit(&gSiMesgQueue, &gPfs[channel], channel) != 0) {
                ret = osPfsInitPak(&gSiMesgQueue, &gPfs[channel], channel);
                if (ret != PFS_ERR_DEVICE && ret != PFS_ERR_NOPACK && ret != PFS_ERR_NEW_PACK) {
                    ret = osPfsRepairId(&gPfs[channel]);
                }
            } else {
                ret = PFS_ERR_CONTRFAIL;
            }
        } else {
            ret = PFS_ERR_CONTRFAIL;
        }
    } else {
        ret = osPfsInitPak(&gSiMesgQueue, &gPfs[channel], channel);
    }
    return ret;
}
s32 PakErrorCode(s32 pfsErr) {
    s32 code;

    switch (pfsErr) {
    case PFS_ERR_NOPACK:
        code = -1;
        break;
    case PFS_ERR_CONTRFAIL:
    case PFS_ERR_DEVICE:
        code = -2;
        break;
    case PFS_ERR_INCONSISTENT:
    case PFS_ERR_ID_FATAL:
        code = -3;
        break;
    case PFS_ERR_NEW_PACK:
        code = -4;
        break;
    case PFS_ERR_EXIST:
        code = -5;
        break;
    default:
        code = -6;
        break;
    }
    return code;
}
s32 CompareBytes(u8* a, u8* b, s32 n) {
    do {
        if (*a++ != *b++) {
            break;
        }
    } while (n--);
    return n;
}
void func_80036124(OSPfs* pfs, s32* max_files, s32* files_used) {
    osPfsNumFiles(pfs, max_files, files_used);
}
void func_80036144(OSMesgQueue* mq) {
    osContStartQuery(mq);
}
void func_80036164(OSContStatus* data) {
    osContGetQuery(data);
}
void func_80036184(OSMesgQueue* mq, OSPfs* pfs, int channel) {
    osGbpakInit(mq, pfs, channel);
}
