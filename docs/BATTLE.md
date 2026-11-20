# The battle system: how a battle starts, runs and ends, and how its results are calculated

A battle in this game is a ring on the field: Brian and up to six enemies (or one boss) take turns inside a circle,
and on each turn one side moves within a smaller ring of its own and acts. This page describes the rules as the
matched C states them: the state a battle keeps, how it starts, how turns pass, what each side can do, how hits,
damage, statuses and growth are calculated, how a battle ends, what differs for a boss, which data it reads, and
where to change it. The names are the source's. The constants are in `include/game/encounter.h` (`BATTLE_`),
`include/quest_types.h` (`STATUS_`, `HIT_`, `ENEMY_ACTION_`, `ENEMY_FLAG_`, `PLAYER_`, `ITEM_`, `ELEMENT_`,
`MONSTER_AI_RANDOM`), `include/structs.h` (`SPELL_` on `SpellDef`), `include/game/spells.h` (`SPELLSLOT_`,
`SPELL_DIR_`, GetSpellAvailability's results), `include/game/player.h` (`BRIAN_STATE_`, `BRIAN_ANIM_`) and
`include/variables.h` (`GAME_STATE_`). Brian's animations are described in `docs/PLAYER.md`, the maps' encounter
data in `docs/MAPS.md`, the save in `docs/SAVE.md`.

**Evidence.** Each rule is marked:
- **[traced]**: the original game was run in an emulator with breakpoints on the functions named. An independent
  calculation from the values each call read and the random numbers it drew equals what the call returned and
  changed, call by call. The cases are listed in "How this was checked".
- **[traced, forced]**: the same, in a case set up by writing memory first. What was set is said: a random seed so a
  roll comes out a given way, a growth counter, a stat, a target's element, an HP kept up. The rule then ran as the
  code has it.
- **[observed]**: seen in a traced run, without an independent calculation (an outcome, a sequence).
- **[static]**: read from the code, not run. Where the code leaves something open, this page says so.

## Where the code is

| Unit | Header | The battle's part |
|---|---|---|
| `src/game/encounter.c` | `game/encounter.h` | random encounters, both battle starts, the intro camera, the round loop (`StepBattleRound`), the turn hand-over (`EndBattleTurn`), the drop (`RollItemDrop`); the battle's state (`gBattleState`, the turn index and count, the centre and radius) |
| `src/game/enemy.c` | `game/enemy.h` | the enemies (the unit's first half is the NPCs): spawning, turn order, the six action handlers, the choice of a move, movement, the melee roll, `DamageEnemy`, the boss effect script player |
| `src/game/spells.c` | `game/spells.h` | spells: which can be cast, the sixteen spell slots, movement and collision, accuracy, the ten effects, the twelve statuses (their rolls, appliers and countdown), spell power |
| `src/game/player.c` | `game/player.h` | Brian: his battle states, the staff, `DamageBrian`, the dodge, `CanBrianAct`, growth (`UpdateBrianGrowth`) |
| `src/game/boss.c` | `game/boss.h` | bosses: loading, spawning, the boss scene, `OnBossDefeated` |
| `src/game/hud.c` | `game/hud.h` | status icons and effects, the monster card, the inventory and item use |
| `src/game/effects.c` | `game/effects.h` | damage numbers, level-up effects (presentation only) |

These are the original compilation units. The battle is spread across them, and moving a function to another unit
would change the byte-identical build, so this table is the map.

Each frame, `FieldLoop` (`src/game/main.c`) calls `RollRandomEncounter` in a submap whose flags have bit 0
(`gSubmapFlags & 1`). In a battle it calls `BattleStartCamera` while `BATTLE_INTRO` is set, otherwise
`StepBattleRound` and then `UpdateEnemies`. Every frame also runs `UpdateBrian` (Brian's state handler; the handler
of states 0 to 2, `BrianFieldMovement`, runs his growth) and `UpdateSpells`.

## The battle's state

`gBattleState` (u16, `src/game/encounter.c`):

| Bit | Set by | Cleared by | Meaning |
|---|---|---|---|
| `BATTLE_ACTIVE` 0x1 | both starts | `StepBattleRound` at the end | a battle is on |
| `BATTLE_ENEMY_TURN` 0x2 | a random start when an enemy goes first; `EndBattleTurn` toggles it | (toggled) | the turn is `gTurnOrder[gTurnIndex]`'s; clear: Brian's |
| `BATTLE_TURN_START` 0x4 | both starts, `EndBattleTurn` | `StepBattleRound` | a turn waits to start |
| `BATTLE_WON` 0x8 | `StepBattleRound` | `UpdateBrian` (the rewards) | won |
| `BATTLE_ESCAPED` 0x10 | `StepBattleRound`, `SpellEffect_Escape` | the next start | Brian got away |
| `BATTLE_BOSS` 0x100 | `StartScriptedBattle` | `StepBattleRound` at the end | a boss battle |
| `BATTLE_INTRO` 0x200 | both starts | `BattleStartCamera` | the opening camera runs |
| `BATTLE_ENEMIES_DOWN` 0x400 | `EndBattleTurn` | the next start | no enemy above 0 HP was left to take a turn: won |
| `BATTLE_NO_ENCOUNTERS` 0x8000 | no code in the game | both starts (they assign the whole word); `InitEncounters` keeps it and clears the rest | random encounters do nothing while it is set |

[static; the starts' values traced] No code in the ROM writes `BATTLE_NO_ENCOUNTERS`, and no assembly outside the C
refers to `gBattleState`: only a memory edit can set it, and a battle's start clears it.

## Starting a battle

**A random encounter** (`RollRandomEncounter`, `IsInEncounterCircle`, `FindEncounterZone`, `CreateEncounter`)
[traced, forced chance]:
1. It runs only outside a battle, with `BATTLE_NO_ENCOUNTERS` clear, while Brian is not busy and is moving. His steps
   add up in `gEncounterWalkDistance`. Every 50 units it starts again from 0, `gEncounterChance` grows by 50, and the
   roll `getRandomNumber(2000) < gEncounterChance` asks for a battle. The chance is reset only when a battle starts:
   a roll that finds no fight spot or zone leaves it as high as it was.
2. The battle needs a fight spot 50 to 90 units away (the distance squared in [2500, 8100]). The first six such spots
   in the submap's table are kept and one is chosen at random. It also needs a zone of the submap's
   `EncounterZoneList` that holds Brian (`x <= Brian.x <= x + width`, the same for z). `docs/MAPS.md` describes
   the map data.
3. `CreateEncounter`: the zone's table is a count followed by that many pack indices, and one pack is chosen at
   random. Each of the pack's spawn entries adds `base_count` enemies, plus `getRandomNumber(count_variance + 1)`
   when the variance is not 0 [traced]. They stand at the pack's positions around the fight spot, rotated by the
   angle Brian approached from.
4. `SpawnEnemy` copies the monster record's attack into the enemy's battle stats (`atkScale` 0x10, sixteenths: 1.0),
   with its agility and defence, and starts its HP at the record's `maxHP`. Its MP starts from the record's `maxHP`
   too, because every record's `maxMP` is 0. Its movement bound is `(f32)agi * 0.2857 + 17.0` [traced].
5. `SortTurnOrder` orders the enemies by their record's agility, highest first. It is a bubble sort that keeps spawn
   order on ties [traced]. If Brian's agility is below the first enemy's, that enemy goes first (`BATTLE_ENEMY_TURN`,
   turn index 0). On a tie or better Brian goes first: the turn index is set to the last, so the first hand-over
   wraps to 0 [traced].
6. `gBattleState = BATTLE_ACTIVE | BATTLE_TURN_START | BATTLE_INTRO`. The battle circle has radius 100 around the
   fight spot, the first turn waits 20 frames (`gTurnDelay`), `SetupBrianForBattle` runs, and music 13 starts. The
   intro camera looks at an enemy chosen at random and, by another roll, from one side or the other.

**A boss battle**: walking within a boss's talk radius (`IsNearBoss`, `gBossTalkRadii`) starts its scene.
`StartBossScene` shows the boss's name and text and sets its camera, and `UpdateBrian` sets `PLAYER_BOSS_SCENE`.
When the message closes, `BrianFieldMovement` calls `StartScriptedBattle` [traced]. It sets
`BATTLE_ACTIVE | BATTLE_TURN_START | BATTLE_BOSS | BATTLE_INTRO`, gives Brian the first turn with no delay, and
centres the battle on the boss. Boss 6's battle is centred at 0, 0 instead. Bosses are numbered 0 to 7 in the code
(`gBossLoaded` and `gBattleBoss` are the boss number + 1). The battle radius is `gBossBattleRadii[gBattleBoss]`: 85,
140, 100, 150, 150, 100, 100 and 190 for bosses 0 to 7. The music is 0, or 0x29 for the last boss, Mammon (boss 7). The boss's movement bound is
`sqrtf(agi) * 3.846 + 17.838` [traced: 59.9688 for an agility of 120, radius 100 for boss 5].

**`SetupBrianForBattle`** [static; its results read in every trace]: `gBattleExp` and `gBattleStone` go to 0.
Brian's movement radius is `agi * 0.2857 + 17`. His battle stats: `atk` is **the four spirits' total** at `atkScale`
0x10, `agi` and `def` are his own, he has no statuses, and `moveScale` is 1.0. `comboWindow` is the total less each
spirit's excess over a quarter of the total, then `/ 10 + 1`, at most 16. He then waits in `BRIAN_STATE_WAIT` for
the length of the battle-start animation.

## Turns

[traced] Brian and the enemies alternate: Brian, an enemy, Brian, the next enemy, and so on.
`EndBattleTurn(actor)` first counts down the actor's own statuses (`CountDownStatuses`, see Statuses). Then, in a
battle:
- after Brian's turn it moves `gTurnIndex` to the next enemy in `gTurnOrder` that is present and above 0 HP,
  wrapping. If there is none it sets `BATTLE_ENEMIES_DOWN`. The next turn waits 20 frames (`gTurnDelay`);
- after an enemy's turn there is no wait;
- either way it toggles `BATTLE_ENEMY_TURN` and sets `BATTLE_TURN_START`.

`StepBattleRound` [traced] acts at a turn's start, once `gTurnDelay` has run out and the actor is no longer busy:
- **Won**: no enemy is left (`gEnemyCount` 0), or `BATTLE_ENEMIES_DOWN` is set.
- **Escaped**: `BATTLE_ESCAPED` is set, or, outside a boss battle, Brian stands outside the battle circle
  (`PLAYER_OUTSIDE_CIRCLE`).
- **Otherwise**: the actor's movement ring, centred where it stands. Its radius is the enemy's movement bound (or
  Brian's movement radius) times its `moveScale`, plus its model radius times its scale.
  - During his turn Brian moves only inside that ring (`BrianBattleMovement`). An enemy is held to it by `MoveEnemy`,
    which sets `ENEMY_FLAG_AT_LEASH` at the edge.
  - Everyone stays inside the battle circle, except Brian outside a boss battle: he may walk out of it, and that is
    the escape.
  - Brian cannot move while an enemy acts, during the intro, or during the turn delay.

Brian's turn ends when his staff swing ends, when his spells have all ended, when an item's use ends, or when he
presses A or Z with no enemy in reach to hit (passing the turn) [traced]. Each of these calls `CountBattleAgility`
and `EndBattleTurn`.

## What Brian can do

`CanBrianAct` [static] holds in a battle only on his own turn (neither `BATTLE_ENEMY_TURN` nor `BATTLE_INTRO`), after
the turn delay, with no spell in the air, while he is not busy, and before the battle is won. It never holds during
a map change (`GAME_STATE_MAP_CHANGE`).

**The staff** [traced]. Each frame of his turn, `FindStaffTarget` looks in turn order for an enemy that is:
- closer than (his radius × his scale + its radius × its scale) + 10;
- within the two models' heights vertically (a flying enemy by its flying height; Mammon always);
- in front of him, within 1.5 of its scaled radius sideways.

A or Z with a target starts `BRIAN_STATE_STAFF`. When the swing's timer runs out:
- **The hit roll** (`RollStaffHit`): the chance (s16) is `agi * 100 / (agi + ((target agi + 7) >> 3))`, and the
  swing hits when `getRandomNumber(100) < chance` [traced: hits and misses]. A miss shows the enemy's MISS.
- **The power** (`CalcStaffPower`) [traced: 0, 1, 2 and 3 spirits over a quarter; the critical forced by the seed]:
  1. `atk` is the spirit total and `quarter = atk >> 2`.
  2. `total` is `atk` less, for each spirit above `quarter`, its excess over it; `count` is how many spirits were
     above.
  3. `damage = (atk >> 1) + total`. If `count` is not 0: `damage = gStaffSpiritScale[count] * damage >> 6`, where
     the table is 64, 64, 57, 60.
  4. `damage = damage * atkScale >> 4`.
  5. A critical when `getRandomNumber(32) == 0` (`getRandomNumber(4)` with all four elements at 50):
     `damage += damage >> 1` and `PLAYER_STAFF_CRITICAL`.
  6. `STATUS_SHRUNK` halves it.
- **The hit**: `DamageEnemy(target, damage, element, HIT_PHYSICAL | HIT_NO_MELEE (| HIT_CRITICAL))`. The element
  passed is that of the last spell Brian chose, and it goes unused because the hit is physical.
  - When `DamageEnemy` returns non-zero, Brian gains 1 MP (up to his maximum), and with `STATUS_DRAIN` he also heals
    `((returned + 3) >> 2) + 1`. `DamageEnemy` returns the incoming damage for a target already at 0 HP (one in its
    last hit reaction), so a hit on such a target counts too, with nothing dealt.
  - `HPXP` grows by 1 on every successful hit roll.
- **The combo** [static]: A or Z pressed in the swing's last `comboWindow` frames sets `PLAYER_COMBO_INPUT`. If the
  target is still above 0 HP, a second swing follows (`BRIAN_STATE_STAFF_COMBO`, 18 frames) at half damage (`* 0.5`).
  The source keeps the original's dead branch here: it sets `BRIAN_STATE_WAIT` and then tests for
  `BRIAN_STATE_STAFF`, so the wait after a swing always takes the combo animation's length (the comment there says
  so).

**Spells** [traced]. The C buttons enter up to three directions (`BrianSpellInput`; `SPELL_DIR_UP` 0, left 1, down
2, right 3). The first direction is the element: up fire (Fire Ball), left earth (Rock), down water (Water Pillar),
right wind (Wind Cutter), as the game's spell banner names them.
- **Refusals**: a direction is refused when the spell it would make is short of MP or of spirits
  (`GetSpellAvailability & (SPELL_LOW_MP | SPELL_LOW_SPIRITS)`). A or Z casts only when the spell can also be cast
  here (`SPELL_NOT_HERE`: `SPELL_IN_BATTLE` / `SPELL_IN_FIELD` in its `on_hit`). Under `STATUS_SILENCE` no spell
  input starts.
- **Finding the spell**: `GetSpellFromPath` looks it up; it sorts the directions after the first, so their order
  does not matter.
- **Casting** (`BrianStartCast`): the MP cost `gSpellMpCosts[presses - 1]` (1, 2, 3) is paid at the start [traced].
  Brian enters `BRIAN_STATE_CAST`, and the spell spawns 18 frames later. With all four elements at 50 it spawns with
  `SPELLSLOT_BOOSTED`: twice the extra shots, and half again the lifetime for the spells `SpawnSpell`'s first
  placement case places.
- **A spell on himself** plays the item-use animation (`PLAYER_CAST_ON_SELF`).
- His turn ends when no spell is left in the air.

**Items** [static]. `ItemData.canBeUsed` holds `ITEM_USE_FIELD`, `ITEM_USE_BATTLE` and `ITEM_NOT_BOSS`; its bit 4
(on items 20 to 24) has no reader. `type`:
- `ITEM_TYPE_HEAL`: `itemArg1` HP and `itemArg2` MP, each up to the maximum; usable only when not already full.
- `ITEM_TYPE_WINGS`: the field only.
- `ITEM_TYPE_SPELL`: casts spell `itemArg1` with `SPELLSLOT_SURE`, so it hits and its statuses take with no rolls.
- `ITEM_TYPE_NONE`: never usable.

The 25 item records:

| Items | Effect |
|---|---|
| 0 to 3 | heal HP 9999, 50, 100, 150 |
| 4 to 7 | restore MP 9999, 10, 20, 40 |
| 8 to 13 | spell items (8 to 10 not in a boss battle) |
| 14 to 19 | the Wings to six destinations |
| 20 to 24 | no effect; the four boss drops are among them |

## What the enemies do

`UpdateEnemies` runs one handler per enemy each frame from `gEnemyActionHandlers`, chosen by
`EnemyAction.actionID`:

| ID | Handler | What it does |
|---|---|---|
| `ENEMY_ACTION_IDLE` 0 | `EnemyIdle` | idles. At its turn (`ENEMY_FLAG_TURN`, set by `FlagTurnEnemy` at the turn's start) it shows the monster card (`gCardEnemy`, 60 frames). Under `STATUS_IMMOBILE` it ends the turn at once [traced]; otherwise it chooses a move |
| `ENEMY_ACTION_APPROACH` 1 | `SelectEnemyAction` | walks toward Brian, then makes its move or ends its turn |
| `ENEMY_ACTION_HIT` 2 | `EnemyKnockback` | the hit reaction `DamageEnemy` starts. At its end an enemy at 0 HP dies: `gBattleExp += exp`, `gBattleStone += stone` [traced] |
| `ENEMY_ACTION_FACE` 3 | `EnemyFaceBrian` | **no code sets this action** |
| `ENEMY_ACTION_MELEE` 4 | `EnemyMeleeAttack` | wind-up, the hit window, recovery |
| `ENEMY_ACTION_CAST` 5 | `EnemyCastAttack` | casts the move's spell; its turn ends at the cast |

**The choice of a move** (`PickEnemyAIOption`) [traced]. It measures the distance to Brian less his radius times his
scale. The monster's AI options are sorted by `max_range`, shortest first:
- the first option whose range times the enemy's scale reaches Brian is chosen;
- if Brian is out of an option's range but within reach after a walk (the movement bound times `moveScale`, added
  to the range), and the next option reaches him, one of the two is taken at random (`getRandomNumber(2)`). Taking
  the nearer one marks `ENEMY_FLAG_OUT_OF_RANGE`, so the enemy approaches first;
- the last option is taken (with `ENEMY_FLAG_OUT_OF_RANGE`) when Brian is within its range plus the movement bound.

`MONSTER_AI_RANDOM` (Mammon's record) then replaces the choice with any of the monster's options at random, keeping
the approach decision.

**The approach** (`SelectEnemyAction`) [static; its outcomes traced]: the enemy turns toward Brian and walks. A boss
moves 0.4 units a frame; a field enemy moves by its walk animation's root motion, played at ten times its rate. It
has four walk cycles:
- with a chosen move out of reach, it makes the move as soon as Brian is within the move's `max_range`, and also when
  the tries run out or it reaches the edge of its ring, even out of reach;
- with no move chosen, it chooses again after each cycle, and ends its turn when none comes in reach within the four.

An enemy that finds no move at its turn approaches the same way.

**`CommitEnemyAIOption`** [traced]: a spell move under `STATUS_SILENCE` ends the turn. Otherwise the action becomes
`ENEMY_ACTION_CAST` or `ENEMY_ACTION_MELEE`, with the move's animation and delay. In a boss battle it also starts the
boss effect script named by the move's low byte.

**A melee hit** (`EnemyMeleeAttack`) [traced]. In the hit window, once per swing (`ENEMY_FLAG_HIT_RESOLVED`), if
Brian is within the move's `hit_range` times the enemy's scale and is not `ACTOR_NO_MELEE_HIT`, it rolls
`EnemyAgilityCheck`: the staff's formula with the enemy as the attacker.
- **A miss** is Brian's dodge (`StartBrianDodge`) and adds 1 to his `agiXP`.
- **A hit**:
  1. `CalcEnemyMeleeDamage` computes `atk * atkScale >> 4`.
  2. A critical when `getRandomNumber(32) == 0`: + half, and `ENEMY_FLAG_CRITICAL`, which becomes `HIT_CRITICAL`.
  3. `STATUS_SHRUNK` halves it.
  4. `DamageBrian(power, HIT_PHYSICAL | HIT_NO_MELEE (| HIT_CRITICAL))`.
  5. `STATUS_DRAIN` heals the enemy `((dealt + 3) >> 2) + 1`.

## Spells in flight and their effects

[traced] A spell lives in one of sixteen slots (`gSpellSlots`) for its lifetime. Each frame it is moved by its
movement kind and tested by its collision kind; a spell with more shots queues the others (`gSpellQueues`, four
queues). When it touches a target (`UpdateSpells`), the target is marked in the slot's hit mask, so each target is hit
once. Then:
1. **The barrier**: if the target has `STATUS_SPELL_BARRIER` and the spell's target type is not `SPELL_TARGET_SELF`,
   nothing more happens: no roll and no effect.
2. **Accuracy** (`RollSpellAccuracy`): `SPELLSLOT_SURE` hits.
   - Otherwise it rolls `getRandomNumber(100) < hit_rate`, and on a target other than the caster also the agility
     roll: `RollStaffHit` on an enemy, `EnemyAgilityCheck` on Brian.
   - `gAccuracyResults` [1, 2, 0] gives 0 for a hit, 1 for a miss on the hit rate, 2 for a dodge; any non-zero
     result is a miss [traced: hits, dodges, a miss on the hit rate].
   - A spell on its own caster still rolls its hit rate [traced: the Escape spell].
   - A miss shows MISS on the caster's own target, or the enemy's MISS. A spell that misses Brian makes him dodge
     (`StartBrianDodge`), without the melee dodge's `agiXP`.
3. **The effect**, by `spell_type` (`gSpellEffectHandlers`): `SPELL_EFFECT_DAMAGE`, `_HEAL`, `_DRAIN_MP`,
   `_ESCAPE`, `_RETURN`, `_STATUS_ONLY`, `_EXIT`, `_INSTANT_KILL`, `_CURE`, `_DAMAGE_CURE` (below).
4. **Statuses**: for each of the twelve slots the spell's `effects[]` names, `RollStatus`; on success
   `RollStatusDuration` and the slot's applier (`gApplyStatusHandlers`) with the level `effects[]` gives. A
   status-only spell all of whose rolls fail shows MISS.
5. **The spell's end**: unless it pierces (`SPELL_PIERCES`), the spell ends at its first hit, the barrier's included.

**Where a spell starts and how it moves** [static]: `SpawnSpell` places it by its `originType`
(`gSpellOriginHandlers`: `SpellOrigin_Feet`, `_Middle`, `_Above`, `_FrontMiddle`, `_FrontAbove`, `_BehindAbove`,
`_TargetFeet`, `_RandomAbove`, `_Ahead`, `_HighAhead`). Each frame its `projectile_type` moves it
(`gSpellMovementHandlers`): kind 0 flies along its velocity, and kinds 1 and 2 only rise or fall, a facing-target spell
of those kinds following its target sideways while the target is not busy. Fire Ball starts in front of Brian and flies (kind 0). Water Pillar
(water 0, target "the first enemy") starts in front of him too, his radius × his scale plus the spell's radius away,
and stays there (kind 1, set on the ground). So it reaches only an enemy right in front of him. In three traced runs
Water Pillar cast at enemies 50 or more units away never hit.

**Spell power** (`CalcSpellPower`) [traced]:
1. The level is the enemy's record `atk`, or, for Brian, his spirits in the spell's element plus a sixteenth of the
   opposed element's and an eighth of each of the other two (`gElementPowerOrder`).
2. `power = (f32)(def->power * 0.02)` and `step = power * 0.2`.
3. Then, level times: `power += step; step *= 1.03`, in single precision with 1.03 a double.
4. The result is truncated.

**`SPELL_EFFECT_DAMAGE`** (`SpellEffect_Damage`) [traced]: the power, halved by the caster's `STATUS_SHRUNK`.
- **On an enemy**: a critical only when Brian has fire at 50 (`PLAYER_AT_50(ELEMENT_FIRE)`), on
  `getRandomNumber(32) == 0` (4 with all four at 50): `+ (power + 1) >> 1` and `HIT_CRITICAL`. Then
  `DamageEnemy(power, element = the spell's path[0], flags = on_hit | critical)`, and `MPXP` + 1 when it returns
  non-zero (a target already at 0 HP included, as for the staff).
- **On Brian** (an enemy's spell): the critical roll always (`getRandomNumber(32)`), then `DamageBrian`.

**The other effects** [traced: heal, drain; observed: escape; static: the rest]:
- `SPELL_EFFECT_HEAL`:
  - on Brian `(spirits[element] >> 1) + maxHP * power / 100`, plus `getRandomNumber(heal >> 2)`; applied (and
    `MPXP` + 1) only when he is below his maximum, though the roll is drawn either way;
  - on an enemy `maxHP * power / 100` plus the same roll.
- `SPELL_EFFECT_DRAIN_MP`: the target loses `(its MP * 11 + 63) >> 6`. Brian gains it, up to his maximum, only when
  he is below it (then `MPXP` + 1), but the enemy loses it either way. Enemies' MP starts at their record's `maxHP`,
  so the drain is worth the enemy's HP figure. The drain cast on Brian adds the MP to `gEnemies[6]` (`GetEnemyIndex`
  returns 6 for Brian), past the table's six entries; no enemy casts the drain spell (see "What each side can
  cast").
- `SPELL_EFFECT_ESCAPE` (Brian's water spell 8, on himself):
  - on Brian outside a boss battle, `BATTLE_ESCAPED` and `MPXP` + 1 [observed: the battle ended escaped];
  - in a boss battle MISS;
  - its enemy branch (the enemy leaves, no experience) has no boss check, and no enemy casts the spell.
- `SPELL_EFFECT_INSTANT_KILL`:
  - outside a boss battle an enemy leaves at once, giving no experience (`MPXP` + 1);
  - a boss shows MISS;
  - on Brian `DamageBrian(currHP * 4)`; no enemy casts it.
- `SPELL_EFFECT_STATUS_ONLY`: on another target the hit reaction with no damage (`DamageEnemy(0, HIT_NO_MELEE)`,
  `MPXP` + 1, or `DamageBrian(0, HIT_NO_MELEE)`); on a self-target spell nothing.
- `SPELL_EFFECT_CURE`: only when the target has a status. It clears every status bit and puts `atkScale`, `def`,
  `agi` and `moveScale` back from the record (Brian's from his own stats, which share the record's layout). On Brian
  it adds `MPXP` + 1, whoever cast it. It does not put back a shrunk target's model scale: `CountDownStatuses`
  restores that only while the bit is set, so a shrunk target that is cured stays at half size for the rest of the
  battle. Brian's damage-and-cure spell (fire 12, on the first enemy) can reach that on an enemy he has shrunk
  [static: not run].
- `SPELL_EFFECT_DAMAGE_CURE`: the damage, then the cure, on the same target. Bosses 1 to 7 and two field monsters
  (set 3's monster 10, set 4's monster 5) cast spells of this kind: they clear Brian's statuses as they hit him, and
  when he had any, the cure adds to his `MPXP`.
- `SPELL_EFFECT_RETURN` / `_EXIT`: the warps, where the map allows them.

## Damage

[traced] `DamageEnemy(index, damage, element, flags, attacker)` and `DamageBrian(power, flags, attacker)` share one
calculation. Their guards differ:
- `DamageEnemy` leaves a target already at 0 HP as it is and returns the incoming damage;
- `DamageBrian` does nothing only in `BRIAN_STATE_DEFEATED`. A hit while his last hit reaction plays, at 0 HP, still
  runs: the reaction restarts and `HPXP` and `defXP` grow;
- a hit of 0 damage starts only the hit reaction.

1. **Not physical** (`HIT_PHYSICAL` clear), **on an enemy only**: the element pair is `gElementMasks[target element]
   | gElementMasks[attack element]`, with the masks [1, 2, 4, 8, 16] for fire, earth, water, wind and none.
   - The same element (1, 2, 4, 8) halves the damage [traced].
   - An opposed pair, fire and water (5) or earth and wind (10), adds a quarter [traced, forced: the target record's
     element set for the case].
   - Anything else, `ELEMENT_NONE` included, changes nothing [traced].

   **Brian takes no element modifier**: `DamageBrian` has no pair rule [traced].
2. **Physical**: a hit from behind, with the attacker more than 135 degrees off the target's facing (by the game's own
   `Atan2`), adds a half [traced: at 3.142 rad].
3. `damage *= atk / (atk + def)`, with the attacker's battle `atk` and the target's `def`, in single precision and
   truncated.
4. `damage += getRandomNumber((u32)(sqrtf(damage) * 0.75 + 1.0))`, then at least 1 [traced: ranges 2 to 10]. This is
   the function's only random draw.
5. HP falls, floored at 0. The returned damage is the whole amount, even when the target had less HP left.
   `STATUS_MP_ABSORB` adds the damage to the target's MP, capped at its maximum. An enemy's maximum is its record's
   `maxMP`, 0, so its MP becomes 0.
6. **The hit reaction** [traced: an enemy's action, `ACTOR_BUSY` and `ACTOR_NO_MELEE_HIT`, and Brian's state; his flags
   and the lengths static]:
   - an enemy's is `ENEMY_ACTION_HIT`, played at full length for a boss and at a tenth for a field enemy. At its end
     an enemy above 0 HP returns to `ENEMY_ACTION_IDLE` with `ACTOR_BUSY` and `ACTOR_NO_MELEE_HIT` cleared, and one at
     0 HP dies [traced]. Brian's is `BRIAN_STATE_HIT`;
   - `HIT_NO_MELEE` makes the target `ACTOR_NO_MELEE_HIT` while it reacts;
   - `HIT_PHYSICAL` plays the hit sound, and `HIT_CRITICAL` shows the critical number style.
7. Brian, when damaged: `HPXP` + 1 and `defXP` + 1 [traced].

The flags share one bit space. A spell passes its `on_hit` as the flags, so `SPELL_PIERCES` (4) arrives as
`HIT_NO_MELEE`, and on_hit's 0x10 and 0x20 (set on some spells) arrive unread.

## Statuses

`ActorStats.statusFlags` holds one bit per slot (`1 << slot`); `statusTimers[slot]` counts the bearer's turns.
- **`RollStatus`** [traced]:
  - sure for `SPELLSLOT_SURE` and for a spell whose target type is `SPELL_TARGET_SELF`;
  - an enemy's spell: `getRandomNumber(64) < gStatusChances[slot]`;
  - Brian's: `getRandomNumber(32)` (28 with wind at 50) `<` the chance.
- **The chances**: `gStatusChances` is 24 for every slot but 3 and 6, which have 16. That is 75% and 50% for Brian
  (86% and 57% with wind at 50), and 37.5% and 25% for an enemy.
- **`RollStatusDuration`** [traced]: on another target `getRandomNumber(3) + 2` turns, on oneself
  `getRandomNumber(5) + 3` (the tables `gStatusDurationOther` and `gStatusDurationSelf` hold 3 and 5 for every slot).
- **`CountDownStatuses`** runs at the bearer's `EndBattleTurn`: a status whose timer reaches 0 is cleared, and its stat
  or flag is put back [traced: the bits and timers; atkScale, the movement scale and the model scale put back. The
  traced defence and busy-flag cases could not tell, the value being the record's already; agility's return ran only
  on Brian, whose record the trace does not hold. Those three: static].

| Slot | Constant | Effect (the applier; its readers) | In a boss battle | When it runs out |
|---|---|---|---|---|
| 0 | `STATUS_SHRUNK` | half model scale; the bearer's staff, melee and spell damage halved | MISS | scale back |
| 1 | `STATUS_DRAIN` | the bearer's staff and melee hits heal it a quarter of the damage, plus one | applies | |
| 2 | `STATUS_ATTACK_UP` | `atkScale` 0x18 (level 1) or 0x20 (level 2) | applies | 0x10 |
| 3 | `STATUS_IMMOBILE` | an enemy's turns end at once (`EnemyIdle` tests the bit) [traced]; Brian cannot move (`BrianFieldMovement` zeroes his stick) but can still cast, swing at an enemy in reach, use an item or pass [traced: staff swings while immobile]. The applier also sets `ACTOR_BUSY`, which the end of the hit reaction the spell starts clears (`EnemyKnockback`, or `BrianState_KnockedDown` for Brian, whom `UpdateBrianGrowth` then keeps clear while he stands or walks) [traced: an enemy] | MISS | the busy flag cleared, for Brian also one his own action had set [traced] |
| 4 | `STATUS_MOVE_SCALE` | `moveScale` 0.5 (1) or 2.0 (2) | applies | 1.0 |
| 5 | `STATUS_AGILITY_SCALE` | agility × 2 (1), × 4 (2), ÷ 2 (3) | applies | the record's |
| 6 | `STATUS_SILENCE` | no spells | MISS | |
| 7 | `STATUS_REVEALED` | the monster card shows HP, defence, attack, agility and element | applies | |
| 8 | `STATUS_SPELL_BARRIER` | another's spell makes no roll and has no effect on it, though one that does not pierce still ends there; a self-target spell is not stopped | applies | |
| 9 | `STATUS_MP_ABSORB` | damage taken is added to MP | applies | |
| 10 | `STATUS_DEFENCE_SCALE` | defence ÷ 2 (1), × 0.75 (2), × 1.5 (3), × 2 (4) | applies | the record's |
| 11 | `STATUS_SLOT11` | its applier sets the bit; nothing reads it, and no spell's effects name it | | |

Every applier but slot 11's was run with its effect checked [traced; slots 3 and 4 forced].

## How a battle ends

- **Won** [observed: the end; traced: the drop roll, `OnBossDefeated`; static: the rewards and the discard]:
  `StepBattleRound` sets `BATTLE_WON`.
  1. In a boss battle it sets the boss's story flag (`SetStoryFlag(gBattleBoss - 1)`) and runs `OnBossDefeated`.
  2. It rolls the drop (`RollItemDrop`): a boss always rolls; otherwise the roll is `getRandomNumber(4) == 0` (3
     with earth at 50). The item is the drop of a random member of the group (`getRandomNumber(gTurnCount)`, those
     already defeated included), and 0xFF means none.
  3. The next `UpdateBrian`:
     - adds `gBattleExp` and `gBattleStone` to Brian's;
     - discards the drop if he already holds three of that item, or 141 items in all;
     - clears his statuses and plays the victory (`BRIAN_STATE_VICTORY`), which turns him back to his heading from
       before the battle and restarts the field music.
  4. `UpdateBrianGrowth` then gives the drop (its message, `AddItemToInventory`).
- **Escaped** [observed]: leaving the circle (not in a boss battle) or the Escape spell. His statuses are cleared and
  there are no rewards.
- **Defeated** [static; the killing hit traced]: when his hit reaction ends at 0 HP:
  - `BRIAN_STATE_DEFEATED` for 230 frames, statuses cleared;
  - music 0x1E and `GAME_STATE_MAP_CHANGE | GAME_STATE_RESPAWN`;
  - the warp to his last save point (`gSavedMap`, `gSavedSubmap`, `gSavedEntrance`).
- **A boss** (`OnBossDefeated`) [traced: boss 5]:
  - bosses 0 to 6 raise Brian's maximum HP by 10, 10, 15, 20, 20, 25, 25 and his maximum MP by 5, 5, 5, 10, 10, 15,
    15 (`gBossHpBonus`, `gBossMpBonus`), each capped at 500, and refill both;
  - Mammon (boss 7) shows its last words and starts the ending instead;
  - the story flag keeps a defeated boss from loading again.

## Growth

Brian has four counters (`BrianData1`):
- `HPXP` grows when a staff swing's hit roll succeeds, and when he takes damage.
- `defXP` grows when he takes damage.
- `MPXP` grows when one of his spells has an effect: each hit `DamageEnemy` returns non-zero for, a heal or MP drain
  that adds, an escape, a warp, a status-only hit on an enemy, an instant kill. It also grows when a cure clears
  statuses on him, whoever cast it (an enemy's damage-and-cure included).
- `agiXP` grows when he dodges a melee attack (not a spell), per 800 units walked out of battle in a submap with
  encounters (1,500 elsewhere), and per 35 units of `CountBattleAgility` [traced]. That counts, at the end of each
  battle turn, how far he stands from where the turn began, carried over from turn to turn.

`UpdateBrianGrowth` runs on each frame Brian stands, walks or runs, in a battle or out of one: its only caller is
`BrianFieldMovement`, the handler of states 0 to 2. It also clears his `ACTOR_BUSY` there unless a message or the
spirit window is open. [traced, forced: each counter set at its threshold and one below, with and without the
element-at-50 bits]
- a counter at `gXPTable[level]` or more gives one point and spends the threshold. For HP and MP the point goes to
  the maximum and the current value; agility and defence get one point each;
- each stat grows at most one point a frame, and its level counter stops at 54 (later points use the 54th
  threshold);
- HP and MP stop at 500, agility and defence at 255, and a stat at its cap has its counter zeroed;
- MP needs four times the table (three times with water at 50); agility and defence need 0.8 of it with wind and
  earth at 50 respectively.

**Spirit levels come from experience**: `exp` at `gSpiritExpTable[level]` or more gives a level (to 98, where
experience stops) and opens the spirit window (`GAME_STATE_SPIRIT_WINDOW`), in which Brian adds one spirit to an
element (`SpiritGainWindow`). An element reaching 50 sets `PLAYER_AT_50(element)`, and all four set
`PLAYER_ALL_AT_50`. Outside a battle his MP also returns, 1 per 60 units walked.

## What differs in a boss battle

[traced: the start, the radius, the movement bound, the first turn, the defeat bonus; static: the rest]
- Brian acts first; the battle centres on the boss (boss 6's at 0, 0), with the boss's own radius.
- Brian cannot leave the circle, and the Escape spell misses.
- The instant kill misses, and `STATUS_SHRUNK`, `STATUS_IMMOBILE` and `STATUS_SILENCE` miss everyone, Brian
  included.
- The drop always rolls; the record's drop may be none.
- Hit reactions play at full length.
- The boss walks 0.4 units a frame toward Brian instead of by root motion, and its moves run the boss effect scripts
  (`StartBossEffectScript`: effects, pushes, camera shakes).
- Its movement bound comes from the square root of its agility.
- Mammon (`gBossBaseData[7]`) stands 30 units lower (`ENEMY_FLAG_SUNK`), carries `actorFlags` 4 and chooses its moves
  at random (`MONSTER_AI_RANDOM`).

## The data a battle reads

| Data | Where | Reader |
|---|---|---|
| monster records (`MonsterBaseData`: HP, agility, defence, attack, element, experience, stone, drop, AI options) | the monster sets (`include/monster_sets.h`) | `SpawnEnemy`, the AI, the damage |
| AI options (`EnemyAIOption`) | the monster sets | `PickEnemyAIOption`, `CommitEnemyAIOption`, `EnemyMeleeAttack` |
| boss records `gBossBaseData`, names, talk radii, scene names | main's binary pieces (`include/variables.h`) | `SpawnBoss`, `IsNearBoss`, `StartBossScene`, `OnBossDefeated` |
| `gBossBattleRadii`, `gBossHpBonus`, `gBossMpBonus` | main's binary pieces | the boss start, `OnBossDefeated` |
| encounter fight spots, zones, packs | each map's typed data (`docs/MAPS.md`) | the encounter functions |
| spells `gSpellDefs[element][index]` (the bosses' in a fifth row), `gSpellMpCosts` | the spell bank (`include/spell_bank.h`) | casting, `CalcSpellPower`, the effects |
| `gElementMasks`, `gElementPowerOrder`, `gAccuracyResults`, `gStatusChances`, `gStatusDurationOther`, `gStatusDurationSelf` | main's binary pieces | the damage, spell power, accuracy, statuses |
| `gXPTable`, `gSpiritExpTable` | `src/game/sound.c` | `UpdateBrianGrowth`, the status page |
| `gStaffSpiritScale` | `src/game/player.c` | `CalcStaffPower` |
| item records `gItemData`, `gWingData` | main's binary pieces | item use |
| Brian's animations by state | `docs/PLAYER.md` | his battle states |

## What each side can cast

Read from the 67 monster records' AI options in the six monster sets (`assets/monsters_*/`) and the 8 boss records'
(`gBossBaseData`), against the spell table in the ROM [static]:
- **The enemies' moves**: 128 moves, 8 of them melee and 120 spells, naming 52 distinct spells. By effect:
  - damage 95;
  - status-only 11 (defence, agility, silence);
  - heal on themselves 4;
  - damage-and-cure 10: bosses 1 to 7, and set 3's monster 10 and set 4's monster 5.

  Three damage spells also roll immobile: water 5, water 6, and boss spell 11, whose immobile always misses because it
  is cast in a boss battle.
- **No enemy casts** drain, escape, return, exit, instant kill or cure. So the instant kill on Brian, the drain on
  Brian (with its `gEnemies[6]` write) and an enemy's escape are never reached in the original game.
- **Brian's spells** reach every effect kind.

## The random numbers

`getRandomNumber(n)` (`src/game/math.c`) is a linear congruential generator: `gRandomSeed = gRandomSeed * 0x41C64E6D +
0x3039`, and the result is `(gRandomSeed >> 16) % n`.
- **The seed** starts at 0x9B0. Every call with n above 0 advances it, wherever it comes from, the battle intro's
  camera choices included.
- **`getRandomNumber(0)`** returns 0 and leaves the seed as it is. A battle reaches it: an enemy's heal too small to
  roll, such as set 0's monster 5 (13 HP; its heal of 2% is 0, so its roll is `getRandomNumber(0)`).
- **Determinism**: a battle's results follow from the seed and the order of the calls, so the same inputs from the
  same seed give the same battle [traced: every traced draw equals this generator run from the seed before it].

## Changing the battle

- **The damage formula** is written twice, in `DamageEnemy` (`src/game/enemy.c`) and `DamageBrian` (`src/game/player.c`): change
  both for a symmetric rule. The element pair rule is in `DamageEnemy` only.
- **Criticals**: the `getRandomNumber(32)` in `CalcStaffPower`, `CalcEnemyMeleeDamage` and `SpellEffect_Damage`.
- **Hit chance**: `RollStaffHit` and `EnemyAgilityCheck` (the same formula), and a spell's `hit_rate`.
- **Turn order and the first turn**: `SortTurnOrder`, `RollRandomEncounter` and `StartScriptedBattle`; the hand-over
  is in `EndBattleTurn`.
- **The encounter rate**: the 50-unit step and the 50-in-2,000 growth in `RollRandomEncounter`.
- **Growth**: `gXPTable` and `gSpiritExpTable` (`src/game/sound.c`), the multipliers in `UpdateBrianGrowth`, and the
  counters where they are incremented.
- **Status chances and durations, the element masks and the accuracy results** are main's binary pieces (named in
  `include/variables.h`, not yet C data). Change them there as data, or replace them with C tables.
- **Monster, spell and encounter data** are typed sources (the monster sets, the spell bank, the maps).

**A changed rule, demonstrated**: a build from this source with `DamageEnemy`'s pair rule changed, the same element
dealing three quarters (`damage -= damage >> 2`) and the opposed element half again (`damage += damage >> 1`).
- **What moved**: the function grew 8 bytes, so the functions after it in its unit moved 8. The unit's code grew 16
  (it is padded to 16), so everything after it in main moved 16: code, data and .bss. The ROM grew 16 bytes, to
  16,777,232.
- **The checks the tools make all passed**:
  - 1,968 symbols follow their input sections; 68 lie outside any input section and are not checked;
  - every word in the binary pieces equal to a main symbol follows;
  - the names, and the ROM offsets built in code with lui + addiu, that point after main all follow; 72 other
    constants in that range (lui + ori, or a load or store base) are not counted;
  - the boss files' table follows;
  - the ROM diff explains every changed byte.

  Words in binary data that might hold a ROM offset cannot be classified by value, as for every earlier changed
  build. The changed ROM played two traced battles normally.
- **In battle**: every same-element and opposed hit dealt what the changed rule gives, and every other hit what
  both rules give [traced]. The original rule's calculation fails on exactly the changed hits.

The byte-identical build stays the reference: a changed build has its own checks.

## For a port

- The generator and the order of its calls decide every result. Any change to what calls `getRandomNumber` changes
  later battles, the presentation's calls included (the battle intro's camera, an enemy's first animation frame at
  spawn). A port that keeps this generator, its rule that `getRandomNumber(0)` leaves the seed alone, and the call
  order reproduces the original's battles from the same seed.
- The arithmetic is single precision with double constants (`* 0.75`, `* 1.03`, `0.2857`, `3.846`), and u16 values
  wrap.
- `GetEnemyIndex` returns 6 for Brian. `SpellEffect_DrainMP`'s branch for a drain cast on Brian writes `gEnemies[6]`,
  past the table; the original never reaches it.
- `ENEMY_ACTION_FACE` and `STATUS_SLOT11` are never reached, and `BATTLE_NO_ENCOUNTERS` is never set.

## How this was checked

**The method**: the original ROM ran in the ares emulator with breakpoints on the battle functions. For each call
the trace recorded:
- the arguments and the values the call read;
- every random draw made while it ran, with its range, call site and the seed before and after;
- what it returned and changed.

A model of the formulas above, written independently from this source, computed each call's result from those
inputs and its own draws. Each draw's range was checked against the model's and each draw against the generator,
and the result was compared with the call's.

**The original ROM**: twelve traced runs and 1,503 calls checked, with no mismatch.
- **Not counted as checked**: 615 records, a kind with no model (the escape, a battle's end, spawning) or a run older
  than a check's context, and every frame of a hit reaction that is neither a death nor its end.
- **Forced**: 41 calls had their entry forced (a seed, a growth counter). The runs' set-ups are in each run's log:
  Brian's stats and position, an enemy's HP kept up, and a target record's element for the element pairs.
- **The runs**: random battles on two fields; a boss battle won and one lost; forced cases for the staff critical,
  the growth thresholds, every status slot that has an effect, the element pairs and the drop roll.

| Kind | Cases run (calls) |
|---|---|
| `DamageEnemy` (58) | the same element 8, the opposed element 4 (forced elements), unrelated elements 17, physical from the front 11 and from behind 1, a hit that kills 20, no damage (the reaction only) 15, a target already at 0 HP 2; each with its hit reaction |
| `DamageBrian` (96) | a spell 94 (a critical and the killing hit among them), a melee hit 2; each with his hit state |
| `RollSpellAccuracy` (182) | hit after the agility roll 142, dodged 27, hit on the caster itself 12, a miss on the hit rate 1 |
| `RollStaffHit` (70), `EnemyAgilityCheck` (116) | hits 60 and 96, misses 10 and 20 |
| `CalcStaffPower` (12) | no spirit over a quarter 6, one 2, two 1, three 3, a critical 1 |
| `CalcSpellPower`, `SpellEffect_Damage` | 125 each, Brian's spells of every element and the enemies' |
| `RollStatus` (36), `RollStatusDuration` (20) | Brian's spell 15, an enemy's 12, sure 9; on another 11, on itself 9 |
| status appliers | slots 0 to 10, each with its effect |
| `EndBattleTurn` (219) | after Brian's turn 97, after an enemy's 122, statuses running out 11, no enemy left to act 6 |
| `EnemyKnockback` (56) | a death 20 (experience, stone, the count), the reaction's end 36 |
| `PickEnemyAIOption` (103), `CommitEnemyAIOption` (95) | the enemies' choices and commits |
| others | `CountBattleAgility` 122, `SortTurnOrder` 12, `CreateEncounter` 8, `RollItemDrop` 9, the battle start 4, `SpellEffect_Heal` 2, `SpellEffect_DrainMP` 2, `OnBossDefeated` 1, growth 14 |

**The changed build**: two runs and 254 calls checked under the changed rule, with no mismatch. Under the original
rule exactly the 10 changed hits (6 same-element, 4 opposed) fail, and every unrelated hit passes.

**Not run**: what is marked [static] above, including `SPELLSLOT_SURE` (an item spell's sure hit), the combo
swing, the defeat sequence after the killing hit, the item effects and Water Pillar's close hit.
