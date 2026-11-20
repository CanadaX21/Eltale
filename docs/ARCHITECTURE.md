# Architecture: where the game's code and data are

A map of the source for a developer who has not worked on it before. It covers:
- how the game runs;
- what each source file holds;
- where each system lives when it spans several files;
- the words the source uses.

[GAME_FLOW.md](GAME_FLOW.md) follows the game from mode to mode. The system pages go deeper:
- [BATTLE.md](BATTLE.md): battles;
- [MAPS.md](MAPS.md): maps;
- [NPCS.md](NPCS.md): NPCs and their talk;
- [PLAYER.md](PLAYER.md): Brian's model and animations;
- [SAVE.md](SAVE.md): saving;
- [TEXT.md](TEXT.md): the game's two text encodings, the glyph tables and the names as the game spells them.

[ASSETS.md](ASSETS.md) says what `./configure` writes from the ROM and how to change it and keep the change.
[DECLARATIONS.md](DECLARATIONS.md) says where every declaration lives. [METHODOLOGY.md](METHODOLOGY.md) says what
counts as a match, what the figures cover and what a byte match does not prove.

## How the game runs

- **Threads.** The boot entry clears `.bss` and calls `boot` (`src/game/boot.c`). `boot` initialises the system and starts
  the idle thread (`idle`), and `idle` starts the PI manager and the main thread, which runs `mainproc`
  (`src/game/main.c`). `mainproc` starts the SDK sample scheduler (`nnsched`, `src/sched/nnsched.c`, its own threads for the
  retrace, graphics and audio tasks) and the audio thread (`AudioThread`, `src/game/audio.c`). All game logic runs on the
  main thread, one frame at a time.
- **The frame.** Each mode's loop reads the controller, updates the systems, builds the frame's display list
  (`BuildFrame`), waits for the scheduler and sends the frame to it, then swaps `gCurrentFrame`. There are two frames,
  `gFrames[2]`. The display lists name some data by RSP segment: segment 2 holds the frame's matrices (`D_2000000`),
segment 1 shared display lists such as Brian's render setup and the shadow quad (`include/variables.h`).
- **Modes.** `gGameMode` picks the loop: field, menu, title, splash, the ending's scenes, the epilogue, the staff
  credits ([GAME_FLOW.md](GAME_FLOW.md)).

The RAM a game uses, outside main's own image (code, data and `.bss` from 0x80000400):

| RAM | What | Where it is described |
|---|---|---|
| 0x800C0000 | the spell bank, resident (`gEffectsBlock`) | the spell records' types: `include/structs.h`, `include/spell_bank.h` |
| 0x80100000 | in play, `gameplay_common`, a block every map uses; at boot, the Controller Pak manager | [MAPS.md](MAPS.md), [SAVE.md](SAVE.md) |
| 0x8010FE00 | an outdoor map; or `interiors_common`, with an interior map after it at 0x801539F0 | [MAPS.md](MAPS.md) |
| 0x801C0000 | `gEventBlob`: the ending's data (two pieces, one for the ending's scenes, one for the staff credits) | [GAME_FLOW.md](GAME_FLOW.md) |
| 0x801E0000, 0x801F8000 | `field_shared` (maps 3, 5, 7, 12); a region block | [MAPS.md](MAPS.md) |
| 0x80200000, 0x80206000 | Brian's animation playing; his model | [PLAYER.md](PLAYER.md) |
| 0x8020C0D0 | the overlay slot: the NPCs' held objects in play; the title, splash and intro, and epilogue overlays | [NPCS.md](NPCS.md) |
| 0x8020E6F0 | `gMonsterFileBuffer`: the area's monster set (battles) or NPC file (towns) | [NPCS.md](NPCS.md), [BATTLE.md](BATTLE.md) |
| 0x802A0000, 0x802A7BC0 | Shannon's block; the boss file (`gBossVAddr`) | [NPCS.md](NPCS.md), [BATTLE.md](BATTLE.md) |
| 0x8039ADB0 | the "cutscene bank", resident from boot (below) | `include/variables.h` |

The framebuffers, the depth buffer, the audio heap and the RSP's buffers are at fixed addresses, declared in
`include/variables.h`.

## The source files

