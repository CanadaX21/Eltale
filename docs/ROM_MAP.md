# ROM map

Where things are in the original Japanese ROM of Eltale Monsters, which function loads each part and where it goes in
RAM: a starting point for reading the code that uses the game's data, or for writing a tool that reads the ROM. It
describes the ROM this repository builds (SHA-1 `4161b5c100ec82b0241b20ca8f81366e23564ccb`, 16 MB).

Offsets are ROM offsets in hex, ends exclusive; RAM addresses are the CPU's. Block and segment names are those of the
splat configuration, `eltail.jp.yaml`, whose comments also give the range of every segment inside a block (and a
name for each map, monster set, boss file and item icon); this page does not repeat them. Each bound but one is stated
by the game's own code or data: a DMA call with constant operands, a row of a pointer table, the boot copy, or the
data's format and content (the cartridge header, fill bytes up to the next block). The exception is 0x70550, where the
splat configuration ends the `main` segment. "Loaded by" names the function that makes the call, so every row can be
checked against the C.

## Blocks

| ROM | Bytes | Block | Holds | Loaded by | RAM |
|---|---|---|---|---|---|
| 0x0-0x1000 | 4,096 | `header`, `boot` | cartridge header and IPL3 boot code | the console's boot ROM |  |
| 0x1000-0x70550 | 456,016 | `main` | the game's main image: the CPU code (to 0x4C580: the boot entry, the 40 C units, the SDK library), the RSP microcode text, then `.data` and `.rodata` | IPL3, at boot | 0x80000400 (`entrypoint`) |
| 0x70550-0x71920 | 5,072 | `main_tail` | the rest of the image: libultra `.rodata`, then the F3DEX 1.23 and audio microcode data. The splat configuration ends the `main` segment at 0x70550; the boot copy does not stop there | IPL3, at boot | 0x8006F950 |
| 0x71920-0x753F0 | 15,056 | `segment1_gfx` | RSP segment 1 (`gSegment1Data` points at it): shared display lists and textures, the sky records' lists among them | `InitGraphics` (`src/game/main.c`) | 0x80095AF0 (`gSegment1Gfx`) |
| 0x753F0-0x851F0 | 65,024 | `gameplay_common` | gameplay common block, loaded whenever no map is loaded (`gCurrentMap` is -1), as at the start of play or after a respawn | `LoadMap` (`src/game/map.c`) | 0x80100000 |
| 0x851F0-0xC8DE0 | 277,488 | `interiors_common` | interiors common block, loaded for the interior maps 13-25 (map header flag 2); those maps' blobs load after it, at 0x801539F0 | `LoadMap` (`src/game/map.c`) | 0x8010FE00 |
| 0xC8DE0-0xEF280 | 156,832 | `region_blocks` | 6 region blocks, `region_0`-`region_5`; a map's header picks one (`regionBlock`) | `LoadMap` (`src/game/map.c`) | by `gRegionBlocks` row |
| 0xEF280-0x3E1090 | 3,087,888 | `maps_outdoor` | map blobs 0-12, the outdoor maps (`map_00`-`map_12`) | `LoadMap` (`src/game/map.c`) | by `gMapTable` row |
| 0x3E1090-0x3F1BA0 | 68,368 | `field_shared` | shared field block, loaded with maps 3, 5, 7 and 12 (a switch in `LoadMap`); no map table row owns it | `LoadMap` (`src/game/map.c`) | 0x801E0000 |
| 0x3F1BA0-0x869620 | 4,684,416 | `maps_interiors_dungeons` | map blobs 13-35: the interiors 13-25, then the forests and dungeons 26-35 | `LoadMap` (`src/game/map.c`) | by `gMapTable` row |
| 0x869620-0x86F6F0 | 24,784 | `brian_model` | Brian's model | `InitBrian` (`src/game/player.c`) | 0x80206000 |
| 0x86F6F0-0x871D10 | 9,760 | `npc_held_objects` | loaded with every NPC area into the overlay area: display lists and textures, not code; the NPC blobs' models take textures from it. The name is Quest 64's label for its same-size file | `LoadNPCArea` (`src/game/enemy.c`) | 0x8020C0D0 |
| 0x871D10-0xA70350 | 2,090,560 | `npc_blobs` | 7 NPC model blobs, `npc_blob_0`-`npc_blob_6`, one per NPC area (`gNpcBlobs`): the characters' models and their names | `LoadNPCArea` (`src/game/enemy.c`) | 0x8020E6F0 (`gMonsterFileBuffer`) |
| 0xA70350-0xA77F10 | 31,680 | `shannon_actor` | an actor loaded with every NPC area (`gShannonActorData`); the name is Quest 64's label for its same-size file | `LoadNPCArea` (`src/game/enemy.c`) | 0x802A0000 (`gShannonActorData`) |
| 0xA77F10-0xCC2280 | 2,401,136 | `monster_sets` | 6 monster sets, `monsters_0`-`monsters_5` (`gMonsterSets`), picked by the submap's area: base data, AI options, names and models | `LoadMonsterSet` (`src/game/enemy.c`) | 0x8020E6F0 (`gMonsterFileBuffer`) |
| 0xCC2280-0xD2E2F0 | 442,480 | `boss_files` | 8 boss files, one per boss number (`gBossFiles`): each boss's model | `LoadMapBoss` (`src/game/boss.c`) | 0x802A7BC0 (`gBossVAddr`) |
| 0xD2E2F0-0xD384B0 | 41,408 | `dialogue_bank` | dialogue and sign text: NPC messages from the start of the bank, signs' names and messages from 0xD37700 | `StartNPCText` (`src/game/enemy.c`), `LoadSignText` (`src/game/signs.c`) | 0x8007B430 (`gTextBuffer`), 0x8007B8E0 (`gSpeakerName`) |
| 0xD384B0-0xD388B0 | 1,024 | `item_icon_palettes` | item icon palettes: colour and grey, 256 RGBA5551 colours each | `Inventory_Init` (`src/game/hud.c`) | 0x803AC460 (`gInventoryPalette`) |
| 0xD388B0-0xD3A8B0 | 8,192 | `item_icons` | item icons, 16x16 CI8, 0x100 bytes per slot; the slot is the item ID. 32 slots fit before the next block (the code states no count); slots 26-31 are all zero | `ItemMenu` (`src/game/hud.c`) | 0x8008C1B8 (a cache of eight icons) |
| 0xD3A8B0-0xD3A930 | 128 | `text_palettes` | text palettes (0x80 bytes) | `loadTextTextureAndPal` (`src/game/text.c`) | 0x80332D30 (`gTextPalettes`) |
| 0xD3A930-0xD490C0 | 59,280 | `font_sheet` | the font sheet (its palettes are the block before) | `loadTextTextureAndPal` (`src/game/text.c`) | 0x803245A0 (`gFontSheet`) |
| 0xD490C0-0xD74AC0 | 178,688 | `spell_bank` | spell bank, loaded once at boot | `InitGraphics` (`src/game/main.c`) | 0x800C0000 (`gEffectsBlock`) |
| 0xD74AC0-0xD86170 | 71,344 | `cutscene_bank` | cutscene bank, resident from boot; it also holds the boss tables (`gBossFiles`, `gBossPlacements`, `gBossBaseData`, `gBossAnimTables`, `gBossNames`, `gBossTalkRadii`) | `InitGraphics` (`src/game/main.c`) | 0x8039ADB0 |
| 0xD86170-0xD86770 | 1,536 | `sky_palettes` | sky palettes, 3 x 256 colours, loaded with `gameplay_common` | `LoadMap` (`src/game/map.c`) | 0x800814B0 (`gSkyFadePalettes`) |
| 0xD86770-0xD86970 | 512 | `rotating_palette` | the rotating palette (0x200 bytes), loaded with `gameplay_common` | `LoadMap` (`src/game/map.c`) | 0x80081AB0 (`gRotatingPalette`) |
| 0xD86970-0xDA4570 | 121,856 | `sky_records` | 8 sky records, `sky_0`-`sky_7` (`gSkyRecords`): 0x800 bytes of palettes, then the sky texture | `LoadMap` (`src/game/map.c`) | 0x800808B0 (`gSkyPalettes`), 0x8007C6B0 (`gSkyTexture`) |
| 0xDA4570-0xDAB5F0 | 28,800 | `pak_menu_module` | Controller Pak menu module | `mainproc` (`src/game/main.c`) | 0x80100000 |
| 0xDAB5F0-0xDC62B0 | 109,760 | `title_overlay` | title screen overlay | `LoadOvl_TitleScreen` (`src/game/overlay.c`) | 0x8020C0D0 |
| 0xDC62B0-0xE22690 | 377,824 | `intro_overlay` | splash and intro overlay | `LoadOvl_Splash_and_Intro` (`src/game/overlay.c`) | 0x8020C0D0 |
| 0xE22690-0xE420D0 | 129,600 | `credits_overlay` | ending and credits overlay | `LoadOvl_Ending` (`src/game/overlay.c`) | 0x8020C0D0 |
| 0xE420D0-0xE43780 | 5,808 | `event_blob_a` | event blob A | `EndingLoop` (`src/game/main.c`) | 0x801C0000 (`gEventBlob`) |
| 0xE43780-0xE5BF60 | 100,320 | `event_blob_b` | event blob B | `AfterCreditsLoop` (`src/game/main.c`) | 0x801C0000 (`gEventBlob`) |
| 0xE5BF60-0xE5ED70 | 11,792 | `instrument_ctl` | instrument bank, `.ctl` (libaudio B1: 1 bank, 47 waves) | `InitMusicPlayer` (`src/game/audio.c`), given the address by `mainproc` | the audio heap |
| 0xE5ED70-0xE98330 | 234,944 | `instrument_tbl` | instrument samples, `.tbl` | `__amDma` (`src/game/audio.c`), streamed in 0x500-byte chunks | streamed |
| 0xE98330-0xECB7B0 | 210,048 | `sequence_bank` | sequence bank (S1: 44 songs, compressed MIDI) | `LoadSequence` (`src/game/audio.c`), one song at a time; `InitMusicPlayer` reads the song table | a music slot's 0x8000-byte buffer (audio heap) |
| 0xECB7B0-0xECD470 | 7,360 | `sfx_ctl` | sound-effect bank, `.ctl` (28 waves) | `InitSoundEffects` (`src/game/audio.c`), given the address by `mainproc` | the audio heap |
| 0xECD470-0xEF21A0 | 150,832 | `sfx_tbl` | sound-effect samples, `.tbl` | `__amDma` (`src/game/audio.c`), streamed in 0x500-byte chunks | streamed |
| 0xEF21A0-0xF2FB70 | 252,368 | `brian_animations` | Brian's 30 animations (`gBrianAnimRom`), loaded one at a time by animation ID | `LoadBrianAnimation` (`src/game/player.c`) | 0x80200000 |
| 0xF2FB70-0xF729B0 | 273,984 | `map_images` | the six region images of the Map page, 244 x 185 CI8 with a palette each (`gMapPageImages` rows 0-5) | `DrawMapPage` (`src/game/start_menu.c`) | by `gMapPageImages` row |
| 0xF729B0-0xF74890 | 7,904 | `map_cards_jp` | the four word cards of the Map page (`gMapPageImages` rows 6-9) | `DrawMapPage` (`src/game/start_menu.c`) | by `gMapPageImages` row |
| 0xF74890-0x1000000 | 571,248 | `rom_padding` | 0xFF to the end of the ROM |  |  |

