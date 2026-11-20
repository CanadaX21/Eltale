# Typed assets: what `./configure` writes from your ROM, and how to change it

Most of the game's data is built from typed sources. `./configure` writes it from your ROM into `assets/`, as C files,
source text and images, and `ninja` compiles it like the C in `src/`. The ROM that comes out is byte-identical to the
original, and every typed record can be read and changed as C. This page covers what applies to every family. Each
family's own page has its format and its limits.

## What `./configure` writes

| Family | Folder in `assets/` | Files | Its page |
|---|---|---|---|
| the 8 boss files | `boss_<name>/` | `records.anim.inc.c`: the animation records (each part's keyframes, each animation's part table, the display-list table and the animation table); `dl_<n>.gfx.inc.c` display lists; `vtx_<n>.vtx.inc.c` vertices; the textures as `.png` | below |
| the 6 monster sets (67 monsters) | `monsters_<n>/` | `anims_<nn>.anim.inc.c`: one model's animation records each; `monster_data.records.inc.c`: the monsters' AI options and base data with their stats, the set's tables and its names; display lists, vertices, textures (517 RGBA16, 3 I4); `raw_*.bin`, `pad_*.bin`: bytes kept as they are | below |
| the 36 maps | `map_<nn>/` | each map's records, display lists, vertices and textures | [MAPS.md](MAPS.md) |
| the NPC files | `npc_blob_<n>/`, `npc_held_objects/`, `shannon_actor/`, `event_blob_a/` (the ending's NPC records) | animation records, display lists, vertices, textures; each file's name pool as source text (`names.npcnames.txt`) | [NPCS.md](NPCS.md) |
| Brian | `brian_model/`, `brian_anim_<nn>/` | the model's display lists, vertices and textures; his 30 animation files | [PLAYER.md](PLAYER.md) |
| the spell bank | `spell_bank/` | the records as typed C (`*.spellrecords.inc.c`); I8 and I4 textures; CI8 textures with the palette their material names (`ci8tex`); raw pieces | below |
| the dialogue bank | `dialogue_bank/` | the NPCs' and the signs' strings as source text (`npc.dialogue.txt`, `signs.dialogue.txt`) | [TEXT.md](TEXT.md) |
| the font | `font/` | the dialogue font's sheet (`font_sheet.ci4.png`) and the text palettes | [TEXT.md](TEXT.md) |
| the title overlay | `title_overlay/` | its art | |

Every record is a symbol in `symbol_addrs.txt`, declared with its C type in its family's header (`include/boss_files.h`,
`monster_sets.h`, `maps.h`, `npcs.h`, `brian.h`, `spell_bank.h`); the images' and raw pieces' names are in
`undefined_syms.jp.txt`. The names are structural: by file, set, map or model and index (`gMonSet0Mon10Anim0Part00Keys`,
`gMonSet0BaseData`). A few families stay binary pieces of the ROM, among them the maps' shared blocks (locked, below),
the audio banks, most of the cutscene bank and the ending's data: [ARCHITECTURE.md](ARCHITECTURE.md) lists what is not
in C.

The records are written by this repository's own splat segment types (`tools/splat_ext/`, which splat loads through
`extensions_path` in `eltail.jp.yaml`). They take each record's layout and field names from the project's C types,
which `tools/c_layout.py` reads from the headers. So the headers are the one definition of these formats, for the
build and for the tools alike. splat's own types write the display lists, vertices and textures, and each texture is
declared in the format the game draws it in.

## How the records are written

- Each record is a C array of its type, in ROM order. A record of a structure type is written one element a line,
  with the field names in a comment; words and bytes up to eight words or sixteen bytes a line. A name string is
  bytes: the monsters' and spells' names are in the HUD font's encoding ([TEXT.md](TEXT.md) gives them as the game
  spells them).
- A pointer is the name of the record it leads to, so it follows a change of layout. A count that the game's code
  uses as an array's length is written `ARRAY_COUNT(that array)` where the two agree, so adding or removing an element
  keeps it right. Floats are the shortest decimals that give the same bits.
- A field named `unk...` is laid out by its type, but no code establishes what it means.
- **Raw data.** Bytes that nothing the game's C code is known to read as a type are declared `RawWord` or `RawByte`
  (`include/raw_data.h`): no type is claimed for them. A raw word that is the address of a named record is written as
  that name, so it still follows a layout change. Bytes that nothing reaches at all (end-of-list commands after a
  texture, alignment padding) are binary pieces.
- **Inferred lengths.** No field or constant states how long some tables are (a monster set's base records, its table
  of animation tables and its name table, a model's animation table and display-list table, and their like in the
  other families). Each is typed over the range the game's code is shown to index (the largest index used, plus one).
  That length is an inference, not a fact read from the data, and the comment over each such table starts "inferred
  length". What follows a table's inferred end is declared raw.
