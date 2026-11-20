#!/usr/bin/env python3
"""bss_order.py: keeps each C unit's .bss in the original game's order under IDO 5.3.

    python3 tools/bss_order.py check [UNIT...]    exit 1 if a unit's .bss would differ from the original layout
    python3 tools/bss_order.py fix [UNIT...]      rewrite the BSS_ORDER_PAD values of the units that differ
    python3 tools/bss_order.py show UNIT          block numbers, the predicted and the original layout
    python3 tools/bss_order.py built              the built objects' .bss against the original layout
UNIT is a C file's path under src/ or its name without .c; none means every C unit under src/. Run from the project
root after ./configure and a first ninja run (the compile commands come from build.ninja, and some units include
headers the build generates).

Why: IDO's front end gives every symbol a block number ("dense number") in the order it meets the symbol in the
compilation unit, and its code generator lays a unit's .bss out by block number modulo 256. With -g, every new
declaration takes a block number (an extern, a typedef, a struct or enum, a forward struct declaration; a prototype,
a repeated declaration and a #define take none), headers included. So a header that gains or loses declarations moves
the .bss of every -g unit that includes it, and the reference build no longer matches. BSS_ORDER_PAD(n)
(include/bss_order.h) takes n more block numbers; this tool computes the values that give each unit its original
layout and writes them into the source.

How: the unit is prepared exactly as the build prepares it (asm-processor, the flags of its rule in build.ninja) and
the compiler is stopped after the front end (cc -Hf), which leaves the symbol table (.T) and the u-code (.B). The
.bss variables are the u-code's gsym, lsym and fsym symbols that nothing initialises. The layout rule: variables whose
address a .data or .rodata initialiser takes come first, in the order of those initialisers; the others by block
number mod 256, ties going to file-scope variables in block order, then function statics in reverse block order;
each aligned to its type, and to 8 when it is 8 bytes or larger. The original layout is the project's own record:
each unit's .bss start in eltail.jp.yaml and each variable's original address in symbol_addrs.txt (function statics
included). To fix a unit, the tool tries pad values in a fixed order (so a given tree always gets the same values)
and keeps the first set that reproduces the original layout.

The symbol-table reading and the layout rule follow zeldaret/oot's tools/ido_block_numbers.py and tools/fix_bss.py
(SPDX-License-Identifier: CC0-1.0), written for IDO 7.1; checked against IDO 5.3 on every unit of this project.
Standard library only."""
import os, re, shutil, struct, subprocess, sys, tempfile
from dataclasses import dataclass
from typing import Optional

ROOT = os.getcwd()
PAD_RE = re.compile(r'^BssOrderPad_(\d+)_([a-hz][01]*)$')
PAD_CALL_RE = re.compile(r'\bBSS_ORDER_PAD\(\s*(\d+)\s*\)')
# Function statics that share their name with another static of the unit, which symbol_addrs.txt cannot tell apart:
# their original offsets in the unit's .bss, by (function, name): a function's name is unique in the program, so no
# rename of a file changes the key. Read from the matched object's relocations, whose bytes are the original ROM's:
# QueueSound stores to .bss+0x2C20, PushSoundQueue to .bss+0x2C24.
STATIC_ORIGINALS = {('QueueSound', 'i'): 0x2C20, ('PushSoundQueue', 'i'): 0x2C24}

# --- the front end's output -----------------------------------------------------------------------------------------

HDR = struct.Struct('>2H23I')
FDR = struct.Struct('>10I2H7I')
SYM = struct.Struct('>3I')


@dataclass
class Entry:
    name: str
    st: int


