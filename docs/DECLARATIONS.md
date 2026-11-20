# Declarations, headers and IDO's .bss order

Every variable and every shared type of the game has one declaration, in a header, and so does every function defined
in C or called from C, but the few listed under "Exceptions"; the C files include what they use. This page says
where each kind is declared, why the original compiler makes that harder than usual, how the project keeps the
original layout anyway, and what a contributor does.

## Where things are declared

| What | Declared in |
|---|---|
| A function the game defines in C, or still in assembly and called from C | the header of its module: `include/game/<module>.h` (`include/sched/nnsched.h`; the title's units share `game/title.h`, the intro's `game/intro.h`, the Controller Pak manager's `game/pak_menu.h`), with the module's prototypes in definition order; the types those prototypes name come before them, the module's variables after. A function still in assembly that no C names (the intro overlay's 31, called from its assembly) has no declaration |
| A variable a game C unit defines | the header of the unit that defines it. A unit's `.bss` variables are declared together, in their storage order (below); its initialised data follows |
| A label of a unit's own assembly (data still written as assembly in its `.c`) | the unit's header |
| A record the asset files define (maps, NPCs, monster sets, boss files, the spell bank, Brian) | its asset header: `include/maps.h`, `npcs.h`, `monster_sets.h`, `boss_files.h`, `spell_bank.h`, `brian.h` |
| A segment bound the linker defines (`<segment>_ROM_START`, `_ROM_END`, `_VRAM`, ...) | `include/segment_symbols.h` |
| A link name for one place inside a C-defined block (`undefined_syms.jp.txt`: `name = gBlock + offset`) | beside the block, in its unit's header |
| Storage no C unit defines yet: assembly, the binary pieces of main's image, the cutscene bank, fixed addresses | `include/variables.h`, under a heading for where it lives; an overlay's own such data in the overlay's header |
| The SDK's functions and variables | the SDK's headers (`include/2.0I`) |
| A type | the header of the module whose interface uses it; the types the whole game shares in `include/quest_types.h` and `include/structs.h`. A module header (`include/game/`, `include/sched/`) includes `common.h` and the headers of the types it names, and compiles on its own; the headers `common.h` itself includes (`quest_types.h`, `structs.h`, `variables.h`) and seven asset and format headers (`boss_files.h`, `monster_sets.h`, `spell_bank.h`, `map_format.h`, `proj_material.h`, `spell_burst.h`, `raw_data.h`) assume `common.h` before them, as every unit has it first |

### Exceptions

Each is stated where it stands.

- **Variables.** A C file declares no variable itself, except under a comment that holds `local declaration:` and the
  reason, only for a variable that no header the file includes declares, and only for these two (the checker holds
  the same list; a new exception goes into both):
  - `src/game/spells.c` declares the five player and enemy variables it uses (RULE-EXCEPTION R25: it includes neither
    `game/player.h` nor `game/enemy.h`, whose prototypes would change its calls);
  - `DrawDialogueBox` (`src/game/message.c`) declares `D_800616B4` at block scope: its type, `WindowOrder`, is the
    function's own.
- **Functions.**
  - RULE-EXCEPTION R25, a function only its own unit calls with another argument list, stays out of its header:
    `ClearHitMaskBit` and `SpellCollision0` (`src/game/spells.c`), `SwitchMusic` (`src/game/audio.c`), `UpdateLightDirections`
    (`src/game/lighting.c`).
  - RULE-EXCEPTION R25, a unit that may not include a module's header declares what it calls from it: `src/game/spells.c`
    (`ShowEnemyMiss`, `EnemyAgilityCheck`, `StartBrianDodge`, `RollStaffHit`) and `src/game/player.c` (`StepLightAnim`).
  - Functions no module of the game defines, declared where they are called: the loaded overlay's entry points at the
    overlay slot (`func_8020C15C`, `func_8020C0D0`, in `src/game/main.c`), the compiler's runtime (`__ull_div`, in
    `src/game/start_menu.c`), and an SDK function the 2.0I headers leave out (`osGbpakInit`, in `src/game/save_menu.c` and
    `src/game/controller.c`).
- **The SDK library units** (`src/libultra/initialize.c` to `src/libultra/visetevent.c`) keep the SDK's own conventions.

`tools/check_decls.py` checks the variable rules (below). The function rules are not machine-checked: the compiler
compares each prototype with its definition, since every unit includes its own module's header.

## Why the original compiler makes this hard: IDO 5.3's .bss order

