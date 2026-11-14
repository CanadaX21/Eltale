"""c_layout.py: the layout of the project's C struct types, read from its headers.

The project's C types are the one definition of the records the build emits as typed data (tools/splat_ext/anim.py); the
ROM map checks its readers' offsets against this module's reading of the same headers. It reads `typedef struct` and
`typedef union` definitions (tagged or not) and lays them out as IDO does for the N64's o32 ABI: each scalar aligned to
its size, an aggregate to its widest member, its size rounded up to that alignment. Plain char is signed: the build
compiles with -signed (configure.py; IDO's own default is unsigned).

Only what the record types need is understood: scalar types, pointers, fixed arrays, nested and anonymous structs and
unions. Anything else in a type it is asked for stops it with a LayoutError naming the declaration, never a silent
guess: a bit field, a function pointer, a name it does not know, a declaration with no name that is not an inline
struct or union, a preprocessor conditional inside the type's body (this module does not evaluate #if), a typedef name
defined twice differently, a typedef of an array type. A pointer's target is not laid out (a struct may point to its own
type). Definitions it is not asked for are never examined.

    layouts = c_layout.Layouts([open('include/structs.h').read(), open('include/quest_types.h').read()])
    part = layouts.get('ModelPart')          # .size, .align, .members (Member: name, offset, type)
    part.member('keyCount').offset           # 4
    layouts.leaf_names('ModelKeyFrame')      # the scalar fields in initializer order

Python 3 standard library only.
"""
import decimal
import re
import struct

# the ABI's scalar types: name -> (size, kind); kind 'f' float, 's' signed, 'u' unsigned
SCALARS = {
    'f32': (4, 'f'), 'float': (4, 'f'), 'f64': (8, 'f'), 'double': (8, 'f'),
    's8': (1, 's'), 'u8': (1, 'u'), 'char': (1, 's'), 'signed char': (1, 's'), 'unsigned char': (1, 'u'),
    's16': (2, 's'), 'u16': (2, 'u'), 'short': (2, 's'), 'unsigned short': (2, 'u'),
    's32': (4, 's'), 'u32': (4, 'u'), 'int': (4, 's'), 'unsigned int': (4, 'u'), 'unsigned': (4, 'u'),
    'long': (4, 's'), 'unsigned long': (4, 'u'),
    's64': (8, 's'), 'u64': (8, 'u'), 'long long': (8, 's'), 'unsigned long long': (8, 'u'),
}
POINTER_SIZE = 4
PP_CONDITIONAL = '__c_layout_preprocessor_conditional__'     # stands for #if / #ifdef / #ifndef / #elif / #else / #endif


class LayoutError(Exception):
    pass


class Scalar(object):
    def __init__(self, name, size, kind):
        self.name, self.size, self.align, self.kind = name, size, size, kind

    def __repr__(self):
        return self.name


class Pointer(object):
    size = align = POINTER_SIZE

    def __init__(self, target):
        self.target = target            # the pointed-to type's name as written; never laid out

    @property
    def name(self):
        return self.target + '*'

    def __repr__(self):
        return self.name


class Array(object):
    def __init__(self, elem, count):
        self.elem, self.count = elem, count
        self.size, self.align = elem.size * count, elem.align

    @property
    def name(self):
        return '%s[%d]' % (self.elem.name, self.count)

    def __repr__(self):
        return self.name


class Member(object):
    def __init__(self, name, type_, offset):
        self.name, self.type, self.offset = name, type_, offset      # name None: an anonymous struct or union

    def __repr__(self):
        return '%s %s @0x%X' % (self.type.name, self.name or '(anonymous)', self.offset)


class Aggregate(object):
    def __init__(self, kind, name, members, size, align):
        self.kind, self.name, self.members, self.size, self.align = kind, name, members, size, align

    def member(self, name):
        """the member called name, looking through anonymous members; its offset is from this aggregate's start"""
        for m in self.members:
            if m.name == name:
                return m
            if m.name is None and isinstance(m.type, Aggregate):
                try:
                    inner = m.type.member(name)
                except LayoutError:
                    continue
                return Member(inner.name, inner.type, m.offset + inner.offset)
        raise LayoutError('%s has no member %s' % (self.name, name))

    def __repr__(self):
        return self.name


# comments and literals in one left-to-right pass, so a "/*" inside a // comment or a string opens nothing
_LEX = re.compile(r'//[^\n]*|/\*.*?\*/|"(?:\\.|[^"\\\n])*"|\'(?:\\.|[^\'\\\n])*\'', re.S)
_CONDITIONAL = re.compile(r'\s*#\s*(if|ifdef|ifndef|elif|else|endif)\b')


