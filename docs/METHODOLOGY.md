# Methodology, standards and limits

How the reconstruction was made, what counts as a match, what the figures cover, and what a byte match does not
prove.

## What counts as a match

A function counts as matched when both hold:

1. **The bytes.** The original compiler, IDO 5.3, turns the function's C into exactly the original machine code,
   compiled in its own translation unit and linked at its original address. The game's flags are `-O2 -g3 -mips2 -o32`;
   six libultra modules take the variants they need, and the title overlay's units and the data-only units (Brian's
   model tables, the asset files `./configure` writes) take `-O2 -g0` (`configure.py` lists them).
2. **The source rules.** The C is ordinary source that someone could have written:
   - no fake code (dummy or `volatile` locals, empty blocks, `do { } while (0)` bodies, self-assignments or `+ 0` /
     `* 1` written only to steer the compiler);
   - no reads or writes through offset casts where a struct member belongs;
   - one definition for each type, and one declaration for each function and variable ("Declarations" below);
   - no assembly code inside the main image's C. A few files read data from your ROM through `GLOBAL_ASM` blocks of
     `.incbin` lines, and an overlay's or the Controller Pak manager's C file includes a function not yet in C with a
     `GLOBAL_ASM` line.

Per-function comparisons were the working filter. The deciding check was the whole ROM: after every change the ROM was
rebuilt from clean and compared with the original, and a change that kept a function's score but altered any byte of
the ROM was reverted.

## Documented exceptions

Where the compiled code could only be reproduced by a form that bends a source rule, the exception was decided for
that function alone, with the evidence recorded, and it is marked in the source where it occurs. Search for these
markers to read them all:

| Marker in `src/` | What it marks |
|---|---|
| `MATCHING_HACK_IDO` | four unused locals (`pad`) that fill a stack slot the original's frame has (`StaffCreditsLoop`, `PollModelInstance`, `ScanControllerPak`, `UpdateStatusVisuals`). They are compiled only when the macro is defined, as it is by default in `include/common.h`. Building with `-DNO_MATCHING_HACKS` leaves them out, and the ROM then differs, as expected. |
| `RULE-EXCEPTION` | mostly one local that holds two values in turn where the original's frame shows a single variable (rule R30). Also: calls through unprototyped declarations, or with no declaration in scope, that pass other arguments than the definition takes or compile differently under its prototype (R25); casts for signedness (R19); and a quirk of the original kept as shipped (R36). Each marker says why. |
| `always passes`, `always true here`, `the original leaves this branch empty`, `fieldMode`, `nothing reads`, `never read`, `guess:` | statements whose form the compiled code requires: tests that always pass, a test the original repeats, an empty `else`, a constant held in a named local, a clamp whose result is not read (as the Quest 64 version of the function has it), and an initial value the compiled code needs. |

## Declarations

Every function another file calls, every game variable and every shared type has one declaration, in a header, and
the C files include what they use. [DECLARATIONS.md](DECLARATIONS.md) says where each kind is declared and lists the
few exceptions:
- calls in the original that pass an argument list the definition does not take, or compile differently under its
  prototype (`RULE-EXCEPTION R25`);
- one file (`src/game/spells.c`) that includes neither `game/player.h` nor `game/enemy.h`, for that reason, and declares the
  five variables it uses from them itself.

**IDO 5.3 and the declarations.** IDO lays a unit's uninitialised variables (`.bss`) out by the block number each
one's declaration gets, counted modulo 256, so moving a shared declaration into a header every file includes would
reorder the `.bss` of other units and change the ROM. The project holds the original order explicitly:
- a `BSS_ORDER_PAD` slot in each unit;
- `tools/bss_order.py`, which computes the slots from IDO's own front end, as zeldaret/oot's tools do;
- `tools/check_decls.py`, which keeps every declaration where it belongs.

[DECLARATIONS.md](DECLARATIONS.md) describes the mechanism and what a contributor runs after an edit. A modified
build is free of the constraint: only the original's byte-identical build depends on the order.

## Where the source and names come from

