#!/usr/bin/env python3
"""check_decls.py: every global variable has one authoritative declaration, and the source agrees with it.

    python3 tools/check_decls.py            (from the project root; exit 1 on any problem)

The rules (docs/DECLARATIONS.md):
  1. one declaration: a variable is declared `extern` once, in one project header (include/, the SDK's include/2.0I
     apart);
  2. declared where it is defined: a variable a game C unit under src/ defines (file scope, not static) is declared
     in a project header (the SDK library units, src/libultra/initialize.c to src/libultra/visetevent.c, keep the SDK's own conventions);
  3. no local declarations: a C file under src/ declares no variable with `extern`, at file scope or inside a function,
     unless the line says why: a comment holding `local declaration:` and the reason (it also covers the extern
     lines right after it); and never one that a header the file includes, directly or through other headers,
     already declares: the reason is for a header the file does not include;
  4. one type: every declaration of a variable, in a header, in a C file or as its definition, has the same type and
     the same array bounds (an unsized `[]` agrees with any bound);
  5. checked by the compiler: the unit that defines a declared variable includes the header that declares it, so the
     compiler compares the two;
  6. headers declare, units define: a project header holds no variable definition (no tentative definition, no static
     variable), which every unit that includes it would define again.
Variables only: functions follow their own rules (docs/DECLARATIONS.md), which this does not check. A type written
in place (`extern struct { ... } name;`) is compared by its text. Standard library only."""
import collections, hashlib, os, re, sys

ROOT = os.getcwd()
LIBRARY = {'src/libultra/initialize.c', 'src/libultra/createthread.c', 'src/libultra/startthread.c', 'src/libultra/pimgr.c', 'src/libultra/createmesgqueue.c', 'src/libultra/visetevent.c'}
LOCAL_OK = 'local declaration:'
# the local declarations docs/DECLARATIONS.md lists, each with its reason in place; a marked one not here is a
# problem (a new exception is added here and to the reference together)
LOCAL_DOCUMENTED = {('src/game/spells.c', n) for n in ('gEnemyCount', 'gEnemies', 'gArrivalFlags', 'gPlayerState',
                                                 'gBrianStats')} | {('src/game/message.c', 'D_800616B4')}
KEYW = {'extern', 'static', 'const', 'volatile', 'unsigned', 'signed', 'struct', 'union', 'enum', 'register', 'void',
        'char', 'short', 'int', 'long', 'float', 'double', 'typedef', 'sizeof'}


def strip(text):
    """comments, strings, preprocessor lines, GLOBAL_ASM blocks and BSS_ORDER_PAD slots blanked; positions kept"""
    out, i, n = list(text), 0, len(text)
    def blank(a, b):
        for k in range(a, b):
            if out[k] != '\n':
                out[k] = ' '
    for m in re.finditer(r'(?ms)^GLOBAL_ASM\(\s*$.*?^\)[ \t]*$', text):
        blank(m.start(), m.end())
    for m in re.finditer(r'\bBSS_ORDER_PAD\s*\(\s*\d+\s*\)', text):
        blank(m.start(), m.end())
    t = ''.join(out)
    # comments and literals first, in one pass (a comment may begin on a #define line and run on: its
    # continuation was read as code, and an apostrophe in it hid the code up to the next one)
    while i < n:
        if t.startswith('/*', i):
            j = t.find('*/', i + 2); j = n if j < 0 else j + 2; blank(i, j); i = j
        elif t.startswith('//', i):
            j = t.find('\n', i); j = n if j < 0 else j; blank(i, j); i = j
        elif t[i] in '"\'':
            q, j = t[i], i + 1
            while j < n and t[j] != q and t[j] != '\n':
                j += 2 if t[j] == '\\' else 1
            blank(i + 1, j); i = j + 1
        else:
            i += 1
    # then the preprocessor lines, with their continuations
    t = ''.join(out)
    for m in re.finditer(r'(?m)^[ \t]*#', t):
        e = m.start()
        while True:
            e2 = t.find('\n', e); e2 = n if e2 < 0 else e2
            if t[e:e2].rstrip().endswith('\\'):
                e = e2 + 1; continue
            break
        blank(m.start(), e2)
    return ''.join(out)