def _strip(text):
    def sub(m):
        s = m.group(0)
        if s.startswith('//'):
            return ' '
        if s.startswith('/*'):
            return ' ' + '\n' * s.count('\n')
        return s
    text = _LEX.sub(sub, text).replace('\\\n', ' ')
    out = []
    for line in text.split('\n'):
        if line.lstrip().startswith('#'):
            out.append(' %s ' % PP_CONDITIONAL if _CONDITIONAL.match(line) else '')
        else:
            out.append(line)
    return '\n'.join(out)


_TOKEN = re.compile(r'''\s*(?:
      (?P<float>(?:\d+\.\d*|\.\d+)(?:[eE][+-]?\d+)?[fFlL]?|\d+[eE][+-]?\d+[fFlL]?)
    | (?P<int>0[xX][0-9A-Fa-f]+[uUlL]*|\d+[uUlL]*)
    | (?P<id>[A-Za-z_]\w*)
    | (?P<str>"(?:\\.|[^"\\])*"|'(?:\\.|[^'\\])*')
    | (?P<p>\S))''', re.X | re.S)


def _int(text):
    t = text.rstrip('uUlL')
    if t[:2] in ('0x', '0X'):
        return int(t, 16)
    if len(t) > 1 and t[0] == '0':
        return int(t, 8)                    # C: a leading zero is octal
    return int(t, 10)


def _tokens(text):
    out = []
    for m in _TOKEN.finditer(text):
        if m.group('int'):
            out.append(_int(m.group('int')))
        elif m.group('float') or m.group('str'):
            out.append('<literal>')         # never an array size or a name
        elif m.group('id'):
            out.append(m.group('id'))
        elif m.group('p'):
            out.append(m.group('p'))
    return out


def _block(tokens, i):
    """tokens[i] is '{': the index just past its matching '}'"""
    depth = 0
    for j in range(i, len(tokens)):
        if tokens[j] == '{':
            depth += 1
        elif tokens[j] == '}':
            depth -= 1
            if depth == 0:
                return j + 1
    raise LayoutError('unbalanced braces')


def _ident(t):
    return isinstance(t, str) and re.match(r'[A-Za-z_]\w*$', t) is not None


