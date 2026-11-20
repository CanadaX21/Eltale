# Eltale's maps: the world data, as the game reads it and as this source builds it

The game's 36 maps (gMapTable's rows, `src/game/sound.c`) are built from typed C: every record a C type from
`include/map_format.h` and `include/quest_types.h`, every reference in and out of a map a name, so a change to a map's
records moves its later data and everything that points at it follows. The original ROM is rebuilt byte for byte.
This page says what the data is, how the game loads and reads it, how the build represents it, and how to change it.

## Where a map lives in RAM

LoadMap (`src/game/map.c`) loads, for the map `gCurrentMap` names:

| What | RAM | When |
|---|---|---|
| gameplay_common (shared block, binary) | 0x80100000 | every map (the Controller Pak menu module loads there instead) |
| an outdoor map (0-12, 26-35) | 0x8010FE00 | its gMapTable row's load address |
| interiors_common (shared block, binary) | 0x8010FE00 | the interiors, 13-25 (row flags & MAP_INTERIOR) |
| an interior map (13-25) | 0x801539F0 | its row's load address |
| the ending's data (gEventBlob) | 0x801C0000 | kept resident by the ending beside the map a scene loads: every map's RAM ceiling |
| field_shared (shared block, binary) | 0x801E0000 | maps 3, 5, 7, 12 |
| region_0 .. region_5 (shared blocks, binary) | 0x801F8000 | the row's regionBlock, through gRegionBlocks, when flags & (MAP_TOWN \| MAP_INTERIOR \| MAP_FIELD) |

The maps share RAM, one at a time. The shared blocks are one binary piece each; the maps name addresses inside them
(display lists, textures, vertices, collision), so the linker asserts each block's RAM and size (`undefined_syms.jp.txt`:
the locks), and each map's end against gEventBlob (its ceiling). A name inside a block is the block's label and the
offset (`gRegion2Tex_02210 = region_2_VRAM + 0x2210`). NPC records name their models and names in the NPC files, which
are built from typed sources too (`gNpcArea6Chr058D0Anims`, `gNpcArea6Name_5C148`, `gShannonAnims`; `docs/NPCS.md`):
each follows its record.

## The map file (sMapData, the gMapTable row)

The row holds the ROM range, the load address and the pointers below (as names); the C types are the game's.

| Header field | Records | Count, and its evidence |
|---|---|---|
| submaps | Submap[] (0x18): model set, light, doors, entrances, area, flags | the header's last word, which no code reads |
| Submap.doors / entrances | MapDoor[] (0x24) / MapEntrance[] (0x14) | doorCount / a u16 no code reads |
| modelSets | MapModelData[] (0x2C) | the largest a submap names + 1 (inferred) |
| MapModelData.transforms | ModelTransform[] (0x18): x, y, z, angle, scale, index, flags | transform_count |
| MapModelData.tile_offsets, wall_defs | u16 indices into the lists; the lists: a count and that many transform indices | the tiles; the lists the offsets name |
| geometry | ModelGeometry[] (0x20): display list, collision faces (GeometryFace, 0x14), collision vertices (GeometryVertex, 8) | the largest a drawn model names + 1 (inferred) |
| lights | MapLight[] (0x20) and LightKey[] (0x3C) | the largest a submap names + 1 |
| propGfx | MapPropGfx[] (8: setup and model display lists, by prop type) | see Props |
| cameras | MapCameras[] per submap, CameraSetup[] (0x38) | one per submap; count |
| cameraNavmesh, actorNavmesh | NavMeshData per submap: wall vertices, indices, groups | one per submap |
| encounters, encounterSets | EncounterZoneList[], EncounterSet[] per submap | up to the last submap whose flags have SUBMAP_ENCOUNTERS |
| npcList | sMapList: count and NpcRecord[] (0x2C) | count |

