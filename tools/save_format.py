"""save_format.py [--check | --write] [ROOT]: the save format, machine-readable, derived from the source.

The decomp's C is the one definition of the save: include/game/save_format.h (the note and its parts), the record types
(SaveSlot and SaveData in include/game/save_format.h, BrianData1 in include/quest_types.h, laid out by tools/c_layout.py as IDO lays
them out) and the code that fills and empties them (WriteSaveRecord and ReadSaveRecord in src/game/save_menu.c). This tool reads
them and writes the definition other tools and ports read (tools/save_format.json): the note's identity and size, its
parts, every field with its offset in the note, its type and the game variable it is saved from and restored into.
It checks what a save needs to stay whole, in any build:
  - each part's type fits its part (SaveSlot in SAVE_RECORD_BYTES, SaveData in SAVE_DATA_BYTES), the parts lie in the
    note in order, the note is SAVE_NOTE_PAGES pages of 256 bytes;
  - every copy of a save-data member fits the game variable it is copied from and into (the declared array sizes);
  - ReadSaveRecord restores each field into the variable WriteSaveRecord saved it from, and nothing else.
--check (the build runs it before the link, configure.py: build/check_save_format.ok): the checks, and the derived
definition must equal tools/save_format.json (exit 1 otherwise: rerun with --write after a deliberate change).
--write: the checks, then writes tools/save_format.json. Without either: prints the definition.
Python 3 standard library and tools/c_layout.py only."""
import json
import os
import re
import sys

args = [a for a in sys.argv[1:] if not a.startswith('--')]
ROOT = args[0] if args else os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(ROOT, 'tools'))
import c_layout  # noqa: E402

OUT = os.path.join(ROOT, 'tools', 'save_format.json')
problems = []


def read(p):
    return open(os.path.join(ROOT, p), encoding='utf-8').read()


def problem(text):
    problems.append(text)


hdr = read('include/game/save_format.h')
# the record's sizes are in save_format.h; gGameState's bits (GAME_STATE_CLEARED) in include/variables.h
K = {m.group(1): int(m.group(2), 0) for m in re.finditer(r'^#define (\w+)\s+(0x[0-9A-Fa-f]+|\d+)\b',
                                                       hdr + '\n' + read('include/variables.h'), re.M)}
qt, menu = read('include/quest_types.h'), read('src/game/save_menu.c')
lay = c_layout.Layouts([qt, hdr])


def comments(text, typename):
    """{member: comment} from a `typedef struct { ... } typename;` body: the comment on each member's line"""
    m = re.search(r'typedef struct\s*\w*\s*\{(.*?)\}\s*%s\s*;' % typename, text, re.S)
    out = {}
    for line in (m.group(1).split('\n') if m else []):
        d = re.match(r'\s*[\w\s\*]+?\b(\w+)\s*(\[[^\]]*\])?\s*;\s*(?:/\*(.*?)\*/)?', line)
        if d:
            out[d.group(1)] = ' '.join((d.group(3) or '').split())
    return out


NOTES = {t: comments(qt if t == 'BrianData1' else hdr, t) for t in ('SaveSlot', 'SaveData', 'BrianData1')}


def tname(t):
    if isinstance(t, c_layout.Scalar):
        return t.name
    if isinstance(t, c_layout.Array):
        return '%s[%d]' % (tname(t.elem), t.count)
    if isinstance(t, c_layout.Aggregate):
        return t.name
    return 'pointer'


def leaves(agg, base, prefix):
    """every scalar leaf of a type: (path, offset from the note's start, scalar type, count)"""
    out = []
    for mem in agg.members:
        t, off, path = mem.type, base + mem.offset, prefix + '.' + mem.name
        count = 1
        while isinstance(t, c_layout.Array):
            count *= t.count
            t = t.elem
        if isinstance(t, c_layout.Aggregate):
            if count != 1:
                raise SystemExit('an array of structs in a save type is not supported: ' + path)
            out += leaves(t, off, path)
        elif isinstance(t, c_layout.Scalar):
            out.append({'path': path, 'offset': off, 'type': t.name, 'kind': t.kind, 'size': t.size, 'count': count})
        else:
            raise SystemExit('a pointer in a save type: ' + path)
    return out


