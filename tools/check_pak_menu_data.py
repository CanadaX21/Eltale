"""check_pak_menu_data.py: every symbol the Controller Pak manager's data and remaining assembly name is
the module's own, or one of the outside names it is known to use.

splat turns a data word into a symbol reference whenever a symbol it knows covers the word's value, whatever the word
holds, and pairs a `lui` with a later immediate into %hi/%lo of a symbol the same way. In the manager (encoded
messages, images, sprite lists, two framebuffer addresses, a clear loop's stride) a value that is not a pointer
could so take another segment's name: the original build still matches (the same address), and a modified build that
moves that symbol rewrites the module's text, pixels or code without a sign.

./configure runs this after the split. Checked:
- each `.word NAME` (with or without an addend) in asm/data/pak_menu_module/;
- each %hi/%lo operand, `jal` target and `.word` in the assembly the unit still includes (its GLOBAL_ASM lines; files
  left in asm/nonmatchings by functions now in C are not built and not read).
A name passes when it is the module's own (one symbol_addrs.txt scopes to `segment:pak_menu_module`, or splat's name
for an address inside the module: its ROM suffix within 0xDA4570-0xDAB870, the .bss named from the ROM end on,
a `.L` label, its jump table), a libultra
call (`jal os...` / `__os...`), or in OUTSIDE below. A word that is not a pointer stays a number when its symbol is
typed in symbol_addrs.txt: bytes for text and images (`type:u8 size:N`), halves for a pair of fixed addresses
(gPakMenuFrameBuffers, `type:u16`). Read-only; exits 1 on any other name."""
import os
import re
import sys

ROM_START, ROM_END = 0xDA4570, 0xDAB5F0 + 0x280     # splat names the .bss from the module's ROM end on: its 0x280 bytes
DATA = 'asm/data/pak_menu_module'
UNIT = 'src/pak_menu_module/pak_menu.c'
OUTSIDE = {
    'gControllerChannel': 'main: the game\'s controller (the manager runs before the scheduler, on main\'s setup)',
    'gContStatus': 'main: the controllers\' status',
    'gControllerBits': 'main: the controllers present',
    'gFontSheet': 'the game\'s font sheet, a fixed RAM buffer main loads (PakMenu_DrawText draws from it)',
    'D_80200010': 'not a variable: the frame-buffer clear loop\'s `lui 0x8020` and its 0x10 stride, which the '
                  'disassembler pairs; it must stay a fixed address (undefined_syms_auto.txt; since Brian\'s animation '
                  'files are linked at 0x80200000, declared and fixed in undefined_syms.jp.txt)',
}

own = set()
for line in open('symbol_addrs.txt', encoding='utf-8'):
    m = re.match(r'\s*(\w+)\s*=\s*0x[0-9A-Fa-f]+\s*;(.*)$', line)
    if m and re.search(r'\bsegment:pak_menu_module\b', m.group(2)):
        own.add(m.group(1))


def is_own(name):
    if name in own or name.startswith('.L') or name.startswith('jtbl_'):
        return True
    suffix = re.search(r'_([0-9A-F]{6})$', name)
    return bool(suffix) and ROM_START <= int(suffix.group(1), 16) < ROM_END


bad = []
count = 0
for f in sorted(os.listdir(DATA)):
    path = os.path.join(DATA, f)
    for n, line in enumerate(open(path, encoding='utf-8'), 1):
        m = re.search(r'\.word\s+([A-Za-z_.]\w*)', line)
        if not m:
            continue
        count += 1
        if not is_own(m.group(1)):
            bad.append('  %s:%d: %s' % (path, n, line.strip()))
asm = re.findall(r'#pragma GLOBAL_ASM\("([^"]+)"\)', open(UNIT, encoding='utf-8').read())
for path in asm:
    for n, line in enumerate(open(path, encoding='utf-8'), 1):
        code = line.split('*/', 1)[-1]
        for kind, name in re.findall(r'(%hi\(|%lo\(|jal\s+|\.word\s+)([A-Za-z_.]\w*)', code):
            count += 1
            if is_own(name) or name in OUTSIDE or (kind.startswith('jal') and re.match(r'(__)?os[A-Z]', name)):
                continue
            bad.append('  %s:%d: %s' % (path, n, line.strip()))
auto = open('undefined_syms_auto.txt', encoding='utf-8').read() + open('undefined_syms.jp.txt', encoding='utf-8').read()
if re.search(r'\bD_80200010\b', ''.join(open(p, encoding='utf-8').read() for p in asm)) and \
        not re.search(r'^\s*D_80200010\s*=\s*0x80200010\s*;', auto, re.M):
    bad.append('  D_80200010 is no longer a fixed address in undefined_syms_auto.txt or undefined_syms.jp.txt: the clear '
               'loop\'s stride would follow whatever now covers 0x80200010')
if bad:
    print("the Controller Pak manager names %d symbol(s) outside the module and its known outside names (a value "
          "that is not a pointer: type its symbol in symbol_addrs.txt):" % len(bad))
    print('\n'.join(bad))
    sys.exit(1)
print("check_pak_menu_data: the Controller Pak manager's data and assembly name only their own symbols and %d known "
      "outside names (%d references, %d assembly files)" % (len(OUTSIDE), count, len(asm)))