Main's other tables into a map: gMapObjectTable (SpawnMapObjects, `src/game/map_objects.c`), gSubmapSpiritTable (SetupMapSpirits,
`src/game/spirits.c`), DrawMapBillboards' sTextures and sHeights (`src/game/map.c`), and gLavaPalette, map 29's palette that
UpdateLavaPalette rotates (a name of map 29's record). Nothing else outside the maps holds an address of their records:
a sweep of every code site that builds an address in the maps' RAM and every aligned word outside them found these
tables and that palette, and otherwise text bytes, the shared blocks' own pointers and data with no structure.

### Models, tiles and what is drawn

- **Placed models** (SortMapModels): a transform's kind is `flags & 0xFF`, or 0 if flags & 0x2000: such a model
  follows a path (DrawMapFrame: `gMapModelPaths[(flags & 0xF00) >> 8]`, its frame counter kept in the low byte, which
  the game rewrites every frame). Doors are models with flags & 0x100 (FindDoorAnim). 0 and 8 are drawn (flags &
  0x4000: the late list); 1-3 float (the game writes their x, y, z); 0x10 props; 0x11, 0x12 flames (DrawFlames: main's
  model and frames, the index is the animation counter, seeded at random when the map loads); 0x40-0x45 scrolling
  textures (DrawMapBillboards: sTextures[kind - 0x40], an RGBA16 tile 32 wide and sHeights rows; sTextures has five
  entries, so a kind 0x45 would read past it: no map places one); 0x7F, 0x80 billboards; 0x81 environment objects;
  0x82 spinning props; 0x83, 0x84 dust emitters. flags & 0x1000 makes a music source (song (flags & 0xF00) >> 8). For
  kinds 0-3, 8, 0x40-0x45, 0x7F, 0x80 and 0x82 the index names a geometry entry.
- **Tiles** (DrawMapFrame, PollWorldCollision): a set of tile_columns x tile_rows tiles of tile_w x tile_d from
  (min_x, min_z); tile = columns x z index + x index; its list is `wall_defs[tile_offsets[tile]]`. An untiled set (tile
  mode 0) is drawn from `wall_defs[tile_offsets[0]]` but polled for collision from `wall_defs` itself: the same list
  while tile_offsets[0] is 0, as in every original set (a change to it would split what is drawn from what collides).
  DrawMapFrame draws
  the camera's tile and a neighbourhood from gTileOffsets1-4 (`src/game/sound.c`) by the set's mode: 1 (157 entries; maps
  26, 27, 29, 30, 35), 2 (89; maps 28, 31, 32, 33), 3 (9; no map), 4 (21; map 9); any other mode tile 0 alone. Mode 1
  lists (2, -6) twice and never (3, -6); mode 4 (-1, -2) twice and never (1, -2): game data, kept. **The tile offsets
  are u16 indices, not pointers**: a list made longer moves the later lists, and the later tiles' offsets must follow.
- **Props** (DrawMapProps): while props are near, loads the table's entries for types 0-3 every frame and draws a type
  only for its props.
  The table is typed over the largest type a prop uses + 1, and further entries that pass the structural test
  (maps 13, 16, 23, 25: types no prop has; marked "drawn by no prop" with the lists only they reach).
- **Collision**: PollWorldCollision polls the models of the sample's tile, then the set's non-tiled list, and stops at
  the first model whose faces give a floor (terrain height); PollModelInstance polls a model only if `flags & 0xFF` is
  below 0x10 (props, flames, billboards and the other special kinds give no floor; a path-following model's low byte
  is its frame). A geometry entry's collision vertices are in the model's
  own space: the transform's position, angle and scale apply (map 9's model 39 draws geometry 13 at scale 2, so its
  vertices raised by 40 raise the floor by 80: run and measured). The collision is not the drawn mesh: a change to one
  does not change the other. Walls are the submap's actor navmesh (PushOutOfWalls). A placed model has no wall of its
  own, except a prop (kind 0x10): Brian, the NPCs and the enemies are pushed out of a circle around each prop near
  them, of their own radius plus 2 x the prop's scale (PushBrianOutOfEnemies, PushNPCOutOfBrian, PushEnemyOut, over
  gPropsNear). A prop placed in a room is solid (seen in a test: map 25's type-1 prop stops Brian).

### Encounters and spirits

- RollRandomEncounter: every 50 units Brian walks the chance grows by 50; a fight starts when rand(2000) < chance, if
  Brian is 50-90 units from a fight spot (at most 6 considered) and inside a zone. CreateEncounter: the zone's
  `encounters` is a count and that many pack indices; it draws one; each of the pack's spawn entries places
  base_count + rand(count_variance + 1) enemies on the set's position list from position_index.
- A pack table has no count: its length is the highest pack any zone names + 1. Submaps can share one (map 11:
  submaps 1 and 2, whose zones reach different highest packs: typed to the farther). Packs after that, which pass the
  structural test (maps 2, 27, 28, 29), are typed with the table and marked; packs inside it that no zone names (map
  2's 6, map 27's 1 and 2, map 29's 2) are typed with the table, unmarked. Neither kind is ever drawn in the original.
- SetupMapSpirits: gSubmapSpiritTable's row for the map and submap; each SpiritSpot is x, -z and a flag; y is polled.

### Textures

RGBA16, CI4 and CI8 images, as the lists load them (a tile's own size, or a block load's render tile). A CI texture's
palette is a record the lists load (G_LOADTLUT), gameplay_common's, or main's gSkyFadedPalette (the sky's, faded at
run time; its ROM source 0xD86170 gives the viewing palette). A texture loaded in several shapes is one image record
in the shape most of its loads use; a run of rows several loads window (map 21: two images back to back, a third load
across both) is one record in the rows' common width; each other load start is a name at its offset.

## How the build represents a map

Each map is a code segment of typed subsegments (`eltail.jp.yaml`): `maprecords` units (C records written by
`tools/splat_ext/anim.py` through the project's C types), `gfx` display lists, `vtx` vertex runs, `rgba16` / `ci4tex` /
`ci8tex` images, and labelled `bin` pieces. Names are structural, by map and index (`gMap29Set00Lists`,
`gMap09Geo13Vertices`, `gMap25Prop1SetupDL`), declared in `include/maps.h` and scoped to their map in
`symbol_addrs.txt`. A table whose length no field states is typed over the range the code is shown to read, its note
starting "inferred length". Bytes nothing reaches are raw: `RawWord` tails after a table, or `bin` pieces, each noted
with what it holds. A raw word that points inside a record is written as the record's name and an offset.

The definitions were generated from the project's ROM map (a research tool that is not part of this repository),
a walk of the ROM's data from the code's own roots, which
also checks them (every typed map's subsegments, names, types, inferred marks, textures, main's references and the
locks against the walk). `./configure` refuses a name a file takes from another map
(`tools/check_asset_names.py`): the maps share RAM.

## Changing a map

Edit a generated file under `assets/map_NN/`, keep the change with `python3 tools/asset_patches.py save FILE` (it is put
back after every extraction), then build and fix the boot checksum as
[ASSETS.md](ASSETS.md#changing-a-file-and-keeping-the-change) describes: `ninja` writes the changed ROM, whose
comparison with the original then fails, as it must for a changed ROM, and
`python3 tools/n64crc.py build/eltail.jp.z64` fixes the header's checksum. A
count printed as a number (an array in another unit, such as a set's transform_count) is changed by hand. A tile list
made longer needs the later tiles' offsets raised by the indices it gained. What to check after a change: main's tables and gLavaPalette
hold the new addresses (the link map, `build/eltail.jp.map`); no byte of the ROM differs without a reason (moved,
relocated or changed on purpose; `cmp -l` against your ROM lists the bytes); every reference into main still points
at its target. The project also walks the changed map with its ROM map's reader (outside this repository) and
compares it with the original but for the intended records. The worked example the maintainers ran: a model added in a mode-1 tile
(map 29) and a mode-4 tile (map 9) with the lists and offsets that follow, a pack no zone named made the one a zone
draws, a floor raised by 40 under a spirit spot, and a prop of a type no prop had placed (map 25).

## Not typed, and known limits

- The shared blocks (gameplay_common, interiors_common, field_shared, region_0-5) are locked binary pieces: their
  contents are named only where a map points into them.
- The NPC files are built from typed sources (`docs/NPCS.md`); the maps' NPC records name their records.
- Inferred lengths stay inferred (each marked); a runtime check covers only what it ran.