def parse_symtab(data):
    h = HDR.unpack_from(data, 0)
    idnMax, cbDnOffset, cbSymOffset = h[5], h[6], h[10]
    cbSsOffset, cbSsExtOffset, ifdMax, cbFdOffset, cbExtOffset = h[16], h[18], h[19], h[20], h[24]
    fds = [FDR.unpack_from(data, cbFdOffset + i * FDR.size) for i in range(ifdMax)]
    cstr = lambda o: data[o:data.index(b'\0', o)].decode('latin-1')
    out = []
    for i in range(idnMax):
        ifd, isym = struct.unpack_from('>II', data, cbDnOffset + i * 8)
        if isym == 0xFFFFF:
            out.append(Entry('', -1))
        elif ifd == 0x7FFFFFFF:
            iss, value, flags = SYM.unpack_from(data, cbExtOffset + isym * 0x10 + 4)
            out.append(Entry(cstr(cbSsExtOffset + iss), flags >> 26))
        else:
            fd = fds[ifd]
            iss, value, flags = SYM.unpack_from(data, cbSymOffset + (fd[4] + isym) * SYM.size)
            out.append(Entry(cstr(cbSsOffset + fd[2] + iss), flags >> 26))
    return out


# u-code: name, length in words, has a constant (the order is the opcode)
_OPS = ('abs 2 0|add 2 0|adj 4 0|aent 4 0|and 2 0|aos 2 0|asym 4 0|bgn 4 0|bgnb 2 0|bsub 2 0|cg1 2 0|cg2 2 0|chkh 2 0|'
        'chkl 2 0|chkn 2 0|chkt 2 0|cia 4 1|clab 4 0|clbd 2 0|comm 4 1|csym 4 0|ctrl 4 0|cubd 2 0|cup 4 0|cvt 4 0|'
        'cvtl 2 0|dec 2 0|def 4 0|dif 4 0|div 2 0|dup 2 0|end 2 0|endb 2 0|ent 4 0|ueof 2 0|equ 2 0|esym 4 0|fill 4 0|'
        'fjp 2 0|fsym 4 0|geq 2 0|grt 2 0|gsym 4 0|hsym 4 0|icuf 4 0|idx 2 0|iequ 4 0|igeq 4 0|igrt 4 0|ijp 2 0|'
        'ilda 6 0|ildv 4 0|ileq 4 0|iles 4 0|ilod 4 0|inc 2 0|ineq 4 0|init 6 1|inn 4 0|int 4 0|ior 2 0|isld 4 0|'
        'isst 4 0|istr 4 0|istv 4 0|ixa 2 0|lab 4 0|lbd 2 0|lbdy 2 0|lbgn 2 0|lca 4 1|lda 6 0|ldap 2 0|ldc 4 1|'
        'ldef 4 0|ldsp 2 0|lend 2 0|leq 2 0|les 2 0|lex 2 0|lnot 2 0|loc 2 0|lod 4 0|lsym 4 0|ltrm 2 0|max 2 0|'
        'min 2 0|mod 2 0|mov 4 0|movv 2 0|mpmv 4 0|mpy 2 0|mst 2 0|mus 4 0|neg 2 0|neq 2 0|nop 2 0|not 2 0|odd 2 0|'
        'optn 4 0|par 4 0|pdef 4 0|pmov 4 0|pop 2 0|regs 4 0|rem 2 0|ret 2 0|rlda 4 0|rldc 4 1|rlod 4 0|rnd 4 0|'
        'rpar 4 0|rstr 4 0|sdef 4 0|sgs 4 0|shl 2 0|shr 2 0|sign 2 0|sqr 2 0|sqrt 2 0|ssym 4 1|step 2 0|stp 2 0|'
        'str 4 0|stsp 2 0|sub 2 0|swp 4 0|tjp 2 0|tpeq 2 0|tpge 2 0|tpgt 2 0|tple 2 0|tplt 2 0|tpne 2 0|typ 4 0|'
        'ubd 2 0|ujp 2 0|unal 2 0|uni 4 0|vreg 4 0|xjp 8 0|xor 2 0|xpar 2 0|mtag 2 0|alia 2 0|ildi 4 0|isti 4 0|'
        'irld 4 0|irst 4 0|ldrc 4 0|msym 4 0|rcuf 4 0|ksym 4 0|osym 4 0|irlv 2 0|irsv 2 0')