def file_statements(s):
    res, start, i, n, kr = [], 0, 0, len(s), False
    while i < n:
        c = s[i]
        if c == '{':
            head = s[start:i]
            d, j = 1, i + 1
            while j < n and d:
                d += {'{': 1, '}': -1}.get(s[j], 0); j += 1
            if (re.search(r'\)\s*$', head) or kr) and '=' not in head and not re.match(r'\s*(typedef|struct|union|enum)\b', head):
                start = j
                kr = False
            i = j
            continue
        if c == ';':
            st = s[start:i]
            if re.search(r'\w\s*\([\w\s,]*\)\s*(register\s+)?[A-Za-z_]', st):
                kr = True
            if not kr:
                res.append((start, i + 1))
            start = i + 1
        i += 1
    return res


def split_top(t):
    out, d, cur = [], 0, ''
    for ch in t:
        if ch in '([{': d += 1
        elif ch in ')]}': d -= 1
        if ch == ',' and d == 0:
            out.append(cur); cur = ''
        else:
            cur += ch
    return out + [cur]


def close_brace(t, i):
    """the index of the brace that closes the one at t[i]"""
    d = 0
    for k in range(i, len(t)):
        d += {'{': 1, '}': -1}.get(t[k], 0)
        if d == 0:
            return k
    return len(t) - 1


def declarators(stmt):
    """[(name, type signature, [bounds])] for an object declaration, [] for anything else"""
    t = ' '.join(stmt.strip().rstrip(';').split())
    t = re.sub(r'^(extern|static)\s+', '', t)
    if not t or t.startswith('typedef'):
        return []
    m = re.match(r'(?:(?:const|volatile)\s+)*(struct|union|enum)\s*(\w*)\s*\{', t)
    if m:
        # a type written in place: a definition of the type alone, or objects of it (compared by the type's text)
        end = close_brace(t, m.end() - 1)
        if not t[end + 1:].strip():
            return []
        body = ' '.join(t[m.end():end].split())
        t = '%s@T%s %s' % (m.group(1), hashlib.sha1(body.encode()).hexdigest()[:12], t[end + 1:].strip())
    if any(re.search(r'[A-Za-z_]\w*\s*\((?!\s*\*)', re.sub(r'\[[^\]]*\]', '', x)) for x in split_top(t.split('=')[0])):
        return []                         # a function's prototype (a name and its parameter list), not an object
    parts, out, base = split_top(t), [], None
    for k, p in enumerate(parts):
        p = p.split('=')[0].strip()
        m = re.search(r'\(\s*\*+\s*([A-Za-z_]\w*)', p)
        if m:
            name = m.group(1)
        else:
            ids = [x for x in re.findall(r'[A-Za-z_]\w*', re.sub(r'\[[^\]]*\]', '', p)) if x not in KEYW]
            if not ids:
                return []
            name = ids[-1]
            if re.search(r'\b%s\s*\(' % re.escape(name), p):
                return []
        if k == 0:
            star = re.search(r'[*(]', p)
            cut = min(star.start() if star else len(p), p.find(name))
            base = p[:cut].strip()
            declr = p[cut:].strip()
        else:
            declr = p
        bounds = [b.strip() for b in re.findall(r'\[([^\]]*)\]', declr)]
        sig = base + ' ' + re.sub(r'\b%s\b' % re.escape(name), '@', declr, count=1)
        sig = re.sub(r'\[[^\]]*\]', '[]', sig)
        sig = re.sub(r'\s*\*\s*', '*', ' '.join(sig.split())).replace('float', 'f32').replace('double', 'f64')
        out.append((name, sig.strip(), bounds))
    return out


