# Brian's model and animations, as the game reads them and as this source builds them

Brian, the player character, is one model (`brian_model`) and thirty animation files (`brian_anim_00` ...
`brian_anim_29`), built here from typed sources. Unlike every other character, his model stays in RAM for the whole
game and his animations are streamed one at a time into a buffer below it. This page describes what the files hold,
how the game loads, selects and plays them, how the build writes them, how to change them, and what is known not to be
typed. The NPCs are described in `docs/NPCS.md`; the record types are `AnimEntry`, `ModelPart` and `ModelKeyFrame` in
`include/quest_types.h`.

## Where the files live in RAM

| File | Size | RAM | Loaded by |
|---|---|---|---|
| `brian_model` | 24,784 B | `brian_model_VRAM`: linked at `brian_model_LINK` (0x80206000) | `InitBrian` (`src/game/player.c`), once a game, from mainproc's outer loop |
| `brian_anim_NN` | 4,016 - 18,688 B | `BRIAN_ANIM_BUFFER` (0x80200000) | `LoadBrianAnimation` (`src/game/player.c`), whenever the animation changes |

In play the RAM around them is:

| RAM | Holds | Bound |
|---|---|---|
| 0x801F8000 | the region block `LoadMap` loads | at most 0x7A70 B (region_1); the region blocks are locked by the link |
| 0x80200000 | the animation playing (`BRIAN_ANIM_BUFFER`) | the largest file, animation 21, 0x4900 B |
| 0x80206000 | his model | 0x60D0 B, ending exactly at the overlay slot |
| 0x8020C0D0 | the overlay slot: the NPCs' held objects in play; the title, intro and epilogue overlays | |

The largest animation file and the model share the 0xC0D0 bytes between the buffer's start and the slot: 43,472 B
are used, 5,888 B are free. Before `InitBrian` runs, the boot-time Controller Pak manager uses 0x80200000 onward as
two frame buffers (`docs/SAVE.md`); nothing else the code is known to load uses this range in play.

The link asserts the real limits (`undefined_syms.jp.txt`): every animation file linked at `BRIAN_ANIM_BUFFER` and ending
below `brian_model_VRAM`, and the model ending below `npc_held_objects_VRAM` (the slot). A file that outgrows its room
stops the link with its name.

## How the game loads, selects and plays an animation

- **Loading.** `LoadBrianAnimation(brian)` keeps the ID it last loaded in a static (`sLoaded`, -1 at boot). When
  `brian->animID` differs, it DMAs `gBrianAnimRom[animID]` (`src/game/sound.c`: each file's ROM start and end) to
  `BRIAN_ANIM_BUFFER`. Every reader of an animation calls it first: `DrawBrian` before drawing and again after
  `AdvanceAnimation` (which may have switched to the next animation), `ApplyBrianMovement` (root motion), and the starts
  that read an animation's first frame at once (the door arrivals in `SpawnBrian` and `BrianState_EnterDoor`,
  `StartBrianPickUp`). The buffer is an immediate in the code (`lui` alone: a symbol would compile to one instruction
  more), so it is a named constant, not a symbol; the model's address is a symbol (`brian_model_VRAM`), and follows a
  change of layout.
- **Selecting.** `gBrianAnims[modelID]` (`src/game/sound.c`, one row) is the model's animation table, `gBrianModelAnims`
  (`src/brian_model/brian_model.c`). The game sets an animation with `SetAnimation(transform, anim, rate, afterAnim,
  afterRate, flags)` (`src/game/model.c`); the IDs it sets are named `BRIAN_ANIM_...` in `include/game/player.h`. Every
  `SetAnimation` call in the ROM is main's C, and Brian's are in `src/game/player.c` and once in `src/game/main.c`.
  `PlayerState.animCue` is only ever 0, `BRIAN_ANIM_DOOR` or `BRIAN_ANIM_DOOR_FLAG1`.
- **Playing.** `AdvanceAnimation(rate, transform, entry)` adds the rate to `animFrame`; past the entry's `frames` it
  switches to `afterAnim` (frame 0) or, when that is the same animation, loops. `totalAnimFrames` counts the frames
  drawn since the animation changed (not reset on a loop). `DrawModel` draws each part of the entry from its keys
  (`SampleSpline`) with the part's display list from the model's table.