OPS = [(n, int(l), h == '1') for n, l, h in (x.split() for x in _OPS.split('|'))]


def parse_ucode(data):
    """[(name, dtype, lexlev, i1, args, const)]"""
    ops, pos, n = [], 0, len(data)
    while pos < n:
        opcode, b1 = data[pos], data[pos + 1]
        dtype = b1 & 0x1F
        lexlev = int.from_bytes(data[pos + 2:pos + 4], 'big')
        i1 = int.from_bytes(data[pos + 4:pos + 8], 'big')
        pos += 8
        name, length, has_const = OPS[opcode]
        args = [int.from_bytes(data[pos + 4 * k:pos + 4 * k + 4], 'big') for k in range(length - 2)]
        pos += 4 * (length - 2)
        const = None
        if has_const:
            const = int.from_bytes(data[pos:pos + 4], 'big')
            pos += 8
            if dtype in (9, 12, 13, 14, 16) or name == 'comm':
                pos += (const + 7) & ~7
        ops.append((name, dtype, lexlev, i1, args, const))
    return ops


# --- the layout rule ------------------------------------------------------------------------------------------------

@dataclass
class Var:
    block: int
    top: bool
    name: str
    size: int
    align: int
    in_data: bool
    func: str


def bss_variables(symtab, ucode):
    init = {op[3] for op in ucode if op[0] == 'init'}
    in_data, out, func = [], [], ''
    for name, dtype, lexlev, i1, args, const in ucode:
        if name in ('gsym', 'lsym', 'fsym'):
            if i1 in init:
                continue
            size = args[0]
            # the code generator aligns a .bss variable by its size, not its type: the largest power of two not
            # above the size, at most 8 (measured on IDO 5.3 test units: a char[3] or a struct of three
            # bytes gets 2, where the u-code records 4)
            align = 8 if size >= 8 else 4 if size >= 4 else 2 if size >= 2 else 1
            out.append(Var(i1, name != 'fsym', symtab[i1].name, size, align, i1 in in_data,
                           '' if name != 'fsym' else func))
        elif name == 'init' and dtype == 10:
            in_data.append(const)
        elif name == 'ent':
            func = symtab[i1].name
    order = {}
    for i, b in enumerate(in_data):
        order.setdefault(b, i)
    return out, order


def predict(variables, data_order, shift=None):
    """[(Var, offset)] in .bss order; shift(var) adds to a variable's block number (a trial pad value)"""
    blk = (lambda v: v.block + shift(v)) if shift else (lambda v: v.block)
    first = sorted((v for v in variables if v.in_data), key=lambda v: data_order[v.block])
    rest = sorted((v for v in variables if not v.in_data),
                  key=lambda v: (blk(v) % 256, not v.top, blk(v) if v.top else -blk(v)))
    out, off = [], 0
    for v in first + rest:
        off = (off + v.align - 1) & ~(v.align - 1)
        out.append((v, off))
        off += v.size
    return out


# --- the project ----------------------------------------------------------------------------------------------------