IDO's front end gives every symbol it meets a *block number* (a "dense number"), in the order it meets them in the
compilation unit, headers included. Its code generator lays the unit's `.bss` out by block number **modulo 256**: the
variables are sorted by `block % 256`, ties going to file-scope variables in block order, then to function statics in
reverse order. A variable whose address a `.data` or `.rodata` initialiser takes comes first, in initialiser order
(confirmed on IDO 5.3 test units; no unit of this game has one). Each variable is aligned by its size, not its type:
the largest power of two not above the size, at most 8, so a 3-byte `char` array gets 2 and a 6-byte struct of shorts
4 (measured on IDO 5.3 test units; the u-code records 4 for the first, which is not what the code generator does).

With `-g` (the game's units are compiled `-O2 -g3`) every new declaration takes block numbers:

| Construct | Block numbers |
|---|---|
| a new `extern` variable, a typedef, a forward `struct x;` | 1 |
| a struct, union or enum definition | 2 (its members none) |
| a typedef of a struct defined in place | 3 |
| a function definition | about 5 (more with string literals and nested blocks) |
| a prototype, a repeated declaration, a `#define` | none |
| a struct tag first named inside a declaration | 1 more: `void f(struct New *p);` takes 1, `extern struct New *p;` takes 2 |

So a header that gains or loses a declaration moves every later block number in every unit that includes it, and a
unit whose variables straddle a multiple of 256 gets another `.bss` order. Nothing in the source changed, but the
variables are at other addresses, every reference to them changes, and the build is no longer the original ROM.
Units compiled without `-g` (the title overlay, Brian's model, the SDK library units) are not affected: there a
declaration takes no block number, and the order is the order of the definitions.

These numbers were measured on this project's units and flags, and the rule predicts the built `.bss` of all of its
C units exactly (`tools/bss_order.py` does the prediction).

Before the mechanism below, the project avoided the problem by declaring each global with an `extern` in every file
that used it, placing declarations where their count put the variables in order, and keeping unused typedefs for
their count. That is gone.

## How the project keeps the original order: `BSS_ORDER_PAD`

`BSS_ORDER_PAD(n)` (`include/bss_order.h`, written by `tools/make_bss_order_h.py`, through `common.h`) expands to `n`
forward struct declarations and nothing else: no code, no data. In a `-g` unit it takes exactly `n` block numbers.
`0` stands for a whole turn (256 declarations): the same order, and the slot stays visible to the tool. A `-g` unit
with more than one `.bss` variable has a slot before its own header's `#include`; a unit with `.bss` function
statics has a second after its `#include` block, which places the statics among the rest:

```c
#include "common.h"
BSS_ORDER_PAD(0)            /* places the unit's state: its header declares it next */
#include "game/map.h"
#include "game/camera.h"
...
BSS_ORDER_PAD(92)           /* only in a unit with .bss statics */
```

The values are not written by hand. `tools/bss_order.py` computes them:

```
python3 tools/bss_order.py check [UNIT...]   # exit 1 if a unit's .bss would differ from the original
python3 tools/bss_order.py fix [UNIT...]     # rewrite the BSS_ORDER_PAD values of the units that differ
python3 tools/bss_order.py show UNIT          # block numbers, the predicted and the original layout
python3 tools/bss_order.py built              # the built objects' .bss against the original
```

It prepares each unit exactly as the build does (asm-processor, the unit's flags from `build.ninja`), stops the
compiler after the front end (`cc -Hf`), reads the block numbers from the symbol table and the u-code, and predicts the
layout. The original layout is the project's own record: each unit's `.bss` start in `eltail.jp.yaml` and each
variable's original address in `symbol_addrs.txt` (two statics that share a name are recorded in the tool by their
function). `fix` tries values in a fixed order and keeps the first set that gives the original layout, so a given tree
always gets the same values, and a second run changes nothing. Before predicting anything, `check` and `fix` require:

- every `.bss` variable recorded: its original address in `symbol_addrs.txt` (or, for a static that shares its name
  with another of the unit, in the tool by function); a variable renamed without its record fails;
- the unit's file-scope `.bss` declared in storage order (block numbers rising with the original offsets): an order
  that would match only by wrapping around 256 is refused, never padded into place;
- every `BSS_ORDER_PAD` one the compiler saw, of its own size: one a line, at file scope, in the unit's own `.c`;
- the first slot before the unit's `.bss` declarations, and in a unit with `.bss` statics, a slot between those
  declarations and the statics; a `-g` unit with more than one `.bss` variable and no slot fails.

A build that does not need the original layout can define `NO_BSS_ORDER_PADS`: the pads drop out.

The symbol-table reading and the layout rule follow zeldaret/oot's `tools/ido_block_numbers.py` and
`tools/fix_bss.py` (CC0-1.0), written for IDO 7.1, checked here against IDO 5.3.

## Units with .bss statics

In three units, function statics sit among the file-scope `.bss` in the original layout: `src/game/map.c`
(`FadeSkyPalette`'s four), `src/game/start_menu.c` (`DrawEnvironmentPage`'s knob) and `src/game/audio.c` (`QueueSound`'s and
`PushSoundQueue`'s counters). A static between two file-scope variables whose declarations are consecutive in the
header must tie with one of them modulo 256, so the spacing of those declarations and the spacing of the statics'
declarations must agree. The second slot shifts the statics as a group; it cannot change their spacing.

- `src/game/map.c`: in `game/map.h`, `gBillboardProps`, `D_80082910`, `gFlameProps`, `gScrollingProps` and
  `gSpinningProps1` stay consecutive, and in `FadeSkyPalette` the static declarations from `gSkyFadeTotal` to
  `gSkyFadeOut` keep their order and spacing. A declaration inserted or removed there is refused: `fix` writes
  nothing and names the statics, their neighbours and the stretch. Comments at both places say so.
- Elsewhere, `fix` places the layout: one declaration added at any other place in these units' state blocks, or in
  the functions that hold their statics, was placed at every place it was tried.

## For contributors

- **A new global.** Define it in its unit; declare it in the unit's header, with the unit's other state if it is
  `.bss`; include the header where it is used. In the reference build every `.bss` variable is one of the original's,
  so record its address in `symbol_addrs.txt` (a renamed one too: `check` fails on a `.bss` variable without one). In
  a modified build its place does not matter.
- **A new function.** Its prototype in its module's header, among the others in definition order.
- **A new type.** In the header of the module whose interface uses it, before the prototypes; a type the whole game
  shares in `quest_types.h`.
- **Any source edit** (a header, or a function body: a declaration added before or between a unit's statics moves
  them). Run `python3 tools/bss_order.py fix`, then build.
  The reference build is the original ROM again.
- **A merge conflict on a `BSS_ORDER_PAD` line.** Take either side and run `fix`: the values follow from the tree.
- **A unit split off with more than one `.bss` variable, compiled `-g`.** Put `BSS_ORDER_PAD(0)` just before its own
  header's `#include` (and a second after its includes if it has `.bss` statics), and run `fix`, which says when a
  unit needs a slot it lacks.
- **What `fix` refuses.** A `.bss` variable with no recorded original address; a unit's `.bss` declared out of
  storage order; more than about 256 minus the span of its
  state declarations inserted between two of them (the state would straddle a turn of 256 and no single value places
  both parts); the stretches of the units with `.bss` statics above. It says which and writes nothing.
- **Modified builds.** The pads emit no code or data. A build that grows a variable moves the `.bss` after it and
  every reference follows through the link; a test build with a `.bss` array doubled was checked
  statically and in an emulator, over the paths it played (the project's own test, not part of this repository). With `NO_BSS_ORDER_PADS` the source compiles and links, and
  the units with `.bss` statics take another `.bss` order.

## The checks

| Check | What it catches | Where it runs |
|---|---|---|
| `tools/check_decls.py` | a variable declared in two headers; a game global no header declares; an `extern` in a C file without a stated reason, with one for a variable a header the file includes already declares, or not among the documented exceptions; declarations that disagree on a type or an array bound; a defining unit that does not include its declaration's header; a variable defined in a header (a tentative definition, a `static`, an `extern` with an initialiser) | by hand (`python3 tools/check_decls.py`) |
| `tools/bss_order.py check` | a unit whose `.bss` would leave the original layout; a `.bss` variable with no recorded original address; `.bss` declared out of storage order; a stale, misplaced or missing `BSS_ORDER_PAD` | by hand |
| `tools/bss_order.py built` | a built object whose named `.bss` variables are not at their original offsets, or a missing object (function statics have no symbol in an object: `check` and the ROM cover them) | by hand, after a build |
| the build's checksum | any byte of the ROM; when it fails, the build runs `tools/bss_order.py check`, whose `FAIL` lines name a unit whose `.bss` left the original layout | every build |

## Limits

- The rule and the numbers were measured on IDO 5.3 with this project's flags and units.
- The function rules are not machine-checked (above).
- The SDK library units keep the SDK's own conventions and are left out of the declaration rules.
