"""anim.py: splat's `anim` segment type, a model's animation records as typed C.

A subsegment of this type holds a run of animation records: in a boss file, everything from the file's start to its
first texture (each part's keyframe array, each animation's part table, the display-list table, the animation table);
in a monster set, one model's records. splat writes it as assets/<dir>/<name>.anim.inc.c, which configure.py compiles
like splat's .gfx.inc.c and .vtx.inc.c files. One definition of the format: the project's own C types. (The `records`
type, tools/splat_ext/records.py, is this writer under another name, for records that are not animation data.)
  - Every record in the run is a symbol in symbol_addrs.txt (its segment, address and size), declared with its C type
    in a header the generated C includes (DECL_HEADERS: `extern ModelKeyFrame gGuiltyAnim0Part05Keys[];`).
  - The types' layouts and field names are read from the project's headers (TYPE_HEADERS) by tools/c_layout.py.
  - Each record becomes a C array of its type, in ROM order, one element a line. A pointer is the name of the record it
    leads to (so it follows a layout change), NULL where it is zero, with a cast where that record is declared with
    another type. A count the game's code reads as an array's length is written ARRAY_COUNT(that array) where the two
    agree (COUNTS), so adding or removing an element keeps it right. Floats are the shortest decimals that give the
    same bits; above each record, the field names in initializer order and the note on its symbol_addrs.txt line (text
    in parentheses). A u8 array (a name in the game's text encoding) is written as bytes, 16 a line; a field the
    code reads as a bit mask or a packed value (HEX) in hex.
  - Fields named unk... keep the placeholder name the type gives them: no code establishes their meaning.
  - A record declared RawWord or RawByte (RAW_TYPES) is raw data: bytes nothing the game's C code is known to do
    reads, so no type is claimed for them. Its words are written in hex, except a word that is the address of a named
    record of the same file, which is written as that name so it follows a layout change; a word that lies in the file
    and names no record stops the extraction (it could not follow).
It stops (and with it the extraction) on anything it cannot represent: a symbol with no size, no declaration or a type
the headers do not lay out, a gap between records other than the compiler's own pad of an array to 4 bytes, a size
that is not a whole number of elements, a pointer to no named record, a union whose first member does not cover it.
And on what would be written but could not follow a change of layout, or would hide a wrong declaration: a pointer
to a record declared with another C type (the cast is written only where the target is declared raw), an integer
field or a RawByte record that holds an address of the file, a raw word that reads as an address outside the file.
Stopping, it removes the file an earlier extraction left and that file's object (./configure goes on to configure.py
whatever splat returns), so no later build uses them.

Authored changes: the generated text is written to <file>.pristine; if asset_patches/<dir>/<name>.anim.inc.c.patch
exists, it is applied (tools/asset_patches.py: exactly, or the extraction stops) and the result is <file>. Edit the
file, then keep the change with `python3 tools/asset_patches.py save assets/<dir>/<name>.anim.inc.c`: it survives
./configure, and a clean clone with the original ROM and asset_patches/ rebuilds the same ROM.
"""
import hashlib
import importlib.util
import re
import sys
from pathlib import Path
from typing import Optional

from util import options, log

from segtypes.common.codesubsegment import CommonSegCodeSubsegment


def _load(name):
    """tools/<name>.py, loaded by its path (nothing is added to sys.path, so no tools/ name can shadow splat's)"""
    path = Path(__file__).resolve().parent.parent / f"{name}.py"
    spec = importlib.util.spec_from_file_location(f"eltale_tools_{name}", path)
    mod = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)
    return mod


c_layout = _load("c_layout")
asset_patches = _load("asset_patches")

# the headers whose typedefs give the records' layouts, and the headers that declare each record with its type
TYPE_HEADERS = ('include/structs.h', 'include/quest_types.h')
DECL_HEADERS = ('include/boss_files.h', 'include/monster_sets.h')
# count fields the game's code reads as the length of a sibling pointer's array: DrawModel loops over anim->count
# parts; SampleSpline reads keyCount keys (1: a still pose); DrawModelPart indexes dlistFrames by frame;
# PickEnemyAIOption picks among aiScriptNumber options
COUNTS = {('AnimEntry', 'count'): 'parts', ('ModelPart', 'keyCount'): 'keys',
          ('ModelPart', 'dlistFrameCount'): 'dlistFrames', ('MonsterBaseData', 'aiScriptNumber'): 'aiOptions'}
