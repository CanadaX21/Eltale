# Saves and the Controller Pak, as the game uses them and as this source builds them

Eltale keeps its saves on the Controller Pak, one pak note per save, up to 16. This page describes what a save
holds, where that state lives in RAM, how the game writes, finds, loads and deletes saves, the boot-time Controller
Pak manager, the tools that read the format from this source, and what a modified build or a port must keep. The
format's constants are `include/game/save_format.h`; the machine-readable definition is `tools/save_format.json`.

## What a save holds

A save is a pak note of `SAVE_NOTE_BYTES` (0x200 bytes, two of the pak's 256-byte pages), with the company code
`"9C"` (0x3943), the game code `"NETJ"` (0x4E45544A), the note name `gSaveNoteName` (エルテイル in the pak's
character set: 53 78 62 51 78, then zeros) and an extension letter of its own (`SaveToPak` gives a new save the
lowest letter from 0x10, the pak's `0`, that no listed save uses). Inside the note:

| Offset | Bytes | Part | Written from | Read into |
|---|---|---|---|---|
| 0x000 | 0x80 | the save list's record | a `gSaveList` entry (`SaveSlot`, 0x68), filled by `WriteSaveRecord` | `gSaveRecordBuffer` + `gSaveRecordBufferTail` (`ScanSaveList`) |
| 0x080 | 0xE0 | the save data | `gSaveData` (`SaveData`, 0xC8) | `gPakReadBuffer` (`LoadSaveSlot`) |
| 0x160 | 0xA0 | nothing | never written: it keeps what the pages held | never read |

- `SaveSlot`: Brian's stats (`BrianData1`, 0x38), the play time and day count, the map and room, the Return and Exit
  spells' destinations, the respawn map, room and entrance, the message speed and music volume, the cleared flag.
- `SaveData`: the opened map objects (0x20 bytes), the spirits found (0x10), the inventory (0x97), the first
  story-flag byte.
- Each part is written in its full size from a smaller variable, so each carries the 0x18 bytes of RAM after it: a
  record carries the next `gSaveList` entry's first bytes (after the sixteenth, `gSaveData`'s), the data carries
  `gSaveRecordBuffer`'s (the last record the list's scan read). The loads never read those bytes back; seen in ares
  as another save's stats inside a record. The read buffers are declared in the parts' full sizes (the record
  buffer as `gSaveRecordBuffer` and `gSaveRecordBufferTail`), and the link asserts both places (the tail right after
  the record buffer, `gPakReadBuffer` past their 0x80 bytes), so no read runs into other state.
- No checksum of the game's own: the pak's file system checks its directory and inodes, not a note's bytes.
- Not saved: the time of day; Brian's position (a load puts him at `gRespawnLocationTbl`'s place for the saved map
  and room, 17 rows, else at the room's entrance 0); `gStoryFlags[1-3]`; `gGameState`'s bits but the cleared flag.
- Saved but not kept: Brian's radius (`stats.radius`). Every load ends in `RespawnAtSavePoint`, which sets it to
  50.0, so an edited radius never takes effect (`tools/save_format.json`'s restore rules).

`WriteSaveRecord` and `ReadSaveRecord` are the one place that says which game variable goes to which field;
`tools/save_format.py` reads them and the record types, and the build fails when the two directions disagree.

## How the game uses it

The save menu (`src/game/save_menu.c`) has three modes, `gMenuState & SAVE_MODE_MASK`, set by `EnterSaveMode`:

- **Load** (`SAVE_MODE_LOAD`): the title's Continue. A on a row: `LoadSaveSlot` reads the note's data part into
  `gPakReadBuffer`, copies it into `gSaveData`, and `ReadSaveRecord` copies both parts into the game state.
- **Save** (`SAVE_MODE_SAVE`): after the inn's rest (the inn's Yes sets `GAME_STATE_SAVE_MENU` with the transition;
  the rest restores HP and MP, advances the time of day and sets the respawn place, then the save mode opens). A on a
  save asks first (prompt 4), then overwrites it; a new row takes a new note (`AllocateSaveNote`, two pages) while the
  pak has room. With too little room for a new note (seen with one free page) the mode opens on prompt 21 (a new save
  does not fit, overwrite one) when the pak holds saves of the game, else on prompt 20. `SaveToPak` writes the record and the data with two `WritePakNote` calls.
- **Delete** (`SAVE_MODE_DELETE`): the title's third choice. A on a row asks (prompt 5); Yes leads to `DeleteSaveSlot`,
  which deletes the note through `DeleteSaveNote` (osPfsDeleteFile).

On the title the stick's x moves between New, Continue and Delete (`gTitleChoice`: `TITLE_CHOICE_NEW_GAME`, `TITLE_CHOICE_CONTINUE`, `TITLE_CHOICE_DELETE`); the D-pad does not.

