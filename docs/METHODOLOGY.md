# Methodology, standards and limits

How the reconstruction was made, what counts as a match, what the figures cover, and what a byte match does not
prove.

## What counts as a match

A function counts as matched when both hold:

1. **The bytes.** The original compiler, IDO 5.3, with the game's flags (`-O2 -g3 -mips2 -o32`, and the variants
   that six libultra modules need, in `configure.py`), turns the function's C into exactly the original machine
   code, compiled in its own translation unit and linked at its original address.
2. **The source rules.** The C is ordinary source that someone could have written: no fake code (dummy or `volatile`
   locals, empty blocks, `do { } while (0)` bodies, self-assignments or `+ 0` / `* 1` written only to steer the
   compiler), no reads or writes through offset casts where a struct member belongs, one declaration for each type
   and global, and no assembly inside C.

Per-function comparisons were the working filter. The deciding check was the whole ROM: after every change the ROM was
rebuilt from clean and compared with the original, and a change that kept a function's score but altered any byte of
the ROM was reverted.

## Documented exceptions

Where the compiled code could only be reproduced by a form that bends a source rule, the exception was decided for
that function alone, with the evidence recorded, and it is marked in the source where it occurs. Search for these
markers to read them all:

| Marker in `src/` | What it marks |
|---|---|
| `MATCHING_HACK_IDO` | four unused locals (`pad`) that fill a stack slot the original's frame has (`AfterCreditsLoop`, `PollModelInstance`, `ScanControllerPak`, `UpdateStatusVisuals`). They are compiled only when the macro is defined, as it is by default in `include/common.h`; building with `-DNO_MATCHING_HACKS` leaves them out, and the ROM then differs, as expected. |
| `RULE-EXCEPTION` | mostly one local that holds two values in turn where the original's frame shows a single variable (rule R30); a few calls through unprototyped declarations whose callers pass other arguments than the definition takes (R25: two comments cite it as R5); two casts for signedness (R19); and one quirk of the original kept as shipped (R36). The rule numbers are those of the decomp-code-rules catalogue in [Game-Decompilation-Skill-For-Claude](https://github.com/lemiur/Game-Decompilation-Skill-For-Claude). |
| `always passes`, `always true here`, `the original leaves this branch empty`, `fieldMode`, `nothing reads`, `never read`, `guess:` | eight statements whose form the compiled code requires: two tests that always pass, a test the original repeats, an empty `else`, a constant held in a named local, a clamp whose result is not read (as the Quest 64 version of the function has it), an initial value that is never read, and a dead assignment without which the compiler picks another register (its comment says it is a guess). |
| `kept for the declaration count` | two unused type declarations. IDO lays out a unit's uninitialised variables according to how many declarations precede them, so removing these would move data. |

## Where the source and names come from

Eltale Monsters shares much of its code with Quest 64. Where a function is the same in both games, the matched C and
the names of the Quest 64 decompilations were the starting point, checked against Eltale's own code. Elsewhere,
names come from the SDK, from the game's own strings and from research into what each function does in the game.
Every name of a function, variable, type and member is a reconstruction. [CREDITS.md](../CREDITS.md) records where
names and matches came from other people's work.

## What the figures cover

The ROM's executable code is the main code segment, ROM offsets `0x1000` to `0x4C580`, 308,608 bytes:

| Part | ROM offsets | Bytes | Form in this repository |
|---|---|---|---|
| Boot entry | `0x1000`-`0x1050` | 80 | hand-written assembly, disassembled from your ROM |
| 40 C translation units | `0x1050`-`0x37EB0` | 224,864 | C in `src/`: 530 functions (224,616 bytes) and 248 bytes of zero padding at the ends of 33 units |
| SDK library code | `0x37EB0`-`0x4C580` | 83,664 (27.1%) | 121 segments of libultra, its audio library and gu routines, disassembled from your ROM |

The 248 bytes are the zero words IDO's assembler puts after a unit's last function to bring its code to a multiple
of 16 bytes; they are part of each compiled object, so they count in the units' size but in no function's size. The
game's data (about 16.4 MB: models, textures, maps, audio, tables) is extracted from your ROM as binary files, except
about 30 KB of initialised data (tables, strings and sound-effect scripts) that sits in C beside the code that uses
it. `tools/verify.py` measures all of these from a build.

## What a byte match proves, and what it does not

It proves that the original compiler, with the original flags and the SDK headers, turns this C into the original
machine code, and that the ROM built from it, with the data from your copy, is identical to the original.

It does not prove that this is the original source. Byte matching fixes the code's behaviour and much of its shape
(control flow, the order of evaluation, often the types), but not the names of anything, the formatting, the comments,
the file names, or which of several equivalent spellings the authors used. Where only one of several plausible forms
reproduced the compiler's choices of registers or stack slots, that form was taken because of its compiled effect,
not because it is known to be the original; the exceptions above are the cases where that choice bends a rule.

## Known limits

- The SDK library code (27.1% of the executable code) and the boot entry are assembly, regenerated from your ROM; they
  are not decompiled.
- The build needs x86-64 Linux (or WSL 2) and the exact toolchain pinned by `tools/setup.sh` and `requirements.txt`.
  Other compiler builds, header versions or splat versions may not reproduce the ROM.
- The development tool's per-function scorer kept two functions open, `FadeScreenIn` and `FadeScreenOut` in
  `src/game/main.c`, because its reference object records a jump table's relocation against a named symbol where the
  compiled object records it against the section. Their bytes in the ROM are identical, which is what the build's
  checksum and `tools/verify.py` test.