The ROM does not record the original file names. The files here are the original compilation units, cut where the
code shows a unit boundary, and named by what they hold (other Quest 64 and Eltale decomps name them by
their ROM offset: the README's file-name table maps the two). One unit holds whatever its original file held, so some systems span several files, and some files
hold more than their name says (below). Every unit has its own header in `include/game/`, named as its module;
it declares the unit's functions, types and state ([DECLARATIONS.md](DECLARATIONS.md)).

| File | Module | What it holds |
|---|---|---|
| `src/game/boot.c` | `game/boot` | `boot` and `idle`: the threads' start |
| `src/sched/nnsched.c` | `sched/nnsched` | the SDK sample scheduler |
| `src/game/main.c` | `game/main` | `mainproc`, the mode loops, `BuildFrame`, `InitNewGame`, `InitGraphics`, the transitions' fades; the game mode, field mode and transition state |
| `src/game/ending_text.c` | `game/ending_text` | the ending's seven scenes (`LoadEndingScene`) and their timed text pages |
| `src/game/player.c` | `game/player` | Brian: his setup, place and arrival, his states, his movement in the field and in battle, the staff, damage to him, growth, spell input, finding someone to talk to; `gPlayerState`, `gBrianStats` |
| `src/game/enemy.c` | `game/enemy` | **the NPCs** (its first half: setting up, talking, wandering) and **the enemies** (its second half: spawning, turn order, actions, AI, damage), and the boss effect scripts |
| `src/game/boss.c` | `game/boss` | the bosses (loading, spawning, their scenes, `OnBossDefeated`) and **the story flags** (`gStoryFlags`: the bosses beaten) |
| `src/game/map.c` | `game/map` | `LoadMap` (with the Return and Exit points), drawing the map (its frame, billboards, sky, props, flames), door animations, the map's dust, palettes; the current map's state |
| `src/game/collision.c` | `game/collision` | the floor and wall polls; door triggers and warps (`CheckDoorTriggers`, `SetDoorWarp`, `gPendingWarp`) |
| `src/game/lighting.c` | `game/lighting` | the time of day, the lights and their animations, palette fades |
| `src/game/shadow.c` | `game/shadow` | shadows |
| `src/game/map_objects.c` | `game/map_objects` | the objects Brian opens (spawning, drawing, opening, the pick-up message) |
| `src/game/spirits.c` | `game/spirits` | the spirits on the maps |
| `src/game/signs.c` | `game/signs` | signs: finding and reading them |
| `src/game/camera.c` | `game/camera` | the camera modes (field, fixed, battle, after a battle), its setups, shake |
| `src/game/spells.c` | `game/spells` | spells: casting, slots, movement, collision, effects, statuses; `SpawnEffectObject` for every effect |
| `src/game/effects.c` | `game/effects` | effects: running dust, puffs, sparkles, damage numbers, staff hits, ground rings, projectiles (`gProjSlots`), glows |
| `src/game/dust.c` | `game/dust` | dust puffs |
| `src/game/encounter.c` | `game/encounter` | **the battle's flow**: random encounters, the battle starts, the round and turns, the drop; `gBattleState` |
| `src/game/model.c` | `game/model` | animating and drawing models (`SetAnimation`, `DrawModel`) |
| `src/game/hud.c` | `game/hud` | the HUD, the monster card, status icons and effects, markers, the MISS label; also **the inventory and item use** (`ItemMenu`, `UseItem`, the item handlers) |
| `src/game/screen_fade.c` | `game/screen_fade` | the screen fade |
| `src/game/spline.c` | `game/spline` | spline sampling (animation keys, camera paths) |
| `src/game/math.c` | `game/math` | random numbers, angles, matrices |
| `src/game/dma.c` | `game/dma` | ROM reads (`dma_write`); `count_digits` |
| `src/game/audio.c` | `game/audio` | the audio thread, the music players, sound effects and their scripts |
| `src/game/sound.c` | `game/sound` | music and sound requests (`RequestMusic`, `PlaySound`), then **main's game tables in its data**: the map table (`gMapTable`), the monster sets' and NPC files' ROM ranges, Brian's animation table, the experience tables (`gXPTable`, `gSpiritExpTable`), the new game's stats, the respawn table, door animations, flames, the maps' dust, the sky records, the opening speech. They are declared in `game/sound.h` because this unit defines them |
| `src/game/start_menu.c` | `game/start_menu` | the Start menu and its pages (status, map, elements, environment) |
| `src/game/save_menu.c` | `game/save_menu` | the save pages (load, save, delete) and the save records (`WriteSaveRecord`, `ReadSaveRecord`) |
| `src/game/message.c` | `game/message` | message windows and the dialogue box |
| `src/game/text.c` | `game/text` | the font and string drawing |
| `src/game/controller.c` | `game/controller` | the controllers and the Controller Pak's notes |
| `src/game/overlay.c` | `game/overlay` | the overlay loaders (`LoadOvl_TitleScreen`, `LoadOvl_Splash_and_Intro`, `LoadOvl_Epilogue`) |
| `src/game/staff_credits.c` | `game/staff_credits` | the staff credits' scenes (`gSceneSequence`: `StartSequenceScene`, `StepSequenceScene`, `EndSequenceScene`), camera paths, the credit images |
| `src/game/boss_tables.c`, `src/game/boss_anims.c` | `game/boss_tables`, `game/boss_anims` | data in the cutscene bank: the boss files' ROM bounds and placements, the bosses' animation tables and names |
| `src/title_overlay/` | `title_overlay/` | the title screen, three files (`game/title.h`) |
| `src/intro_overlay/` | `intro_overlay/` | the splash and intro, two files (`game/intro.h`); its 32 functions are still assembly |
| `src/pak_menu_module/pak_menu.c` | `pak_menu_module/pak_menu` | the boot-time Controller Pak manager (`game/pak_menu.h`) |
| `src/brian_model/brian_model.c` | `brian_model/brian_model` | Brian's model's tables (`include/brian.h`) |
| `src/libultra/initialize.c` to `src/libultra/visetevent.c` | `libultra/` | six libultra modules written in C |

The epilogue overlay (segment `credits_overlay`) is assembly, disassembled at build time. The SDK library code is assembly too; the platform report (`PLATFORM.md`) says
what the game takes from it.

## Where a system lives

| System | Code | Data | Reference |
|---|---|---|---|
| The game's flow | `src/game/main.c` | | [GAME_FLOW.md](GAME_FLOW.md) |
| Brian | `src/game/player.c`, `src/game/model.c` | `brian_model`, `brian_anim_NN`, `gBrianStats` | [PLAYER.md](PLAYER.md), [BATTLE.md](BATTLE.md) |
| NPCs and talk | `src/game/enemy.c` (first half), `src/game/message.c`, `src/game/text.c` | the NPC files, the maps' NPC lists, the dialogue bank | [NPCS.md](NPCS.md) |
| Battles | `src/game/encounter.c`, `src/game/enemy.c` (second half), `src/game/spells.c`, `src/game/player.c`, `src/game/boss.c`, `src/game/hud.c`, `src/game/effects.c` | the monster sets, the boss files, the spell bank | [BATTLE.md](BATTLE.md) |
| Items | `src/game/hud.c` | the item records, names and descriptions (binary, in the cutscene bank: `gItemData`); the check and use handler tables (main's data still in assembly: the build writes `asm/data/500A0.data.s`, which names each handler) | `ITEM_TYPE_` and `ITEM_USE_` (`include/quest_types.h`), [BATTLE.md](BATTLE.md) |
| Maps and places | `src/game/map.c`, `src/game/collision.c`, `src/game/map_objects.c`, `src/game/spirits.c`, `src/game/signs.c`, `src/game/lighting.c`, `src/game/camera.c` | the 36 maps, the shared blocks, `gMapTable` (`src/game/sound.c`) | [MAPS.md](MAPS.md) |
| Progress | `src/game/boss.c` (the bosses beaten), `src/game/map_objects.c` (objects opened), `src/game/spirits.c` (spirits found) | | [SAVE.md](SAVE.md) |
| Saving | `src/game/save_menu.c`, `src/game/controller.c`, the Pak manager | | [SAVE.md](SAVE.md) |
| Menus and the HUD | `src/game/start_menu.c`, `src/game/save_menu.c`, `src/game/message.c`, `src/game/hud.c` | the menu sheet, the HUD art (cutscene bank) | |
| Audio | `src/game/audio.c`, `src/game/sound.c` (the requests) | the sequence and sound banks | |
| The ending's scenes, the epilogue, the staff credits | `src/game/ending_text.c`, the epilogue overlay, `src/game/staff_credits.c` | `gEventBlob` (two pieces), `gEndingScenes`, `gSceneSequence` | [GAME_FLOW.md](GAME_FLOW.md) |

## Words the source uses

- **map, submap.** A map is one of the 36 files in `gMapTable` (`gCurrentMap`). A submap is one of its parts, with its
  own models, doors, entrances, cameras, lights and flags (`gCurrentSubmap`, `gSubmapHeader`). The game's rooms and
  local areas are submaps. Identifiers say submap; some older ones say room for the same thing (`gExitRoom`,
  `gReturnRoom`, `InitRoomCamera`, `LoadRoomSigns`, the maps' `RoomN` symbols).
- **area.** A submap's `area` picks the NPC file of a submap without random battles, and the monster set of one with
  them (`SUBMAP_ENCOUNTERS`). Both load at `gMonsterFileBuffer`.
- **the kinds of place.** `MAP_TOWN`, `MAP_INTERIOR`, `MAP_FIELD` and `MAP_DUNGEON` (`sMapData.flags`) sort the maps;
  `SUBMAP_` flags mark a submap's own rules.
- **place, change of place.** A map and submap, and the countdown that moves Brian between them
  ([GAME_FLOW.md](GAME_FLOW.md)).
- **field.** Play outside the menus and scenes: `MODE_GAMEPLAY`, `FieldLoop`. Battles happen in the field.
- **actor.** Anything placed and animated: `ActorTransform` holds its place, motion, model and animation. Brian, an
  enemy, an NPC and a map object each have one.
- **battle actor.** `MonsterBattleData`: an actor with its battle stats (`ActorStats`). Brian has one
  (`gPlayerState.brian`) as well as every enemy. The name is the Quest 64 decompilations'.
- **enemy record.** `EnemyAction`: one of the six enemies in a battle (`gEnemies`): its action, timers, HP and MP,
  AI, base data and battle actor. The name is the Quest 64 decompilations'; it is not one action.
- **Brian's stats.** `BrianData1` (`gBrianStats`): his HP, MP, agility, defence, experience, spirits and growth
  counters, the 0x38 bytes a save keeps. The name is the Quest 64 decompilations'.
- **story flags.** `gStoryFlags`: the bosses beaten, one bit each (`SetStoryFlag` after a boss battle won). An NPC with
  `NPC_TALK_BOSS_FLAG` changes its line by one.
- **talk type.** `NpcRecord.talkType`: what talking to an NPC does ([NPCS.md](NPCS.md)).
- **game mode, field mode, transition.** `gGameMode`, `gFieldMode`, `gTransitionStyle` ([GAME_FLOW.md](GAME_FLOW.md)).
- **Return point, Exit point, save point.** Where the Return and Exit spells lead (`gReturnDest`, `gExitDest`, set by
  `LoadMap`), and where Brian comes back after a defeat or a load (`gSavedMap`).
- **the cutscene bank.** A block loaded once at boot and kept resident at 0x8039ADB0 (`cutscene_bank` in the splat
  config). Despite the name, most of what the code reads from it is HUD and menu art, the item records, names and
  descriptions, and the bosses' data and scene texts. Most of it is still binary, named by address in
  `include/variables.h`.
- **the overlay slot.** RAM at 0x8020C0D0 that the title, splash and intro, and epilogue overlays share with the NPCs'
  held objects.
- **event blobs.** The two pieces of the ending's data (`event_blob_a` for the ending's scenes, `event_blob_b` for the
  staff credits' scenes and their credit images), loaded at `gEventBlob`.
- **the ending, the epilogue, the staff credits.** The game's end, in order: the ending's seven scenes (`MODE_ENDING`,
  `EndingLoop`, `gEndingScenes`), the epilogue (`MODE_EPILOGUE`, `EpilogueLoop`: a narration over a painting of the
  castle in the clouds, with no credits), the staff credits (`MODE_STAFF_CREDITS`, `StaffCreditsLoop`,
  `gSceneSequence`: fourteen scenes of the world with the credit images, then End), then a new game. The epilogue's
  overlay keeps its segment's historical name, `credits_overlay`, which does not describe what it shows. One song
  (20) runs from the epilogue to the end of the staff credits ([GAME_FLOW.md](GAME_FLOW.md)).
- **typed sources, raw, inferred length.** Asset files built from C types (the maps, NPC files, monster sets, boss
  files, the spell bank, Brian); bytes no evidence types are labelled raw; a table whose length no code states says
  "inferred length" ([MAPS.md](MAPS.md), [NPCS.md](NPCS.md)).

## Names kept from the Quest 64 decompilations

About 600 of the game's functions are identical to Quest 64's, and many names come from the Quest 64 decompilations,
checked against this game's code. A Quest name is kept where it is accurate enough. That covers `EnemyAction`,
`MonsterBattleData` and `BrianData1` above, the lower-case function names (`getRandomNumber`, `dma_write`,
`string_copy`), and `include/quest_types.h`, which still holds the types the whole game shares. A Quest name that the
code contradicts is replaced, with the Quest name in the comment: for example `MODE_MENU` (`MODE_PAUSED`), `MODE_EPILOGUE`
(`MODE_CREDITS`) and the screen fade's flags.

## Not in C yet

- the intro overlay's 32 functions;
- the title's menu drawing;
- the epilogue overlay (segment `credits_overlay`);
- two functions of the Controller Pak manager;
- the SDK library code;
- the binary pieces of main's data and the cutscene bank, named by address in `include/variables.h`.

The repository's README gives the current figures.
