# Changelog

What changed in this repository, newest first. Every state listed builds the byte-identical Japanese ROM (SHA-1
`4161b5c100ec82b0241b20ca8f81366e23564ccb`) from the C, with all 530 tracked compiled C functions byte-matched.

## Headers, modified builds, the boss files and the title and intro overlays

The ROM still builds byte-identical; the figures for the main image are unchanged.

- **One header per file.** Every function another file calls is declared once, in the header of the file that
  defines it (`include/game/<file>.h`, `include/sched/nnsched.h`, `include/game/title.h` and `intro.h` for the
  overlays), copied from its definition, and each file includes its own header, so the compiler checks those
  definitions against the declarations their callers use. Before, each C file declared what it called itself (680
  declarations of 467 functions). Corrected on the way: `DrawModel`'s matrix parameter (six files declared it `s32`)
  and `UseSpellItem`'s parameter (`MonsterBattleData*`, as the other item handlers take). A few calls in the original
  pass an argument list the definition does not take, or compile differently under its prototype; the files that
  make them declare those functions themselves without a prototype (`src/game/spells.c` calls three with no
  declaration in scope), and say why (`RULE-EXCEPTION R25`). [docs/METHODOLOGY.md](docs/METHODOLOGY.md) lists the
  other declarations a C file keeps.
- **Global variables** are still declared by each file that uses them: moving a declaration into a shared header
  changes the order IDO gives a file's `.bss`, and with it the ROM. [docs/METHODOLOGY.md](docs/METHODOLOGY.md) explains.
- **Header dependencies**: after you edit a header, `ninja` recompiles every file that includes it
  (`tools/c_deps.py` lists them; before, only a clean build did).
