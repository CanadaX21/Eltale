"""eltale_save.py COMMAND PAK [options]: inspect and edit Eltale Monsters saves in a Controller Pak image.

The save format comes from tools/save_format.json (written from the decomp's source by tools/save_format.py), so this
tool knows the save only as the source defines it. The image is libultra's Controller Pak file system: page 0 the ID
area (a 32-byte block at 0x20, 0x60, 0x80 and 0xC0, each with two checksums over its first 28 bytes), page 1 the inode
table (a page number per page, the chain of each note's pages; byte 1 the sum of bytes 10-255) and page 2 its copy,
pages 3-4 the note table (16 entries of 32 bytes: game code, company code, first page, status, the data sum, the
extension, the name), pages 5-127 the notes' data.
Images: a raw 32 KiB pak (ares's <rom>.pak, Project64's .mpk) or Mupen64Plus's 4 x 32 KiB .mpk (--pak 0-3 picks one).

    check PAK                         the file system: ID blocks, inode table and copy, note chains; exit 1 on damage
    list PAK                          every note; for each Eltale save a summary
    show PAK [--ext E] [--json]       one save, every field (default: the only save, or name it by --ext)
    export PAK OUT.json [--ext E]     that save as JSON: every field, and the note's 512 bytes in hex
    import PAK IN.json [--ext E] [-o OUT]   write the JSON's field values into that save (the rest kept)
    set PAK PATH=VALUE ... [--ext E] [-o OUT]   edit fields: record.stats.maxHP=200, data.inventory[3]=7,
                                      record.playTime=600 (paths as `show` prints them)
    scrub PAK [--ext E] [-o OUT]      zero the bytes the game never reads back (each part's 0x18-byte tail, the note's
                                      last 0xA0 bytes): other saves' data the game wrote along, made deterministic

Writes go to OUT (default: the image itself, after a copy to IMAGE.bak); after a write the tool checks the file
system again and that no byte outside the save's note changed. The game keeps no checksum of its own, so an edited
save loads as written: keep values the game can use (item ids 0-25, real map numbers). Python 3 standard library."""
import json
import os
import re
import shutil
import struct
import sys

PAGE, PAK = 0x100, 0x8000
FMT = {('u', 1): '>B', ('s', 1): '>b', ('u', 2): '>H', ('s', 2): '>h', ('u', 4): '>I', ('s', 4): '>i', ('f', 4): '>f'}
PAK_CHARS = {0x00: '', 0x0F: ' '}
PAK_CHARS.update({0x10 + i: c for i, c in enumerate('0123456789')})
PAK_CHARS.update({0x1A + i: c for i, c in enumerate('ABCDEFGHIJKLMNOPQRSTUVWXYZ')})
PAK_CHARS.update({0x34 + i: c for i, c in enumerate('!"#\'*+,-./:=?@')})
PAK_CHARS.update({0x42 + i: c for i, c in enumerate(
    '。゛゜ァィゥェォッャュョヲンアイウエオカキクケコサシスセソタチツテトナニヌネノハヒフヘホマミムメモヤユヨラリルレロワ'
    'ガギグゲゴザジズゼゾダヂヅデドバビブベボパピプペポ')})


def pak_text(b):
    return ''.join(PAK_CHARS.get(c, '\\x%02x' % c) for c in b)


def die(text):
    sys.exit('eltale_save: ' + text)


def load_format(path=None):
    p = path or os.path.join(os.path.dirname(os.path.abspath(__file__)), 'save_format.json')
    return json.load(open(p, encoding='utf-8'))