def body(name):
    m = re.search(r'^void %s\([^)]*\) \{\n(.*?)^\}' % name, menu, re.S | re.M)
    if not m:
        raise SystemExit('src/game/save_menu.c: no %s' % name)
    return m.group(1)


# what WriteSaveRecord saves and ReadSaveRecord restores, read from their code
wr, rd = body('WriteSaveRecord'), body('ReadSaveRecord')
save_map = {f: v for f, v in re.findall(r's->(\w+) = (\w+);', wr)}
load_map = {f: v for v, f in re.findall(r'(\w+) = s->(\w+);', rd)}
for d, s_ in re.findall(r'dst = &?gSaveData\.(\w+);\s*src = (\w+);', wr):
    save_map['SaveData.' + d] = s_
for s_, d in re.findall(r'src = &?gSaveData\.(\w+);\s*dst = (\w+);', rd):
    load_map['SaveData.' + s_] = d
if re.search(r'if \(gGameState & GAME_STATE_CLEARED\) \{\s*s->cleared = 1;', wr):
    save_map['cleared'] = 'gGameState & GAME_STATE_CLEARED'
if re.search(r'if \(s->cleared != 0\) \{\s*gGameState \|= GAME_STATE_CLEARED;', rd):
    load_map['cleared'] = 'gGameState & GAME_STATE_CLEARED'
for f in sorted(set(save_map) | set(load_map)):
    if save_map.get(f) != load_map.get(f):
        problem('%s: WriteSaveRecord saves it from %s, ReadSaveRecord restores it into %s'
                % (f, save_map.get(f), load_map.get(f)))

# the declared sizes of the variables the save data's members are copied from and into
defs = {}
# every C file under src/, folders included (a layout may put the units in folders)
for dp, dirs, files in os.walk(os.path.join(ROOT, 'src')):
    dirs.sort()
    for f in sorted(files):
        if f.endswith('.c'):
            fn = os.path.relpath(os.path.join(dp, f), os.path.join(ROOT, 'src')).replace(os.sep, '/')
            for t, n, b in re.findall(r'^(?:static )?(u8|s8) (g\w+)\[([^\]]+)\];', read('src/' + fn), re.M):
                defs[n] = (fn, b)

slot, data, stats = lay.get('SaveSlot'), lay.get('SaveData'), lay.get('BrianData1')
if slot.size > K['SAVE_RECORD_BYTES']:
    problem('SaveSlot is 0x%X bytes: it does not fit the record (SAVE_RECORD_BYTES 0x%X)' % (slot.size, K['SAVE_RECORD_BYTES']))
if data.size > K['SAVE_DATA_BYTES']:
    problem('SaveData is 0x%X bytes: it does not fit the save data (SAVE_DATA_BYTES 0x%X)' % (data.size, K['SAVE_DATA_BYTES']))
if K['SAVE_RECORD_OFFSET'] + K['SAVE_RECORD_BYTES'] > K['SAVE_DATA_OFFSET']:
    problem('the record runs into the save data')
if K['SAVE_DATA_OFFSET'] + K['SAVE_DATA_BYTES'] > K['SAVE_NOTE_BYTES']:
    problem('the save data runs past the note (SAVE_NOTE_BYTES 0x%X)' % K['SAVE_NOTE_BYTES'])
if K['SAVE_NOTE_BYTES'] != K['SAVE_NOTE_PAGES'] * 0x100:
    problem('SAVE_NOTE_BYTES is not SAVE_NOTE_PAGES pages of 256 bytes')