class Project:
    def __init__(self, root):
        self.root = root = os.path.abspath(root)
        text = open(os.path.join(root, 'build.ninja'), encoding='utf-8').read().replace('$\n', '')
        self.vars, self.rules, self.builds = {}, {}, {}
        cur = None
        for line in text.split('\n'):
            if line.startswith('rule '):
                cur = line.split()[1]; self.rules[cur] = {}; continue
            if line.startswith('build '):
                cur = None
                m = re.match(r'build (\S+): (\S+)\s+(\S+)', line)
                if m:
                    self.builds[m.group(3)] = (m.group(1), m.group(2))
                continue
            m = re.match(r'(\s*)(\w+) = (.*)$', line)
            if m and m.group(1) and cur:
                self.rules[cur][m.group(2)] = m.group(3)
            elif m and not m.group(1):
                self.vars[m.group(2)] = m.group(3); cur = None
        self.units = sorted(s for s, (o, r) in self.builds.items()
                            if r.endswith('_cc') and s.startswith('src/') and s.endswith('.c'))
        self.bss_start, starts = {}, []
        for line in open(os.path.join(root, 'eltail.jp.yaml'), encoding='utf-8'):
            m = re.search(r'type:\s*(\.?bss),\s*vram:\s*(0x[0-9A-Fa-f]+),\s*name:\s*"?([\w/]+)"?', line)
            if m:
                starts.append(int(m.group(2), 16))
                if m.group(1) == '.bss':
                    self.bss_start[m.group(3)] = int(m.group(2), 16)
        self.starts = sorted(set(starts))
        self.addrs = {}
        for line in open(os.path.join(root, 'symbol_addrs.txt'), encoding='utf-8'):
            m = re.match(r'\s*(\w+)\s*=\s*(0x[0-9A-Fa-f]+)\s*;', line)
            if m:
                self.addrs.setdefault(m.group(1), []).append(int(m.group(2), 16))

    def expand(self, s):
        for _ in range(6):
            s = re.sub(r'\$(\w+)', lambda m: self.vars.get(m.group(1), '$' + m.group(1)), s)
        return s

    def unit_source(self, unit):
        if unit in self.builds:
            return unit
        hits = [s for s in self.units if os.path.basename(s)[:-2] == unit]
        if len(hits) != 1:
            raise SystemExit(f'bss_order: no single C unit called {unit}')
        return hits[0]

    def g_debug(self, src):
        """True when the unit is compiled with -g (only then do declarations take block numbers)"""
        return '-g3' in self.rules[self.builds[src][1]]['command'] or ' -g ' in self.rules[self.builds[src][1]]['command']

    def compile_args(self, src):
        cmd = self.rules[self.builds[src][1]]['command']
        m = re.search(r'-c -32 (.*) -o \$out \$in', cmd)
        args = self.expand(m.group(1)).split()
        out, i = ['-c', '-32'], 0
        while i < len(args):
            if args[i] == '-I':
                p = args[i + 1]
                out += ['-I', p if p.startswith('/') else os.path.normpath(os.path.join(self.root, p))]
                i += 2
            else:
                out.append(args[i]); i += 1
        return out

    def front_end(self, src, keep=None):
        args = self.compile_args(src)
        sys.path.insert(0, os.path.join(self.root, 'tools/asm-processor'))
        import asm_processor
        opt = [x for x in args if x in {'-g3', '-g', '-O0', '-O1', '-O2', '-framepointer', '-KPIC'}]
        if '-mips2' not in args:
            opt.append('-mips1')
        stem = os.path.basename(src)[:-2]
        td = keep or tempfile.mkdtemp(prefix='bss_order_')
        os.makedirs(td, exist_ok=True)
        pre = os.path.join(td, stem + '.c')
        here = os.getcwd()
        os.chdir(self.root)
        try:
            with open(pre, 'wb') as f:
                asm_processor.run(self.expand('$ASM_PROC_FLAGS').split() + opt + [os.path.join(self.root, src)],
                                  outfile=f)
        finally:
            os.chdir(here)
        cc = os.path.join(self.root, self.expand('$ido_cc'))
        r = subprocess.run([cc] + args + ['-I', os.path.dirname(os.path.join(self.root, src)), '-Hf', pre],
                           cwd=td, capture_output=True, text=True)
        try:
            symtab = parse_symtab(open(os.path.join(td, stem + '.T'), 'rb').read())
            ucode = parse_ucode(open(os.path.join(td, stem + '.B'), 'rb').read())
        except OSError:
            hint = ('\n(a header the build generates is missing: run ninja once first)'
                    if 'build/include' in r.stderr else '')
            raise SystemExit(f'bss_order: the front end failed on {src} (exit {r.returncode}):\n{r.stderr[-3000:]}{hint}')
        finally:
            if not keep:
                shutil.rmtree(td, ignore_errors=True)
        return symtab, ucode

    def original_offsets(self, src, variables):
        """{id(var): original offset}; names the record does not place are left out"""
        # the config names a unit of main by its path under src/, folders included where the units are kept in
        # folders, and an overlay's unit by its file name, relative to the overlay's folder
        rel = os.path.relpath(src, 'src').replace(os.sep, '/')[:-2]
        unit = rel if rel in self.bss_start else os.path.basename(src)[:-2]
        start = self.bss_start.get(unit)
        if start is None:
            return {}, [v.name for v in variables]
        later = [s for s in self.starts if s > start]
        end = later[0] if later else start + 0x100000
        out, unknown = {}, []
        for v in variables:
            if v.func and (v.func, v.name) in STATIC_ORIGINALS:
                out[id(v)] = STATIC_ORIGINALS[(v.func, v.name)]
                continue
            hits = sorted({a - start for a in self.addrs.get(v.name, []) if start <= a < end})
            m = re.match(r'D_([0-9A-F]{8})(?:_[0-9A-F]+)?$', v.name)
            if not hits and m and start <= int(m.group(1), 16) < end:
                hits = [int(m.group(1), 16) - start]          # a placeholder's name is its address
            if sum(1 for w in variables if w.name == v.name) > 1:
                hits = []                                      # two statics of one name: the record can't tell them apart
            if len(hits) == 1:
                out[id(v)] = hits[0]
            else:
                unknown.append(v.name)
        return out, unknown


