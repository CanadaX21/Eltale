# Changelog

What changed in this repository, newest first. Every state listed builds the byte-identical Japanese ROM (SHA-1
`4161b5c100ec82b0241b20ca8f81366e23564ccb`) from the C, with all 530 tracked compiled C functions byte-matched.

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