for mem in data.members:
    var = save_map.get('SaveData.' + mem.name)
    size = mem.type.size if hasattr(mem.type, 'size') else None
    if var is None:
        problem('SaveData.%s: WriteSaveRecord fills it from nothing' % mem.name)
    elif var not in defs:
        problem('SaveData.%s is copied from %s, whose definition (u8 or s8 array) this tool did not find' % (mem.name, var))
    else:
        n = int(defs[var][1], 0)
        if size > n:
            problem('SaveData.%s (0x%X bytes) is copied into %s, defined in src/%s with %d bytes: a load would overrun it'
                    % (mem.name, size, var, defs[var][0], n))

# the note's name and extension templates: the bytes of main's binary piece that holds them (as the build has them)
undef = read('undefined_syms.jp.txt')
name = {}
for sym, n in (('gSaveNoteName', 16), ('gNewNoteExtName', 4)):
    m = re.search(r'^%s = _(\w+)_bin \+ (0x[0-9A-Fa-f]+);' % sym, undef, re.M)
    raw = open(os.path.join(ROOT, 'assets', m.group(1) + '.bin'), 'rb').read()
    name[sym] = raw[int(m.group(2), 16):int(m.group(2), 16) + n]

PAK_CHARS = {0x00: '', 0x0F: ' '}
PAK_CHARS.update({0x10 + i: c for i, c in enumerate('0123456789')})
PAK_CHARS.update({0x1A + i: c for i, c in enumerate('ABCDEFGHIJKLMNOPQRSTUVWXYZ')})
PAK_CHARS.update({0x34 + i: c for i, c in enumerate('!"#\'*+,-./:=?@')})
PAK_CHARS.update({0x42 + i: c for i, c in enumerate(
    '。゛゜ァィゥェォッャュョヲンアイウエオカキクケコサシスセソタチツテトナニヌネノハヒフヘホマミムメモヤユヨラリルレロワ'
    'ガギグゲゴザジズゼゾダヂヅデドバビブベボパピプペポ')})


def pak_text(b):
    return ''.join(PAK_CHARS.get(c, '\\x%02x' % c) for c in b)


def fields(agg, base, part):
    out = []
    for mem in agg.members:
        key = mem.name if part == 'record' else 'SaveData.' + mem.name
        out.append({'name': mem.name, 'offset': base + mem.offset, 'bytes': mem.type.size, 'type': tname(mem.type),
                    'comment': NOTES['SaveSlot' if part == 'record' else 'SaveData'].get(mem.name, ''),
                    'saved_from': save_map.get(key), 'restored_into': load_map.get(key)})
    return out


