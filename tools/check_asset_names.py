"""check_asset_names.py: every name a typed asset file uses for an address is that file's own, or one
whose address no change of the file's layout can move.

splat names the addresses in a display list (a vertex load, a texture load, a call) by looking them up among the
symbols it knows (its gfx type, dl_handler and vtx_handler, before it makes a name of its own). A symbol scoped to
another top-level segment is skipped when the two segments share an exclusive_ram_id (the boss files, the monster
sets, the maps each share one), but not when their ids differ or one has none; and a symbol scoped to no segment at
all (a name of main's, such as the boss buffer's or the monster buffer's) is taken at its address whatever its type.
So a typed file could be given the name of another segment's record, or of a buffer, at an address inside its own
RAM: the original build would still match (the same address), and a modified build would point at a record that did
not move with the file's.

./configure runs this after the split and the kept changes (tools/asset_patches.py). In every typed segment's
generated C (assets/<segment>/*.inc.c, comments and strings left out), each identifier must be one of:
  - a name undefined_syms.jp.txt (the linker script's list) defines from this segment's own objects; from a block the
    link locks in place (both its RAM address and its size asserted, as interiors_common's and region_0's); from a
    symbol of no segment; or as a fixed address outside this segment's RAM. A name it defines from another segment's
    objects (a texture of another boss file) belongs to that segment's layout, and a fixed address inside this
    segment's RAM is no record of it (the NPC areas' names lie in the monster sets' RAM): both are refused, but for
    a reference the definitions make on purpose: a record of another typed segment that symbol_addrs.txt scopes to
    this segment as well (`segment:<this> defined:True`: an NPC area's lists take the held objects' textures and
    lists), which follows that segment's record, as it should;
  - otherwise, a name symbol_addrs.txt scopes to this segment (`segment:<name>` in its comment; a name may be scoped
    to several);
  - otherwise, not a symbol, or a symbol of no segment whose address is outside this segment's RAM (its vram and
    length from eltail.jp.yaml).
Anything else stops ./configure, naming the file, the name and what it is.

The binary maps' data windows (asm/data/map_*.data.s: a binary map's words that point into main's .bss, disassembled so
they follow main's layout) are checked as well. The disassembler names every pointer word in a window, and maps share
RAM: with a map built from typed sources at the same address it gave a binary map's own vertex load a typed map's
record name (map 11's window and map 12). In a window, each name a word uses must be a label of that window
or a name symbol_addrs.txt gives no segment at an address outside the map's RAM (main's .bss buffers).

    python3 tools/check_asset_names.py    (from the project's root; Python 3 standard library only)
"""
import collections
import glob
import os
import re
import sys

ASSETS = 'assets'
SYMBOLS = 'symbol_addrs.txt'
LINKER_NAMES = 'undefined_syms.jp.txt'
CONFIG = 'eltail.jp.yaml'
ASSET_C = ('.gfx.inc.c', '.vtx.inc.c', '.anim.inc.c', '.records.inc.c', '.maprecords.inc.c', '.spellrecords.inc.c', '.npcrecords.inc.c',
           '.gfxrecords.inc.c')
_SYM = re.compile(r'\s*([A-Za-z_]\w*)\s*=\s*(0x[0-9A-Fa-f]+)\s*;(.*)')
_SEGMENT = re.compile(r'\bsegment:(\w+)')
_DEF = re.compile(r'\s*([A-Za-z_]\w*)\s*=\s*([^;]*);')
_VRAM_LOCK = re.compile(r'\s*ASSERT\(\s*(\w+)_VRAM\s*==')
_SIZE_LOCK = re.compile(r'\s*ASSERT\(\s*(\w+)_ROM_END\s*-\s*(\w+)_ROM_START\s*==')
_IDENT = re.compile(r'\b[A-Za-z_]\w*\b')
_COMMENTS = re.compile(r'/\*.*?\*/|//[^\n]*', re.S)
_STRINGS = re.compile(r'"(?:\\.|[^"\\\n])*"|\'(?:\\.|[^\'\\\n])*\'')
_TOP = re.compile(r'^  - name:\s*(\w+)')
_TOP_END = re.compile(r'^  - \[(0x[0-9A-Fa-f]+)')
_FIELD = re.compile(r'^    (start|vram):\s*(0x[0-9A-Fa-f]+)')


def symbols(path=SYMBOLS):
    """({name: set of segments} for scoped names, {name: address} for names scoped to no segment)"""
    scoped, unscoped = collections.defaultdict(set), {}
    with open(path, encoding='utf-8') as f:
        for line in f:
            m = _SYM.match(line)
            if not m:
                continue
            segs = _SEGMENT.findall(m.group(3))
            if segs:
                scoped[m.group(1)].update(segs)
            else:
                unscoped[m.group(1)] = int(m.group(2), 16)
    return scoped, unscoped


def linker_list(path=LINKER_NAMES):
    """({name: the expression that defines it}, the segments locked in place: RAM address and size both asserted)"""
    defs, vram, size = {}, set(), set()
    if os.path.isfile(path):
        with open(path, encoding='utf-8') as f:
            for line in f:
                m = _VRAM_LOCK.match(line)
                if m:
                    vram.add(m.group(1))
                    continue
                m = _SIZE_LOCK.match(line)
                if m and m.group(1) == m.group(2):
                    size.add(m.group(1))
                    continue
                m = _DEF.match(line)
                if m:
                    defs[m.group(1)] = m.group(2).strip()
    return defs, vram & size


