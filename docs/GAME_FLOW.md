# The game's flow: modes, loops and changes of place

How the game moves between its title, its field, its menus and its ending, and what happens when Brian goes through a
door, warps, rests at an inn, is defeated or beats the last boss. Every value below has a name in the headers, with a
comment that says which code sets and reads it. This page says how they fit together; [ARCHITECTURE.md](ARCHITECTURE.md)
says where the code is. Everything here is read from the code (`src/game/main.c` for the loops); none of it was traced
running, apart from what [BATTLE.md](BATTLE.md) and [SAVE.md](SAVE.md) report for their own parts.

## mainproc and the game modes

The main thread runs `mainproc` (`src/game/main.c`). It sets up the video, the audio, the scheduler and the controllers,
runs the boot-time Controller Pak manager (`ControllerPakMenu`, loaded at 0x80100000: [SAVE.md](SAVE.md)), starts the
scheduler and the audio thread, and then runs two loops:

- **the outer loop: one new game each time round.** It clears the story flags, the opened map objects and the spirits
  found, sets Brian up (`InitBrian`), empties the inventory, resets the time of day and the sound, and calls
  `InitNewGame`, which keeps only `GAME_STATE_CLEARED` of `gGameState`, starts on `MODE_SPLASH`, and puts the first field
  start in map 13, submap 17, with `FIELD_MODE_NEW_GAME`;
- **the inner loop: one mode at a time.** It runs the loop of `gGameMode` until the mode changes, then the next mode's.
  It leaves for the outer loop, a new game, when `GAME_STATE_RESTART` is set.

| Mode (`gGameMode`) | Loop | What it is | How it ends |
|---|---|---|---|
| `MODE_NONE` (0) | none | the inner loop turns it into `MODE_GAMEPLAY`: a new field start | |
| `MODE_GAMEPLAY` (1) | `FieldLoop` | the field, and the battles in it | Start: `MODE_MENU`; a change of place: `MODE_NONE`; after the last boss: `MODE_ENDING` |
| `MODE_MENU` (2) | `MenuLoop` | the Start menu (`StartMenu`), and its save pages (`SaveMenu`) for the inn's save and the title's Continue and Delete | B or Start: back to the field; after Continue or Delete: the field (a game loaded) or the title |
| `MODE_TITLE` (3) | `TitleLoop` | the title overlay | its choice (`gTitleChoice`): New Game to the field, Continue or Delete to `MODE_MENU`; with B, or left alone, back to `MODE_SPLASH` |
| `MODE_SPLASH` (4) | `IntroLoop` | the splash and intro overlay | to the title |
| `MODE_EPILOGUE` (5) | `EpilogueLoop` | the epilogue: a narration over a painting of the castle in the clouds, no credits; the overlay `LoadOvl_Epilogue` loads (its segment keeps the historical name `credits_overlay`) | its frame function returns 0: `MODE_STAFF_CREDITS` |
| `MODE_ENDING` (6) | `EndingLoop` | the ending's seven scenes | `MODE_EPILOGUE` |
| `MODE_STAFF_CREDITS` (7) | `StaffCreditsLoop` | the staff credits: fourteen scenes of the world with the credit images, then End | `GAME_STATE_RESTART`: a new game |

`BuildFrame` builds every mode's display list. Its first branch, for the field, the ending's scenes and the staff
credits, draws the world: camera, lights, sky, map, Brian (unless `GAME_STATE_HIDE_BRIAN`), NPCs, map objects,
billboards, enemies, the boss, sparkles, shadows, dust, effects, the screen fade, the message box, the HUD (not after the
last boss: `FIELD_MODE_LAST_BOSS`) and the item menu, and in the staff credits the credit images
(`DrawCreditImages`). The other modes draw their menu or overlay.

## gGameState

`gGameState` (`include/variables.h`, every bit named there) is the word the loops and the field share.