rec_off, dat_off = K['SAVE_RECORD_OFFSET'], K['SAVE_DATA_OFFSET']
definition = {
    'about': 'The save format of Eltale Monsters (the Japanese original), derived from the decomp\'s source by '
             'tools/save_format.py (include/game/save_format.h, which defines SaveSlot and SaveData; BrianData1 in '
             'include/quest_types.h; WriteSaveRecord and ReadSaveRecord). Offsets are from the note\'s start; every '
             'value is big-endian; f32 is IEEE single.',
    'note': {'company_code': K['SAVE_NOTE_COMPANY_CODE'], 'company_code_text': K['SAVE_NOTE_COMPANY_CODE'].to_bytes(2, 'big').decode(),
             'game_code': K['SAVE_NOTE_GAME_CODE'], 'game_code_text': K['SAVE_NOTE_GAME_CODE'].to_bytes(4, 'big').decode(),
             'name_bytes': name['gSaveNoteName'].hex(), 'name_text': pak_text(name['gSaveNoteName']),
             'extension_template': name['gNewNoteExtName'].hex(), 'first_extension': K['SAVE_NOTE_FIRST_EXTENSION'],
             'bytes': K['SAVE_NOTE_BYTES'], 'pages': K['SAVE_NOTE_PAGES'], 'saves_listed': K['SAVE_SLOTS']},
    'parts': [
        {'part': 'record', 'offset': rec_off, 'bytes': K['SAVE_RECORD_BYTES'], 'type': 'SaveSlot', 'type_bytes': slot.size,
         'written_from': 'gSaveList[slot] (WriteSaveRecord fills it first)', 'read_into': 'gSaveRecordBuffer, then gSaveList[slot]',
         'tail': 'bytes %d-%d: the RAM after the gSaveList entry when it was written (the next entry\'s start; after the '
                 '16th entry, gSaveData\'s); never read back' % (slot.size, K['SAVE_RECORD_BYTES'] - 1)},
        {'part': 'data', 'offset': dat_off, 'bytes': K['SAVE_DATA_BYTES'], 'type': 'SaveData', 'type_bytes': data.size,
         'written_from': 'gSaveData', 'read_into': 'gPakReadBuffer, then gSaveData',
         'tail': 'bytes %d-%d: the RAM after gSaveData when it was written (gSaveRecordBuffer\'s start: the last record '
                 'the save list read); never read back' % (data.size, K['SAVE_DATA_BYTES'] - 1)},
        {'part': 'unused', 'offset': dat_off + K['SAVE_DATA_BYTES'], 'bytes': K['SAVE_NOTE_BYTES'] - dat_off - K['SAVE_DATA_BYTES'],
         'content': 'never written: whatever the pages held before the note was allocated'}],
    'record': fields(slot, rec_off, 'record'),
    'data': fields(data, dat_off, 'data'),
    'types': {t: {'bytes': lay.get(t).size, 'members': [
        {'name': m.name, 'offset': m.offset, 'bytes': m.type.size, 'type': tname(m.type), 'comment': NOTES[t].get(m.name, '')}
        for m in lay.get(t).members]} for t in ('SaveSlot', 'SaveData', 'BrianData1')},
    'leaves': leaves(slot, rec_off, 'record') + leaves(data, dat_off, 'data'),
    'restore_rules': {
        'cleared': 'nonzero sets gGameState\'s GAME_STATE_CLEARED (0x%X); a load never clears it' % K['GAME_STATE_CLEARED'],
        'position': 'not saved: a load puts Brian at gRespawnLocationTbl\'s place for the saved map and room (17 rows), '
                    'else at the room\'s entrance 0 (RespawnAtSavePoint)',
        'radius': 'record.stats.radius is saved, then overwritten: every load ends in RespawnAtSavePoint, which '
                  'sets gPlayerState.stats->radius (Brian\'s) to 50.0, so an edited radius never takes effect',
        'not_saved': ['the time of day (gTimeOfDay)', 'gStoryFlags[1-3]', 'gGameState\'s other bits', 'Brian\'s position']},
}

if problems:
    for p in problems:
        print('FAIL  ' + p)
    sys.exit(1)
text = json.dumps(definition, indent=1, ensure_ascii=False) + '\n'
if '--write' in sys.argv:
    open(OUT, 'w', encoding='utf-8', newline='\n').write(text)
    print('written %s: %d record fields, %d data fields, %d leaves' % (OUT, len(definition['record']),
                                                                      len(definition['data']), len(definition['leaves'])))
elif '--check' in sys.argv:
    have = open(OUT, encoding='utf-8').read() if os.path.exists(OUT) else ''
    if have != text:
        print('FAIL  tools/save_format.json differs from the definition the source gives: rerun tools/save_format.py --write '
              'after a deliberate change')
        sys.exit(1)
    print('OK    the save format: SaveSlot 0x%X of 0x%X, SaveData 0x%X of 0x%X, the note 0x%X; %d fields saved and restored '
          'alike; tools/save_format.json current' % (slot.size, K['SAVE_RECORD_BYTES'], data.size, K['SAVE_DATA_BYTES'],
                                                    K['SAVE_NOTE_BYTES'], len(save_map)))
else:
    sys.stdout.write(text)
