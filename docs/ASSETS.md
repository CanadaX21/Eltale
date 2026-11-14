# The boss files and the monster sets: typed asset files

Two families of the game's model data are built from typed sources: the 8 boss files and the 6 monster sets (67
monsters). `./configure` writes them from your ROM into `assets/`, as C files and images, and `ninja` compiles them
like the C in `src/`. The ROM they make is byte-identical to the original, and their records can be read and changed
as C. The rest of the game's data (the maps, the NPC files, Brian's animations, the spell bank, the cutscene bank apart
from its boss tables, which are C files in `src/game/`, and the other blocks [ROM_MAP.md](ROM_MAP.md) lists) is still
copied as binary pieces of the ROM: nothing on this page applies to it.

## What `./configure` writes

| Folder | Files |
|---|---|
| `assets/boss_<name>/` | `records.anim.inc.c`: the boss file's animation records (each part's keyframes, each animation's part table, the display-list table and the animation table); `dl_<n>.gfx.inc.c`: display lists; `vtx_<n>.vtx.inc.c`: vertices; the textures as `.png` images |
| `assets/monsters_<n>/` | `anims_<nn>.anim.inc.c`: one model's animation records each; `monster_data.records.inc.c`: the set's monster records (each monster's AI options and base data with its stats, the set's table of animation tables and its name table, and the names); `mon<nn>_dl_<nn>.gfx.inc.c`: display lists; `mon<nn>_vtx_<nn>.vtx.inc.c`: vertices; `tex_<nnn>.<format>.png`: textures (517 RGBA16 and 3 I4 in the six sets); `raw_*.bin`, `pad_*.bin`: bytes kept as they are (below) |

Every record is a symbol in `symbol_addrs.txt`, declared with its C type in `include/boss_files.h` or
`include/monster_sets.h`; the names of the textures and raw pieces are in `undefined_syms.jp.txt`. Names are
structural: by boss, or by set and monster index (`gMonSet0Mon10Anim0Part00Keys`, `gMonSet0BaseData`). A monster's
model is its own index in every set.

The records are written by this repository's own splat extensions, the `anim` and `records` segment types
(`tools/splat_ext/anim.py`, `tools/splat_ext/records.py`; splat loads them through `extensions_path` in
`eltail.jp.yaml`). They take each record's layout and field names from the project's own C types in `include/quest_types.h`
(`ModelKeyFrame`, `ModelPart`, `AnimEntry`, `MonsterBaseData`, `EnemyAIOption`), which `tools/c_layout.py` reads, so the
headers are the one definition of these formats. splat's own types write the display lists, vertices and textures;
each texture is declared in the format the game draws it in.

## How the records are written

- Each record is a C array of its type, in ROM order. A record of a structure type is written one element a line,
  with the field names in a comment; words and bytes (the raw records, and the names, which are bytes in the game's
  text encoding) up to eight words or sixteen bytes a line.
- A pointer is the name of the record it leads to, so it follows a change of layout. A count that the game's code
  uses as an array's length is written `ARRAY_COUNT(that array)` where the two agree, so adding or removing an element
  keeps it right. Floats are the shortest decimals that give the same bits.
- A field named `unk...` is laid out by its type, but no code establishes what it means.
- **Raw data (the monster sets).** Bytes that nothing the game's C code is known to read as a type are declared
  `RawWord` or `RawByte` (`include/monster_sets.h`): no type is claimed for them. A raw word that is the address of a
  named record is written as that name, so it still follows a layout change. Bytes that nothing reaches at all
  (end-of-list commands after a texture, alignment padding) are the `raw_*.bin` and `pad_*.bin` pieces.
- **Inferred lengths (the monster sets).** No field or constant states how long some tables are: a set's base
  records, its table of animation tables and its name table, and each model's animation table and display-list table.
  Each is typed over the range the game's code is shown to index (the largest index used, plus one). That length is an
  inference, not a fact read from the data, and the comment over each such table starts "inferred length". What
  follows a table's inferred end is declared raw.
- **The boss files' records are all typed**, without those two rules: no record is declared raw, the two animations of
  one boss that the game's code does not play (Beigis' entries 7 and 8) are typed like the rest, and no table's length
  is marked as inferred.

## Changing a record, and keeping the change

