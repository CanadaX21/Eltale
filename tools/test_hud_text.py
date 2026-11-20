"""test_hud_text.py [--list]: every HUD string of the game through the HUD font's encoding (tools/text_codec.py,
HudEncoding; text/hud_font.json). Run it from the repository's root after ./configure.

The strings are read where the build reads them, so the test covers exactly what the ROM holds:
  - the monsters' names in the six monster sets and the spells' names in the spell bank: the u8 arrays whose names
    end in Name or carry Name_ (gMonSet0Mon00Name, gSpellName0_00, ...) that ./configure writes from your ROM into
    assets/monsters_*/ and assets/spell_bank/;
  - the bosses' names: gBossNames' eight pointers, read from baserom.jp.z64 through the cutscene bank's place in
    eltail.jp.yaml (its ROM start and its RAM address) and gBossNames' address in symbol_addrs.txt;
  - the placeholder for a spell not yet learned: gUnlearnedSpellName, in main's binary piece that
    undefined_syms.jp.txt names.
Each string must decode with no cell the table leaves unnamed and no unused code, and encode back to the same bytes.
--list prints each string's symbol and text (the names in the game's own Japanese). Exit status 1 on any failure.
Python 3 standard library only."""
import glob
import os
import re
import struct
import sys

ROOT = os.getcwd()
sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__))))
from text_codec import HudEncoding, END, TextError      # noqa: E402

NAME_ARRAY = re.compile(r'\bu8 (g\w*Name(?:_\w+|\d*_\d+)?)\[\] = \{([^}]*)\};')


def fail(msg):
    sys.exit('test_hud_text: ' + msg)


def read(path):
    with open(os.path.join(ROOT, path), encoding='utf-8') as f:
        return f.read()


def strings():
    """[(symbol, bytes without the 0xFF)] for every HUD string"""
    out = []
    files = sorted(glob.glob(os.path.join(ROOT, 'assets', 'monsters_*', '*.records.inc.c')) +
                   glob.glob(os.path.join(ROOT, 'assets', 'spell_bank', '*.spellrecords.inc.c')))
    if not files:
        fail('no generated records in assets/: run ./configure first')
    for f in files:
        with open(f, encoding='utf-8') as fh:
            for m in NAME_ARRAY.finditer(fh.read()):
                b = bytes(int(x, 16) for x in re.findall(r'0x[0-9A-Fa-f]{2}', m.group(2)))
                if not b or b[-1] != END:
                    fail('%s in %s does not end with 0xFF' % (m.group(1), os.path.relpath(f, ROOT)))
                out.append((m.group(1), b[:-1]))
    rom = open(os.path.join(ROOT, 'baserom.jp.z64'), 'rb').read()
    y = read('eltail.jp.yaml')
    m = re.search(r'^\s*- name: cutscene_bank\b.*?^\s*start: (0x[0-9A-Fa-f]+)\s*$.*?^\s*vram: (0x[0-9A-Fa-f]+)', y, re.M | re.S)
    if not m:
        fail('eltail.jp.yaml: no cutscene_bank segment with a start and a vram')
    bank_rom, bank_ram = int(m.group(1), 16), int(m.group(2), 16)
    m = re.search(r'^gBossNames\s*=\s*(0x[0-9A-Fa-f]+);', read('symbol_addrs.txt'), re.M)
    if not m:
        fail('symbol_addrs.txt: no gBossNames')
    table = bank_rom + int(m.group(1), 16) - bank_ram
    for k in range(8):
        p = struct.unpack_from('>I', rom, table + 4 * k)[0]
        a = bank_rom + p - bank_ram
        out.append(('gBossNames[%d]' % k, rom[a:rom.index(END, a)]))
    m = re.search(r'^gUnlearnedSpellName\s*=\s*_([0-9A-Fa-f]+)_bin\s*\+\s*(0x[0-9A-Fa-f]+);', read('undefined_syms.jp.txt'), re.M)
    if not m:
        fail('undefined_syms.jp.txt: no gUnlearnedSpellName in a binary piece')
    a = int(m.group(1), 16) + int(m.group(2), 16)
    out.append(('gUnlearnedSpellName', rom[a:rom.index(END, a)]))
    return out


def main():
    enc = HudEncoding()
    bad, cells = [], set()
    items = strings()
    for sym, b in items:
        try:
            text, _ = enc.decode(b + bytes([END]))
        except (TextError, ValueError) as e:
            bad.append('%s: %s' % (sym, e))
            continue
        if '{' in text:
            bad.append('%s: %s (a cell the table leaves unnamed, or an unused code)' % (sym, text))
        if enc.encode(text) != b + bytes([END]):
            bad.append('%s: %r does not encode back to its bytes' % (sym, text))
        cells.update(x for x in b if x < 0x80)
        if '--list' in sys.argv[1:]:
            print('%-22s %s' % (sym, text))
    print('%d HUD strings: %d monster names, %d spell names, %d boss names, %d placeholder; %d glyph cells used; %s'
          % (len(items), sum(1 for s, _ in items if s.startswith('gMonSet')),
             sum(1 for s, _ in items if s.startswith('gSpellName')),
             sum(1 for s, _ in items if s.startswith('gBossNames')),
             sum(1 for s, _ in items if s == 'gUnlearnedSpellName'), len(cells),
             'every one decodes and encodes back' if not bad else '%d failed' % len(bad)))
    if bad:
        print('\n'.join('  ' + x for x in bad[:40]))
        return 1
    return 0


if __name__ == '__main__':
    sys.exit(main())