At boot, IPL3 copies 1 MB from 0x1000 to 0x80000400; the game's image is its first 0x70920 bytes (to 0x71920), and
the game's boot entry (at 0x1000) clears `.bss` after it. [METHODOLOGY.md](METHODOLOGY.md), "What the figures cover",
splits the CPU code into the boot entry, the C units and the SDK library.

## Pointer tables

Tables in the main image (and one in the cutscene bank) that give the ROM range of each file in a block (for the
sky records, its start).

| Table | RAM | ROM | Rows x size | A row holds | Indexes | Read by |
|---|---|---|---|---|---|---|
| `gMapTable` | 0x80056BB0 | 0x577B0 | 36 x 0x44 | `sMapData`: ROM start +0x4, end +0x8, RAM +0xC, then pointers into the blob | map blobs 0-35 | `LoadMap` (`src/game/map.c`) |
| `gNpcBlobs` | 0x80056880 | 0x57480 | 7 x 0x8 | `RomRange`: start, end | NPC blobs 0-6, by NPC area | `LoadNPCArea` (`src/game/enemy.c`) |
| `gMonsterSets` | 0x800561A0 | 0x56DA0 | 6 x 0x14 | `MonsterSet`: start, end, then RAM pointers into the set (+0x8 models, +0xC base data, +0x10 names) | monster sets 0-5, by area | `LoadMonsterSet` (`src/game/enemy.c`) |
| `gRegionBlocks` | 0x80057540 | 0x58140 | 6 x 0xC | start, end, RAM | region blocks 0-5 | `LoadMap` (`src/game/map.c`) |
| `gBrianAnimRom` | 0x80055F98 | 0x56B98 | 30 x 0x8 | start, end | Brian's animations, by animation ID | `LoadBrianAnimation` (`src/game/player.c`) |
| `gBossFiles` | 0x803AB390 | 0xD850A0 | 8 x 0x8 | `bossFile`: start, end (the table is in the cutscene bank) | boss files, by boss number | `LoadMapBoss` (`src/game/boss.c`) |
| `gMapPageImages` | 0x800614E4 | 0x620E4 | 10 x 0xC | RAM +0x0, start +0x4, end +0x8 | rows 0-5 the region images, 6-9 the word cards | `DrawMapPage` (`src/game/start_menu.c`) |
| `gSkyRecords` | 0x80060C48 | 0x61848 | 8 x 0x8 | ROM start, then a segment-1 display list whose texture loads give the record's length | sky records 0-7 | `LoadMap` (`src/game/map.c`) |

