# The NPC files, as the game reads them and as this source builds them

The characters the game places beside Brian (townspeople, shopkeepers, Shannon, the ending's figures) are models in
three kinds of file, built here from typed sources: the seven NPC area files (`npc_blob_0` ... `npc_blob_6`), Shannon's
block (`shannon_actor`) and the held objects (`npc_held_objects`). This page describes what the files hold, how the game
reaches them, how the build writes them, and what is known not to be typed. The map records that place the NPCs are
described in `docs/MAPS.md`; the record type is `NpcRecord` in `include/quest_types.h`.

## Where the files live in RAM

`LoadNPCArea(area)` (`src/game/enemy.c`) runs when a submap without random battles is entered (`SetupNPCs`) and for each
ending scene (`LoadEndingScene`). It loads three files, always at the same addresses:

| File | Size | RAM | Loaded by | The next RAM in use |
|---|---|---|---|---|
| `npc_held_objects` | 9,760 B | 0x8020C0D0 (the overlay slot, `func_8020C0D0`) | `LoadNPCArea` | `gMonsterFileBuffer` (0x8020E6F0), exactly at its end |
| `npc_blob_<area>` | 176,288 - 392,272 B | `gMonsterFileBuffer` (0x8020E6F0) | `LoadNPCArea`, the area's `gNpcBlobs` row | `gShannonActorData` (0x802A0000) |
| `shannon_actor` | 31,680 B | `gShannonActorData` (0x802A0000) | `LoadNPCArea` | `gBossVAddr` (0x802A7BC0), exactly at its end |

The area is the submap's `area` field (`Submap.area`), the same index that picks a monster set in a battle area: the NPC
files and the monster sets share `gMonsterFileBuffer`, and the title, intro and epilogue overlays load over the held
objects' slot. Brian's model (`brian_model`, at `brian_model_LINK`, 0x80206000 in the original, loaded once by `InitBrian`)
lies just below the slot and stays loaded; one NPC file's lists take a texture from it (`docs/PLAYER.md`).

The link asserts each envelope (`undefined_syms.jp.txt`): every file at its loader's address, and its end below the next
RAM in use. The largest area file, `npc_blob_0`, has 203,968 bytes of room below `gShannonActorData`; the held objects
and Shannon's block have none in the original, so they can change but not grow.

## What places an NPC

An `NpcRecord` (0x2C bytes) places one NPC: its submap, talk type and parameter, its two messages (offsets into the
dialogue bank: `DLG_NPC_<offset>`), its model (`anims`, the model's animation table), its name (a pointer into a name
pool), the animation it plays when talked to (`talkAnim`), its position, angle and scale. Two kinds of list hold them:

- each map's NPC list (`gMap<map>NpcList`, the map header's `npcList`): `SetupNPCs` spawns the records of the current
  submap; the submap's area decides which file is loaded, so a record names a model in that area's file (or in
  Shannon's block, which every area loads);
- the ending's seven scenes (`gEndingScenes` in the ending's data, `event_blob_a`, loaded by `EndingLoop`):
  `LoadEndingScene` loads a map and submap and spawns the scene's list (`gEndingScene0Npcs` ... `gEndingScene6Npcs`, 12
  records). The ending's data stays binary, but for these records, which are typed in place (an `npcrecords` window), so
  they name their models, names and strings and follow any change; its size and RAM are locked by the link.

157 map records and the 12 ending records place 141 distinct models: 140 in the area files and Shannon's one. A record's
model pointer is the split: a model is exactly what its animation table reaches.

## What an NPC does when talked to

`FindTalkTarget` (`src/game/player.c`) picks the NPC Brian can talk to: in front of him, within 15 units, or 25 for a record
with `NPC_REC_WIDE_TALK`. It finds none while the item menu, a spell's input, the spirit window, a change of place or a
visit to the save pages is on, or while a message the game opened is still holding (`PLAYER_MESSAGE_HOLD`). A press
starts `StartNPCTalk(index)` (`src/game/enemy.c`). The NPC turns to face Brian (`NPC_STATE_TALK`), unless its record has
`NPC_REC_NO_TURN`. Then the record's `talkType` decides the text, and what else happens:

