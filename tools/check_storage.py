"""check_storage.py [ROOT]: every C declaration of a variable whose storage an assembly row reserves fits it.

Main's game state is defined in C (each file's .bss), so a type that grows takes its storage with it and the link
moves what follows. What an assembly row still reserves (an `asm/data/<name>.bss.s` file of the config's `bss`
subsegments: labels and .space, for state no C file defines, such as the library's and the SDK's) is declared in C by
externs, and C never compares such a declaration with the space the row gives the label: a declaration that grows
past it overruns the next variable with no error. This checks it: for each label of every assembly .bss row that a C
file or header declares, sizeof(the declared type) times the declared count must not exceed the space from the label
to the next label (or the row's end). An unsized array ([]) is checked for one element. A declared type this tool
cannot lay out (tools/c_layout.py's limits), or a bound it does not evaluate, is listed as not checked. Exit 1 on a
misfit. The room ends at the next label, so a struct whose field carries a label of its own reads short: the SDK's
__osContPifRam and __osPfsPifRam (0x40 each, a field labelled at 0x3C) would, if their declarations were checked; the
tool reads the game's C and headers, not the SDK's include/2.0I.

The build runs it before the link (configure.py: build/check_storage.ok, an input of the ELF), so a misfit stops the
build. Python 3 standard library and tools/c_layout.py only."""
import glob
import os
import re
import sys

ROOT = sys.argv[1] if len(sys.argv) > 1 else os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(ROOT, 'tools'))
import c_layout  # noqa: E402

KEYW = {'extern', 'static', 'const', 'volatile', 'unsigned', 'signed', 'struct', 'union', 'enum', 'register', 'void',
        'char', 'short', 'int', 'long', 'float', 'double', 'typedef', 'sizeof'}


def read(p):
    with open(p, encoding='utf-8', errors='replace') as f:
        return f.read()


def strip(text):
    """comments, strings and preprocessor lines blanked (positions kept)"""
    out = re.sub(r'/\*.*?\*/', lambda m: re.sub(r'[^\n]', ' ', m.group(0)), text, flags=re.S)
    out = re.sub(r'//[^\n]*', '', out)
    out = re.sub(r'"(?:\\.|[^"\\])*"', '""', out)
    return re.sub(r'(?m)^[ \t]*#.*(?:\\\n.*)*$', '', out)


def statements(s):
    """file-scope statements ending in ';' (function bodies and initializer braces skipped)"""
    res, start, i, n = [], 0, 0, len(s)
    while i < n:
        c = s[i]
        if c == '{':
            head = s[start:i]
            d, j = 1, i + 1
            while j < n and d:
                d += {'{': 1, '}': -1}.get(s[j], 0)
                j += 1
            if re.search(r'\)\s*$', head) and '=' not in head and not re.match(r'\s*(typedef|struct|union|enum)\b', head):
                start = j
            i = j
            continue
        if c == ';':
            res.append(s[start:i])
            start = i + 1
        i += 1
    return res


# -- the rows: the config's `bss` subsegments, each written by splat as asm/data/<name>.bss.s --------------------------
rows = re.findall(r'type:\s*bss,\s*vram:\s*0x[0-9A-Fa-f]+,\s*name:\s*"?([\w.]+)"?', read(os.path.join(ROOT, 'eltail.jp.yaml')))
space = {}                                       # label -> (row, addr, bytes to the next label or the row's end)
for row in rows:
    p = os.path.join(ROOT, 'asm', 'data', row + '.bss.s')
    if not os.path.exists(p):
        continue
    cur = None
    for line in read(p).splitlines():
        m = re.match(r'(?:glabel|dlabel)\s+(\S+)', line)
        if m:
            cur = m.group(1)
            space[cur] = [row, None, 0]
            continue
        m = re.match(r'/\* \w+ ([0-9A-F]{8}) \*/\s+\.space\s+(0x[0-9A-Fa-f]+|\d+)', line)
        if m and cur is not None:
            if space[cur][1] is None:
                space[cur][1] = int(m.group(1), 16)
            space[cur][2] += int(m.group(2), 0)

# -- the C: declarations of those labels, and the types to lay them out ------------------------------------------------
files = {}
for base in ('src', 'include'):
    for dp, dn, fn in os.walk(os.path.join(ROOT, base)):
        for f in sorted(fn):
            if f.endswith(('.c', '.h')):
                files[os.path.relpath(os.path.join(dp, f), ROOT).replace(os.sep, '/')] = read(os.path.join(dp, f))