@dataclass
class Unit:
    src: str
    variables: list
    data_order: dict
    pads: list          # [(line, first block number, count)]
    original: dict
    unknown: list
    debug: bool
    pad_problems: list


def source_pads(path):
    """{line: [values]} of the BSS_ORDER_PAD calls in a file, comments left out"""
    text = open(path, encoding='utf-8', errors='replace').read()
    text = re.sub(r'/\*.*?\*/', lambda m: re.sub(r'[^\n]', ' ', m.group(0)), text, flags=re.S)
    text = re.sub(r'//[^\n]*', '', text)
    out = {}
    for k, line in enumerate(text.split('\n'), 1):
        for m in PAD_CALL_RE.finditer(line):
            out.setdefault(k, []).append(int(m.group(1)))
    return out


def pad_depths(path):
    """{line: brace depth} of the lines that hold a BSS_ORDER_PAD call (0 = file scope); comments, strings and
    character constants left out"""
    text = open(path, encoding='utf-8', errors='replace').read()
    text = re.sub(r'/\*.*?\*/|//[^\n]*|"(?:\\.|[^"\\\n])*"|\'(?:\\.|[^\'\\\n])*\'',
                  lambda m: re.sub(r'[^\n]', ' ', m.group(0)), text, flags=re.S)
    out, depth = {}, 0
    for k, line in enumerate(text.split('\n'), 1):
        m = PAD_CALL_RE.search(line)
        if m:
            out[k] = depth + line[:m.start()].count('{') - line[:m.start()].count('}')
        depth += line.count('{') - line.count('}')
    return out


def order_problems(u):
    """the unit's file-scope .bss must be declared in storage order (block numbers rising with the original offsets).
    A layout that matches only through a wrap, a declaration out of order put back in place by a pad that carries the
    state across a multiple of 256, is the count-based placement the declarations no longer use."""
    tops = sorted((u.original[id(v)], v.block, v.name) for v in u.variables
                  if v.top and not v.in_data and id(v) in u.original)
    return [f'{b[2]} is declared before {a[2]} but stored after it' for a, b in zip(tops, tops[1:]) if b[1] <= a[1]]