When the menu opens (`OpenSaveMenu`), `ScanControllerPak` reads the pak's 16 notes (osPfsFileState) and keeps those
with the game's codes and name, and `ScanSaveList` reads each one's record into `gSaveList`. The name test is `CompareBytes`, which makes
n + 1 comparisons: it compares 17 bytes, the name's 16 and then `gSaveNoteName[16]` (0x30, the first byte after the
name) against the stack byte after `OSPfsState.game_name`, which osPfsFileState never writes. A note counts as the
game's while that byte is not 0x30 (0x00 in ares). Pak errors go through `PakErrorCode` to the menu's prompts. Its
repair action (`RepairSaveSlot`, `RepairPak`), after `PFS_ERR_ID_FATAL`: a Rumble Pak or a Transfer Pak (osMotorInit,
osGbpakInit) gets `PFS_ERR_CONTRFAIL`; otherwise osPfsInitPak runs again and, unless it reports no pak, a new pak or
a device error, osPfsRepairId repairs the ID (after a 0 too). Any other error is answered with osPfsInitPak.

## The boot-time Controller Pak manager

`pak_menu_module` (ROM 0xDA4570-0xDAB5F0, `src/pak_menu_module/pak_menu.c`) is the menu the game shows before its
title. `mainproc` copies the module to 0x80100000 (the RAM `gameplay_common` takes later) by its ROM symbols, then calls
`ControllerPakMenu` before the scheduler starts. It reads the pads itself and draws by the CPU into two framebuffers
(0x80200000 and 0x80225800, cleared first). Its results (0 nothing to do or a pak error, 2 a good pak without Start, 3
left by the player) are not read by `mainproc`.

