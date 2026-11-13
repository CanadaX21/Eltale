#!/usr/bin/env python3
"""tools/verify.py: check a finished build on its own terms. Run it from the repository root after `ninja`:

    python3 tools/verify.py

It uses only the Python standard library and reads only your baserom.jp.z64, the repository's files and the build's
outputs (the ROM, the linker map and the compiled objects). It checks that:

  1. build/eltail.jp.z64 is byte-identical to baserom.jp.z64, with the SHA-1 in eltail.jp.sha1;
  2. no C file of the main code (src/game, src/sched, src/libultra) and no project header in include/ has active
     assembly: no GLOBAL_ASM / INCLUDE_ASM / INCLUDE_RODATA pragma or macro, no asm statement (comments are ignored;
     the SDK headers in include/2.0I are not project code and are not scanned). The overlays' C files
     (src/title_overlay, src/intro_overlay) may include a function still in assembly with a GLOBAL_ASM pragma: they
     are game code outside the main code region, counted on their own at the end;
  3. the splat config (eltail.jp.yaml) splits the code region into C translation units and assembly as stated:
     the C units in one run, assembly only for the boot entry before them and the SDK library code after them;
  4. the linker map puts code from objects compiled from src/*.c, and nothing else, in the C units' address range;
  5. the compiled C objects of the main code define the expected number of functions, and their sizes give the
     coverage figures printed at the end;
  6. the overlays' objects hold the expected functions, compiled from C or still in assembly.

It exits with status 1 if any check fails. What it does not do: it cannot tell whether the C is the original
authors' text (names, layout), only that the original compiler turns it into the original bytes.
"""
import glob, hashlib, os, re, struct, sys

EXPECTED = dict(
    c_units=40, functions=530, function_bytes=224_616, c_unit_bytes=224_864,
    entry_bytes=80, sdk_asm_segments=121, sdk_asm_bytes=83_664,
    # the overlays (game code outside the main code region): (functions compiled from C, functions in all)
    overlays={'title_overlay': (7, 8), 'intro_overlay': (0, 32)},
)
MAIN_DIRS = ('src/game/', 'src/sched/', 'src/libultra/')
ROM_BASE, VRAM_BASE = 0x1000, 0x80000400          # the main code segment: ROM offset 0x1000 runs at 0x80000400

results = []


def report(ok, text):
    results.append(ok)
    print('%-4s %s' % ('OK' if ok else 'FAIL', text))


def strip_c_comments(src):
    """Remove // and /* */ comments, keeping string and character literals intact."""
    out, i, n = [], 0, len(src)
    while i < n:
        c = src[i]
        if c in '"\'':
            j = i + 1
            while j < n and src[j] != c:
                j += 2 if src[j] == '\\' else 1
            out.append(src[i:j + 1]); i = j + 1
        elif src.startswith('//', i):
            j = src.find('\n', i); i = n if j < 0 else j
        elif src.startswith('/*', i):
            j = src.find('*/', i + 2); i = n if j < 0 else j + 2; out.append(' ')
        else:
            out.append(c); i += 1
    return ''.join(out)