sdk = [r for r in files if r.startswith('include/2.0I/')]
headers = [r for r in files if r.endswith('.h')]
defines = {}
for r in headers:
    for m in re.finditer(r'(?m)^\s*#\s*define\s+([A-Za-z_]\w*)\s+\(?\s*(0x[0-9A-Fa-f]+|\d+)\s*\)?\s*$', files[r]):
        v = m.group(2)                           # C's reading: 0x hex, a leading 0 octal
        defines.setdefault(m.group(1), int(v, 16) if v[:2] in ('0x', '0X') else int(v, 8) if len(v) > 1 and v[0] == '0' else int(v))
# a typedef of a scalar or a pointer (c_layout lays out struct and union typedefs only)
alias = {}
for r in headers:
    for m in re.finditer(r'typedef\s+((?:unsigned\s+|signed\s+|const\s+)*\w+(?:\s+\w+)?)\s*(\**)\s*(\w+)\s*;', strip(files[r])):
        base, stars, name = m.group(1).strip(), m.group(2), m.group(3)
        alias.setdefault(name, 4 if stars else c_layout.SCALARS.get(base, (None,))[0] if base in c_layout.SCALARS else base)
FNPTR = re.compile(r'typedef\s+[^;()]*\(\s*\*\s*(\w+)\s*\)\s*\([^;]*\)\s*;')


def layout_texts(extra):
    """the headers (function-pointer typedefs laid out as one word: c_layout reads no function pointer) and a file"""
    texts = []
    for r in headers + extra:
        texts.append(FNPTR.sub(lambda m: 'typedef struct { u32 fn; } %s;' % m.group(1), files[r]))
    return texts


_lay = {}


def size_of(type_text, unit):
    t = re.sub(r'\b(const|volatile|extern|static)\b', ' ', type_text).strip()
    if '*' in t:
        return 4
    t = re.sub(r'^(struct|union)\s+', '', t).strip()
    seen = set()
    while t in alias and t not in seen:              # follow scalar and pointer typedefs
        seen.add(t)
        v = alias[t]
        if isinstance(v, int):
            return v
        t = v
    if t in c_layout.SCALARS:
        return c_layout.SCALARS[t][0]
    for key in ([None] + ([unit] if unit else [])):
        if key not in _lay:
            try:
                _lay[key] = c_layout.Layouts(layout_texts([] if key is None else [key]))
            except Exception:
                _lay[key] = None
        if _lay[key] is not None:
            try:
                return _lay[key].get(t).size
            except Exception:
                pass
    return None


DECL = re.compile(r'^(?P<type>[A-Za-z_][\w ]*?) *(?P<ptr>\**) *(?P<name>[A-Za-z_]\w*) *(?P<bounds>(?:\[[^\]]*\])*)$')
checked, unchecked, misfits = 0, [], []
for rel, text in sorted(files.items()):
    if rel in sdk:
        continue
    for st in statements(strip(text)):
        s = ' '.join(st.split())
        if not s.startswith('extern ') or '(' in s.split('[')[0]:
            continue                             # functions and function pointers are not storage this checks
        if '(' in s:                             # an expression in a bound (a sizeof, a cast): listed, not guessed
            names = [n for n in re.findall(r'(\w+)\s*\[', s) if n in space]
            unchecked += ['%s (%s: a bound this tool does not evaluate)' % (n, rel) for n in names]
            continue
        parts = [x.strip() for x in s[len('extern '):].split(',')]
        first = DECL.match(parts[0])
        if not first:
            continue
        decls = [first] + [DECL.match(first.group('type') + ' ' + x) for x in parts[1:]]
        for m in decls:
            if not m or m.group('name') not in space:
                continue
            name, type_text = m.group('name'), m.group('type') + m.group('ptr')
            bounds = re.findall(r'\[([^\]]*)\]', m.group('bounds'))
            count, known = 1, True
            for b in (x.strip() for x in bounds):
                if not b:
                    continue                     # unsized: one element is checked
                try:
                    count *= int(b, 0)
                except ValueError:
                    if b in defines:
                        count *= defines[b]
                    else:
                        known = False
            size = size_of(type_text, rel if rel.endswith('.c') else None)
            row, addr, room = space[name]
            if size is None or not known:
                unchecked.append('%s (%s: %s)' % (name, rel, type_text))
                continue
            need = size * count
            checked += 1
            if need > room:
                misfits.append('%s: %s declares %d bytes (%s%s), the row %s reserves %d at 0x%08X'
                               % (name, rel, need, type_text, ''.join('[%s]' % b for b in bounds), row, room, addr))
print('check_storage: %d declarations of %d assembly-reserved labels checked in %d rows; %d misfit(s); %d not checked'
      % (checked, len(space), len(rows), len(misfits), len(unchecked)))
for m in misfits:
    print('  MISFIT ' + m)
for u in unchecked:
    print('  not checked (type not laid out): ' + u)
sys.exit(1 if misfits else 0)