`./configure` rewrites `assets/` from the ROM every time it runs, so an edit made there does not last on its own.
`tools/asset_patches.py` keeps a change to a record file as a patch, and the extraction applies it.

1. Run `./configure` once, so that `assets/` exists.
2. Edit a record file. For example, monster 0's `modelScale` in `assets/monsters_0/monster_data.records.inc.c`
   (`0.07f`; `0.105f` draws it at 1.5 times its size), or a keyframe in `assets/boss_guilty/records.anim.inc.c`.
3. Keep the change:

   ```
   python3 tools/asset_patches.py save assets/monsters_0/monster_data.records.inc.c
   ```

   This writes `asset_patches/monsters_0/monster_data.records.inc.c.patch`: a unified diff of the file against its
   generated text (kept beside it as `monster_data.records.inc.c.pristine`).
4. Run `ninja`. It writes the changed ROM to `build/eltail.jp.z64` and then compares it with the original's SHA-1,
   which now fails (`build/eltail.jp.z64: FAILED`): that comparison is for the original build, and a changed ROM is
   expected to fail it. If `ninja` stops with any other error, the ROM in `build/` is not the changed one.
5. If your change makes a file larger or smaller, everything after it in the ROM moves, and the main image, which
   holds the references to those places, changes with it. The main image lies in the first megabyte after the boot
   code, which the console's boot code checksums, so recompute the cartridge header's checksum (the two words at 0x10,
   for the CIC-6102 boot code) before running such a ROM on hardware or in an emulator that checks it. This repository
   does not include a tool for that. A change that keeps every file's size changes nothing in that range, and the
   checksum stays valid.

At every later `./configure`, the record file is written from the ROM again and its patch applied. A patch applies
exactly or not at all:

- made against this very generated text, each hunk must match at the line it names;
- made against an earlier text (a newer version of the tools or of the definitions changed the generated file), each
  hunk's context must occur exactly once after the previous hunk, at its own line or elsewhere, and the extraction
  reports each hunk it placed at another line;
- otherwise the extraction stops and names the patch. Make the change again in the new file and `save` it.

`python3 tools/asset_patches.py check` applies every patch to its pristine file in memory and reports;
`python3 tools/asset_patches.py list` lists the patches and the files they change. A clean clone of the repository,
your ROM and your `asset_patches/` folder rebuild the same changed ROM. The repository ships no patches: the original
build uses none.

## What is not kept

- **Textures, vertices and display lists.** Only the record files (`*.anim.inc.c`, `*.records.inc.c`) have this patch
  step. `./configure` also rewrites the textures (`.png`), the vertex files (`.vtx.inc.c`) and the display lists
  (`.gfx.inc.c`) from the ROM, and nothing keeps a change to them: an edit there is lost at the next `./configure`.
  To work on them, make the change after `./configure`, build with `ninja` without running `./configure` again, and
  keep your own copy of the files you changed. `asset_patches.py save` refuses these files.
- **A patch depends on the generated text around the change.** When a newer version of this repository changes that
  text inside a patch's context (a comment the writer emits, a note in `symbol_addrs.txt`), the patch has to be made
  again (above).

## Patch files hold lines of the original game's data

A patch is a diff of generated text, and that text is written from your ROM. Its context lines, and the lines it
replaces, are the original game's records: a few lines of record values around each change. Sharing a patch file
shares those lines.

## Limits

- **Size.** A changed file must still fit the RAM it is loaded into, and the linker refuses a build where it does not:
  a boss file must end at or before the depth buffer, and each monster set at or below 0x802A0000, where the NPC
  areas' actor data begins. The game has been run with a monster set 0x40 bytes longer and with no larger change. A
  higher bound read from the code, 0x802A7BC0, has not been verified at run time; the linker enforces 0x802A0000.
- **ROM size.** A grown set moves everything after it, and the ROM grows past 16 MiB: 16,777,280 bytes with a set
  0x40 bytes longer, against the original's 16,777,216. Emulators (ares, Project64) run it; real hardware and flash
  cartridges have not been tried.
- **What was tested.** For the monster sets, in one field area: a fight against a monster stored after a size change,
  with its animations and AI options exercised; a change to one monster's animation keys and to another's
  `modelScale`. For the boss files: one boss (Guilty), with a size change inside his file and an added animation key,
  through his speech and fight. In an emulator and on Project64. Every other path is untested in a changed build.