def read_elf_functions(path):
    """(text_size, [(name, size)]) for the STT_FUNC symbols defined in .text of a 32-bit big-endian ELF object."""
    data = open(path, 'rb').read()
    if data[:4] != b'\x7fELF' or data[4] != 1 or data[5] != 2:
        raise ValueError('%s is not a 32-bit big-endian ELF file' % path)
    shoff, = struct.unpack_from('>I', data, 0x20)
    shentsize, shnum, shstrndx = struct.unpack_from('>HHH', data, 0x2E)
    secs = [struct.unpack_from('>10I', data, shoff + k * shentsize) for k in range(shnum)]
    def cstr(off):
        return data[off:data.index(b'\0', off)].decode('ascii', 'replace')
    names = [cstr(secs[shstrndx][4] + s[0]) for s in secs]
    text = names.index('.text') if '.text' in names else None
    funcs = []
    for s in secs:
        if s[1] != 2:                                            # SHT_SYMTAB
            continue
        strtab = secs[s[6]]
        for k in range(s[5] // (s[9] or 16)):
            nm, value, size, info, other, shndx = struct.unpack_from('>IIIBBH', data, s[4] + k * (s[9] or 16))
            if (info & 0xF) == 2 and shndx == text:              # STT_FUNC, defined in .text
                funcs.append((cstr(strtab[4] + nm), size))
    return (secs[text][5] if text is not None else 0), funcs


def main():
    # 1. the ROM -----------------------------------------------------------------------------------------------------
    want = open('eltail.jp.sha1').read().split()[0].lower()
    built = open('build/eltail.jp.z64', 'rb').read()
    base = open('baserom.jp.z64', 'rb').read()
    got = hashlib.sha1(built).hexdigest()
    report(built == base and got == want,
           'ROM: build/eltail.jp.z64 SHA-1 %s, %s baserom.jp.z64 (expected %s)'
           % (got, 'identical to' if built == base else 'DIFFERENT from', want))

    # 2. no active assembly in the project's C ------------------------------------------------------------------
    pat = re.compile(r'#\s*pragma\s+GLOBAL_ASM|\bINCLUDE_ASM\b|\bINCLUDE_RODATA\b|\b__asm__\b|\basm\s*(volatile\s*)?\(')
    headers = sorted(h for h in glob.glob('include/**/*.h', recursive=True) if not h.replace(os.sep, '/').startswith('include/2.0I/'))
    cfiles = sorted(f.replace(os.sep, '/') for f in glob.glob('src/**/*.c', recursive=True))
    main_files = [f for f in cfiles if f.startswith(MAIN_DIRS)]
    ovl_files = [f for f in cfiles if f not in main_files]
    hits, ovl_asm = [], {}
    for f in main_files + headers:
        code = strip_c_comments(open(f, encoding='utf-8', errors='replace').read())
        hits += ['%s: %s' % (f, m.group(0)) for m in pat.finditer(code)]
    for f in ovl_files:
        code = strip_c_comments(open(f, encoding='utf-8', errors='replace').read())
        names = re.findall(r'#\s*pragma\s+GLOBAL_ASM\(\s*"[^"]*/([A-Za-z_]\w*)\.s"\s*\)', code)
        ovl_asm[f] = names
        # an overlay's file may include its functions still in assembly (#pragma GLOBAL_ASM("...")); any other
        # assembly in it is a finding
        rest = re.sub(r'#\s*pragma\s+GLOBAL_ASM\(\s*"[^"]*"\s*\)', '', code)
        hits += ['%s: %s' % (f, m.group(0)) for m in pat.finditer(rest)]
    report(not hits, 'source: %d C files of the main code and %d project headers, %d with active assembly%s; the '
           'overlays\' %d C files include %d functions still in assembly (GLOBAL_ASM)'
           % (len(main_files), len(headers), len(hits), (': ' + '; '.join(hits[:5])) if hits else '',
              len(ovl_files), sum(len(v) for v in ovl_asm.values())))

    # 3. the splat config's code region ------------------------------------------------------------------------------
    subs = []
    for line in open('eltail.jp.yaml', encoding='utf-8'):
        m = re.match(r'\s*- \[0x([0-9A-Fa-f]+),\s*([A-Za-z_.]+)(?:,\s*"?([\w/.]+)"?)?\]', line)
        if m:
            subs.append((int(m.group(1), 16), m.group(2), m.group(3)))
    code = []
    for k, (start, kind, name) in enumerate(subs[:-1]):
        if kind not in ('c', 'asm', 'hasm'):
            break                                                # the first data subsegment ends the code region
        code.append((start, subs[k + 1][0], kind, name))
    c_spans = [(a, b) for a, b, kind, _ in code if kind == 'c']
    asm_spans = [(a, b, name) for a, b, kind, name in code if kind != 'c']
    c_lo, c_hi = c_spans[0][0], c_spans[-1][1]
    contiguous = all(c_spans[k][1] == c_spans[k + 1][0] for k in range(len(c_spans) - 1))
    before = [s for s in asm_spans if s[1] <= c_lo]
    after = [s for s in asm_spans if s[0] >= c_hi]
    inside = [s for s in asm_spans if s not in before and s not in after]
    entry_bytes = sum(b - a for a, b, _ in before)
    sdk_bytes = sum(b - a for a, b, _ in after)
    c_bytes = c_hi - c_lo
    ok = (contiguous and not inside and len(c_spans) == EXPECTED['c_units'] and c_bytes == EXPECTED['c_unit_bytes']
          and entry_bytes == EXPECTED['entry_bytes'] and len(after) == EXPECTED['sdk_asm_segments']
          and sdk_bytes == EXPECTED['sdk_asm_bytes'])
    report(ok, 'split: code 0x%X-0x%X; %d C units in one run 0x%X-0x%X (%s bytes); assembly: %d segment(s) before '
               'them (%d bytes, the boot entry), %d after them (%s bytes, SDK library code), %d among them'
           % (code[0][0], code[-1][1], len(c_spans), c_lo, c_hi, format(c_bytes, ','), len(before), entry_bytes,
              len(after), format(sdk_bytes, ','), len(inside)))

    # 4. the linker map ------------------------------------------------------------------------------------------------
    v_lo, v_hi = c_lo - ROM_BASE + VRAM_BASE, c_hi - ROM_BASE + VRAM_BASE
    mp = open('build/eltail.jp.map', encoding='utf-8', errors='replace').read()
    pieces = [(int(a, 16), int(s, 16), obj) for a, s, obj in
              re.findall(r'^\s*\.text\s+0x([0-9a-f]+)\s+0x([0-9a-f]+)\s+(\S+\.o)\s*$', mp, re.M)]
    in_range = sorted(p for p in pieces if v_lo <= p[0] < v_hi and p[1])
    foreign = [obj for _, _, obj in in_range if not re.match(r'build/src/.+\.c\.o$', obj)]
    covered = sum(s for _, s, _ in in_range)
    gaps = any(in_range[k][0] + in_range[k][1] != in_range[k + 1][0] for k in range(len(in_range) - 1))
    report(not foreign and not gaps and covered == c_bytes and in_range and in_range[0][0] == v_lo,
           'link: 0x%08X-0x%08X holds %d .text sections, %s bytes, all from objects compiled from src/*.c%s'
           % (v_lo, v_hi, len(in_range), format(covered, ','),
              '' if not foreign else '; NOT from C: ' + ', '.join(foreign[:5])))

    # 5. the functions compiled from C --------------------------------------------------------------------------------
    objs = sorted(o.replace(os.sep, '/') for o in glob.glob('build/src/**/*.c.o', recursive=True))
    nfunc = fbytes = tbytes = 0
    main_objs = 0
    for o in objs:
        if not o[len('build/'):].startswith(MAIN_DIRS):
            continue
        tsize, funcs = read_elf_functions(o)
        if not tsize:
            continue                                             # a data unit (the boss tables): no code
        main_objs += 1
        tbytes += tsize
        nfunc += len(funcs)
        fbytes += sum(size for _, size in funcs)
    report(main_objs == EXPECTED['c_units'] and nfunc == EXPECTED['functions'] and fbytes == EXPECTED['function_bytes']
           and tbytes == c_bytes,
           'objects: %d compiled C objects of the main code define %d functions, %s bytes of function code; their .text '
           'is %s bytes (the other %d bytes are the zero words that pad each object\'s code to 16 bytes)'
           % (main_objs, nfunc, format(fbytes, ','), format(tbytes, ','), tbytes - fbytes))

    # 6. the overlays ------------------------------------------------------------------------------------------------
    ovl = {}
    for o in objs:
        rel = o[len('build/'):-2]                                # src/title_overlay/title_screen.c
        if rel.startswith(MAIN_DIRS):
            continue
        folder = rel.split('/')[1]
        tsize, funcs = read_elf_functions(o)
        asm = set(ovl_asm.get(rel, []))
        c_f = [(n, s) for n, s in funcs if n not in asm]
        a_f = [(n, s) for n, s in funcs if n in asm]
        t = ovl.setdefault(folder, [0, 0, 0, 0])
        t[0] += len(c_f); t[1] += len(c_f) + len(a_f); t[2] += sum(s for _, s in c_f); t[3] += sum(s for _, s in c_f + a_f)
    got_ovl = {k: (v[0], v[1]) for k, v in ovl.items()}
    report(got_ovl == EXPECTED['overlays'],
           'overlays: %s' % '; '.join('%s %d of %d functions compiled from C (%s of %s bytes of function code)'
                                      % (k, v[0], v[1], format(v[2], ','), format(v[3], ',')) for k, v in sorted(ovl.items())))

    total = c_bytes + entry_bytes + sdk_bytes
    print()
    print('Coverage (from this build):')
    print('  functions compiled from C:   %d (%s bytes of function code)' % (nfunc, format(fbytes, ',')))
    print('  C units in the ROM:          %s bytes (functions + %d bytes of alignment padding)'
          % (format(c_bytes, ','), tbytes - fbytes))
    print('  boot entry (assembly):       %d bytes' % entry_bytes)
    print('  SDK library code (assembly): %s bytes in %d segments, %.1f%% of the %s bytes of executable code'
          % (format(sdk_bytes, ','), len(after), 100.0 * sdk_bytes / total, format(total, ',')))
    print('  the main code region is 0x1000-0x4C580; game code outside it (the overlays):')
    for k, v in sorted(ovl.items()):
        print('    %-14s %d of %d functions compiled from C (%s of %s bytes of function code)'
              % (k, v[0], v[1], format(v[2], ','), format(v[3], ',')))
    print()
    if all(results):
        print('RESULT: OK')
        return 0
    print('RESULT: FAIL')
    return 1


if __name__ == '__main__':
    try:
        sys.exit(main())
    except FileNotFoundError as e:
        sys.exit('verify: %s not found: run it from the repository root after a successful ./configure && ninja'
                 % e.filename)