## Code outside the C

The figures in the README count only the CPU code at 0x1000-0x4C580. Code elsewhere in the ROM:

- **Blocks the game loads at run time.** The title, intro and credits overlays share the overlay area at 0x8020C0D0
  (`LoadOvl_TitleScreen`, `LoadOvl_Splash_and_Intro` and `LoadOvl_Ending` in `src/game/overlay.c` load them), and
  `src/game/main.c` calls into them through `func_8020C0D0`, the loaded overlay's entry. The title and the intro
  overlays are C units (`src/title_overlay/`, `src/intro_overlay/`), counted apart from the README's main figures;
  their functions not yet in C, and the whole credits overlay, are assembly that splat disassembles from your ROM
  during the build, with every reference by name. The Controller Pak menu module loads to 0x80100000, where
  `gameplay_common` goes, and `mainproc` calls `func_801009EC` in it: assembly as well. The event blobs and the spell
  and cutscene banks may hold code too; nothing in the C calls into them, and they are extracted as binary data.
- **In the main image,** the RSP microcode text after the CPU code (from 0x4C580), binary data.
- **In `boot`,** the cartridge's IPL3.

## What the code reads

- **Maps.** `gMapTable[map]` is the map's 0x44-byte header, `sMapData` in `include/quest_types.h` (the current map's
  is `gMapHeader`): the blob's ROM range and RAM address, then pointers into it: geometry, model sets
  (`MapModelData`), submaps (`Submap`), lights (`MapLight`, `LightKey`), prop display lists, cameras, the two wall
  navmeshes (`NavMeshData`), the encounter tables, the NPC list (`NpcRecord`) and the region block's index. `LoadMap`
  (`src/game/map.c`) loads `gameplay_common` and the two palette blocks after it whenever no map is loaded,
  `interiors_common` when the header's flags have bit 1 set, the region block when `flags & 7`, the map's blob at the
  header's `ram`, and `field_shared` for maps 3, 5, 7 and 12.