- **objdiff**: `objdiff.json` lists every C file in `src/game/`, `src/sched/` and `src/libultra/` (the main image's 40
  units and the cutscene bank's two data files); one with a long path, `src/libultra/createmesgqueue.c`, was missing.
  The overlays' C files are built for objdiff but not listed.
- **The boot code's and the scheduler's state** are defined in `src/game/boot.c` and `src/sched/nnsched.c`, each
  array with its size, and the threads are started with `STACK_TOP(stack)`: the size is no longer written in the call.
  The boot stack is named (`gBootStack`).
- **Constants**: `src/game/hud.c` writes its doubles as values (`-3.141592653589793`, ...); 89 extern declarations
  their files never used are gone.
- **Modified builds**: code and data can change size in any C file and the game still boots and plays (every
  reference follows the layout; the linker checks main's RAM limit and the overlays' room). `asm/entry.s`, the boot
  entry, is kept by hand so its `.bss` clear takes its size from the linker.
- **The SDK library inside the main image** is identified: 179 more functions carry their SDK names.
  [docs/PLATFORM.md](docs/PLATFORM.md) is the platform boundary that came of it: the library services the game uses,
  at 375 call sites, what the code assumes about the hardware, notes for a port, and how the 329 library functions
  were identified (verified 284, probable 24, unresolved 21), with the evidence for each probable and unresolved one.
  The behaviour it describes is read from the code, not observed running.
- **Main's data tables** (the start menu's textures and tables, the sound effects' scripts) are C.
- **The 8 boss model files** are built from typed sources: splat writes their animation records, display lists,
  vertices and textures from your ROM when you build (`include/boss_files.h` names their symbols); the cutscene bank's
  boss tables are C (`src/game/boss_tables.c`, `src/game/boss_anims.c`).
- **The title screen overlay** is three C files (`src/title_overlay/`), 7 of its 8 functions in C, and **the splash
  and intro overlay** two (`src/intro_overlay/`): its data, its scene tables (`include/scene2d.h`) and its structure in
  C, its 32 functions still assembly from your ROM. Their figures are counted apart from the main image's.

## Documentation

No change to the source or the build.

- **A ROM map**, [docs/ROM_MAP.md](docs/ROM_MAP.md): every block of the ROM with what it holds, the function that loads
  it and the RAM it goes to; the pointer tables that index the data; the C types that describe it; the code outside
  the C; and the open questions.
- **The README** has a quick start, a guide to where to start reading the code, and an index of the documentation.
- **The figures' scope**, stated precisely: they cover the CPU code at ROM 0x1000-0x4C580. Code elsewhere in the ROM
  (the RSP microcode, the overlays and other blocks loaded at run time) is copied from your ROM as binary data and
  counted by none of them.
- [docs/METHODOLOGY.md](docs/METHODOLOGY.md): the comments on the R25 exceptions cite R25 (it said two cited R5), and
  `tools/verify.py` measures the code figures (it said all of them).
- `.gitattributes`: `include/macro.inc` counts as assembly in GitHub's language statistics.

## Names, types and data structures

The compiled code is unchanged: these are names, types, a symbol's address and comments, and the ROM still builds
byte-identical. 18 files: 15 C units, `include/quest_types.h`, `symbol_addrs.txt` and `undefined_syms.jp.txt`.
Placeholder member accesses in `src/` (`->unk…`, `.unk…`) went from 236 to 96.

- **Actor state.** `ActorTransform`, the block at the head of Brian's and every enemy's battle data
  (`MonsterBattleData`) and the transform in every NPC and map object, has its last placeholder members named from the
  code that sets and reads them:
  - `rootX`..`rootZ` and `rootMoveX`..`rootMoveZ`: the animation's root position and its move since the last frame
    (ApplyRootMotion). A map object keeps its use point there instead.
  - `stateFlags` (was `unk60`), with its bits as named constants at every use: `ACTOR_BUSY` (in an action: input, the
    battle turn and the Start menu wait for its end), `ACTOR_NO_MELEE_HIT`, `ANIM_LOOPED`, `ANIM_CHANGED`,
    `NPC_WIDE_TALK` and `MAPOBJ_OPENING`.
- **`StartBrianHit` is `StartBrianDodge`.** It runs only when an attack misses Brian (the melee agility check, a missed
  spell). Being hit is `DamageBrian`.
- **The frame's matrices** (`mkst`): `list` (the projection, the HUD's orthographic projection and the view: the draw
  functions' matrix list) and `drawMtx` (one matrix per draw from there on).
- **Monster and boss data**, checked against the game's data (all 67 monster records, the 8 boss records and their 128
  AI options):
  - `AI_ACTION_SPELL`: bit 15 of an AI option's action tag marks a spell. The bits below it are the spell, and in a
    boss battle the low byte also names the boss effect script.
  - Types by their uses: `gBossNames` is a table of name pointers (`u8*[]`); `gFieldMonsterAnims`, `gFieldEnemyNames`
    and `MonsterSet.namePtr` are `AnimEntry**` and `u8**` (were `void*`).
  - `MonsterBaseData`: `monsterID` is each record's own index, used for both the model and the name. `currHP` equals
    `maxHP` and `currMP` / `maxMP` are 0 in every record; SpawnEnemy and SpawnBoss start an enemy's MP from its max HP.
    Fields that are 0 everywhere and never read are marked as such.
  - **`BossData` is `BossPlacement`**: one record of `gBossPlacements[8]`, with the map, submap, boss file, the model's
    animation table (`anims`), the position and the facing (`x`, `z`, `yrot`; were `x`, `y`, `z`).
  - **`gBossTalkRadii`** is now at its first radius, 0x803AB490 (was 0x803AB48C), and IsNearBoss reads
    `gBossTalkRadii[gBossLoaded - 1]`: `gBossLoaded` is the placement + 1. The code forms the address one radius
    before the table, so splat names only that folded address, and the symbol is defined in `undefined_syms.jp.txt`
    as well.

## First publication

The matching decompilation: all 530 tracked compiled C functions byte-matched (224,616 bytes), built with the original
compiler (IDO 5.3) into the byte-identical ROM from your own copy of the game. The hand-written boot entry and the SDK
library code (27.1% of the ROM's executable code) are kept as assembly, outside those figures. The C files are named by
what they hold (README, "Source file names"); the setup fetches and checks the toolchain, and `tools/verify.py`
recomputes the figures from your build.