# fields the game's code reads as bit masks or as packed values, written in hex: an AI option's action_tag (bit 15
# AI_ACTION_SPELL, below it the spell's element and index) and its move_flags
HEX = {('EnemyAIOption', 'action_tag'), ('EnemyAIOption', 'move_flags')}
# declared types that claim nothing about the bytes (include/monster_sets.h defines them): name -> element size
RAW_TYPES = {'RawWord': 4, 'RawByte': 1}
# the console's RAM as the game's code addresses it, above the boot vectors: a raw word in it reads as an address
RAM_LO, RAM_HI = 0x80000400, 0x80800000

_DECL = re.compile(r'^\s*extern\s+([A-Za-z_]\w*(?:\s*\*)*)\s*([A-Za-z_]\w*)\s*\[\s*\]\s*;', re.M)
_SYM = re.compile(r'^\s*([A-Za-z_]\w*)\s*=\s*0x[0-9A-Fa-f]+\s*;\s*//(.*)$')


class Raw(object):
    """a raw record's element: a word or a byte nothing is claimed about"""

    def __init__(self, name):
        self.name, self.size = name, RAW_TYPES[name]


def _fail(msg, seg=None):
    """stop the extraction. ./configure runs configure.py even when splat fails, so a segment's files from an earlier
    extraction would still be built: remove its generated file and that file's object first, so a later build stops
    on them instead of using them (the .pristine copy stays: a patch is regenerated against it)"""
    if seg is not None:
        out = seg.out_path()
        obj = options.opts.build_path / (str(out.relative_to(options.opts.base_path)) + ".o")
        for p in (out, obj):
            if p.is_file():
                p.unlink()
    print('\nError: anim: ' + msg, file=sys.stderr, flush=True)      # at a line's start, after splat's progress bar
    log.error('anim: ' + msg)