| `talkType` | Name | The text | What else |
|---|---|---|---|
| 1 | `NPC_TALK_MESSAGE` | `message` | nothing (a value the switch does not name does the same) |
| 2 | `NPC_TALK_ITEM` | `message` while Brian holds none of the item `talkParam` (its low byte), `altMessage` while he holds one | gives the item each time he holds none; `PLAYER_ITEM_GIVEN` plays its sound when the talk ends |
| 5 | `NPC_TALK_BOSS_FLAG` | `message` until the boss `talkParam` is beaten (`IsStoryFlagSet`), `altMessage` after | with `LAST_BOSS` it would also restart the game (`GAME_STATE_RESTART`); no record has that |
| 6 | `NPC_TALK_INN` | `message`, with a Yes / No (`GAME_STATE_INN_CHOICE`) | Yes rests Brian and opens the save pages ([GAME_FLOW.md](GAME_FLOW.md)) |

The 169 records use 1 (74 records), 2 (16), 5 (62) and 6 (17); the talk type 5 records name the bosses 0 to 4. The
story flags are the bosses beaten, one bit each: `SetStoryFlag` sets the boss's bit when its battle is won, the boss's
number being its row in `gBossPlacements` (`LoadMapBoss` tests it; `LAST_BOSS` the last).

When every message window has closed, `BrianState_TalkEnd` ends the talk. `ResumeNpcAfterTalk` then sends the NPC back
to its own business:
- a wandering one (`NPC_KIND_WANDERS`) stands again (`NPC_STATE_STAND`) with a random timer;
- one with `NPC_REC_NO_TURN` stands;
- any other turns back to its own facing (`NPC_STATE_TURN_BACK`).

`UpdateNPCs` runs those states, and walks a wandering NPC about (`NPC_STATE_WALK`). At most three walk at once
(`gWalkingNpcCount`). A talk that interrupts a walk never counts it down, a bug of the original: after three such
talks no NPC of the submap sets off again until the next field start.

**To change what an NPC does:**
- to make its line change once a boss is beaten, give its record `NPC_TALK_BOSS_FLAG`, the boss's number in `talkParam`,
  and the two messages;
- to make it give an item, give it `NPC_TALK_ITEM` and the item's id.

The records in the generated files print `talkType` as a number; the names are in `include/quest_types.h`, and the
generator reads `NPC_TALK_ITEM` and `NPC_TALK_BOSS_FLAG` from there to know which records' `altMessage` is a string.

## A model

A model is the monster sets' and the bosses' format (`AnimEntry`, `ModelPart`, `ModelKeyFrame` in
`include/quest_types.h`, drawn by `DrawModel`, `src/game/model.c`):

- an animation table: one `AnimEntry` per animation (its part count, its frame count, its part table, the model's
  display-list table);
- a part table per animation: one `ModelPart` per part (its keyframes, key count, frame count, pivot, parent, display
  list index, flags, an optional per-frame display-list index table);
- keyframe arrays: `ModelKeyFrame` (position, three angles in degrees, three scales, tension, continuity, bias, a linear
  flag, the frame);
- the display-list table (`Gfx*`), its display lists, their vertex runs and textures.

**The animations the code plays.** Every call that sets an NPC's animation is in main's C (`SpawnNPCs`, `UpdateNPCs`,
`StartNPCTalk`, `ResumeNpcAfterTalk`; no overlay, event file or library routine calls `SetAnimation` or writes an NPC):
animation 0 (standing; every NPC), animation 1 where the NPC walks (record `kind` 2) or turns to face Brian when talked
to (record `flags` bit 1 clear), and, also only with bit 1 clear, the record's `talkAnim` once the turn is done (with
bit 1 set `StartNPCTalk` starts no turn, and `UpdateNPCs` never reaches the talk animation: 3 records, each with
`talkAnim` 0). The ending only draws its NPCs: animation 0. So an
animation table is typed over the entries the code plays (the largest played index + 1, an inferred length: no field
states it), and the entries after it are raw data (177 of the 466 the models' tables hold by the structural test).

**Shared records.** Models in one file share records freely: variants of one figure share keyframe arrays, part
tables, display lists and textures. A shared record is written once, named after the first model that reaches it, and
its note lists the others.

**Textures.** 600 RGBA16 and 69 I4 images; no texture uses a palette. An I4 texture is loaded as I8 at half its width
and drawn on a 4-bit tile. Most I4 textures are grey: the list's `gsDPSetPrimColor` command colours them (texture x
shade x the primitive colour), so one grey texture draws in many colours, and an NPC's colours are edited in its display
lists, not in the image.