- **NPC areas.** `LoadNPCArea` (`src/game/enemy.c`) loads `npc_held_objects` to 0x8020C0D0, `shannon_actor` to
  0x802A0000 and the area's blob, `gNpcBlobs[area]`, to 0x8020E6F0. A blob holds the characters' models (each an
  `AnimEntry` array with its `ModelPart`s, keys and display lists). The map headers' NPC records place them, and so do
  the ending scenes' own records (`LoadEndingScene` in `src/game/ending_text.c`).
- **Monsters.** `gMonsterSets[area]` is a `MonsterSet`: the set's ROM range, its model table (`anims`), its base data
  (`baseStats`, an array of `MonsterBaseData`) and its names (`namePtr`). `LoadMonsterSet` (`src/game/enemy.c`) loads
  the set for a field submap's area to 0x8020E6F0, the RAM the NPC blobs use. An encounter's enemy type indexes the
  base data; a record's `monsterID` picks its model and name, and `PickEnemyAIOption` chooses among its
  `aiScriptNumber` AI options (`EnemyAIOption`).
- **Bosses.** The eight placements, `gBossPlacements` (`BossPlacement`: map, submap, boss number, the model's
  animation table, position and facing), are in the cutscene bank with the bosses' base data, animation tables, names
  and talk radii. `LoadMapBoss` (`src/game/boss.c`) loads `gBossFiles[bossNumber]`, the boss's model, to 0x802A7BC0.