def extern_statements(s):
    """[(start, end)] of every statement that begins with `extern`, file scope or inside a function (its end is the
    first `;` outside braces: a type written in place holds its own)"""
    res = []
    for m in re.finditer(r'\bextern\b', s):
        a = m.start()
        b = max(s.rfind(';', 0, a), s.rfind('{', 0, a), s.rfind('}', 0, a)) + 1
        if s[b:a].strip():
            continue
        d, k = 0, a
        while k < len(s):
            if s[k] == '{':
                d += 1
            elif s[k] == '}':
                d -= 1
            elif s[k] == ';' and d == 0:
                break
            k += 1
        res.append((a, k + 1))
    return res


def include_closure(rel, includes, files):
    """the project headers a file includes, directly or through other headers"""
    seen, todo = set(), [rel]
    while todo:
        f = todo.pop()
        for inc in includes.get(f, ()):
            for cand in ('include/' + inc, os.path.normpath(os.path.join(os.path.dirname(f), inc)).replace(os.sep, '/')):
                if cand in files and cand not in seen:
                    seen.add(cand); todo.append(cand)
                    break
    return seen


def bound(b):
    try:
        return int(b, 0)
    except ValueError:
        return b.replace(' ', '')


def main():
    files = []
    for base in ('src', 'include'):
        for dp, dn, fn in os.walk(os.path.join(ROOT, base)):
            rel_dp = os.path.relpath(dp, ROOT).replace(os.sep, '/')
            if rel_dp.startswith('include/2.0I'):
                continue
            files += [os.path.relpath(os.path.join(dp, f), ROOT).replace(os.sep, '/') for f in fn if f.endswith(('.c', '.h'))]
    header_decl = collections.defaultdict(list)      # name -> [(header, sig, bounds)]
    local_decl = collections.defaultdict(list)       # name -> [(file, line, sig, bounds, allowed)]
    definition = {}                                  # name -> (unit, sig, bounds)
    includes = {}
    problems = []
    for rel in sorted(files):
        text = open(os.path.join(ROOT, rel), encoding='utf-8', errors='replace').read()
        s = strip(text)
        lines = text.split('\n')
        includes[rel] = set(re.findall(r'(?m)^\s*#\s*include\s+"([^"]+)"', text))
        # a comment holding the marker covers the extern lines right after it (a group with one stated reason)
        covered = set()
        for m in re.finditer(re.escape(LOCAL_OK), text):
            end = text.find('*/', m.start())
            k = text.count('\n', 0, end if end >= 0 else m.start()) + 1
            while k < len(lines) and lines[k].lstrip().startswith('extern '):
                covered.add(k + 1)
                k += 1
        for a, e in extern_statements(s):
            line = text.count('\n', 0, a) + 1
            ok = LOCAL_OK in lines[line - 1] or line in covered
            # an initialiser outside braces makes an extern a definition (IDO defines it in every including unit)
            body = re.sub(r'\{[^{}]*\}', '', re.sub(r'\{[^{}]*\}', '', s[a:e]))
            for name, sig, bounds in declarators(s[a:e]):
                if rel.endswith('.h'):
                    header_decl[name].append((rel, sig, bounds))
                    if '=' in body:
                        problems.append(f'6 {name}: {rel}:{line} defines it (an extern with an initialiser): a header '
                                        f'declares a variable, and one C unit defines it')
                else:
                    local_decl[name].append((rel, line, sig, bounds, ok))
        if rel.endswith('.c'):
            for a, e in file_statements(s):
                t = ' '.join(s[a:e].split())
                if not t or re.match(r'(extern|static|typedef)\b', t):
                    continue
                for name, sig, bounds in declarators(t):
                    definition.setdefault(name, (rel, sig, bounds))
        else:
            # 6. a header holds no variable definition (a tentative definition, a static variable)
            for a, e in file_statements(s):
                t = ' '.join(s[a:e].split())
                if not t or re.match(r'(extern|typedef)\b', t):
                    continue
                for name, sig, bounds in declarators(t):
                    problems.append(f'6 {name}: {rel}:{text.count(chr(10), 0, a) + 1} defines it'
                                    f'{" (static)" if t.startswith("static") else ""}: a header declares a variable '
                                    f'with extern, and one C unit defines it')
    # 1. one declaration
    for name, ds in sorted(header_decl.items()):
        hs = sorted({h for h, s, b in ds})
        if len(hs) > 1:
            problems.append(f'1 {name}: declared in {len(hs)} headers: {", ".join(hs)}')
        elif len(ds) > 1:
            problems.append(f'1 {name}: declared {len(ds)} times in {hs[0]}')
    # 2. declared where it is defined
    for name, (unit, sig, bounds) in sorted(definition.items()):
        if unit in LIBRARY or not unit.startswith('src/'):
            continue
        if name not in header_decl:
            problems.append(f'2 {name}: {unit} defines it; no header declares it')
    # 3. no local declarations
    allowed = 0
    closure = {}
    fileset = set(files)
    for name, ds in sorted(local_decl.items()):
        for f, line, sig, bounds, ok in ds:
            if f in LIBRARY:
                continue
            if ok:
                if f not in closure:
                    closure[f] = include_closure(f, includes, fileset)
                seen = sorted({h for h, s, b in header_decl.get(name, []) if h in closure[f]})
                if seen:
                    problems.append(f'3 {name}: declared in {f}:{line} with a stated reason, but {f} includes '
                                    f'{", ".join(seen)}, which declares it: the reason is for a header the file does '
                                    f'not include')
                elif (f, name) not in LOCAL_DOCUMENTED:
                    problems.append(f'3 {name}: declared in {f}:{line} with a stated reason, but not among the '
                                    f'documented exceptions (docs/DECLARATIONS.md; a new one is listed there and in '
                                    f'LOCAL_DOCUMENTED in tools/check_decls.py, together)')
                else:
                    allowed += 1
            else:
                problems.append(f'3 {name}: declared in {f}:{line} (declare it once, in a header; or say why it stays: '
                                f'`/* {LOCAL_OK} <reason> */`)')
    # 4. one type
    every = collections.defaultdict(list)
    for name, ds in header_decl.items():
        every[name] += [(h, s, b) for h, s, b in ds]
    for name, ds in local_decl.items():
        every[name] += [(f, s, b) for f, line, s, b, ok in ds]
    for name, (unit, sig, bounds) in definition.items():
        if name in every:
            every[name].append((unit, sig, bounds))
    for name, ds in sorted(every.items()):
        sigs = sorted({s for f, s, b in ds})
        if len(sigs) > 1:
            problems.append(f'4 {name}: types differ: ' + '; '.join(f'{s} ({", ".join(sorted({f for f, x, b in ds if x == s}))})' for s in sigs))
            continue
        width = max(len(b) for f, s, b in ds)
        for k in range(width):
            given = {bound(b[k]) for f, s, b in ds if len(b) > k and b[k]}
            if len(given) > 1:
                problems.append(f'4 {name}: array bound {k + 1} differs: {", ".join(str(x) for x in sorted(given, key=str))}')
    # 5. checked by the compiler
    for name, (unit, sig, bounds) in sorted(definition.items()):
        if name in header_decl and unit not in LIBRARY:
            h = header_decl[name][0][0]
            if h.replace('include/', '', 1) not in includes.get(unit, set()):
                problems.append(f'5 {name}: {unit} defines it but does not include {h}, which declares it')
    n_decl = sum(len(v) for v in header_decl.values())
    print(f'check_decls: {len(header_decl)} variables declared in headers ({n_decl} declarations), '
          f'{sum(1 for n in definition if n in header_decl)} of them defined in C; '
          f'{sum(len(v) for v in local_decl.values())} declarations in C files ({allowed} with a stated reason); '
          f'{len(problems)} problem(s)')
    for p in problems[:200]:
        print('  rule ' + p)
    if len(problems) > 200:
        print(f'  ... and {len(problems) - 200} more')
    return 1 if problems else 0


if __name__ == '__main__':
    sys.exit(main())
