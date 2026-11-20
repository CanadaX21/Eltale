"""test_eltale_save.py: tests for tools/eltale_save.py on synthetic Controller Pak images.

Each image is built here from libultra's file-system layout (four ID blocks with their checksums, the inode table and
its copy, the note table, data pages) and holds no game data: an Eltale save with test values written through the
definition (tools/save_format.json), and a note of another game. The tests run the tool as a program and check what it
prints and writes: check, list, show, export and import (a byte-identical round trip), set (only the named bytes
change), scrub, a Mupen64Plus 4-pak image, and the damage the check must report (an ID block, every ID block, the inode
sum, the inode copy, a broken page chain) and the refusals (a file that is no pak image, a second save with no --ext).
Prints each case and RESULT: OK or RESULT: FAIL. Python 3 standard library."""
import json
import os
import struct
import subprocess
import sys
import tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
TOOL = os.path.join(HERE, 'eltale_save.py')
FMT = json.load(open(os.path.join(HERE, 'save_format.json'), encoding='utf-8'))
PAGE, PAK = 0x100, 0x8000
fails = []


def id_block(seed):
    b = bytearray(struct.pack('>IIIIHH', 0x12345600 | seed, 0x0004F424, 0x009C3482, 0, 0, 0) + bytes(8))
    b[24:28] = b'\x00\x01\x01\x00'                           # device id, banks 1, version
    s = sum(struct.unpack('>14H', bytes(b[:28]))) & 0xFFFF
    return bytes(b[:28]) + struct.pack('>HH', s, (0xFFF2 - s) & 0xFFFF)


def blank():
    d = bytearray(PAK)
    blk = id_block(0x21)
    for at in (0x20, 0x60, 0x80, 0xC0):
        d[at:at + 32] = blk
    ino = bytearray(PAGE)
    for p in range(5, 128):
        ino[2 * p:2 * p + 2] = b'\x00\x03'
    ino[1] = sum(ino[10:256]) & 0xFF
    d[PAGE:2 * PAGE] = ino
    d[2 * PAGE:3 * PAGE] = ino
    return d