- **Sky.** A submap's light record (`MapLight.skyRecord`) picks one of `gSkyRecords`. When it changes, `LoadMap`
  reads a fixed 0x800 bytes of palettes to `gSkyPalettes` and 0x4000 bytes of texture to `gSkyTexture`, so a shorter
  record's read runs into the next record, and the last one's runs 0x1000 bytes into `pak_menu_module`.
- **Audio.** Standard libaudio files: two `.ctl` bank files (B1), whose samples are streamed from their `.tbl` blocks
  by `__amDma` in 0x500-byte chunks, and an S1 sequence bank of 44 compressed-MIDI songs. `mainproc` passes their ROM
  addresses to `InitMusicPlayer` and `InitSoundEffects` (`src/game/audio.c`), which read the two banks and the song
  table into the audio heap; `LoadSequence` reads one song at a time into a music slot's 0x8000-byte buffer.
- **Text.** `StartNPCText` (`src/game/enemy.c`) reads 0x4B0 bytes of a message from the dialogue bank, at the
  message's offset, into `gTextBuffer`. `LoadSignText` (`src/game/signs.c`) reads 0x20 bytes into `gSpeakerName` and
  0x200 bytes of text into `gTextBuffer`, each at 0xD37700 plus an offset from the sign's data.
  `loadTextTextureAndPal` (`src/game/text.c`) loads the font sheet and its palettes.
- **Brian.** `InitBrian` loads the model; `LoadBrianAnimation` (both in `src/game/player.c`) loads one animation at a
  time from `gBrianAnimRom` to 0x80200000.
- **Menus.** `ItemMenu` (`src/game/hud.c`) reads an item's icon slot by item ID into a cache of eight icons, with the
  palettes `Inventory_Init` loads. `DrawMapPage` (`src/game/start_menu.c`) draws the Map page from `gMapPageImages`.

## Open questions

- **Unreached bytes.** Some ranges in the data blocks are reached by no record that the code's tables lead to: about
  49 KB over the 36 map blobs, about 103 KB in the NPC blobs and about 1.4 KB in the monster sets. They include single
  end-of-display-list commands right after textures, palette-sized ranges, and, in NPC blobs 4 and 5,
  self-contained animation data for models that the map headers' NPC lists do not place. This map does not follow the
  ending scenes' NPC records (`LoadEndingScene`) or the event blobs' contents.
- **Animations the C does not play.** The C plays a model's animations 0 and 1, its hit animation and the ones its AI
  options name. About a third of the monster models' animation entries, and two of the bosses', are none of these;
  whether the event code plays them is not established.
- **The last sky record's read.** Nothing found in the code or in the ROM's display lists reads the part of
  `gSkyTexture` that the last sky record's read fills from `pak_menu_module`. That comes from reading the code, not from
  a run; display lists built at run time, address arithmetic from another symbol and the event code were not checked.
- **Sign messages.** Their offsets are not enumerated, so whether a sign's fixed 0x200-byte read can run past the
  dialogue bank's end is not settled.
- **Not partitioned.** The region blocks are located and the parts the maps reach are typed, but they are not split
  further; nor are the spell and cutscene banks, apart from the boss records. The spell effects' geometry is not
  decoded.
- **Not decoded.** Nothing on this page converts content: textures, models, samples and songs are located and typed,
  not decoded. Item icon slots 26-31 are all zero (reserved or unused).
- **RAM at run time.** The RAM addresses here are those the code states. The audio files go to buffers allocated
  from the audio heap at run time, so no address is given for them.