- **The blink.** Part 0's display list is chosen per drawn frame from `gBrianModelDlFrames[totalAnimFrames % 80]`
  where the part's `dlistFrames` points at it (animations 0, 1, 2, 13, 25 and 27): list 0, with lists 23 and 24 at
  frames 20-21 and 28-29. Lists 0, 23 and 24 each set a texture and branch to one shared list
  (`gBrianModelDL_03E30`).
- **The face.** The head's lists differ only in the face texture they load before that branch:

  | List | Face | Texture |
  |---|---|---|
  | 0 (`gBrianModelDL_00`) | eyes open | `gBrianModelTex_00560` (32x32 RGBA16) |
  | 23 (`gBrianModelDL_23`) | eyes closed | `gBrianModelTex_01560` (32x32 RGBA16) |
  | 24 (`gBrianModelDL_24`) | eyes half closed | `gBrianModelTex_00D60` (32x32 RGBA16) |

  Part 0 draws list 0 in 21 animations; the per-frame table in animations 0, 1, 2, 13, 25 and 27 (the blink, open
  -> closed -> half -> open); and list 23 throughout animations 7, 8 and 9 (the dodge, the hit, defeated: eyes closed).
  A change to Brian's face therefore changes all three textures, or the face reverts while he blinks and in those three
  animations.

## Which state plays which animation

`UpdateBrian` runs `gBrianStateHandlers[PlayerState.state]` each frame. Standing, his idle (`gBrianIdleAnim`) is
`BRIAN_ANIM_STAND`, `BRIAN_ANIM_BATTLE_STAND` in a battle, or `BRIAN_ANIM_STAND_LOW_HP` with HP under a third.

| ID | Name | Frames | Parts | File | Played by |
|---|---|---|---|---|---|
| 0 | `BRIAN_ANIM_STAND` | 70 | 23 | 5,584 B | the idle; `src/game/main.c` on leaving the save mode |
| 1 | `BRIAN_ANIM_WALK` | 300 | 23 | 9,120 B | `BrianState_Walk` (state 1; rate want * 14 + 3); `BrianWalkThroughDoor` (rate 10) |
| 2 | `BRIAN_ANIM_RUN` | 300 | 23 | 9,120 B | `BrianState_Run` (state 2) |
| 3 | `BRIAN_ANIM_CAST` | 30 | 24 | 9,200 B | `BrianStartCast`: a spell at a target (state 14) |
| 4 | none | 13 | 24 | 6,800 B | structurally verified; no normal-game playback path identified |
| 5 | none | 16 | 23 | 5,728 B | structurally verified; no normal-game playback path identified |
| 6 | none | 27 | 23 | 7,920 B | structurally verified; no normal-game playback path identified |
| 7 | `BRIAN_ANIM_DODGE` | 20 | 24 | 6,800 B | `StartBrianDodge`: an attack missed (state 3 for its frames) |
| 8 | `BRIAN_ANIM_HIT` | 15 | 24 | 6,800 B | `DamageBrian` (state 5 for its frames) |
| 9 | `BRIAN_ANIM_DEFEATED` | 300 | 23 | 14,256 B | `BrianState_KnockedDown` at HP 0 (state 6) |
| 10 | `BRIAN_ANIM_STAFF_SWING` | 30 | 23 | 7,664 B | `StartBrianStaffSwing` (state 13) |
| 11 | `BRIAN_ANIM_STAFF_COMBO` | 36 | 23 | 8,432 B | `BrianState_StaffSwing`: the second swing (state 16) |
| 12 | `BRIAN_ANIM_SIGN_START` | 30 | 23 | 4,016 B | `UpdateBrian`: reading a sign; then 13 |
| 13 | `BRIAN_ANIM_SIGN_LOOP` | 60 | 23 | 6,352 B | after 12 |
| 14 | `BRIAN_ANIM_SIGN_END` | 30 | 23 | 4,480 B | `BrianState_FaceAndMessage`, after the message |
| 15 | `BRIAN_ANIM_STOP` | 32 | 23 | 9,120 B | `BrianState_Walk`, `BrianState_Run`: stopping from speed, from its frame 3 (state 4) |
| 16 | `BRIAN_ANIM_VICTORY` | 83 | 23 | 18,576 B | `UpdateBrian`: a battle won (state 15 for its frames) |
| 17 | `BRIAN_ANIM_BATTLE_STAND` | 27 | 23 | 5,520 B | the idle in a battle |
| 18 | `BRIAN_ANIM_BATTLE_START` | 35 | 24 | 9,200 B | `SetupBrianForBattle` (state 3 for its frames) |
| 19 | none | 20 | 23 | 5,008 B | structurally verified; no normal-game playback path identified |
| 20 | `BRIAN_ANIM_DOOR` | 90 | 23 | 15,200 B | through a door model whose door's flags have bit 0 clear (`animCue`; root motion) |
| 21 | `BRIAN_ANIM_DOOR_FLAG1` | 105 | 23 | 18,688 B | through a door model whose door's flags have bit 0 set (`animCue`; root motion) |
| 22 | `BRIAN_ANIM_USE_LOOP` | 10 | 24 | 6,800 B | after 23 |
| 23 | `BRIAN_ANIM_USE_START` | 18 | 23 | 6,720 B | `StartBrianUseItem` (state 12), `BrianStartCast` for a spell on himself; then 22 |
| 24 | `BRIAN_ANIM_USE_END` | 24 | 23 | 6,720 B | `BrianState_Cast`, `BrianState_UseItem` at their end (state 3) |
| 25 | `BRIAN_ANIM_STAND_LOW_HP` | 24 | 23 | 5,104 B | the idle with HP under a third |
| 26 | `BRIAN_ANIM_DOOR_KIND3` | 53 | 23 | 11,456 B | `BrianWalkThroughDoor`, door kind 3 (state 3 for 75 frames; then walking at rate 10) |
| 27 | `BRIAN_ANIM_NO_TARGET` | 40 | 23 | 7,088 B | `UpdateBrian`: A with nothing to act on; a sign with `SIGN_NO_POSE` |
| 28 | `BRIAN_ANIM_PICK_UP` | 41 | 23 | 7,088 B | `StartBrianPickUp`: taking an item from a map object (state 11; root motion); then 29 |
| 29 | `BRIAN_ANIM_PICK_UP_END` | 50 | 23 | 7,808 B | after 28 |