def place_problems(u, depths):
    """where the slots stand: each at file scope; the first before the unit's file-scope .bss declarations; in a unit
    with .bss function statics, one after those declarations and before the statics"""
    out = [f'line {line}: BSS_ORDER_PAD inside a block (a function?): a slot stands at file scope'
           for line, d in sorted(depths.items()) if d]
    tops = [v.block for v in u.variables if v.top and not v.in_data]
    statics = [v.block for v in u.variables if not v.top and not v.in_data]
    if u.pads and tops and u.pads[0][1] > min(tops):
        out.append(f'line {u.pads[0][0]}: the first BSS_ORDER_PAD comes after the unit\'s .bss declarations: it stands '
                   f'before the unit\'s own header\'s #include')
    if statics and tops and not any(max(tops) < first and first + count <= min(statics) for line, first, count in u.pads):
        out.append('the unit has .bss function statics and no BSS_ORDER_PAD between its .bss declarations and them: '
                   'put a second BSS_ORDER_PAD(0) after its #include block')
    return out


def analyse(proj, src):
    symtab, ucode = proj.front_end(src)
    variables, data_order = bss_variables(symtab, ucode)
    first, count = {}, {}
    for i, e in enumerate(symtab):
        m = PAD_RE.match(e.name)
        if m:
            line = int(m.group(1))
            first.setdefault(line, i)
            count[line] = count.get(line, 0) + 1
    pads = sorted(((line, first[line], count[line]) for line in first), key=lambda p: p[1])
    original, unknown = proj.original_offsets(src, variables)
    debug = proj.g_debug(src)
    # every slot in the unit's file must be one group of the front end's tags, of its own size: a slot in a header
    # (its tags carry the header's line), two on one line, or one the compiler never saw (#if 0) would be inert or
    # misread
    problems = []
    written = source_pads(os.path.join(proj.root, src))
    for line, values in sorted(written.items()):
        if len(values) > 1:
            problems.append(f'line {line}: {len(values)} BSS_ORDER_PAD calls on one line (one a line)')
        elif debug and count.get(line) != (values[0] or 256):
            problems.append(f'line {line}: BSS_ORDER_PAD({values[0]}) took {count.get(line, 0)} block numbers, not '
                            f'{values[0] or 256} (inside #if, or not at file scope?)')
    for line in count:
        if line not in written:
            problems.append(f'block numbers from a BSS_ORDER_PAD at line {line} of another file (a header?): '
                            f'a slot belongs in the unit\'s own .c')
    u = Unit(src, variables, data_order, pads, original, unknown, debug, problems)
    if debug:
        problems += place_problems(u, pad_depths(os.path.join(proj.root, src)))
        if not pads and sum(1 for v in variables if not v.in_data) >= 2:
            problems.append('the unit has .bss and no BSS_ORDER_PAD slot: put BSS_ORDER_PAD(0) before its own '
                            'header\'s #include')
    return u


def differences(u, shift=None):
    return [(v, off, u.original[id(v)]) for v, off in predict(u.variables, u.data_order, shift)
            if id(v) in u.original and u.original[id(v)] != off]


def candidates(n):
    """every n-tuple of values 0-255, those with more trailing zero bits first (zeldaret/oot's order): the same tree
    always gets the same answer"""
    def seq(nu):
        if nu == 8:
            yield 0
        else:
            for i in range(1 << (7 - nu)):
                yield (2 * i + 1) << nu
    def impl(n, mn):
        if n == 1:
            yield from ((x,) for x in seq(mn))
            return
        for mna in reversed(range(mn + 1, 9)):
            for a in seq(mna):
                for b in impl(n - 1, mn):
                    yield (a, *b)
        for a in seq(mn):
            for mnb in reversed(range(mn, 9)):
                for b in impl(n - 1, mnb):
                    yield (a, *b)
    for nu in reversed(range(9)):
        yield from impl(n, nu)


def solve(u):
    if not u.pads:
        return None
    for values in candidates(len(u.pads)):
        counts = [v if v else 256 for v in values]
        def shift(var, counts=counts):
            return sum(c - p[2] for c, p in zip(counts, u.pads) if var.block >= p[1])
        if not differences(u, shift):
            return values
    return None


