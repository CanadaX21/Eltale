# Eltale Monsters decompilation

A matching decompilation of **Eltale Monsters** (Nintendo 64, Japan). Compiled with the game's original compiler, the
C source in this repository, together with the SDK code and game data that the build extracts from your own ROM,
rebuilds the original Japanese ROM byte for byte.

**Eltale Monsters has a 100% byte-matched reconstruction of all 530 tracked compiled C functions, producing a
byte-identical Japanese ROM. SDK library assembly remains outside the tracked C matching scope.**

**Contents:** [Status](#status) · [Quick start](#quick-start) · [Building](#building) ·
[Verify it yourself](#verify-it-yourself) · [Exploring the code](#exploring-the-code) ·
[Repository layout](#repository-layout) · [Documentation](#documentation) · [Credits](#credits) · [Legal](#legal)

It builds on Mallos31's decompilation of Eltale Monsters, [EM](https://github.com/Mallos31/EM) (the ROM split, the
build and the first matches), and on the Quest 64 decompilations ([Mallos31/Quest](https://github.com/Mallos31/Quest),
[Rainchus/Quest64-Decomp](https://github.com/Rainchus/Quest64-Decomp) and its fork by EltaleLibrarian), whose matched C
and names were the starting point for much of the code the two games share. [CREDITS.md](CREDITS.md) lists everyone
whose work is in it.

This is an AI-assisted project. The matching was done by Claude (Anthropic's AI model) through the decomp.wtf matching
service; I set the rules, made the calls on what counts as a match, and reviewed the work. The people credited did not
take part in it: the credit is for the work it builds on, not an endorsement.

This repository contains no ROM image and no game assets (graphics, models, maps, music or sound samples). The only
assembly code in it is the boot entry's 20 instructions (`asm/entry.s`), kept by hand because a modified build needs its
.bss clear to take its size from the linker; every other piece of assembly is disassembled from your ROM when you
build. The only game data in it are the tables, strings and sound-effect scripts that the reconstructed C holds as
initialisers beside the code that uses them (about 65 KB). You need your own copy of the game: the build extracts
everything else from it. This is not a PC port.

## Status

| | |
|---|---|
| Tracked compiled C functions byte-matched | **530 of 530** (100%) |
| Their code | 224,616 of 224,616 bytes |
| Built ROM | byte-identical to the original (SHA-1 `4161b5c100ec82b0241b20ca8f81366e23564ccb`) |
| Kept as assembly, outside the C figures | the hand-written boot entry (80 bytes) and the SDK library code (libultra, its audio library and the gu math routines): 83,664 bytes in 121 segments, **27.1%** of the 308,608 bytes of CPU code in the main image (ROM 0x1000-0x4C580) |
| Game code loaded at run time, counted apart | the title screen overlay: 7 of its 8 functions in C (3,704 of 12,552 bytes of function code); the splash and intro overlay: its data, tables and structure in C, its 32 functions still assembly; the credits overlay and the Controller Pak menu module: assembly |

"Tracked compiled C functions" means every function in the 40 C translation units of the main image (`src/game/`,
`src/sched/`, `src/libultra/`): the game's own code and six libultra modules that are written in C here as well. The
SDK library code kept as assembly is disassembled from your ROM during the build; it has not been decompiled. The 40 C
units take 224,864 bytes of the ROM: the 530 functions plus 248 bytes of zero padding that the compiler's assembler
adds at the ends of 33 of them. `tools/verify.py` recomputes every figure in this table from your own build. The
game's code that is loaded at run time is counted by none of the main figures: the title and intro overlays are C
units (`src/title_overlay/`, `src/intro_overlay/`) whose functions not yet in C are disassembled from your ROM during
the build, as are the credits overlay and the Controller Pak menu module; the RSP microcode is copied as binary data
([docs/ROM_MAP.md](docs/ROM_MAP.md#code-outside-the-c)).

A build whose code or data changes size boots and plays: every reference in the C, in the overlays and in the boss
model files follows a layout change, the linker places each file's sections, and it refuses a build that outgrows
main's RAM, the title or intro overlay's room, or a boss file's buffer. This has been tested as far as it was run (test builds in an emulator and on
Project64); paths not run, among them the scenes after a boss fight, are not validated.

A byte match proves that the original compiler turns this C into the original machine code. It does not prove that
this is the text the original authors wrote: names, formatting and the choice between equivalent forms are
reconstructions. [docs/METHODOLOGY.md](docs/METHODOLOGY.md) describes the matching standard, the documented exceptions
and the limits. [CHANGELOG.md](CHANGELOG.md) lists what changed since the first publication; the built ROM has not
changed.

## Quick start

On x86-64 Linux or WSL 2 (under WSL, in the Linux file system, not under `/mnt/c`), with the packages in
[step 1](#1-install-the-build-dependencies) installed:

```
git clone --recursive https://github.com/CanadaX21/Eltale.git
cd Eltale
cp /path/to/your/rom.z64 baserom.jp.z64    # the original Japanese ROM (SHA-1 in step 3)
tools/setup.sh
./configure
ninja                                       # prints: build/eltail.jp.z64: OK
python3 tools/verify.py
```

[Building](#building) explains each step.

## Building

The build runs on x86-64 Linux, natively or in WSL 2 on Windows 10 or 11 (the compiler is an x86-64 Linux program).
It has been tested on Debian 13 and Ubuntu 24.04, each from a clean installation with only the packages below.

### 1. Install the build dependencies

On Debian or Ubuntu:

```
sudo apt-get update
sudo apt-get install git curl ca-certificates python3 python3-venv python3-dev build-essential ninja-build binutils-mips-linux-gnu
```

`build-essential` and `python3-dev` are there because pip builds one pinned package (rabbitizer) from source on
Python 3.12 and later; `build-essential` also gives the C preprocessor (`cpp`) that the build uses to list each C
file's headers, so that `ninja` recompiles a file after a header it includes changes.

### 2. Clone the repository

```
git clone --recursive https://github.com/CanadaX21/Eltale.git
cd Eltale
```

Under WSL, clone into the Linux file system (your home directory, for example), not under `/mnt/c`.

### 3. Prepare your ROM

Copy your ROM of the game into the repository's root as `baserom.jp.z64`. It must be the original Japanese release,
in big-endian (`.z64`) byte order, with this SHA-1:

```
4161b5c100ec82b0241b20ca8f81366e23564ccb  baserom.jp.z64
```

`sha1sum baserom.jp.z64` shows yours. A dump in another byte order (`.v64`, `.n64`) has a different SHA-1: convert it
to `.z64` first.

### 4. Set up the tools

```
tools/setup.sh
```

This fetches the submodules if the clone was made without them, makes a Python environment in `.venv` with the pinned
packages in `requirements.txt` (pip checks each one's hash; to build three of them from source it fetches setuptools,
which is not pinned), downloads **IDO 5.3**, the original C compiler (Silicon Graphics), as the static recompilation
for Linux published by the decompals project ([ido-static-recomp](https://github.com/decompals/ido-static-recomp),
release `per-function`), into `tools/ido_5.3/usr/lib`, and checks the SDK headers in `include/2.0I`.

The C includes the **Nintendo 64 SDK headers** (libultra 2.0I). The 19 files the build reads, `ultra64.h` and 18 in
`PR/`, are in `include/2.0I` exactly as they come in the SDK, with their copyright notices: the build cannot work
without them. If they are ever missing from your copy, `tools/setup.sh --sdk-headers /path/to/2.0I` copies them from
the 2.0I include folder of an SDK copy you have (the folder that holds `ultra64.h` and `PR/`).

The compiler and the headers are checked file by file against `tools/checksums/` before the build can use them: the
lists name the exact files the byte-identical build was made with, and a copy whose files differ is refused.

### 5. Build

```
./configure
ninja
```

`./configure` checks your ROM's SHA-1, splits the ROM with splat (the disassembled SDK code and the game's data go to
`asm/` and `assets/`) and writes `build.ninja`. `ninja` compiles the C with IDO 5.3, assembles, links and compares
the result with the original. A matching build prints

```
build/eltail.jp.z64: OK
```

and keeps a copy of its outputs in `expected/`. If a later build does not match, `sha1sum` reports `FAILED`, and
`python3 first_diff.py` names the first symbols that differ from that last matching build.

After changing symbols or the splat configuration, run `ninja -t clean`, delete `assets/` and everything in `asm/`
except the hand-kept `asm/entry.s` (`git clean -fdX asm assets` removes exactly the files git ignores there; splat
does not remove the files of an earlier split, and `configure.py` builds whatever it finds there), then
`./configure && ninja`.

## Verify it yourself

```
python3 tools/verify.py
```

checks, from your build alone: that the ROM is byte-identical to yours; that no C file of the main image and no project
header contains assembly (no `GLOBAL_ASM` or `INCLUDE_ASM`, no `asm` statement); that the splat configuration keeps
assembly in the main image only for the boot entry and the SDK library code; that the linker filled the C units'
address range only from objects compiled from `src/`; that those objects define the 530 functions; and how many of
the overlays' functions are compiled from C. It prints the figures in the Status table.

To see that the ROM really is built from this C, change one constant in a function, for example a `* 40` to `* 41`
in `DrawSpellBanner` (`src/game/hud.c`), and run `ninja`: the checksum fails. Then `git checkout src/game/hud.c` and
`ninja` again: OK.

## Exploring the code

| To find | Start at |
|---|---|
| Where the game starts | the boot entry at ROM 0x1000 (assembly), `boot` and `idle` in `src/game/boot.c`, then `mainproc` in `src/game/main.c`: the start-up, then a loop that runs one game mode at a time (`gGameMode`): `FieldLoop` for play, `TitleLoop`, `IntroLoop`, `SaveModeLoop`, `CreditsLoop`, `EndingLoop`, `AfterCreditsLoop` |
| The game's types, structures and constants | `include/quest_types.h` |
| A function's declaration | its file's header for a function other files call: `include/game/player.h` declares those of `src/game/player.c` (the scheduler's in `include/sched/nnsched.h`, the overlays' in `include/game/title.h` and `intro.h`); a file includes the headers of the files whose functions it calls, and declares in itself, where needed, a function only it calls. A few calls in the original pass an argument list the function's definition does not take, or compile differently under its prototype, so a few files declare such a function themselves without a prototype, and `src/game/spells.c` calls three of them with no declaration in scope: each says why (`RULE-EXCEPTION R25`). [docs/METHODOLOGY.md](docs/METHODOLOGY.md) lists every kind of declaration kept in the calling file |
| A function or a variable by name or address | `symbol_addrs.txt` (names and addresses for splat and the linker); `undefined_syms.jp.txt` (data the C uses that the split does not name, defined by address) |
| A file by what it holds | [Source file names](#source-file-names), below |
| What is where in the ROM, and which function loads it | [docs/ROM_MAP.md](docs/ROM_MAP.md), with `eltail.jp.yaml` for every segment |
| What the game needs from the N64's system software (for a port) | [docs/PLATFORM.md](docs/PLATFORM.md) |
| Why some code is written the way it is | the comments at the code, and the documented exceptions in [docs/METHODOLOGY.md](docs/METHODOLOGY.md) |

## Repository layout

| Path | Contents |
|---|---|
| `src/game/`, `src/sched/`, `src/libultra/` | the main image's 40 C translation units: the game's code, the SDK sample scheduler, and the libultra files built as C; and two data files of the cutscene bank, the boss tables (file names below) |
| `src/title_overlay/`, `src/intro_overlay/` | the title screen overlay and the splash and intro overlay as C units: a function still in assembly is included from your ROM with a `GLOBAL_ASM` line |
| `include/` | the project's headers: types and structures (`quest_types.h`, `structs.h`, `scene2d.h`, `boss_files.h`), the functions each file defines (`include/game/`, `include/sched/`), macros (`common.h`); in `include/2.0I`, the 19 SDK headers the build reads, unmodified |
| `asm/entry.s` | the boot entry, kept by hand (the one assembly source file in the repository) |
| `eltail.jp.yaml` | the splat configuration: how the ROM is split into code, data and assets |
| `symbol_addrs.txt`, `undefined_syms.jp.txt` | symbol names and addresses for splat and the linker |
| `configure`, `configure.py` | check the ROM, run splat, write `build.ninja` |
| `tools/setup.sh`, `tools/checksums/` | fetch the toolchain parts that are not in this repository, and verify them and the SDK headers |
| `tools/verify.py` | the independent check above |
| `first_diff.py`, `tools/make_expected.py` | find the first difference from the last matching build |
| `tools/objdiff_gen.py`, `tools/trim_padding.py` | per-function comparison with [objdiff](https://github.com/encounter/objdiff) (`objdiff.json`, written by `./configure`) |
| `tools/splat`, `tools/asm-processor` | submodules: [splat](https://github.com/ethteck/splat) and [asm-processor](https://github.com/simonlindholm/asm-processor) at pinned commits |
| `docs/` | the methodology, the ROM map and the platform boundary ([Documentation](#documentation)) |

### Source file names

The original file names of the game's code are not known: the ROM keeps none. The names in `src/game/` describe
what each file holds and are this project's own. The scheduler and libultra files carry their known SDK names. Other
Quest 64 and Eltale decomps name files by their ROM offset (splat's default); this table maps the two. The boot
code's offset name is 1000, although its code starts at 0x1050, after the boot entry.

| File | ROM offset | | File | ROM offset |
|---|---|---|---|---|
| `game/boot.c` | 0x1000 | | `game/screen_fade.c` | 0x272D0 |
| `game/main.c` | 0x1870 | | `game/spline.c` | 0x27600 |
| `game/ending_text.c` | 0x3B40 | | `game/math.c` | 0x27A90 |
| `game/player.c` | 0x3F50 | | `game/dma.c` | 0x28D20 |
| `game/enemy.c` | 0x9D50 | | `game/audio.c` | 0x29150 |
| `game/boss.c` | 0xDAE0 | | `game/sound.c` | 0x2B0F0 |
| `game/map.c` | 0xE2A0 | | `game/start_menu.c` | 0x2B530 |
| `game/collision.c` | 0x113F0 | | `game/save_menu.c` | 0x30600 |
| `game/lighting.c` | 0x12660 | | `game/message.c` | 0x337B0 |
| `game/shadow.c` | 0x13DE0 | | `game/text.c` | 0x35EC0 |
| `game/map_objects.c` | 0x140D0 | | `game/controller.c` | 0x363B0 |
| `game/spirits.c` | 0x147A0 | | `game/overlay.c` | 0x36DB0 |
| `game/signs.c` | 0x14D30 | | `game/ending.c` | 0x36E60 |
| `game/camera.c` | 0x15150 | | `sched/nnsched.c` | 0x1130 |
| `game/spells.c` | 0x17360 | | `libultra/initialize.c` | 0x376C0 |
| `game/effects.c` | 0x1B200 | | `libultra/createthread.c` | 0x37980 |
| `game/dust.c` | 0x1E570 | | `libultra/startthread.c` | 0x37AD0 |
| `game/encounter.c` | 0x1FA00 | | `libultra/pimgr.c` | 0x37C20 |
| `game/model.c` | 0x20DA0 | | `libultra/createmesgqueue.c` | 0x37E10 |
| `game/hud.c` | 0x216E0 | | `libultra/visetevent.c` | 0x37E40 |
| `game/boss_tables.c` | 0xD850A0 (data) | | `title_overlay/title_screen.c` | 0xDAB5F0 |
| `game/boss_anims.c` | 0xD856F0 (data) | | `title_overlay/title_menu.c` | 0xDAC190 |
| `intro_overlay/intro.c` | 0xDC62B0 | | `title_overlay/title_fade.c` | 0xDAE420 |
| `intro_overlay/intro_screens.c` | 0xDC9350 | | | |

## Documentation

| File | What it covers |
|---|---|
| [docs/METHODOLOGY.md](docs/METHODOLOGY.md) | what counts as a match, the documented exceptions, what the figures cover, and what a byte match does not prove |
| [docs/ROM_MAP.md](docs/ROM_MAP.md) | the ROM block by block: what each block holds, the function that loads it and the RAM it goes to, the pointer tables, the C types that describe the data, and the open questions |
| [docs/PLATFORM.md](docs/PLATFORM.md) | where the game's code meets the N64's system software: the library services it uses and from where, what it assumes about the hardware as far as the evidence shows, notes for a port, and how the 329 library functions linked into the game were identified (the grades and their bases, and the evidence for each probable or unresolved one) |
| [CHANGELOG.md](CHANGELOG.md) | what changed since the first publication |
| [CREDITS.md](CREDITS.md) | the work this builds on, and where names and matches came from |

## Credits

[CREDITS.md](CREDITS.md): the decompilations this builds on, the names and matches that came from other people's work,
and the tools. Thanks also to the wider Quest 64 and Eltale community, whose years of shared research this builds on.

## Legal

This project is not affiliated with or endorsed by the rights holders of Eltale Monsters or by Nintendo. It contains
no ROM image and no game assets (see the top of this page for the data the source holds); building it requires your
own legally obtained copy of the game. IDO (Silicon
Graphics) is proprietary and is not part of this repository: `tools/setup.sh` downloads it. The 19 Nintendo 64 SDK
header files in `include/2.0I` are proprietary too (Nintendo's SDK, with Silicon Graphics' copyright notices, kept
in each file): they are included unmodified only because the build cannot work without them, and nothing here
grants any right to them.

No licence is granted for this repository: nothing in it is offered under open-source terms. The reconstructed code
derives from the game and from the community work credited in [CREDITS.md](CREDITS.md), whose rights are unaffected.