The names say what plays an animation, not what it shows. Animations 4, 5, 6 and 19 are **structurally verified
animations with no identified normal-game playback path**: complete files that the loader's table and the model's table
both list (each row bounds its file, each entry counts a part table that fills it), which load and play through the game's
own loader and draw path when their ID is set, but which no code sets, so no code indexes either table with them in normal
play (the project's ROM map, a research tool outside this repository, grades their walks content). They are typed source, and their notes say so ("not played"). This is a
distinction between structural understanding and runtime use; it is not a rule for other unplayed data (the NPC files and
the monster sets declare the records only an unplayed entry reaches raw: `docs/NPCS.md`, `include/monster_sets.h`). Animations 3 (the cast), 4, 7 (the dodge), 8 (the hit), 18 (the battle start) and 22 have
24 parts: the 24th (part 23) hangs from part 21 and draws no list.

## The model

| Offset | Holds | In the source |
|---|---|---|
| +0x000 | the display-list table, 25 lists by index (index 22 holds none) | `gBrianModelDLists`, `src/brian_model/brian_model.c` |
| +0x064 | the animation table, 30 entries (part count, frames, part table, display-list table) | `gBrianModelAnims`, the same file |
| +0x1CC | part 0's display list per drawn frame, 80 entries | `gBrianModelDlFrames`, the same file |
| +0x30C | 41 words no part points at (raw) | `gBrianModelRaw_0030C`, the same file |
| +0x3B0 | 23 textures: 19 RGBA16, 2 I4, 2 IA4 (no palettes) | `assets/brian_model/tex_*.png` |
| +0x3940 | vertex runs and display lists, each run before the list that loads it | `assets/brian_model/vtx_*`, `dl_*`, `lists_03E30` |

The skeleton: part 2 is the root; parts 1, 7 and 12 hang from it, part 0 from part 1, parts 5, 10 and 22 from part 1,
and so on (each part's parent is in its record). Part 22 draws no list (its index is the table's empty one): parts 15 and
18 hang from it. Part 21 changes parent between animations (part 6 or part 11). Part 0's list is the one that changes per
frame.

## How the build represents the files

- `brian_model` is a code segment in a RAM family of its own, extracted at 0x80206000 (its yaml `vram`) and linked at
  `brian_model_LINK`, which `configure.py` writes into the linker script (the original's 0x80206000; `brian_model_VRAM` is
  wherever the link puts it). Its first 0x3B0 bytes are the C unit
  `src/brian_model/brian_model.c`, written once from the ROM and kept as source: its animation table names, entry by
  entry, the part table of the file `gBrianAnimRom` loads for that ID, which no address could tell apart (all thirty
  files are linked at the same address), and counts its parts with `ARRAY_COUNT` against the size `include/brian.h`
  declares, so an animation file whose part table has another length does not compile. The rest is written from the ROM
  at extraction: each texture an image, each vertex run `vtx`, each display list `gfx`, and lists 0, 23 and 24 with the
  list they branch to as one `gfxrecords` unit (they follow one another with no 16-byte boundary between them, and
  every C unit pads itself to 16).
- Each `brian_anim_NN` is a code segment at `BRIAN_ANIM_BUFFER` in the `brian_anim` RAM family, one `anim` run: each
  part's keys (`gBrianAnimNNPartPPKeys`), then its part table (`gBrianAnimNNParts`). A part that uses the per-frame
  table names `gBrianModelDlFrames`.
- Names are structural: by animation and part, by display-list index, and by offset in the model for textures and
  vertex runs (`gBrianModelTex_01F60` is the texture NPC area 0's lists, Auden's and Gylmyne's, take from the model: a
  record of the model, so it follows it).
- `include/brian.h` declares the records; the model's unit, the generated files and any game unit
  that uses a record include it.
- The project's ROM map (outside this repository) reads each entry of the model's table with its file at the buffer and compares the
  definitions with that walk.

## Changing Brian

- **Keys and parts:** edit `assets/brian_anim_NN/anim_NN.anim.inc.c`, then keep the change with
  `python3 tools/asset_patches.py save <file>`. A part's key count follows its array (`ARRAY_COUNT`). A file may grow
  until it reaches the model; the link says so if it does not fit.
- **Textures, vertices and lists:** edit the files in `assets/brian_model/` and keep them the same way (an image whole).
- **The model's tables:** edit `src/brian_model/brian_model.c` (source). An entry must keep pointing at its own file's
  part table: the loader loads file k for animation k.
- **More room for the model:** lower `brian_model_LINK` (`undefined_syms.jp.txt`); the model may then grow by as much,
  as long as it stays above the largest animation file (the link asserts both ends). Nothing else changes: every
  reference into the model is a name. Leave the segment's `vram` in `eltail.jp.yaml` as it is: it is the address splat
  extracts the original ROM at, whose own pointers name the records there; `configure.py` writes the section's link
  address as `brian_model_LINK` in the linker script after the split.
- **A new animation:** not without code: the loader's table and the model's table have thirty rows, and the IDs are
  set by the C code.
- After a change: `ninja`, then `python3 tools/n64crc.py build/eltail.jp.z64` (a modified build fails the checksum step
  by design).

## For a port

- The model is resident; one animation file is resident at a time, reloaded on a change of ID (at most twice a frame).
  A port can keep all thirty in memory and drop the streaming: nothing reads the buffer but the current animation.
- The buffer's address is an immediate in `LoadBrianAnimation`; the link's asserts tie the animation files to it.
- `sLoaded` survives a new game while `InitBrian` sets `animID` to -1. Every path into play the code has calls
  `SpawnBrian` (which sets an animation) before Brian is first drawn, so the load with ID -1 (it would read the row
  before `gBrianAnimRom`) is not reached; a port that draws him earlier must not rely on it.
- Part 22's list index names the table's empty entry; `DrawModelPart` tests it and draws nothing.

## Not typed, and known limits

- 41 words after the per-frame table (`gBrianModelRaw_0030C`): by their values more per-frame indices (as if the table
  ran to 121 entries: 23 at 87-91, 24 at 92); no part points at them and no code is known to read them.
- Animations 4, 5, 6 and 19: structurally verified, with no normal-game playback path identified; what they show is
  known from their keys and from forcing them at run time (they play without fault).
- The animation names follow the code that plays them; what an animation looks like is not recorded here.