class Pak(object):
    """one 32 KiB Controller Pak inside an image file (several in a Mupen64Plus .mpk)"""

    def __init__(self, path, index=0):
        self.path, self.index = path, index
        self.image = bytearray(open(path, 'rb').read())
        if self.image[:11] == b'123-456-STD':
            die('%s is a DexDrive image: not supported (export the note with a pak manager first)' % path)
        if len(self.image) % PAK or not self.image:
            die('%s is %d bytes: not a Controller Pak image (32 KiB, or 4 x 32 KiB for Mupen64Plus)' % (path, len(self.image)))
        if not 0 <= index < len(self.image) // PAK:
            die('%s holds %d pak(s): no pak %d' % (path, len(self.image) // PAK, index))
        self.base = index * PAK

    @property
    def d(self):
        return self.image[self.base:self.base + PAK]

    def page(self, n):
        return self.d[n * PAGE:(n + 1) * PAGE]

    def problems(self):
        """the damage a game would see (and self.warnings: what libultra repairs on its own)"""
        out, d = [], self.d
        self.warnings = []
        good, bad = 0, []
        for at in (0x20, 0x60, 0x80, 0xC0):
            blk = d[at:at + 32]
            words = struct.unpack('>14H', blk[:28])
            s, c, ic = sum(words) & 0xFFFF, struct.unpack('>H', blk[28:30])[0], struct.unpack('>H', blk[30:32])[0]
            if c == s and ic == (0xFFF2 - s) & 0xFFFF:
                good += 1
            else:
                bad.append('ID block at 0x%02X: checksums %04X %04X, the bytes give %04X %04X' % (at, c, ic, s, (0xFFF2 - s) & 0xFFFF))
        if not good:
            out += bad + ['no valid ID block: the game reports a damaged pak (osPfsInitPak: PFS_ERR_ID_FATAL)']
        else:
            self.warnings += ['%s (%d of 4 copies valid: libultra repairs it from a valid one)' % (b, good) for b in bad]
        ino = self.page(1)
        if sum(ino[10:256]) & 0xFF != ino[1]:
            out.append('inode table: byte 1 is 0x%02X, the sum of bytes 10-255 is 0x%02X' % (ino[1], sum(ino[10:256]) & 0xFF))
        if ino != self.page(2):
            out.append('inode table: page 2 (its copy) differs from page 1')
        seen = {}
        for n in self.notes():
            if n['pages'] is None:
                out.append('note %d: a broken page chain (%s)' % (n['file'], n['chain_error']))
                continue
            for p in n['pages']:
                if p in seen:
                    out.append('page %d is in note %d and note %d' % (p, seen[p], n['file']))
                seen[p] = n['file']
        return out

    def notes(self):
        out, d, ino = [], self.d, self.page(1)
        for i in range(16):
            e = d[3 * PAGE + 32 * i:3 * PAGE + 32 * (i + 1)]
            game, company, start, status, _res, datasum = struct.unpack('>IHHBBH', e[:12])
            if game == 0 and company == 0:
                continue
            pages, p, err = [], start, None
            while True:
                if (p >> 8) != 0:
                    err = 'bank %d: only bank 0 is supported' % (p >> 8)
                    break
                if not 5 <= (p & 0xFF) < 128:
                    err = 'page %d is outside the data pages' % p
                    break
                if len(pages) > 123 or (p & 0xFF) in pages:
                    err = 'the chain loops'
                    break
                pages.append(p & 0xFF)
                nxt = struct.unpack('>H', ino[2 * (p & 0xFF):2 * (p & 0xFF) + 2])[0]
                if nxt == 1:
                    break
                p = nxt
            out.append({'file': i, 'game_code': game, 'company_code': company, 'status': status, 'data_sum': datasum,
                        'ext': e[12:16], 'name': e[16:32], 'pages': None if err else pages, 'chain_error': err})
        return out

    def note_bytes(self, n):
        return b''.join(self.page(p) for p in n['pages'])

    def write_note(self, n, data):
        assert len(data) == len(n['pages']) * PAGE
        for k, p in enumerate(n['pages']):
            at = self.base + p * PAGE
            self.image[at:at + PAGE] = data[k * PAGE:(k + 1) * PAGE]


def eltale_notes(pak, fmt):
    nt = fmt['note']
    return [n for n in pak.notes() if n['game_code'] == nt['game_code'] and n['company_code'] == nt['company_code']
            and n['name'].hex() == nt['name_bytes']]


def pick(pak, fmt, ext):
    saves = eltale_notes(pak, fmt)
    if ext is not None:
        saves = [n for n in saves if n['ext'][0] == ext]
    if len(saves) != 1:
        die('%d Eltale saves match%s: name one with --ext (list shows them)' % (len(saves), '' if ext is None else ' --ext 0x%02X' % ext))
    if saves[0]['pages'] is None:
        die('that save\'s page chain is broken: ' + saves[0]['chain_error'])
    if len(saves[0]['pages']) * PAGE < fmt['note']['bytes']:
        die('that save is %d bytes, the format needs %d' % (len(saves[0]['pages']) * PAGE, fmt['note']['bytes']))
    return saves[0]


def leaf_values(fmt, note):
    out = {}
    for lf in fmt['leaves']:
        f = FMT[(lf['kind'], lf['size'])]
        vals = [struct.unpack(f, note[lf['offset'] + k * lf['size']:lf['offset'] + (k + 1) * lf['size']])[0]
                for k in range(lf['count'])]
        out[lf['path']] = vals if lf['count'] > 1 else vals[0]
    return out


def describe(fmt, note):
    v = leaf_values(fmt, note)
    rec = fmt['parts'][0]
    dat = fmt['parts'][1]
    return {'values': v,
            'record_tail': note[rec['offset'] + rec['type_bytes']:rec['offset'] + rec['bytes']].hex(),
            'data_tail': note[dat['offset'] + dat['type_bytes']:dat['offset'] + dat['bytes']].hex(),
            'unused': note[fmt['parts'][2]['offset']:fmt['parts'][2]['offset'] + fmt['parts'][2]['bytes']].hex()}


def set_leaf(fmt, note, path, value):
    m = re.fullmatch(r'([\w.]+)(?:\[(\d+)\])?', path)
    if not m:
        die('not a field path: ' + path)
    lf = next((x for x in fmt['leaves'] if x['path'] == m.group(1)), None)
    if lf is None:
        die('no field %s (show lists the paths)' % m.group(1))
    k = int(m.group(2)) if m.group(2) else 0
    if not 0 <= k < lf['count'] or (m.group(2) is None and lf['count'] > 1 and not isinstance(value, list)):
        die('%s has %d elements: give an index' % (lf['path'], lf['count']))
    vals = value if isinstance(value, list) else [value]
    if m.group(2) is None and isinstance(value, list) and len(vals) != lf['count']:
        die('%s takes %d values' % (lf['path'], lf['count']))
    for j, v in enumerate(vals):
        at = lf['offset'] + (k + j) * lf['size']
        f = FMT[(lf['kind'], lf['size'])]
        try:
            note[at:at + lf['size']] = struct.pack(f, float(v) if lf['kind'] == 'f' else int(v, 0) if isinstance(v, str) else int(v))
        except struct.error as e:
            die('%s = %s: %s' % (path, v, e))


def save_image(pak, before, note_pages, out):
    """write the image (to out, or in place after a .bak), then check the file system and that only the note changed"""
    after = pak.d
    changed = [i for i in range(PAK) if before[i] != after[i]]
    allowed = set()
    for p in note_pages:
        allowed.update(range(p * PAGE, (p + 1) * PAGE))
    if any(i not in allowed for i in changed):
        die('internal: a byte outside the save changed: nothing written')
    probs = pak.problems()
    if probs:
        die('the file system would not check: nothing written:\n  ' + '\n  '.join(probs))
    target = out or pak.path
    if not out:
        shutil.copyfile(pak.path, pak.path + '.bak')
    open(target, 'wb').write(bytes(pak.image))
    again = Pak(target, pak.index)
    if again.d != after or again.problems():
        die('the written image does not read back as written')
    print('written %s: %d bytes of the save changed%s' % (target, len(changed), '' if out else ' (the old image: %s.bak)' % pak.path))


def main():
    if hasattr(sys.stdout, 'reconfigure'):
        sys.stdout.reconfigure(encoding='utf-8', errors='backslashreplace')   # the pak's katakana on any console
    a = sys.argv[1:]
    if len(a) < 2 or a[0] not in ('check', 'list', 'show', 'export', 'import', 'set', 'scrub'):
        sys.exit(__doc__)
    cmd, path = a[0], a[1]
    opt = {}
    rest = []
    i = 2
    while i < len(a):
        if a[i] in ('--ext', '--pak', '-o', '--format'):
            opt[a[i]] = a[i + 1]
            i += 2
        elif a[i] == '--json':
            opt['--json'] = True
            i += 1
        else:
            rest.append(a[i])
            i += 1
    fmt = load_format(opt.get('--format'))
    pak = Pak(path, int(opt.get('--pak', 0)))
    ext = int(opt['--ext'], 0) if '--ext' in opt else None
    if cmd == 'check':
        probs = pak.problems()
        notes = pak.notes()
        print('%s, pak %d: %d notes, %d free pages' % (path, pak.index, len(notes), free_pages(pak)))
        for w in pak.warnings:
            print('  NOTE    ' + w)
        for p in probs:
            print('  PROBLEM ' + p)
        print('RESULT: %s' % ('OK' if not probs else 'DAMAGED'))
        sys.exit(1 if probs else 0)
    if cmd == 'list':
        el = {n['file'] for n in eltale_notes(pak, fmt)}
        for n in pak.notes():
            code = n['game_code'].to_bytes(4, 'big').decode('latin-1')
            comp = n['company_code'].to_bytes(2, 'big').decode('latin-1')
            print('note %2d: %s %s "%s" ext %s, %s' % (n['file'], code, comp, pak_text(n['name']), n['ext'].hex(),
                  'pages %s' % n['pages'] if n['pages'] is not None else 'BROKEN: ' + n['chain_error']))
            if n['file'] in el and n['pages'] is not None:
                v = leaf_values(fmt, pak.note_bytes(n))
                print('         Eltale save "%s": map %d room %d, HP %d/%d, MP %d/%d, play time %d:%02d, day %d%s' % (
                    pak_text(n['ext'][:1]), v['record.map'], v['record.room'], v['record.stats.currHP'],
                    v['record.stats.maxHP'], v['record.stats.currMP'], v['record.stats.maxMP'], v['record.playTime'] // 60,
                    v['record.playTime'] % 60, v['record.dayCount'], ', cleared' if v['record.cleared'] else ''))
        return
    n = pick(pak, fmt, ext)
    before = pak.d
    note = bytearray(pak.note_bytes(n))
    if cmd in ('show', 'export'):
        info = describe(fmt, note)
        info.update({'note_file': n['file'], 'extension': n['ext'].hex(), 'pages': n['pages']})
        if cmd == 'export':
            info['note_bytes'] = bytes(note[:fmt['note']['bytes']]).hex()
            json.dump(info, open(rest[0], 'w', encoding='utf-8'), indent=1)
            print('written %s' % rest[0])
        elif '--json' in opt:
            print(json.dumps(info, indent=1))
        else:
            for k, v in info['values'].items():
                print('%-32s %s' % (k, v))
            for k in ('record_tail', 'data_tail'):
                print('%-32s %s' % (k, info[k]))
            print('%-32s %s' % ('unused (nonzero bytes)', sum(1 for i in range(0, len(info['unused']), 2) if info['unused'][i:i + 2] != '00')))
        return
    if cmd == 'import':
        vals = json.load(open(rest[0], encoding='utf-8'))['values']
        for k, v in vals.items():
            set_leaf(fmt, note, k, v)
    elif cmd == 'set':
        if not rest:
            die('set needs PATH=VALUE')
        for kv in rest:
            k, v = kv.split('=', 1)
            set_leaf(fmt, note, k, v)
    elif cmd == 'scrub':
        for part in fmt['parts'][:2]:
            note[part['offset'] + part['type_bytes']:part['offset'] + part['bytes']] = bytes(part['bytes'] - part['type_bytes'])
        u = fmt['parts'][2]
        note[u['offset']:u['offset'] + u['bytes']] = bytes(u['bytes'])
    pak.write_note(n, bytes(note))
    save_image(pak, before, n['pages'], opt.get('-o'))


def free_pages(pak):
    ino = pak.page(1)
    return sum(1 for p in range(5, 128) if struct.unpack('>H', ino[2 * p:2 * p + 2])[0] == 3)


if __name__ == '__main__':
    main()