def segments_of(path=CONFIG):
    """(the top-level segments' names, {name: (vram, vram + its length in ROM)} for those with a vram)"""
    tops, cur = [], None
    with open(path, encoding='utf-8') as f:
        for line in f:
            m = _TOP.match(line)
            if m:
                cur = {'name': m.group(1)}
                tops.append(cur)
                continue
            m = _TOP_END.match(line)
            if m:
                tops.append({'name': None, 'start': int(m.group(1), 16)})
                cur = None
                continue
            m = _FIELD.match(line)
            if m and cur is not None and m.group(1) not in cur:
                cur[m.group(1)] = int(m.group(2), 16)
    ranges = {}
    for a, b in zip(tops, tops[1:]):
        if a.get('name') and 'vram' in a and 'start' in a and 'start' in b:
            ranges[a['name']] = (a['vram'], a['vram'] + b['start'] - a['start'])
    return [t['name'] for t in tops if t.get('name')], ranges


def owner(expr, names):
    """the top-level segment whose objects a linker-list expression starts from (the longest name that prefixes its
    first identifier), or None (a number, or a symbol of no segment)"""
    m = re.match(r'\s*\(?\s*([A-Za-z_]\w*)', expr)
    if not m:
        return None
    first = m.group(1)
    best = None
    for n in names:
        if (first == n or first.startswith(n + '_')) and (best is None or len(n) > len(best)):
            best = n
    return best


def check(root='.'):
    """(error lines, segments checked, files checked)"""
    scoped, unscoped = symbols(os.path.join(root, SYMBOLS))
    defs, locked = linker_list(os.path.join(root, LINKER_NAMES))
    names, ranges = segments_of(os.path.join(root, CONFIG))
    typed = sorted(set(s for segs in scoped.values() for s in segs))
    errors, nseg, nfiles = [], 0, 0
    for seg in typed:
        d = os.path.join(root, ASSETS, seg)
        if not os.path.isdir(d):
            continue
        files = [n for n in sorted(os.listdir(d)) if n.endswith(ASSET_C)]
        nseg += 1 if files else 0
        lo, hi = ranges.get(seg, (None, None))
        for name in files:
            nfiles += 1
            path = os.path.normpath(os.path.join(d, name))
            with open(path, encoding='utf-8') as f:
                text = _STRINGS.sub('""', _COMMENTS.sub(' ', f.read()))
            bad = collections.OrderedDict()
            for ident in _IDENT.findall(text):
                if ident in bad:
                    continue
                if ident in defs:
                    t = owner(defs[ident], names)
                    num = re.fullmatch(r'\s*(0x[0-9A-Fa-f]+|\d+)\s*', defs[ident])
                    on_purpose = seg in scoped.get(ident, ()) and t in scoped.get(ident, ())
                    if t is not None and t != seg and t not in locked and not on_purpose:
                        bad[ident] = ('a name the linker list defines from %s\'s objects (%s), which move with %s, not '
                                      'with %s' % (t, defs[ident], t, seg))
                    elif num and lo is not None and lo <= int(num.group(1), 0) < hi:
                        bad[ident] = ('a fixed address the linker list gives (%s), inside %s\'s RAM (0x%08X-0x%08X), not '
                                      'one of its records' % (defs[ident].strip(), seg, lo, hi))
                elif ident in scoped:
                    if seg not in scoped[ident]:
                        bad[ident] = 'a record of %s, not of %s' % (', '.join(sorted(scoped[ident])), seg)
                elif ident in unscoped and lo is not None and lo <= unscoped[ident] < hi:
                    bad[ident] = ('a name of no segment at 0x%08X, inside %s\'s RAM (0x%08X-0x%08X), not one of its '
                                  'records' % (unscoped[ident], seg, lo, hi))
            for ident, what in bad.items():
                errors.append('%s names %s, %s (splat names a display list\'s addresses from the symbols it knows: in a '
                              'modified build the name would not follow this file\'s records)' % (path, ident, what))
    return errors, nseg, nfiles


def check_windows(root='.'):
    """(error lines, windows checked): the binary maps' data windows name only their own labels and names of no segment
    outside the map's RAM"""
    _, unscoped = symbols(os.path.join(root, SYMBOLS))
    _, ranges = segments_of(os.path.join(root, CONFIG))
    errors, n = [], 0
    for path in sorted(glob.glob(os.path.join(root, 'asm', 'data', 'map_*.data.s'))):
        seg = re.match(r'(map_\d\d)_', os.path.basename(path)).group(1)
        lo, hi = ranges.get(seg, (None, None))
        with open(path, encoding='utf-8') as f:
            text = f.read()
        labels = set(re.findall(r'(?m)^\s*(?:glabel|dlabel)\s+(\w+)', text))
        n += 1
        for ident in sorted(set(re.findall(r'\.word\s+([A-Za-z_]\w*)', text))):
            if ident in labels:
                continue
            if ident in unscoped and lo is not None and not (lo <= unscoped[ident] < hi):
                continue
            errors.append('%s names %s, which is neither a label of this window nor a name of no segment outside %s\'s '
                          'RAM (the disassembler names a window\'s pointer words from the symbols it knows: in a modified '
                          'build the word could follow another map\'s records)' % (os.path.normpath(path), ident, seg))
    return errors, n


def main():
    errors, nseg, nfiles = check()
    werrors, nwin = check_windows()
    for e in errors + werrors:
        print('Error: check_asset_names: %s' % e, file=sys.stderr)
    if errors or werrors:
        return 1
    print('check_asset_names: %d typed segments, %d generated files: every name is the file\'s own segment\'s, or fixed '
          'by the link; %d data windows of binary maps: every name a label of its window or a buffer outside the map'
          % (nseg, nfiles, nwin))
    return 0


if __name__ == '__main__':
    sys.exit(main())