| Bit | Name | Set by | What it does |
|---|---|---|---|
| 0x1 | `GAME_STATE_ITEM_MENU` | R in the field (`UpdateBrian`) | the item menu is open |
| 0x2 | `GAME_STATE_MESSAGE` | `OpenMessageWindow`, and the talks and pick-ups | a message window holds play |
| 0x4 | `GAME_STATE_SPELL_INPUT` | `BrianSpellInput` | a spell's directions are being entered |
| 0x8 | `GAME_STATE_SPIRIT_WINDOW` | `UpdateBrian` (a spirit picked up), `UpdateBrianGrowth` | the window that adds a spirit is open |
| 0x10 | `GAME_STATE_INN_CHOICE` | an inn's NPC (`NPC_TALK_INN`) | its Yes / No is up; `BrianState_TalkEnd` acts on the answer |
| 0x20 | `GAME_STATE_FADE_IN` | the end of a change of place | the next loop fades the screen in once its place is loaded |
| 0x40 | `GAME_STATE_LOAD_NEXT` | the end of a change of place, `InitNewGame`, the scenes | the loop loads its next place at its top |
| 0x80 | `GAME_STATE_MAP_CHANGE` | a door, a warp, a defeat, the inn, the last boss, a scene's end | the mode loop's countdown runs (`gModeCountdown`): to the next place, to the save pages after the inn, or to the next scene |
| 0x100 | `GAME_STATE_MAP_MUSIC` | the end of a change of place | the next field start picks the map's music |
| 0x200 | `GAME_STATE_STOP_MUSIC` | `EndSequenceScene` after the last scene | the loop stops the music and waits for it as it ends |
| 0x400 | `GAME_STATE_SAVE_MENU` | the inn's Yes, the title's Continue and Delete | a visit to the save pages; the field keeps its place |
| 0x800 | `GAME_STATE_RESPAWN` | a defeat (`BrianState_KnockedDown`) | the next field start puts Brian at his save point with 1 HP |
| 0x1000, 0x2000, 0x4000 | `GAME_STATE_UNSET_*` | no code | each would stop something if set (the Start menu, Brian's movement, the HUD and talking); the game never sets them |
| 0x8000 | `GAME_STATE_RESTART` | `EndSequenceScene` after the last scene | `mainproc` starts a new game |
| 0x10000 | `GAME_STATE_HIDE_BRIAN` | the ending's scenes without Brian, the staff credits' scenes | Brian is not drawn; `SpawnBrian` clears it |
| 0x20000 | `GAME_STATE_CLEARED` | the end of the staff credits, a load, the save list's scan | the game has been finished; it is saved with the game |

## A field start

When `FieldLoop` starts with `GAME_STATE_LOAD_NEXT` set, it loads the place `gNextMap` / `gNextSubmap` names before its
first frame. After a defeat (`GAME_STATE_RESPAWN`) it first takes the save point (`RespawnAtSavePoint`). Then, in order:

- `LoadMap` (the map file and the blocks it needs: [MAPS.md](MAPS.md));
- `SpawnMapObjects`, `SetupMapSpirits`, `LoadRoomSigns`;
- `SpawnBrian` (his place, and his arrival by `gArrivalFlags`);
- `SetupNPCs` (the area's NPC files, not in a submap with `SUBMAP_ENCOUNTERS`: [NPCS.md](NPCS.md));
- `ResetEnemies`, `LoadMapBoss`, `InitRoomCamera`;
- the light animators, `InitEncounters`, the spell, effect, dust and HUD resets.

With `GAME_STATE_FADE_IN` it fades the screen in (`FadeScreenIn`, by `gTransitionStyle`), and with
`GAME_STATE_MAP_MUSIC` it picks the map's music. `gFieldMode` then goes back to `FIELD_MODE_NORMAL`.

`gFieldMode` says how the field was entered:

| Value | Name | Set by | Read by |
|---|---|---|---|
| 0 | `FIELD_MODE_NORMAL` | `FieldLoop`, once the place is loaded | |
| 1 | `FIELD_MODE_NEW_GAME` | `InitNewGame` | `SetupNPCs`: the opening speech |
| 2 | `FIELD_MODE_LOADED` | `MenuLoop`, after a load | no code tests it |
| 4 | `FIELD_MODE_LAST_BOSS` | `OnBossDefeated` | `FieldLoop` (the ending next), `BuildFrame` (no HUD), `BrianState_Victory` (no map music) |

## A change of place

### Through a door

1. `CheckDoorTriggers` (`src/game/collision.c`) finds a door Brian walked into: inside its box, facing its way unless the
   door has `DOOR_ANY_FACING`, and its key test passed (`CheckDoorKeyItem`: `DOOR_NEEDS_KEY`, `DOOR_REFUSES_KEY`).
   `SetDoorWarp` copies its destination (map, submap, entrance), its flags and its angle into `gPendingWarp`.
2. `FieldLoop` sets `GAME_STATE_MAP_CHANGE`. `ClampCameraToTarget` keeps the camera's angle for the next submap
   (`Camera.arrivalYaw`), `StartBrianDeparture` makes Brian leave by the door's kind and sets the countdown, and the
   transition is `TRANSITION_BLACK`.
3. While no message is open, `gModeCountdown` counts down. At `gFadeOutAt` the screen fades out
   (`FadeScreenOut`, `FADE_HOLD`). At 0:
   - the mode becomes `MODE_NONE`, or `MODE_ENDING` after the last boss;
   - `GAME_STATE_FADE_IN`, `GAME_STATE_LOAD_NEXT` (not on an inn's rest) and `GAME_STATE_MAP_MUSIC` are set;
   - `gNextMap` / `gNextSubmap` take the pending warp's.
4. The loop ends; `mainproc` runs `FieldLoop` again, and the field start loads the new place. `SpawnBrian` puts Brian at
   the entrance and makes his arrival from the same door flags (`gArrivalFlags`): standing (`DOOR_ARRIVE_STANDING`),
   or by the kind (walking in, or through the door model). The kinds are listed at `MapDoor.flags`
   (`include/quest_types.h`).

`LoadMap` also keeps the points the Return and Exit spells lead to, by the kinds of the maps. Leaving a `MAP_TOWN` for a
`MAP_FIELD` records the field's entrance as the Return point. Entering a `MAP_DUNGEON` records its entrance as the
Exit point. Maps 6, 11 and 26 have special cases written into `LoadMap`.

### Warps, a defeat, the inn

| What | Where it goes | Conditions | Transition |
|---|---|---|---|
| The Wings (`UseWings`) | the item's `gWingData` row: a map, a submap and Brian's place | not in a `MAP_INTERIOR` or `MAP_DUNGEON` map, nor a `SUBMAP_NO_RETURN` submap (`CheckIfTeleportPossible`) | `TRANSITION_WHITE` |
| Return (`SpellEffect_Return`) | the Return point | only where the map's flags are exactly `MAP_FIELD`, not in a `SUBMAP_NO_RETURN` submap | `TRANSITION_BLACK`: the field start's, which the spell keeps |
| Exit (`SpellEffect_Exit`) | the Exit point | in a `MAP_DUNGEON` map but maps 30 and 34, not in a `SUBMAP_NO_EXIT` submap | `TRANSITION_BLACK`, likewise |
| A defeat (`BrianState_KnockedDown`) | the save point's map and submap (`gSavedMap`, `gSavedSubmap`); `RespawnAtSavePoint` puts Brian at that submap's place in `gRespawnLocationTbl`, or at its entrance 0 | `GAME_STATE_RESPAWN`: 1 HP, full MP, no statuses | `TRANSITION_BLACK_SLOW` |
| The inn (an `NPC_TALK_INN` NPC, Yes) | stays in place | `GAME_STATE_SAVE_MENU`: full HP and MP, the day moves on, this place becomes the save point | `TRANSITION_BLACK_SLOW` |

A warp puts Brian down standing (`WARP_ARRIVAL`): the Wings at the place they name (an entrance of -1: `gBrianSpawnX`,
`gBrianSpawnZ`, `gBrianSpawnYaw`), Return and Exit at the entrance `LoadMap` recorded for their point
(`gReturnEntrance`, `gExitEntrance`). After the inn's rest the field loop ends without loading (`GAME_STATE_LOAD_NEXT` is not set), and
`EnterSaveMode(SAVE_MODE_SAVE)` opens the save pages in `MODE_MENU`. When they close, the field comes back in the same
place, with `GAME_STATE_FADE_IN` and the map's music.

### Battles

A battle does not change the mode: it runs inside `FieldLoop` (`gBattleState`, [BATTLE.md](BATTLE.md)). Random battles
happen only in submaps with `SUBMAP_ENCOUNTERS`, and those submaps set up no NPCs.

## The Start menu and the save pages (MODE_MENU)

Start in the field sets `MODE_MENU` when Brian is free (not `ACTOR_BUSY`), Z is not held, and no battle, message, spell
input, item menu or change of place is on. `MenuLoop` then
builds `StartMenu`'s frames: the pages, or `SaveMenu` when `MENU_SAVE` is set. B or Start clears `MENU_OPEN`, and
`StartMenu` returns to `MODE_GAMEPLAY`, or after the title's Continue or Delete to `MODE_TITLE` or the field.
`EnterSaveMode(mode)` opens the save pages with `SAVE_MODE_LOAD`, `SAVE_MODE_SAVE` or `SAVE_MODE_DELETE`
([SAVE.md](SAVE.md)). After a load, `MenuLoop` puts Brian at the save point and sets `FIELD_MODE_LOADED`. The menu's own
states inside its pages stay numbers in `gMenuState`'s low bits and in `gSaveMenuState`.

## The opening and the ending

**A new game.** `InitNewGame`'s field start loads map 13, submap 17. `SetupNPCs` sees `FIELD_MODE_NEW_GAME` and starts
the opening speech (`gOpeningSpeechText`) with `StartNPCTextFromRAM`, the submap's first NPC as the speaker, then opens
the message window. `SpawnBrian` set `PLAYER_MESSAGE_HOLD`, because Brian is put down away from an entrance: the
message state holds until every window has closed, and no one can be talked to meanwhile.

**The last boss.** `OnBossDefeated(LAST_BOSS)` (`src/game/boss.c`):
1. It shows the boss's last words, sets `PLAYER_MESSAGE_HOLD` and `GAME_STATE_MAP_CHANGE`, a 0x40-frame countdown
   (the fade at 0x3E), `TRANSITION_WHITE_SLOW` and `FIELD_MODE_LAST_BOSS`.
2. When the countdown ends, `FieldLoop` sets `MODE_ENDING`.
3. `EndingLoop` loads the ending's data (`event_blob_a`, at `gEventBlob`) and plays its seven scenes (`gEndingScenes`,
   `LoadEndingScene`). Each scene gives a map and submap, the camera's place and angles, Brian's place, the NPCs, the
   text pages and the music. Brian is hidden unless the scene's flag bit 0 says he appears. A scene's NPC
   records must name the scene's submap (`SpawnNPCs` keeps only those), and their models must be in that submap's NPC
   area file (`LoadNPCArea(gSubmapHeader->area)`). The scene table and the records sit in `event_blob_a`, whose size the
   link locks: a scene can change its map, its places and its NPCs' places, but not gain NPCs. The scene table
   itself is a binary piece (`gEventBlobARaw_013E8`): `tools/asset_patches.py` keeps changes to the generated records,
   display lists, vertex runs and images, not to a binary piece, so a changed table needs a way of its own to survive
   `./configure`.
4. `MODE_EPILOGUE` follows, the epilogue: `EpilogueLoop` loads the overlay (`LoadOvl_Epilogue`; its segment keeps
   the historical name `credits_overlay`, though it shows no credits), queues the credits' song (20) and runs the
   overlay's 2D scene: a painting of the castle in the clouds, with a narration scrolling over it that closes the
   story. When the overlay's frame function returns 0, `BuildFrame` sets `MODE_STAFF_CREDITS`.
5. Then `MODE_STAFF_CREDITS`, the staff credits: `StaffCreditsLoop` loads `event_blob_b` and plays fourteen scenes
   (`gSceneSequence`: `StartSequenceScene`, `StepSequenceScene`, `EndSequenceScene`; not the ending's
   `gEndingScenes`), each a camera path over a map with its credit images (`DrawCreditImages`): the staff's roles
   and names, and in the last scene End. It mutes the sound effects and asks for no music: the epilogue's song plays
   on, fades out in the last scene and stops at its end. On the original ROM `BuildFrame` ran 3,571 times in the
   epilogue and 9,571 times in the staff credits. After the last scene, `EndSequenceScene` sets `MODE_NONE` with
   `GAME_STATE_RESTART`, `GAME_STATE_STOP_MUSIC` and `GAME_STATE_LOAD_NEXT`, and the loop sets `GAME_STATE_CLEARED`.
6. `mainproc` starts a new game from the splash screen, and `InitNewGame` keeps `GAME_STATE_CLEARED`.

An NPC with `NPC_TALK_BOSS_FLAG` whose flag is `LAST_BOSS` would also set `GAME_STATE_RESTART`, through a branch of
`StartNPCTalk`. None of the 169 NPC records has that flag, so the branch never runs.

## What stays a number, and why

- **A door's kind** (bits 8-11 of its flags): 0 to 4, described at `MapDoor.flags` (kind 0: two doors, both
  `DOOR_LEAVE_STANDING`). `StartBrianDeparture` and
  `SpawnBrian` read them differently (`& 0xF` and `& 7`), so one name for both would hide that.
- **The menus' inner states** (`gMenuState`'s low bits, `gSaveMenuState`), **the message windows' flag word**, and the
  **sound and music ids**. Naming them needs a study of their own: every menu's screens, the windows' flags, what
  each sound and piece of music is.
- **The camera's modes and flags, the effect bits, the NPCs' and the map objects' animation ids, the light records'
  flags and the title menu's states.** Left as numbers. The effect bits are explained at `ProjSlot.flags`, the camera
  modes at `Camera.mode` and the NPCs' animations in [NPCS.md](NPCS.md); naming each family is a study of its own.
- **The three `GAME_STATE_UNSET_*` bits.** No code sets them. Their names claim no purpose; their comments say what the
  reading code would do with one set.