It returns at once with no controller, with no pak on the game's controller, with a Rumble Pak or a Transfer Pak
(told apart only after osPfsInitPak's `PFS_ERR_ID_FATAL`), or on another pak error. It shows the menu when Start is
held at power-on, and on its own when a memory pak's ID area is damaged. Its eleven states (`gPakMenuState`, the
entry's jump table), read from its code; the delete, the repair and the menu's exit were also run in ares:

| State | Screen | What it does |
|---|---|---|
| 0 | none | scans the pak: osPfsNumFiles, osPfsFileState for 16 notes, osPfsFreeBlocks; on `PFS_ERR_NEW_PACK` re-inits |
| 1 | the top row: ノートを選ぶ / 終了 | A on the left: state 2; A on the right or B: return 3 (with no notes the hand stays right) |
| 2 | the note list | up and down move the hand over six rows and scroll the 16 notes; A on a note in use: state 4; B: state 1 |
| 3 | `gPakMenuDeleteMessage` (データを消去します。よろしいですか？), はい / やめる | the hand starts on やめる; A on はい: osPfsDeleteFile, then state 2 (state 1 with no note left), state 8 on `PFS_ERR_NOPACK`, else 7; A on やめる or B: state 4 |
| 4 | the note's row: 消去 / キャンセル | A on 消去: state 3; A on キャンセル or B: state 2 |
| 5 | `gPakMenuDamagedMessage` (the pak is damaged; repair it?) | A on the left: osPfsRepairId each frame until it succeeds (state 6), the pak goes (8), or 181 tries (10); the right: state 9 |
| 6 | (state 5's last frame) | osPfsInitPak until 0 (state 0), `PFS_ERR_NOPACK` (8) or 181 tries (10) |
| 7 | `gPakMenuDeleteErrorMessage` | A: osPfsInitPak until 0 (state 0), damaged (5) or no pak (8) |
| 8 | `gPakMenuNoPakMessage` | the left: osPfsInitPak until a pak answers (state 0, or 5 damaged), asking again after 181 tries; the right: return 3 |
| 9 | `gPakMenuRepairDeclinedMessage` | the left: osPfsInitPak until 0, 5 or 8; the right: return 3 |
| 10 | `gPakMenuRepairFailedMessage` | as state 9 |

A pak pulled out is noticed in states 1 to 4 (osContStartQuery each frame: state 8); elsewhere only by the next pak
call's `PFS_ERR_NOPACK`. The stick and the C buttons move the hand (the module folds the stick into the C buttons'
bits); the D-pad does nothing. Up and down repeat after 32 frames, then every eighth. States 7, 9 and 10 retry
osPfsInitPak without a limit while it returns something other than 0, 1 or 10. Seen in ares: a deletion through
state 3 (one of two saves deleted, the pak's file system intact, 121 free pages), and a damaged pak repaired (both
saves kept; the repaired ID blocks differ between runs only in osPfsRepairId's time-based serial and their checksums).

The module's .bss (0x80107080, 0x280 bytes) is neither copied nor cleared: it starts with what RAM held. State 0
resets the list's row and scroll. On the path straight to state 5 (a damaged pak at power-on), state 5 marks every
note unused before it draws, but the scroll and the dialog's choice are what RAM left (zero in ares); a scroll above 10
would make `PakMenu_DrawNoteRow` read past the 16-entry arrays. A port should clear the module's state before the
first call.

Fourteen of its sixteen functions are C (one an empty function nothing calls); the entry and `PakMenu_DrawText` (the game's text codes drawn by the CPU,
the twin of `DrawString` in `src/game/text.c`) are the original assembly. The module is relocatable: a build that grows
its first function moves every function, the jump table, the data and the .bss, and the sprite lists' 668 pointers,
the palette pointer and the entry's call follow (checked statically and in ares). splat would name any data word
after a symbol covering its value: the six messages and an image whose last word had been so named are typed as
bytes, the framebuffer pair as halves. `tools/check_pak_menu_data.py` (run by `configure`) fails when the module's
data or its remaining assembly names any symbol but its own and five known outside names (main's three controller
variables, the font sheet, the clear loop's stride as a fixed address) besides libultra's calls.

## Tools

- `tools/save_format.py`: derives the format from this source (the constants, the record types' layout, the copies in
  `WriteSaveRecord` / `ReadSaveRecord`, the note name from the ROM) and checks it: the parts fit, they do
  not overlap, every saved field is restored alike. `--write` writes `tools/save_format.json`; `--check` compares, and
  the build runs it before the link (`build/check_save_format.ok`).
- `tools/save_format.json`: the note's identity, the parts, every field with its offset, type and size (46 leaves),
  and the restore rules: the definition for other tools and for ports.
- `tools/eltale_save.py COMMAND IMAGE [options]`: reads a Controller Pak image (a raw 32 KiB pak: ares's `<rom>.pak`,
  Project64's `.mpk`; or a Mupen64Plus `.mpk` of four paks, `--pak 0-3`). `check` (the file system: the ID blocks,
  the inode table and its copy, the note table, the chains), `list`, `show [--json]`, `export` and `import` (a note
  as a file), `set PATH=VALUE` (a field by its JSON path), `scrub` (zero the tails and the unused bytes). An edit
  changes only the note's pages, checks the image afterwards and keeps a `.bak` beside it.
  `tools/test_eltale_save.py` runs its 19 cases on synthetic images.

## Original and modified builds

Checked in ares (a build where 16 KB more .bss moves every save variable after `gDust`, among them the save list, the
save data, the buffers, `gInventory` and the controllers' state):

- the original: two saves, an overwrite and the loads, every saved field restored (field by field against the RAM
  before the save); a deletion from the title; a full pak (prompt 21); a damaged pak (the boot manager);
- the modified build: its own save and load; its save loaded by the original, and the original's two saves loaded
  by it, every field restored; the boot manager's repair;
- a save edited by `tools/eltale_save.py` and loaded: the game shows the edited values.

The format does not depend on where the save state lives in RAM: the tails differ with the layout, and nothing reads
them. What a modified build must keep for its saves to stay compatible:

- the note's identity (codes, name, two pages) for old saves to be listed;
- the parts' offsets and sizes, and each field's offset and type (`tools/save_format.py` shows a change as a new
  definition: compare the JSON);
- `SaveSlot` within 0x80 bytes and `SaveData` within 0xE0 (the build fails otherwise). Growing a record needs the
  note's free bytes (0x160 on) or a larger note, and a version of its own: the original has none.

## For a port

A save is 0x130 meaningful bytes (0x68 of record, 0xC8 of data), big-endian, written as 0x160 bytes of a
0x200-byte note; the list is up to 16 notes. A port that stores
notes as files can keep the note layout unchanged and replace `ScanControllerPak`, `AllocateSaveNote`,
`DeleteSaveNote`, `ReadPakNote`, `WritePakNote` and the pak calls of `OpenSaveMenu` and `RepairPak` (osPfsInitPak,
osMotorInit, osGbpakInit); the tails can be written as zeros. The boot manager only
maintains a real pak (deletion, repair) and can be left out where there is none. `tools/save_format.json` gives every
field for converting saves between a pak image and a port's files.

## Not covered, and known limits

- `ControllerPakMenu` and `PakMenu_DrawText` are assembly (their findings: above, and the module's source comment).
- The "no pak" paths were read from the code, not run (the project's emulator harness, not part of this repository, sets no accessory per port); the boot
  manager's states 7 to 10 likewise.
- The 17-byte name test depends on a stack byte osPfsFileState never writes: zero in ares; real hardware is not
  checked.
- Project64's `.mpk` is read as a raw 32 KiB image; a file written by Project64 itself has not been tested.
