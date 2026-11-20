#ifndef SAVE_FORMAT_H
#define SAVE_FORMAT_H
#include "common.h"
/* The game's save format: what a save is, where it lives on the Controller Pak, and which game state each part
   holds. Its two record types are defined here: SaveSlot, the save list's record, and SaveData, the save
   data (BrianData1, Brian's stats in a record, is a type the game shares: include/quest_types.h).

   A save is one Controller Pak note: company code SAVE_NOTE_COMPANY_CODE ("9C"), game code SAVE_NOTE_GAME_CODE
   ("NETJ"), the note name gSaveNoteName (the pak's character set: 53 78 62 51 78, the katakana for "Eltale", then
   zeros) and an extension letter of its own (SaveToPak gives a new save the lowest letter from
   SAVE_NOTE_FIRST_EXTENSION that no listed save uses: 0x10, 0x11, ...). A note is SAVE_NOTE_BYTES long (SAVE_NOTE_PAGES
   pages of the pak) and holds two parts:
     SAVE_RECORD_OFFSET, SAVE_RECORD_BYTES: the save list's record. SaveToPak writes it from the gSaveList entry
       WriteSaveRecord filled (a SaveSlot, 0x68 bytes: Brian's stats, the play time and day count, the map and room,
       the Return and Exit spells' destinations, the respawn place, the message speed and music volume, the cleared
       flag); ScanSaveList reads it into gSaveRecordBuffer and its tail, and copies the SaveSlot into gSaveList.
     SAVE_DATA_OFFSET, SAVE_DATA_BYTES: the save data. SaveToPak writes it from gSaveData (a SaveData, 0xC8 bytes: the
       opened map objects, the spirits found, the inventory, the first story-flag byte); LoadSaveSlot reads it into
       gPakReadBuffer and copies the SaveData into gSaveData, and ReadSaveRecord copies both parts into the game state.
   Each part is written in its full size from a smaller variable, so it also carries the RAM that follows the variable:
   a record carries the next gSaveList entry's first 0x18 bytes (after the 16th entry, gSaveData's), the save data
   carries gSaveRecordBuffer's first 0x18 bytes (the last record ScanSaveList read). The loads never read those bytes
   back. The rest of the note, from SAVE_RECORD_BYTES + SAVE_DATA_BYTES on, is never written: it keeps what the pages
   held before. The game keeps no checksum of its own: the pak's file system checks its directory, not a note's data.
   Not saved: the time of day, Brian's position (a load puts him at gRespawnLocationTbl's place for the saved map and
   room, else at the room's entrance 0), gStoryFlags[1-3], gGameState's bits but GAME_STATE_CLEARED. */
#define SAVE_NOTE_COMPANY_CODE 0x3943           /* "9C" */
#define SAVE_NOTE_GAME_CODE 0x4E45544A          /* "NETJ" */
#define SAVE_NOTE_FIRST_EXTENSION 0x10          /* the extension letter of the first save ('0' in the pak's set) */
#define SAVE_NOTE_BYTES 0x200                   /* one note per save */
#define SAVE_NOTE_PAGES 2                       /* SAVE_NOTE_BYTES in the pak's 256-byte pages */
#define SAVE_RECORD_OFFSET 0
#define SAVE_RECORD_BYTES 0x80
#define SAVE_DATA_OFFSET 0x80
#define SAVE_DATA_BYTES 0xE0
#define SAVE_SLOTS 16                           /* gSaveList's entries: the saves the list shows */
#define PAK_NOTES 16                            /* a Controller Pak's note table: 16 notes of any game */

/* the save menu's modes: gMenuState & SAVE_MODE_MASK, set by EnterSaveMode */
#define SAVE_MODE_MASK 0xC
#define SAVE_MODE_LOAD 0                        /* the title's Continue: A on a row loads that save */
#define SAVE_MODE_SAVE 4                        /* after the inn's rest: A saves, on a new row while the pak has room */
#define SAVE_MODE_DELETE 8                      /* the title's third choice: A on a row deletes that save */

/* the gGameState bits the save system sets and reads, GAME_STATE_SAVE_MENU and GAME_STATE_CLEARED, are with the
   others in include/variables.h */

/* ---- the save data block (save_menu.c: WriteSaveRecord fills it from the game state, ReadSaveRecord copies it back) ---- */
typedef struct {
    u8 objectsOpened[0x20];               /* 0x00: gObjectsOpened, a bit per map object opened */
    u8 spiritsFound[0x10];                 /* 0x20: gSpiritsFound, a bit per spirit found */
    u8 inventory[0x97];                 /* 0x30: gInventory, all 151 bytes (the 150 slots and the byte after them) */
    u8 storyFlags;                       /* 0xC7: gStoryFlags[0], the bosses beaten (bytes 1-3 are not saved) */
} SaveData;                     /* 0xC8: a save note holds it at 0x80, in 0xE0 bytes (the 0x18 after it are not read back) */

typedef struct {
    BrianData1 stats;               /* 0x00: gBrianStats */
    u16 playTime;                   /* 0x38: minutes (gPlayTime) */
    u16 dayCount;                   /* 0x3A: gDayCount (counts up to 9999) */
    s32 map;                        /* 0x3C: where the save was made (the save list names it, DrawSaveList) */
    s32 room;                       /* 0x40 */
    s32 returnMap;                  /* 0x44: the Return destination (gReturnDest-D8) */
    s32 returnRoom;
    s32 returnEntrance;
    s32 exitMap;                    /* 0x50: the Exit destination (gExitDest-E4) */
    s32 exitRoom;
    s32 exitEntrance;
    u16 respawnMap;                 /* 0x5C: gSavedMap-B0 */
    u16 respawnRoom;
    u16 respawnEntrance;
    s8 messageSpeed;                /* gMessageSpeed */
    s8 musicVolume;                 /* gMusicVolumeSetting */
    u16 cleared;                    /* gGameState's GAME_STATE_CLEARED: the game has been finished */
} SaveSlot;                         /* 0x68 */

#endif