def rewrite(proj, src, pads, values):
    path = os.path.join(proj.root, src)
    lines = open(path, encoding='utf-8').read().split('\n')
    for (line, first, count), value in zip(pads, values):
        text = lines[line - 1]
        if not PAD_CALL_RE.search(text):
            raise SystemExit(f'bss_order: {src}:{line} should hold BSS_ORDER_PAD(...) (a pad inside a header?)')
        lines[line - 1] = PAD_CALL_RE.sub(f'BSS_ORDER_PAD({value})', text, count=1)
    open(path, 'w', encoding='utf-8', newline='\n').write('\n'.join(lines))


def pad_value(count):
    return 0 if count == 256 else count


def built_differences(proj, src):
    """the built object's named .bss symbols against their original offsets: ([(name, built, original)], the number
    compared), or None when the object is missing. Function statics have no symbol in the object: check and the ROM
    cover them."""
    obj = os.path.join(proj.root, proj.builds[src][0])
    if not os.path.exists(obj):
        return None
    sec = subprocess.run(['mips-linux-gnu-readelf', '-SW', obj], capture_output=True, text=True).stdout
    idx = next((m.group(1) for m in re.finditer(r'\[\s*(\d+)\]\s+\.bss\s', sec)), None)
    if idx is None:
        return [], 0
    syms = subprocess.run(['mips-linux-gnu-readelf', '-sW', obj], capture_output=True, text=True).stdout
    found = []
    for line in syms.split('\n'):
        p = line.split()
        if len(p) >= 8 and p[0].endswith(':') and p[6] == idx and p[3] == 'OBJECT':
            found.append(Var(0, True, p[7], int(p[2]), 1, False, ''))
            found[-1].block = int(p[1], 16)          # the built offset, kept in .block for the comparison
    original, unknown = proj.original_offsets(src, found)
    return ([(v.name, v.block, original[id(v)]) for v in found if id(v) in original and original[id(v)] != v.block],
            sum(1 for v in found if id(v) in original))


def statics_among(u):
    """[(static, the file-scope variable before it, the one after it)] in the original layout"""
    lay = sorted((u.original[id(v)], k, v) for k, v in enumerate(u.variables) if id(v) in u.original and not v.in_data)
    out = []
    for i, (off, k, v) in enumerate(lay):
        if not v.top:
            before = next((w for o, j, w in reversed(lay[:i]) if w.top), None)
            after = next((w for o, j, w in lay[i + 1:] if w.top), None)
            out.append((v, before, after))
    return out


def unsolved_text(u):
    st = statics_among(u)
    if not u.pads:
        return 'the unit has no BSS_ORDER_PAD slot: put BSS_ORDER_PAD(0) before its own header\'s #include'
    text = ('the unit\'s .bss declarations stay together in its header, in storage order: more than about 256 minus '
            'their span inserted between two of them cannot be absorbed')
    if st:
        # the rigid stretch: from the variable after the first static to the one before the last (in storage order);
        # with the statics all between the same two neighbours there is none
        after_first, before_last = st[0][2], st[-1][1]
        rigid = (after_first and before_last and u.original[id(after_first)] <= u.original[id(before_last)])
        text += ('; and its function statics sit among its file-scope .bss ('
                 + '; '.join(f'{v.func}::{v.name} between {b.name if b else "the start"} and '
                             f'{a.name if a else "the end"}' for v, b, a in st)
                 + '): ' + (f'the declarations from {after_first.name} to {before_last.name} stay consecutive in the '
                            f'header, and ' if rigid else '')
                 + 'the statics\' declarations in their functions keep their order and spacing (docs/DECLARATIONS.md, '
                   '"Units with .bss statics")')
    return text