**The held objects.** Shared textures (faces and small details most area files' models use) and a few meshes (two
models of area 1 draw `gNpcHeldDL_01`, a mesh of their own held in this file). Some lists load one image in several
shapes: a 16x1 strip at +0x80 is read as I4 by most areas, as I8 by area 5 and as RGBA16 by area 0, the larger reads
running on into the next image; an image's bytes run to the next image a list loads, and those reads are noted on it.
What a played animation's load reads is typed even past its own image. Loads read across images in four places: the
strip (`gNpcHeldTex_01`, read as I8 and RGBA16, into `Tex_02`); `gNpcHeldDL_01`'s 16x24 load of `gNpcHeldTex_10`, 7 rows into
`gNpcHeldTex_11` (which only a list no table names loads whole); area 0's list at 0x802633B0 (map 15's NPC 5), a 16x40
load of `gNpcArea0Tex_067` into `Tex_068`; and area 5's list at 0x8023A600 (map 23's NPC 6), a 16x32 load of
`gNpcArea5Tex_044` through `Tex_045` and `Tex_046`, whose image takes the 6 rows only that load reads (16x22). Those
images are one picture to that load: resizing one of them changes what it draws.

**Names.** Each file ends with its name pool: the speaker names its records point at, in the dialogue font's encoding (`text/encoding.json`),
each ending in 0xFF; `StartNPCText` (and `StartNPCTextFromRAM`) copies 0x20 bytes of one into `gSpeakerName`, so a name may
be at most 0x20 bytes. The strings lie on 4-byte steps with zero fill, the pool padded to 16 (the original's layout; the
code needs neither). The copy takes 0x20 bytes whatever the name's length: for a pool's last names it reads 0x8 to 0x18
bytes past the file's end, into whatever RAM follows (Shannon's: 0x10 bytes of the boss file buffer, `gBossVAddr`). The
game shows a name only up to its 0xFF, so this is harmless here; a port that gives each file exactly its size must
allow the read, or copy by length.

## How the build represents the files

Each file is a code segment at its load address, of, in ROM order:

- `anim` units: the animation records (typed C through the project's types, `tools/splat_ext/anim.py`);
- images (`rgba16`, `i4`: PNG files), each display list a `gfx` unit (gbi macros) and each vertex run a `vtx` unit;
- `records` units: raw display lists and vertex runs (below);
- `bin` pieces: bytes no played animation reads (no played list loads them and no record the code uses names them):
  textures only lists no played animation draws load, shaped by those loads (each piece's note gives the shape), bytes
  no list reads at all, lone end-of-display-list commands, zero pads before a texture-aligned unit;
- the name pool, one `npcnames` part (`tools/splat_ext/npcnames.py`): source text, `assets/<file>/names.npcnames.txt`,
  through the dialogue's encoding (`text/encoding.json`, its `npc_names` bank; `tools/text_build.py` encodes it at build
  time and defines each name as a label).

Names: a model's records are named from its area and its animation table's offset in the original file
(`gNpcArea0Chr06984Anims`, `...Anim0Part00Keys`, `...DLists`, `...DL_03`, `...Vtx_03`), which is the identity the map
records use; Shannon's block holds one model (`gShannonAnims` ...); a name string keeps its offset in the original file
(`gNpcArea0Name_5FB20`, as the dialogue's `DLG_NPC_<offset>`); textures are numbered per file (`gNpcArea0Tex_000`),
the held objects' records in ROM order (`gNpcHeldTex_00`, `gNpcHeldDL_01`). `include/npcs.h` declares every record with
its type. Where a model is placed by a map record, its note says which (`map 0 NPC 0 labelled Edgar`): the labels are
English labels by record, not the ROM's names, which are the name pools' Japanese text.

**Typed or raw.** A record is typed where the game's C code is shown to reach it; a table the code indexes is typed over
the range its indices reach and its note starts "inferred length"; everything else is raw data, declared `RawWord`,
`RawByte`, `RawGfx` (a display-list command: its second word a name where the opcode makes it an address) or `RawVtx`
(a vertex, in hex), with a note saying why. A raw word that is a record's address, or an address inside one, is written
as that name (and the offset), so raw data follows a change of layout too; a vertex holds no address, and the extraction
stops on a `RawVtx` row that holds one (as display-list commands among vertex rows would), as does check 21 of the ROM
map. Raw here:

| What | Count | Bytes (about) |
|---|---|---|
| animation entries past the largest the code plays, with what only they reach | 177 entries | most of the raw bytes |
| models no record or code reaches (found by the animation table's structural test): area 0 +0xD90 (3 animations), area 4 +0x3600 and +0x5634 (3 each), area 5 +0x1423C and +0x142BC (6 each) | 5 | 85,616 only they reach |
| animations no table names (a part table and its keyframes): area 0 +0x3EA8 (17 parts, 75 frames), area 1 +0x1116C and +0x18984 (7 parts, 95 frames each) | 3 | 13,836 |
| meshes in the held objects no table names, each a vertex run and the list after it that loads it: 25 vertices and a 31-command list (+0x1FA0), 35 vertices and a 55-command list (+0x2230) | 2 | 1,648 |
| textures only lists no played animation draws load (raw pieces, each shaped by its loads) | 40 | |
| bytes no list, record or pointer reaches (the held objects +0x988; area 5 +0x16A90, +0x1A230, +0x1A4B0; area 6 +0x23560) | 5 | 2,824 |
| lone end-of-display-list commands after a texture | 41 | 328 |
| names no record points at: area 4 ドルレイ, ハンプ; area 5 赤バラ騎士, 白バラ騎士 (Red Rose Knight, White Rose Knight) | 4 | |

Of the 2,132,000 bytes, 1,879,823 are typed records and 245,252 raw; the rest are the compiler's pads.

## Changing an NPC

Edit the generated files under `assets/<file>/` after `./configure`, then keep the change with
`python3 tools/asset_patches.py save <file>` (it goes to `asset_patches/`; `./configure` writes the files from the ROM
again and puts kept changes back):

- an animation: the keyframe arrays and part tables (`*.anim.inc.c`); a count the code reads as an array's length is
  written `ARRAY_COUNT(...)`, so adding or removing keys keeps it right;
- colours: the `gsDPSetPrimColor` commands in the display lists (`*.gfx.inc.c`);
- textures: the PNG files (same size; where one load reads several images as one picture, as above, keep them as they
  are or change that load too);
- names: `names.npcnames.txt` (text; the build stops on a name longer than 0x20 bytes).

A change that alters a record's size moves every later record of the file, and every reference follows: the maps' and
the ending's records name the models and names, the area files name the held objects' records, and gNpcBlobs names the
files' ROM bounds. The link stops a file that outgrows its envelope (the held objects and Shannon's block are full: they
can only shrink or stay the same size). Tested: a key added to a model's standing animation (area 0) and to the first
model of area 6 (every record of the file moves, among them those the ending names), a key removed from Shannon's (her
table and name move inside the block), a command removed from a held list (the held mesh moves under area 1's
references), a name made longer and a colour changed; the original ROM still builds byte for byte without the changes.
A grown file moves everything after it in the ROM, and the ROM grows by the net change (the fill at its end is not
trimmed: the test's ROM is 0x30 bytes over 16 MiB).

## Not typed, and known limits

- **The unreached models and names.** No record, list or code the static reading covers reaches the five models, the
  three animations or the four names above. Area 4 and area 5 each hold two such models and two such names; whether the
  game ever shows them (a scripted scene, a cut feature) is not known: a runtime log of what is drawn would settle it.
- **Brian's model** is built from typed sources (`docs/PLAYER.md`): the one texture an NPC list takes from it
  (`gBrianModelTex_01F60`, Auden's and Gylmyne's) is the model's own record, so it follows the model.
- **The ending's data** is binary but for its NPC records: its text, text pages and scene table hold addresses inside
  it, so its size and RAM are locked.
- **The held objects and Shannon's block** cannot grow without moving a fixed load address in main (the overlay slot,
  `gMonsterFileBuffer`, `gShannonActorData`, `gBossVAddr`).
- **Record fields.** `NpcRecord.unkC` and `unk1A` (zero in every record), and `kind` values other than 2, have no use
  the code shows.
- **Labels** in comments are English labels by record; the identifiers stay structural.