class N64SegAnim(CommonSegCodeSubsegment):
    SUFFIX = "anim"                 # the generated file is <name>.<SUFFIX>.inc.c; also the splat mode's name
    WHAT = "animation records"
    WRITER = "tools/splat_ext/anim.py"
    TYPES = "AnimEntry, ModelPart, ModelKeyFrame"

    def __init__(self, rom_start, rom_end, type, name, vram_start, args, yaml):
        super().__init__(rom_start, rom_end, type, name, vram_start, args=args, yaml=yaml)
        self.file_text: Optional[str] = None

    def get_linker_section(self) -> str:
        return ".data"

    def out_path(self) -> Path:
        return options.opts.asset_path / self.dir / f"{self.name}.{self.SUFFIX}.inc.c"

    def patch_path(self) -> Path:
        rel = self.out_path().relative_to(options.opts.asset_path)
        return options.opts.base_path / asset_patches.PATCHES / (str(rel) + ".patch")

    def cache(self):
        """splat's --use-cache key: everything the written file depends on besides the ROM"""
        files = [self.patch_path()] + [options.opts.base_path / h for h in TYPE_HEADERS + DECL_HEADERS]
        files += list(options.opts.symbol_addrs_paths)
        # the writer itself: its text decides the written file as much as the definitions do
        here = Path(__file__).resolve().parent
        files += sorted(here.glob("*.py")) + [here.parent / "c_layout.py", here.parent / "asset_patches.py"]
        digest = hashlib.sha1()
        for p in files:
            digest.update(str(p).encode() + (p.read_bytes() if p.is_file() else b"<none>"))
        return (self.yaml, self.rom_end, digest.hexdigest())

    def should_scan(self) -> bool:
        return options.opts.is_mode_active(self.SUFFIX)

    def should_split(self) -> bool:
        return self.extract and options.opts.is_mode_active(self.SUFFIX)

    # -- reading the definitions -----------------------------------------------------------------------------------
    def _read(self, rel):
        p = options.opts.base_path / rel
        if not p.is_file():
            _fail(f"{rel} not found", self)
        return p.read_text(encoding="utf-8")

    def _definitions(self):
        layouts = c_layout.Layouts([self._read(h) for h in TYPE_HEADERS])
        decls = {}
        for h in DECL_HEADERS:
            for m in _DECL.finditer(self._read(h)):
                decls[m.group(2)] = re.sub(r'\s+', '', m.group(1))
        notes = {}
        for p in options.opts.symbol_addrs_paths:
            if p.is_file():
                for line in p.read_text(encoding="utf-8").split("\n"):
                    m = _SYM.match(line)
                    if m:
                        found = re.findall(r'\(([^()]*)\)', m.group(2))
                        if found:
                            notes[m.group(1)] = '; '.join(found)
        return layouts, decls, notes

    # -- writing ------------------------------------------------------------------------------------------------------
    def scan(self, rom_bytes: bytes):
        try:
            self.file_text = self.render(rom_bytes)
        except Exception as ex:         # whatever stops the reading stops it through _fail: no stale file is built
            _fail(f"{self.name}: {type(ex).__name__}: {ex}", self)

    def render(self, rom_bytes: bytes) -> str:
        assert isinstance(self.rom_start, int) and isinstance(self.rom_end, int) and isinstance(self.vram_start, int)
        layouts, decls, notes = self._definitions()
        lo, hi = self.vram_start, self.vram_start + self.rom_end - self.rom_start
        seg = self.get_most_parent()
        file_lo = seg.vram_start
        file_hi = file_lo + (seg.rom_end - seg.rom_start)     # the file as loaded: what a raw word may point into
        here = sorted((s for lst in seg.seg_symbols.values() for s in lst
                       if s.user_declared and lo <= s.vram_start < hi), key=lambda s: s.vram_start)
        if not here or here[0].vram_start != lo:
            _fail(f"{self.name}: no record symbol at its start (0x{lo:08X})", self)
        by_addr = {}
        for lst in seg.seg_symbols.values():
            for s in lst:
                if s.user_declared:
                    by_addr.setdefault(s.vram_start, s)

        def elem_type(cdecl, name):
            if cdecl in RAW_TYPES:
                return Raw(cdecl)
            if cdecl.endswith('*'):
                return c_layout.Pointer(cdecl[:-1])
            try:
                return layouts.get(cdecl)
            except c_layout.LayoutError as ex:
                _fail(f"{name}: {ex}", self)

        records = []            # (symbol, declared type text, element type, count)
        for k, s in enumerate(here):
            if not s.given_size:
                _fail(f"{s.name}: no size in symbol_addrs.txt", self)
            if s.name not in decls:
                _fail(f"{s.name}: not declared in {', '.join(DECL_HEADERS)}", self)
            et = elem_type(decls[s.name], s.name)
            if s.given_size % et.size:
                _fail(f"{s.name}: size 0x{s.given_size:X} is not a whole number of {decls[s.name]} (0x{et.size:X})", self)
            end = s.vram_start + s.given_size
            nxt = here[k + 1].vram_start if k + 1 < len(here) else None
            if nxt is not None and nxt != end:
                # the compiler starts every array on a 4-byte boundary: up to 3 zero bytes after a byte array
                at = self.rom_start + end - lo
                if not (0 < nxt - end < 4 and nxt % 4 == 0 and not any(rom_bytes[at:at + nxt - end])):
                    _fail(f"{s.name}: ends at 0x{end:08X}, the next record starts at 0x{nxt:08X}", self)
            records.append((s, decls[s.name], et, s.given_size // et.size))
        tail = hi - (records[-1][0].vram_start + records[-1][0].given_size)
        if not 0 <= tail < 16 or any(rom_bytes[self.rom_end - tail:self.rom_end]):
            _fail(f"{self.name}: {tail} bytes after the last record that are not its zero pad to 16", self)
        # an initializer writes zeros into a type's padding: a nonzero padding byte could not be stated
        for s, cdecl, et, n in records:
            if isinstance(et, c_layout.Aggregate):
                held = set()
                for _, t, o in layouts.leaves(et):
                    held.update(range(o, o + t.size))
                pad = [o for o in range(et.size) if o not in held]
                base = self.rom_start + s.vram_start - lo
                for k in range(n):
                    if any(rom_bytes[base + k * et.size + o] for o in pad):
                        _fail(f"{s.name}[{k}]: a nonzero byte in {cdecl}'s padding", self)

        counts = {}             # records written so far: name -> element count (ARRAY_COUNT needs a complete type)
        rom = lambda ram: self.rom_start + ram - lo

        def pointer_text(value, where, want=None):
            if value == 0:
                return 'NULL'
            s = by_addr.get(value)
            if s is None:
                _fail(f"{where}: 0x{value:08X} is not the start of a named record", self)
            have = decls.get(s.name)
            if want is not None and have is not None and have != re.sub(r'\s+', '', want):
                # A cast only where the record is declared raw: the field's type is then the one claim made about
                # those bytes. Two different C types for one record is a wrong declaration, and a cast would hide it
                # (the bytes built are the same either way, so nothing else would notice).
                if have not in RAW_TYPES:
                    _fail(f"{where}: points at {s.name}, which is declared {have}, not {want}", self)
                return f"({want}*){s.name}"
            return s.name

        def scalar_text(t, off, where):
            if isinstance(t, c_layout.Pointer):
                return pointer_text(c_layout.decode(t, rom_bytes, off)[1], where, t.target)
            if t.kind == 'f':
                try:
                    return c_layout.float_text(t, rom_bytes, off)
                except c_layout.LayoutError as ex:
                    _fail(f"{where}: {ex}", self)
            value = c_layout.decode(t, rom_bytes, off)
            if t.size == 4 and file_lo <= (value & 0xFFFFFFFF) < file_hi:
                # written as a number it would not follow a change of layout: the type should say pointer
                _fail(f"{where}: an integer field holds 0x{value & 0xFFFFFFFF:08X}, an address in this file", self)
            return str(value)

        def items(agg, off, where):
            """an aggregate's initializer items: anonymous members looked through, a union's first member"""
            out = []
            ms = agg.members
            if agg.kind == 'union':
                first = ms[0]
                if first.type.size < agg.size and any(rom_bytes[off + first.type.size:off + agg.size]):
                    _fail(f"{where}: union {agg.name}'s first member does not hold all its bytes", self)
                ms = ms[:1]
            for m in ms:
                o = off + m.offset
                if m.name is None and isinstance(m.type, c_layout.Aggregate):
                    out += items(m.type, o, where)
                    continue
                pair = COUNTS.get((agg.name, m.name))
                if pair and isinstance(m.type, c_layout.Scalar):
                    ptr = agg.member(pair)
                    target = c_layout.decode(ptr.type, rom_bytes, off + ptr.offset)[1]
                    value = c_layout.decode(m.type, rom_bytes, o)
                    ts = by_addr.get(target)
                    if ts is not None and counts.get(ts.name) == value and decls.get(ts.name) not in RAW_TYPES:
                        out.append(f"ARRAY_COUNT({ts.name})")
                        continue
                if (agg.name, m.name) in HEX and isinstance(m.type, c_layout.Scalar) and m.type.kind == 'u':
                    out.append(f"0x{c_layout.decode(m.type, rom_bytes, o):0{2 * m.type.size}X}")
                    continue
                out.append(value_text(m.type, o, f"{where}.{m.name}"))
            return out

        def value_text(t, off, where):
            if isinstance(t, c_layout.Aggregate):
                return '{ ' + ', '.join(items(t, off, where)) + ' }'
            if isinstance(t, c_layout.Array):
                return '{ ' + ', '.join(value_text(t.elem, off + k * t.elem.size, f"{where}[{k}]")
                                        for k in range(t.count)) + ' }'
            return scalar_text(t, off, where)

        def signature(t):
            if isinstance(t, c_layout.Aggregate):
                ms = t.members[:1] if t.kind == 'union' else t.members
                parts = []
                for m in ms:
                    if m.name is None and isinstance(m.type, c_layout.Aggregate):
                        parts.append(signature(m.type))
                    elif isinstance(m.type, c_layout.Aggregate):
                        parts.append(f"{m.name} {{ {signature(m.type)} }}")
                    elif isinstance(m.type, c_layout.Array):
                        parts.append(f"{m.name}[{m.type.count}]")
                    else:
                        parts.append(m.name)
                return ', '.join(parts)
            return ''

        def raw_rows(et, base, n, name, ram):
            """a raw record's rows: words (a word that is a named record's address as that name) or bytes, in hex"""
            texts = []
            if et.size == 1:
                # bytes cannot name a record: a byte record that holds an address of this file at a word boundary
                # would keep it as numbers, and it would not follow a change of layout. Declare it RawWord.
                for k in range((-ram) % 4, n - 3, 4):
                    value = int.from_bytes(rom_bytes[base + k:base + k + 4], 'big')
                    if file_lo <= value < file_hi:
                        _fail(f"{name}: declared {et.name}, but its bytes at +0x{k:X} are 0x{value:08X}, an address "
                              f"in this file: declare it RawWord, so the address is written as a name", self)
            for k in range(n):
                off = base + k * et.size
                if et.size == 4:
                    value = int.from_bytes(rom_bytes[off:off + 4], 'big')
                    target = by_addr.get(value)
                    if target is not None:
                        texts.append(f"({et.name}){target.name}")
                    elif file_lo <= value < file_hi:
                        _fail(f"{name}[{k}]: the raw word 0x{value:08X} lies in this file but is not the start of a "
                              f"named record: it could not follow a change of layout", self)
                    elif RAM_LO <= value < RAM_HI:
                        _fail(f"{name}[{k}]: the raw word 0x{value:08X} reads as an address outside this file: "
                              f"nothing here says what it points at, so it could not follow that thing", self)
                    else:
                        texts.append(f"0x{value:08X}")
                else:
                    texts.append(f"0x{rom_bytes[off]:02X}")
            per = 8 if et.size == 4 else 16
            return ['    ' + ', '.join(texts[k:k + per]) + ',' for k in range(0, len(texts), per)]

        any_raw = any(isinstance(et, Raw) for _, _, et, _ in records)
        lines = [options.opts.generated_c_preamble, "",
                 f"/* {self.dir}'s {self.WHAT}, ROM 0x{self.rom_start:X}-0x{self.rom_end:X} (RAM 0x{lo:08X}),",
                 f" * written from the ROM at extraction by {self.WRITER}. Layouts and field names: the",
                 f" * project's C types (include/quest_types.h: {self.TYPES}). A field named",
                 " * unk... is laid out by its type, but no code establishes what it means."]
        if any_raw:
            lines += [" * A record declared RawWord or RawByte is raw data: nothing the game's C code is known to do",
                      " * reads it, so no type is claimed for it. A word of it that is the address of a named record",
                      " * is written as that name, so it follows a change of layout."]
        lines += [" * To change a record, edit this file, then keep the change with",
                  " * python3 tools/asset_patches.py save <this file> (it goes to asset_patches/): ./configure",
                  " * rewrites this file from the ROM and applies the saved change.",
                  " * Only record files like this one are kept that way. ./configure also rewrites the textures",
                  " * (.png), vertex files and display lists in this folder from the ROM, and nothing keeps a",
                  " * change made to them: an edit there is lost at the next ./configure. */"]
        for s, cdecl, et, n in records:
            note = notes.get(s.name)
            lines.append("")
            if note:
                lines.append(f"/* {note} */")
            base = rom(s.vram_start)
            if isinstance(et, Raw):
                lines.append(f"{cdecl} {s.name}[] = {{")
                lines += raw_rows(et, base, n, s.name, s.vram_start)
            elif isinstance(et, c_layout.Scalar) and et.name == 'u8':
                lines.append(f"{cdecl} {s.name}[] = {{")
                texts = [f"0x{rom_bytes[base + k]:02X}" for k in range(n)]
                lines += ['    ' + ', '.join(texts[k:k + 16]) + ',' for k in range(0, n, 16)]
            else:
                sig = signature(et)
                decl = f"{cdecl[:-1]}* {s.name}[]" if cdecl.endswith('*') else f"{cdecl} {s.name}[]"
                lines.append(f"{decl} = {{" + (f"   /* {sig} */" if sig else ""))
                for k in range(n):
                    lines.append(f"    {value_text(et, base + k * et.size, f'{s.name}[{k}]')},")
            lines.append("};")
            counts[s.name] = n
            s.defined = True
        lines.append("")
        return "\n".join(lines)

    def split(self, rom_bytes: bytes):
        if self.file_text is None:
            return
        try:
            self._write()
        except Exception as ex:         # as in scan: a stop always goes through _fail
            _fail(f"{self.name}: {type(ex).__name__}: {ex}", self)

    def _write(self):
        out = self.out_path()
        out.parent.mkdir(parents=True, exist_ok=True)
        with open(str(out) + ".pristine", "w", encoding="utf-8", newline="\n") as f:
            f.write(self.file_text)
        text, applied, notes = self.file_text, None, []
        p = self.patch_path()
        if p.is_file():
            try:
                text = asset_patches.apply(self.file_text, p.read_text(encoding="utf-8"),
                                           str(p.relative_to(options.opts.base_path)), notes)
            except asset_patches.PatchError as ex:
                _fail(f"{self.name}: the authored change does not apply: {ex}", self)
            applied = p
        with open(out, "w", encoding="utf-8", newline="\n") as f:
            f.write(text)
        if applied:
            log.write(f"anim: {out.relative_to(options.opts.base_path)}: applied "
                      f"{applied.relative_to(options.opts.base_path)}")
            for note in notes:
                log.write(f"anim: note: {note}")