def main(argv):
    if not argv or argv[0] not in ('check', 'fix', 'show', 'built'):
        print(__doc__.split('\n\n')[0]); return 2
    mode = argv[0]
    verbose = '-v' in argv[1:]
    names = [a for a in argv[1:] if a != '-v']
    proj = Project(ROOT)
    srcs = [proj.unit_source(n) for n in names] if names else proj.units
    bad = 0
    if mode == 'built':
        compared = 0
        for src in srcs:
            r = built_differences(proj, src)
            if r is None:
                bad += 1
                print(f'MISSING {src}: no built object ({proj.builds[src][0]}): build first')
                continue
            d, n = r
            compared += n
            if d:
                bad += 1
                print(f'MOVED {src}: ' + ', '.join(f'{n} at 0x{b:X}, originally 0x{o:X}' for n, b, o in d[:4])
                      + (f' (+{len(d) - 4} more)' if len(d) > 4 else ''))
        print(f'bss_order built: {len(srcs)} units, {compared} named .bss variables compared (function statics have '
              f'no symbol in the objects: check and the ROM cover them), {bad} with .bss away from the original or '
              f'not built')
        return 1 if bad else 0
    unknown, with_state, pads_total = [], 0, 0
    for src in srcs:
        u = analyse(proj, src)
        diff = differences(u)
        with_state += 1 if u.variables else 0
        pads_total += len(u.pads)
        unknown += [f'{os.path.basename(src)[:-2]}:{n}' for n in u.unknown]
        if mode == 'show':
            print(f'{src}: {len(u.variables)} .bss variables, pads {[(l, pad_value(c)) for l, f, c in u.pads]}'
                  f'{"" if u.debug else " (compiled without -g: declarations take no block numbers)"}')
            for v, off in predict(u.variables, u.data_order):
                o = u.original.get(id(v))
                mark = '  ' if o == off else ('??' if o is None else '!=')
                print(f'  {mark} 0x{off:05X} (original {"-" if o is None else hex(o)}) blk {v.block:5} '
                      f'[{v.block % 256:3}] {"file" if v.top else "func"}{" in-data" if v.in_data else ""} '
                      f'{v.func + "::" if v.func else ""}{v.name}')
            continue
        if source_pads(os.path.join(proj.root, src)) and not u.debug:
            print(f'FAIL {src}: BSS_ORDER_PAD in a unit compiled without -g has no effect; remove it'); bad += 1
            continue
        if u.pad_problems:
            print(f'FAIL {src}: ' + '; '.join(u.pad_problems)); bad += 1
            continue
        if u.unknown:
            # every .bss variable is compared with its original place, or the unit fails (the review: a
            # renamed variable without its record slipped past the storage-order rule and the layout check)
            print(f'FAIL {src}: {len(u.unknown)} .bss variables with no recorded original address '
                  f'({", ".join(u.unknown[:4])}): record each in symbol_addrs.txt (a function static that shares its '
                  f'name with another of the unit: in STATIC_ORIGINALS in tools/bss_order.py, by function)'); bad += 1
            continue
        order = order_problems(u)
        if order:
            print(f'FAIL {src}: its .bss is not declared in storage order ({"; ".join(order[:3])}): declare the '
                  f'unit\'s .bss in its header in the order it is stored (no pad value is written for an order that '
                  f'only matches by wrapping around 256)'); bad += 1
            continue
        if not diff:
            if verbose:
                print(f'OK   {src}: {len(u.variables)} .bss variables at their original offsets'
                      + (f', pads {[pad_value(c) for l, f, c in u.pads]}' if u.pads else ''))
            continue
        if mode == 'check':
            bad += 1
            print(f'FAIL {src}: {len(diff)} .bss variables away from their original offsets (first: '
                  + ', '.join(f'{v.name} at 0x{off:X}, originally 0x{o:X}' for v, off, o in diff[:3])
                  + f'); run: python3 tools/bss_order.py fix {src}')
            continue
        values = solve(u)
        if values is None:
            bad += 1
            print(f'FAIL {src}: no BSS_ORDER_PAD values reproduce the original layout: {unsolved_text(u)}')
            continue
        rewrite(proj, src, u.pads, values)
        print(f'FIXED {src}: pads {[pad_value(c) for l, f, c in u.pads]} -> {list(values)}')
    if mode != 'show':
        print(f'bss_order {mode}: {len(srcs)} units ({with_state} with .bss, {pads_total} BSS_ORDER_PAD slots), '
              f'{bad} failed')
    return 1 if bad else 0


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