class Layouts(object):
    """the typedef'd structs and unions in a set of header texts, laid out on demand"""

    def __init__(self, texts):
        self.defs = {}          # typedef name -> (kind, data): 'struct' / 'union' (body), 'alias', 'tag', or a refusal
        self.tags = {}          # struct / union tag -> (kind, body tokens)
        self.cache = {}
        self.busy = set()
        for text in texts:
            self._scan(_tokens(_strip(text)))

    def _define(self, name, entry):
        old = self.defs.get(name)
        if old is not None and old != entry:
            entry = ('refused', 'defined twice differently in the headers read')
        self.defs[name] = entry

    def _scan(self, t):
        i = 0
        while i < len(t):
            if t[i] != 'typedef':
                i += 1
                continue
            j = i + 1
            if j < len(t) and t[j] in ('struct', 'union'):
                kind = t[j]
                j += 1
                tag = None
                if j < len(t) and _ident(t[j]):
                    tag = t[j]
                    j += 1
                if j < len(t) and t[j] == '{':
                    end = _block(t, j)
                    body = t[j + 1:end - 1]
                    if tag:
                        if tag in self.tags and self.tags[tag] != (kind, body):
                            self.tags[tag] = ('refused', 'struct %s defined twice differently' % tag)
                        else:
                            self.tags[tag] = (kind, body)
                    k = end
                    while k < len(t) and t[k] != ';':
                        if _ident(t[k]) and (k == end or t[k - 1] == ','):     # a declarator's name (not *Name)
                            if k + 1 < len(t) and t[k + 1] == '[':
                                self._define(t[k], ('refused', 'a typedef of an array type'))
                            else:
                                self._define(t[k], (kind, body))
                        k += 1
                    i = k + 1
                    continue
                # typedef struct Tag Name; (no body): an alias of the tag
                k = j
                while k < len(t) and t[k] != ';':
                    k += 1
                if tag and k - j == 1 and _ident(t[j]):
                    self._define(t[j], ('tag', (kind, tag)))
                i = k + 1
                continue
            # typedef <scalar specifier> Name;
            k = j
            while k < len(t) and t[k] != ';':
                k += 1
            spec = t[j:k]
            if len(spec) >= 2 and all(_ident(x) for x in spec):
                self._define(spec[-1], ('alias', tuple(spec[:-1])))
            i = k + 1

    # -- types ------------------------------------------------------------------------------------------------------
    def get(self, name):
        """the layout of a typedef'd struct or union (an Aggregate), or of a scalar type (a Scalar)"""
        if name in self.cache:
            return self.cache[name]
        if name in SCALARS:
            size, kind = SCALARS[name]
            return Scalar(name, size, kind)
        if name not in self.defs:
            raise LayoutError('type %s is not defined in the headers read' % name)
        if name in self.busy:
            raise LayoutError('type %s contains itself' % name)
        kind, data = self.defs[name]
        self.busy.add(name)
        try:
            if kind == 'refused':
                raise LayoutError('typedef %s: %s' % (name, data))
            if kind == 'alias':
                spec = ' '.join(x for x in data if x not in ('const', 'volatile'))
                if spec in SCALARS:
                    size, k = SCALARS[spec]
                    lay = Scalar(name, size, k)
                elif len(data) == 1:
                    lay = self.get(data[0])
                else:
                    raise LayoutError('typedef %s: %s is not a type this module lays out' % (name, spec))
            elif kind == 'tag':
                tkind, tag = data
                lay = self._tag(tag, name)
            else:
                lay = self._aggregate(kind, name, data)
        finally:
            self.busy.discard(name)
        self.cache[name] = lay
        return lay

    def _tag(self, tag, owner):
        if tag not in self.tags:
            raise LayoutError('%s: struct %s has no body in the headers read' % (owner, tag))
        kind, body = self.tags[tag]
        if kind == 'refused':
            raise LayoutError('%s: %s' % (owner, body))
        key = 'struct ' + tag
        if key in self.busy:
            raise LayoutError('struct %s contains itself' % tag)
        self.busy.add(key)
        try:
            return self._aggregate(kind, tag, body)
        finally:
            self.busy.discard(key)

    def _aggregate(self, kind, name, body):
        if PP_CONDITIONAL in body:
            raise LayoutError('%s: its body has a preprocessor conditional (#if, #ifdef, #else...), which this module '
                              'does not evaluate' % name)
        members = []
        i = 0
        while i < len(body):
            spec, inline, i = self._specifier(body, i, name)
            if body[i] == ';':          # an anonymous struct or union member: only an inline body declares one
                if not inline:
                    raise LayoutError('%s: a declaration with no name (%s)' % (name, self._spec_name(spec)))
                members.append((None, spec))
                i += 1
                continue
            while True:
                stars = 0
                while body[i] == '*':
                    stars += 1
                    i += 1
                while body[i] in ('const', 'volatile'):
                    i += 1
                if body[i] == '(':
                    raise LayoutError('%s: a function pointer or parenthesised declarator' % name)
                mname = body[i]
                if not _ident(mname):
                    raise LayoutError('%s: cannot read the declaration at %r' % (name, mname))
                i += 1
                if stars:
                    mtype = Pointer(self._spec_name(spec))
                    for _ in range(stars - 1):
                        mtype = Pointer(mtype.name)
                else:
                    mtype = self._resolve(spec, '%s.%s' % (name, mname))
                dims = []
                while body[i] == '[':
                    if not isinstance(body[i + 1], int) or body[i + 2] != ']':
                        raise LayoutError('%s.%s: an array size that is not a number' % (name, mname))
                    dims.append(body[i + 1])
                    i += 3
                if body[i] == ':':
                    raise LayoutError('%s.%s: a bit field' % (name, mname))
                for d in reversed(dims):
                    mtype = Array(mtype, d)
                members.append((mname, mtype))
                if body[i] == ',':
                    i += 1
                    continue
                if body[i] != ';':
                    raise LayoutError('%s.%s: expected ; after the declaration' % (name, mname))
                i += 1
                break
        out, off, align, size = [], 0, 1, 0
        for mname, mtype in members:
            align = max(align, mtype.align)
            if kind == 'struct':
                off = (off + mtype.align - 1) // mtype.align * mtype.align
                out.append(Member(mname, mtype, off))
                off += mtype.size
                size = off
            else:
                out.append(Member(mname, mtype, 0))
                size = max(size, mtype.size)
        size = (size + align - 1) // align * align
        return Aggregate(kind, name, out, size, align)

    def _specifier(self, body, i, owner):
        """the type specifier at body[i] -> (spec, inline, next index). spec is a laid-out type (a scalar keyword, or
        an inline struct or union body: inline True), or ('name', N) / ('tag', T) for a name, laid out only when the
        member is not a pointer"""
        while body[i] in ('const', 'volatile'):
            i += 1
        if body[i] == PP_CONDITIONAL:
            raise LayoutError('%s: a preprocessor conditional inside it' % owner)
        if body[i] in ('struct', 'union'):
            kind = body[i]
            i += 1
            tag = None
            if _ident(body[i]):
                tag = body[i]
                i += 1
            if body[i] == '{':
                end = _block(body, i)
                agg = self._aggregate(kind, '%s.%s' % (owner, tag or kind), body[i + 1:end - 1])
                return agg, True, end
            if tag is None:
                raise LayoutError('%s: %s with neither a tag nor a body' % (owner, kind))
            return ('tag', tag), False, i
        words = []
        while isinstance(body[i], str) and body[i] in ('unsigned', 'signed', 'short', 'long', 'int', 'char'):
            words.append(body[i])
            i += 1
        if words:
            spec = ' '.join(words)
            spec = {'signed': 'int', 'unsigned': 'unsigned int', 'long int': 'long', 'short int': 'short',
                    'unsigned short int': 'unsigned short', 'unsigned long int': 'unsigned long'}.get(spec, spec)
            if spec not in SCALARS:
                raise LayoutError('%s: scalar type %s' % (owner, spec))
            size, kind = SCALARS[spec]
            return Scalar(spec, size, kind), False, i
        name = body[i]
        if not _ident(name):
            raise LayoutError('%s: cannot read a type at %r' % (owner, name))
        return ('name', name), False, i + 1

    @staticmethod
    def _spec_name(spec):
        if isinstance(spec, tuple):
            return spec[1] if spec[0] == 'name' else 'struct ' + spec[1]
        return spec.name

    def _resolve(self, spec, where):
        """a member's own type (not behind a pointer), laid out now"""
        if isinstance(spec, tuple):
            if spec[0] == 'name':
                try:
                    return self.get(spec[1])
                except LayoutError as ex:
                    raise LayoutError('%s: %s' % (where, ex))
            return self._tag(spec[1], where)
        return spec

    # -- helpers for emitters and checks ------------------------------------------------------------------------------
    def leaves(self, name_or_type):
        """the scalar and pointer fields of a type in C initializer order, as (path, type, offset): anonymous members
        are looked through, a union contributes its first member (the one an initializer sets), an array each element"""
        t = self.get(name_or_type) if isinstance(name_or_type, str) else name_or_type
        out = []

        def walk(t, path, base):
            if isinstance(t, Aggregate):
                ms = t.members[:1] if t.kind == 'union' else t.members
                for m in ms:
                    walk(m.type, path + ([m.name] if m.name else []), base + m.offset)
            elif isinstance(t, Array):
                for k in range(t.count):
                    walk(t.elem, path[:-1] + ['%s[%d]' % (path[-1], k)] if path else ['[%d]' % k], base + k * t.elem.size)
            else:
                out.append(('.'.join(path), t, base))
        walk(t, [], 0)
        return out

    def leaf_names(self, name):
        return [p for p, _, _ in self.leaves(name)]