def add_note(d, slot, game, company, name, ext, data):
    ino = bytearray(d[PAGE:2 * PAGE])
    free = [p for p in range(5, 128) if ino[2 * p:2 * p + 2] == b'\x00\x03']
    pages = free[:(len(data) + PAGE - 1) // PAGE]
    for k, p in enumerate(pages):
        ino[2 * p:2 * p + 2] = struct.pack('>H', pages[k + 1] if k + 1 < len(pages) else 1)
        d[p * PAGE:(p + 1) * PAGE] = data[k * PAGE:(k + 1) * PAGE].ljust(PAGE, b'\x00')
    ino[1] = sum(ino[10:256]) & 0xFF
    d[PAGE:2 * PAGE] = ino
    d[2 * PAGE:3 * PAGE] = ino
    d[3 * PAGE + 32 * slot:3 * PAGE + 32 * (slot + 1)] = (struct.pack('>IHHBBH', game, company, pages[0], 2, 0, 0)
                                                          + ext.ljust(4, b'\x00') + name.ljust(16, b'\x00'))
    return pages


def save_bytes(values):
    """an Eltale save: every leaf from values (path -> value), the tails and the unused bytes 0xEE"""
    note = bytearray(b'\xEE' * FMT['note']['bytes'])
    for lf in FMT['leaves']:
        f = {('u', 1): '>B', ('s', 1): '>b', ('u', 2): '>H', ('s', 2): '>h', ('u', 4): '>I', ('s', 4): '>i', ('f', 4): '>f'}[(lf['kind'], lf['size'])]
        v = values.get(lf['path'], 0)
        vals = v if isinstance(v, list) else [v] * lf['count']
        for k in range(lf['count']):
            note[lf['offset'] + k * lf['size']:lf['offset'] + (k + 1) * lf['size']] = struct.pack(f, vals[k])
    return bytes(note)


NAME = bytes.fromhex(FMT['note']['name_bytes'])
GAME, COMPANY = FMT['note']['game_code'], FMT['note']['company_code']
VALUES = {'record.stats.currHP': 77, 'record.stats.maxHP': 90, 'record.stats.currMP': 12, 'record.stats.maxMP': 20,
          'record.stats.radius': 50.0, 'record.playTime': 615, 'record.dayCount': 9, 'record.map': 15, 'record.room': 5,
          'record.cleared': 1, 'data.inventory': [3, 7, 9] + [0xFF] * 148, 'data.storyFlags': 0x05}


def run(*args, ok=True):
    r = subprocess.run([sys.executable, '-I', TOOL] + [str(a) for a in args], capture_output=True, text=True,
                       encoding='utf-8')
    return r.returncode, r.stdout + r.stderr


def case(name, cond, detail=''):
    print(('OK    ' if cond else 'FAIL  ') + name + ((': ' + detail) if detail and not cond else ''))
    if not cond:
        fails.append(name)


with tempfile.TemporaryDirectory() as t:
    p = lambda n: os.path.join(t, n)
    img = blank()
    save_pages = add_note(img, 0, GAME, COMPANY, NAME, b'\x10', save_bytes(VALUES))
    add_note(img, 3, 0x4E5A4C45, 0x3031, bytes([0x1D, 0x1E, 0x1A, 0x1B]), b'\x00', bytes(range(256)) * 3)
    open(p('a.pak'), 'wb').write(img)
    rc, out = run('check', p('a.pak'))
    case('check: a well-formed image', rc == 0 and 'RESULT: OK' in out, out)
    rc, out = run('list', p('a.pak'))
    case('list: both notes, the save summarised', rc == 0 and out.count('note ') == 2 and 'HP 77/90' in out
         and 'play time 10:15' in out and 'cleared' in out, out)
    rc, out = run('show', p('a.pak'), '--json')
    v = json.loads(out)['values'] if rc == 0 else {}
    case('show: the values written', v.get('record.stats.maxHP') == 90 and v.get('record.playTime') == 615
         and v.get('data.inventory', [])[:3] == [3, 7, 9] and v.get('data.storyFlags') == 5
         and abs(v.get('record.stats.radius', 0) - 50.0) < 1e-6, out[:300])
    rc, out = run('export', p('a.pak'), p('a.json'))
    rc2, out2 = run('import', p('a.pak'), p('a.json'), '-o', p('b.pak'))
    case('export, then import: byte-identical', rc == 0 and rc2 == 0 and open(p('a.pak'), 'rb').read() == open(p('b.pak'), 'rb').read(), out + out2)
    rc, out = run('set', p('a.pak'), 'record.stats.maxHP=200', 'data.inventory[1]=25', 'record.playTime=60', '-o', p('c.pak'))
    a, c = open(p('a.pak'), 'rb').read(), open(p('c.pak'), 'rb').read()
    diff = [i for i in range(PAK) if a[i] != c[i]]
    rec, dat = save_pages[0] * PAGE, save_pages[0] * PAGE
    want = {rec + 6, rec + 7, dat + 0x80 + 0x30 + 1, rec + 0x38, rec + 0x39}
    case('set: only the named fields\' bytes change', rc == 0 and set(diff) <= want and len(diff) >= 3, '%s %s' % (diff, out))
    rc, out = run('show', p('c.pak'), '--json')
    v = json.loads(out)['values'] if rc == 0 else {}
    case('set: the new values read back', v.get('record.stats.maxHP') == 200 and v.get('data.inventory', [0, 0])[1] == 25
         and v.get('record.playTime') == 60, out[:200])
    rc, out = run('set', p('a.pak'), 'record.stats.maxHP=70000', '-o', p('x.pak'))
    case('set: a value out of the field\'s range is refused', rc != 0 and not os.path.exists(p('x.pak')), out)
    rc, out = run('scrub', p('a.pak'), '-o', p('d.pak'))
    rc2, out2 = run('show', p('d.pak'), '--json')
    s = json.loads(out2) if rc2 == 0 else {}
    case('scrub: the never-read bytes zeroed, the values kept', rc == 0 and set(s.get('record_tail', 'x')) == {'0'}
         and set(s.get('data_tail', 'x')) == {'0'} and set(s.get('unused', 'x')) == {'0'}
         and s.get('values') == json.loads(run('show', p('a.pak'), '--json')[1])['values'], out + out2)
    # in place, with the copy
    open(p('e.pak'), 'wb').write(img)
    rc, out = run('set', p('e.pak'), 'record.dayCount=33')
    case('set in place: a .bak of the old image', rc == 0 and open(p('e.pak.bak'), 'rb').read() == bytes(img), out)
    # Mupen64Plus: four paks in one file, the save in the third
    open(p('m.mpk'), 'wb').write(bytes(blank()) * 2 + bytes(img) + bytes(blank()))
    rc, out = run('show', p('m.mpk'), '--pak', '2', '--json')
    case('a 4-pak image: --pak 2 reads the save', rc == 0 and json.loads(out)['values']['record.stats.maxHP'] == 90, out[:200])
    rc, out = run('show', p('m.mpk'), '--pak', '0')
    case('a 4-pak image: pak 0 has no save', rc != 0 and '0 Eltale saves' in out, out)
    # damage
    for name, mutate, want_rc in (
            ('one ID block bad (three good): a note, still well-formed', lambda d: d.__setitem__(0x25, d[0x25] ^ 0xFF), 0),
            ('every ID block bad: a damaged pak', lambda d: [d.__setitem__(at + 5, d[at + 5] ^ 0xFF) for at in (0x20, 0x60, 0x80, 0xC0)], 1),
            ('the inode sum wrong', lambda d: d.__setitem__(PAGE + 1, d[PAGE + 1] ^ 1), 1),
            ('the inode copy differs', lambda d: d.__setitem__(2 * PAGE + 100, d[2 * PAGE + 100] ^ 1), 1)):
        dd = bytearray(img)
        mutate(dd)
        open(p('dmg.pak'), 'wb').write(dd)
        rc, out = run('check', p('dmg.pak'))
        case('check: ' + name, rc == want_rc and ('PROBLEM' in out if want_rc else 'NOTE' in out and 'RESULT: OK' in out), out)
    dd = bytearray(img)
    ino = bytearray(dd[PAGE:2 * PAGE])
    ino[2 * save_pages[0]:2 * save_pages[0] + 2] = struct.pack('>H', save_pages[0])     # a chain that loops
    ino[1] = sum(ino[10:256]) & 0xFF
    dd[PAGE:2 * PAGE] = ino
    dd[2 * PAGE:3 * PAGE] = ino
    open(p('loop.pak'), 'wb').write(dd)
    rc, out = run('check', p('loop.pak'))
    case('check: a page chain that loops', rc == 1 and 'loops' in out, out)
    rc, out = run('show', p('loop.pak'))
    case('show: a broken save is refused', rc != 0 and 'broken' in out, out)
    # refusals
    open(p('short.bin'), 'wb').write(bytes(1000))
    rc, out = run('check', p('short.bin'))
    case('a file that is no pak image is refused', rc != 0 and 'not a Controller Pak image' in out, out)
    two = bytearray(img)
    add_note(two, 1, GAME, COMPANY, NAME, b'\x11', save_bytes({'record.stats.maxHP': 55}))
    open(p('two.pak'), 'wb').write(two)
    rc, out = run('show', p('two.pak'))
    rc2, out2 = run('show', p('two.pak'), '--ext', '0x11', '--json')
    case('two saves: --ext needed, --ext 0x11 picks the second', rc != 0 and '--ext' in out and rc2 == 0
         and json.loads(out2)['values']['record.stats.maxHP'] == 55, out + out2[:200])
print('RESULT: %s' % ('OK' if not fails else 'FAIL (%d)' % len(fails)))
sys.exit(1 if fails else 0)