Eltale Monsters shares much of its code with Quest 64. Where a function is the same in both games, the matched C and
the names of the Quest 64 decompilations were the starting point, checked against Eltale's own code. Elsewhere, names
come from the SDK, from the game's own strings and from research into what each function does in the game. Every
name of a function, variable, type and member is a reconstruction. Names inherited that way are corrected when the code
shows them wrong; a name without such evidence stays. The credits (CREDITS.md) record where names and matches came
from other people's work.

## What the figures cover

The figures cover the CPU code at the start of the main image, ROM offsets `0x1000` to `0x4C580`, 308,608 bytes:

| Part | ROM offsets | Bytes | Form in this repository |
|---|---|---|---|
| Boot entry | `0x1000`-`0x1050` | 80 | hand-written assembly, kept by hand in `asm/entry.s` (its `.bss` clear takes its size from the linker) |
| 40 C translation units | `0x1050`-`0x37EB0` | 224,864 | C in `src/`: 530 functions (224,616 bytes) and 248 bytes of zero padding at the ends of 33 units |
| SDK library code | `0x37EB0`-`0x4C580` | 83,664 (27.1%) | 121 segments of libultra, its audio library and gu routines, disassembled from your ROM |

The 248 bytes are the zero words IDO's assembler puts after a unit's last function to bring its code to a multiple of
16 bytes; they are part of each compiled object, so they count in the units' size but in no function's size.

Most of the game's data (models, textures, maps, text, audio, tables) is extracted from your ROM, as typed source or
as binary pieces ([ASSETS.md](ASSETS.md)). At most about 68 KB of initialised data sits in C beside the code that uses
it: tables, strings, the sound-effect scripts, 22 display lists and the vertices of five quads, the intro's scene
tables and Brian's model tables. That figure counts the
units' data and read-only sections, compiler constants included, without the spans the C reads from your ROM.
`tools/verify.py` measures the code figures from a build.

Code elsewhere in the ROM is outside these figures. The RSP microcode that follows the CPU code (from `0x4C580`) is
copied from your ROM as binary data. Of the blocks the game loads at run time:
- the title screen overlay, the splash and intro overlay and the Controller Pak manager are C units
  (`src/title_overlay/`, `src/intro_overlay/`, `src/pak_menu_module/`);
- their functions not yet in C (one of the title's 8, all 32 of the intro's, two of the Controller Pak manager's 16)
  are disassembled from your ROM during the build;
- `tools/verify.py` counts them apart;
- the epilogue overlay (its segment keeps the historical name `credits_overlay`) is disassembled from your ROM during
  the build, with every reference by name, and not decompiled.

[ARCHITECTURE.md](ARCHITECTURE.md) lists what is not in C.

## What a byte match proves, and what it does not

It proves that the original compiler, with the original flags and the SDK headers, turns this C into the original
machine code, and that the ROM built from it, with the data from your copy, is identical to the original.

It does not prove that this is the original source. Byte matching fixes the code's behaviour and much of its shape
(control flow, the order of evaluation, often the types). It does not fix the names of anything, the formatting, the
comments, the file names, or which of several equivalent spellings the authors used. Where only one of several
plausible forms reproduced the compiler's choices of registers or stack slots, that form was taken because of its
compiled effect, not because it is known to be the original; the exceptions above are the cases where that choice
bends a rule.

## Known limits

- **What is still assembly.** The SDK library code (27.1% of the CPU code above) is assembly, regenerated from your
  ROM, and the boot entry is assembly kept by hand; neither is decompiled. Of the code loaded at run time, the epilogue
  overlay and the overlays' and the Controller Pak manager's functions not yet in C are assembly disassembled from your
  ROM. The RSP microcode is binary.
- **The toolchain.** The build needs x86-64 Linux (or WSL 2) and the exact toolchain pinned by `tools/setup.sh` and
  `requirements.txt`. Other compiler builds, header versions or splat versions may not reproduce the ROM.
- **Two functions the scorer keeps open.** The development tool's per-function scorer kept `FadeScreenIn` and
  `FadeScreenOut` in `src/game/main.c` open. Its reference object records a jump table's relocation against a named symbol,
  where the compiled object records it against the section. Their bytes in the ROM are identical, which is what the
  build's checksum and `tools/verify.py` test.