def decode(t, data, off):
    """a scalar's or a pointer's value at data[off] (big-endian): int, float, or ('ptr', address)"""
    if isinstance(t, Pointer):
        return ('ptr', struct.unpack_from('>I', data, off)[0])
    size, kind = t.size, t.kind
    if kind == 'f':
        return struct.unpack_from('>f' if size == 4 else '>d', data, off)[0]
    fmt = {1: 'b', 2: 'h', 4: 'i', 8: 'q'}[size]
    return struct.unpack_from('>' + (fmt if kind == 's' else fmt.upper()), data, off)[0]


def float_text(t, data, off):
    """a float field as the shortest C literal that gives the same bits (IDO reads 'f'-suffixed decimals); NaN and
    infinities raise (no C89 literal spells them)"""
    if t.size != 4:
        raise LayoutError('only f32 fields are written as literals')
    bits = struct.unpack_from('>I', data, off)[0]
    if bits & 0x7F800000 == 0x7F800000:
        raise LayoutError('a NaN or infinite float (0x%08X)' % bits)
    if bits == 0:
        return '0.0f'
    if bits == 0x80000000:
        return '-0.0f'
    v = struct.unpack('>f', struct.pack('>I', bits))[0]
    for p in range(1, 10):
        s = '%.*g' % (p, v)
        if struct.pack('>f', float(s)) == struct.pack('>I', bits):
            break
    d = decimal.Decimal(s)                      # the shortest digits; written positionally unless very large or small
    if -7 <= d.adjusted() <= 15:
        s = format(d, 'f')
        if '.' not in s:
            s += '.0'
    else:
        mant, exp = ('%.*e' % (p - 1, v)).split('e')
        s = (mant if '.' in mant else mant + '.0') + 'e' + str(int(exp))
    return s + 'f'