- **The boss files' records are all typed**: no record is raw, the two animations of one boss that the game's code
  does not play (Beigis' entries 7 and 8) are typed like the rest, and no table's length is marked as inferred.
- **The spell bank** is read whole to `gEffectsBlock` (0x800C0000) by `InitGraphics` and stays resident. It is packed
  on 8-byte boundaries, so its C units are runs of records between two images or raw pieces rather than one per list.

## Changing a file, and keeping the change

`./configure` rewrites `assets/` from the ROM every time it runs, so an edit made there does not last on its own.
`tools/asset_patches.py` keeps it apart, under `asset_patches/` at the same path, and the extraction puts it back:

- **text** (records, display lists, vertex runs) is kept as a unified diff against the generated text, which is
  kept beside the file as `FILE.pristine`;
- **an image** is kept whole (an edited PNG; its size may change, with the display list that loads it), with a
  `.base` file that names the image it replaces;
- **source text** (the dialogue bank, the NPC files' name pools) is kept the same way, and the build encodes it.

1. Run `./configure` once, so that `assets/` exists.
2. Edit a generated file. For example monster 0's `modelScale` in `assets/monsters_0/monster_data.records.inc.c`
   (`0.07f`; `0.105f` draws it at 1.5 times its size), a keyframe in `assets/boss_guilty/records.anim.inc.c`, a
   texture, or a line of dialogue in `assets/dialogue_bank/npc.dialogue.txt`.
3. Keep the change: `python3 tools/asset_patches.py save FILE` (or `save --all` for every edited file).
4. Build: `ninja`. It writes the changed ROM to `build/eltail.jp.z64` and then compares it with the original's SHA-1,
   which now fails (`build/eltail.jp.z64: FAILED`): that comparison is for the original build, and a changed ROM is
   expected to fail it. If `ninja` stops with any other error, the ROM in `build/` is not the changed one.
5. Fix the cartridge header's boot checksum: `python3 tools/n64crc.py build/eltail.jp.z64`. The console's boot code
   checksums the megabyte after it (ROM 0x1000 to 0x101000, main's code and data) and does not start the game when
   the two words at 0x10 disagree. A change that moves later data changes the references to it in main, so most size
   changes reach that range; the tool rewrites the two words only when they differ, and `--check` only compares them.

At every later `./configure`, the generated files are written from the ROM again and the kept changes put back.

- **A text change applies exactly or not at all.** Made against this very generated text, each hunk must match at the
  line it names. Made against an earlier text (a newer version of the tools or the definitions changed the file), each
  hunk's context must occur exactly once after the previous hunk, and the extraction reports each hunk it placed so.
  Otherwise `./configure` stops and names the patch: make the change again and `save` it.
- **An image is put back** only when the freshly extracted image is the one its `.base` names; otherwise
  `./configure` stops and names it.
- **An edit not kept yet stops `./configure`** before the split would overwrite it: keep it with `save`, or drop it with
  `python3 tools/asset_patches.py restore FILE`.

`python3 tools/asset_patches.py check` applies every kept change in memory and reports; `list` lists them. A clean clone
of the repository, your ROM and your `asset_patches/` folder rebuild the same changed ROM. The repository ships no kept
changes: the original build uses none.

## Kept changes hold lines of the original game's data

A kept change is a diff of generated text, or an image, and both are written from your ROM. A patch's context lines,
and the lines it replaces, are the original game's records; a kept image is the whole picture. Sharing them shares that
data.

## Limits

- **Room.** A changed file must still fit the RAM it is loaded into, and the linker refuses a build where it does not:
  a boss file must end at or before the depth buffer (`gDepthBuffer`), a monster set at or below `gShannonActorData`
  (0x802A0000), a map before the ending's data (`gEventBlob`), the spell bank at or below 0x80100000, and main's own
  RAM before the spell bank. The maps' shared blocks (`gameplay_common`, `interiors_common`, `field_shared`, the six
  region blocks) are locked: the maps hold addresses inside them, so their size and place cannot change. Each family's
  page gives its own bounds.
- **ROM size.** A grown file moves everything after it, and the ROM grows past 16 MiB (by the net change; the fill at
  the end is not trimmed). Emulators (ares, Project64) run it; real hardware and flash cartridges have not been tried.
- **What was tested.** For the monster sets, in one field area: a fight against a monster stored after a size change,
  with its animations and AI options exercised, and changes to one monster's animation keys and another's
  `modelScale`. For the boss files: one boss (Guilty), with a size change inside his file and an added animation key,
  through his speech and fight. The other families' pages list theirs. In an emulator and on Project64; every other
  path is untested in a changed build.
- **The HUD's names** are bytes in the generated records, not source text: change one with the bytes the HUD font's
  codec gives ([TEXT.md](TEXT.md)).
