"""text_codec.py: the game's two text encodings, read from text/encoding.json and text/hud_font.json.

The game draws text with two fonts, each with an encoding of its own:
  - the dialogue font (Encoding, text/encoding.json): the dialogue bank, the signs, the NPCs' names, the menus' text;
    everything that goes through DrawString and the message window;
  - the HUD font (HudEncoding, text/hud_font.json, below): the monsters', bosses' and spells' names and the placeholder
    for a spell not yet learned, drawn by DrawHUDText.
A string's bytes mean nothing without knowing which of the two draws it.

One implementation of each encoding for every tool that reads or writes the game's text: the dialogue bank's splat
type (tools/splat_ext/dialogue.py) decodes it at extraction, tools/text_build.py encodes it at build time, and the
project's ROM map (a research tool outside this repository) reads the original's strings through it;
tools/test_hud_text.py checks every HUD string against the HUD font's. Each definition (byte kinds, codes, the
glyph table) is data, in text/; this module holds only what a
definition cannot: the source text's syntax and, for the dialogue font, the rule that places the row selects.

The dialogue font's encoding:

The bytes (DrawString and the message window's code, which every string goes through):
  - a byte below 0x7F is a glyph: cell N of the current row of the font sheet;
  - 0x7F is a space (the pen advances, nothing is drawn);
  - a byte with bit 0x80 is a code, its bits 0x60 the kind and its low five bits the index: 0x80-0x9F select a row of
    the sheet (it has 30: 0x9E and 0x9F would read past it), 0xA0-0xBF a palette, 0xC0-0xDF the font or the spacing,
    0xE0-0xFF control (0xE0 a new line, 0xE1-0xE8 a speed, 0xF0 wait for a button and start a new page, 0xF2 the save
    prompt, 0xFF the end, ...).
The encoder states only what the sheet has: a glyph or a row select past its 30 rows of 30 cells stops it (a byte the
rules cannot state is still possible as {bytes ...} or {code XX}).

The row-select rule (the original's, which rebuilds every original string byte for byte): a glyph is preceded by a
row select when no glyph came directly before it in the string, or when the glyph before it is in another row. So
every run of glyphs opens with a select, and inside a run a select appears only where the row changes. A select the
rule would not write is kept in the source as {row N}.

The source text (one string's body; newlines in the file are only layout):
  - a character of the glyph table (text/encoding.json, "char") is that glyph; ASCII stands for the fullwidth form too;
  - U+3000 (or an ASCII space) is 0x7F; {sp} is 0x7F too (the writer uses it at the end of a line, where an editor
    could strip a space);
  - \\n is 0xE0, \\f is 0xF0, \\e is 0xF2;
  - {pal N}, {font N}, {spacing N}, {speed N}, {menu}, {row N}, {code XX} are the other codes (see "codes");
  - {R,C} is the glyph in row R, cell C, written so where the table gives the cell no character, or not one alone;
  - {bytes XX XX ...} is raw bytes (only for a string the rules above cannot state: none of the original's).
The end (0xFF) is implicit. Python 3 standard library only.
"""
import json
import os
import re
import unicodedata

HERE = os.path.dirname(os.path.abspath(__file__))
DEFAULT = os.path.join(os.path.dirname(HERE), 'text', 'encoding.json')
HUD_DEFAULT = os.path.join(os.path.dirname(HERE), 'text', 'hud_font.json')

END, NEWLINE, PAGE, SAVE, SPACE = 0xFF, 0xE0, 0xF0, 0xF2, 0x7F
ESCAPES = {'n': NEWLINE, 'f': PAGE, 'e': SAVE}
ESCAPE_OF = dict((v, k) for k, v in ESCAPES.items())


class TextError(Exception):
    pass


class Encoding(object):
    """the definition: rows x cells, the glyph table both ways, the alignment of a bank's strings"""

    def __init__(self, path=None, data=None):
        if data is None:
            with open(path or DEFAULT, encoding='utf-8') as f:
                data = json.load(f)
        self.data = data
        self.rows = data['font']['rows']
        self.cells = data['font']['cells']
        self.char_of = {}               # (row, cell) -> character, for a cell the table names alone
        self.cell_of = {}               # character -> (row, cell)
        named = {}
        for g in data['glyphs']:
            if g.get('char'):
                named.setdefault(g['char'], []).append((g['row'], g['cell']))
        for ch, cells in named.items():
            if len(cells) == 1:         # a character named by two cells is written as {R,C} in both
                self.char_of[cells[0]] = ch
                self.cell_of[ch] = cells[0]
        for ch, cell in list(self.cell_of.items()):
            if len(ch) == 1 and 0x21 <= ord(ch) <= 0x7E:
                self.cell_of.setdefault(chr(ord(ch) + 0xFEE0), cell)     # the fullwidth form of an ASCII character

    # -- one string ------------------------------------------------------------------------------------------------
    def decode(self, data, start=0):
        """the source text of the string at data[start:] and the offset after its 0xFF. Lossless: the text encodes back
        to the same bytes, or the string is written as {bytes ...}."""
        end = data.index(END, start) + 1
        raw = bytes(data[start:end])
        text = self._decode(raw)
        if self.encode(text) != raw:
            text = '{bytes ' + ' '.join('%02X' % b for b in raw[:-1]) + '}'
        return text, end

    def _decode(self, raw):
        out = []
        row = None                      # the current row
        prev_glyph = False              # was the byte before a glyph? (as the encoder keeps it)
        i, n = 0, len(raw) - 1          # the 0xFF is implicit
        while i < n:
            b = raw[i]
            if b & 0x80 and (b & 0x60) == 0:            # a row select
                r = b & 0x1F
                nxt = raw[i + 1] if i + 1 < n else None
                # the encoder writes this select itself before the glyph that follows (the rule); otherwise it is
                # stated, and the encoder then counts it as opening a run in that row
                if not (nxt is not None and nxt < SPACE and (not prev_glyph or row != r)):
                    out.append('{row %d}' % r)
                    prev_glyph = True
                row = r
                i += 1
                continue
            if b < SPACE:
                ch = self.char_of.get((row, b)) if row is not None and b < self.cells else None
                out.append(ch if ch else '{%s,%d}' % ('?' if row is None else row, b))
                prev_glyph = True
            elif b == SPACE:
                out.append('　')
                prev_glyph = False
            elif b in ESCAPE_OF:
                out.append('\\' + ESCAPE_OF[b])
                prev_glyph = False
            else:
                out.append(self._code_text(b))
                prev_glyph = False
            i += 1
        return ''.join(out)

    @staticmethod
    def _code_text(b):
        kind, idx = b & 0x60, b & 0x1F
        if kind == 0x20:
            return '{pal %d}' % idx
        if kind == 0x40:
            return ('{font %d}' if idx < 16 else '{spacing %d}') % idx
        if 0xE1 <= b <= 0xE8:
            return '{speed %d}' % (b & 7 if b & 7 else 8)
        if b == 0xFE:
            return '{menu}'
        return '{code %02X}' % b

    def encode(self, text):
        """the bytes of one string's source text, 0xFF included"""
        out = bytearray()
        row = None
        prev_glyph = False
        i, n = 0, len(text)

        def glyph(r, c):
            nonlocal row, prev_glyph
            if not (0 <= r < self.rows and 0 <= c < self.cells):
                raise TextError('glyph {%d,%d} out of range: the sheet has %d rows of %d cells'
                                % (r, c, self.rows, self.cells))
            if not prev_glyph or row != r:
                out.append(0x80 | r)
                row = r
            out.append(c)
            prev_glyph = True

        def code(b):
            nonlocal prev_glyph
            out.append(b)
            prev_glyph = False

        while i < n:
            ch = text[i]
            if ch in '\r\n':
                i += 1
                continue
            if ch == '\\':
                e = text[i + 1:i + 2]
                if e not in ESCAPES:
                    raise TextError('unknown escape \\%s at %d' % (e, i))
                code(ESCAPES[e])
                i += 2
                continue
            if ch == '{':
                j = text.find('}', i)
                if j < 0:
                    raise TextError('an unclosed { at %d' % i)
                tok = text[i + 1:j].strip()
                i = j + 1
                m = re.match(r'^(\d+)\s*,\s*(\d+)$', tok)
                if m:
                    glyph(int(m.group(1)), int(m.group(2)))
                    continue
                m = re.match(r'^(pal|font|spacing|speed|row|code)\s+([0-9A-Fa-f]+)$', tok)
                if m:
                    k, v = m.group(1), m.group(2)
                    num = int(v, 16) if k == 'code' else int(v)
                    if k == 'pal' and num < 32:
                        code(0xA0 | num)
                    elif k == 'font' and num < 16:
                        code(0xC0 | num)
                    elif k == 'spacing' and 16 <= num < 32:
                        code(0xC0 | num)
                    elif k == 'speed' and 1 <= num <= 8:
                        code(0xE0 + num)                # 0xE1-0xE8 (0xE0 is a new line)
                    elif k == 'row' and num < self.rows:
                        out.append(0x80 | num)
                        row = num
                        prev_glyph = True       # the next glyph in this row follows without another select
                    elif k == 'code' and 0x80 <= num <= 0xFE:
                        code(num)
                    else:
                        raise TextError('{%s} out of range' % tok)
                    continue
                if tok == 'menu':
                    code(0xFE)
                    continue
                if tok == 'sp':
                    code(SPACE)
                    continue
                m = re.match(r'^bytes((?:\s+[0-9A-Fa-f]{2})+)$', tok)
                if m:
                    out.extend(int(x, 16) for x in m.group(1).split())
                    prev_glyph = False
                    row = None
                    continue
                raise TextError('unknown token {%s}' % tok)
            if ch in ' 　':
                code(SPACE)
                i += 1
                continue
            cell = self.cell_of.get(ch)
            if cell is None:
                raise TextError('%r (U+%04X) is not in the glyph table' % (ch, ord(ch)))
            glyph(*cell)
            i += 1
        if END in out:
            raise TextError('a 0xFF inside the string')
        out.append(END)
        return bytes(out)

    # -- a bank's run of strings ---------------------------------------------------------------------------------------
    def split_part(self, data, align):
        """the strings of a run: (offset, source text); a string starts on an `align`-byte boundary at a byte that is
        not 0xFF, and the 0xFF bytes up to the next boundary are fill"""
        out, pos = [], 0
        while pos < len(data):
            if data[pos] == END:        # no string holds a 0xFF before its end: outside one it is fill
                pos += 1
                continue
            if pos % align:
                raise TextError('a string starts at 0x%X, not on a %d-byte boundary' % (pos, align))
            text, end = self.decode(data, pos)
            out.append((pos, text))
            pos = end
        return out

    def layout(self, strings, align, pad):
        """the bytes of a run of encoded strings, each on an `align`-byte boundary with 0xFF fill, the run padded to
        `pad` with 0xFF; and each string's offset"""
        out, offsets = bytearray(), []
        for s in strings:
            while len(out) % align:
                out.append(END)
            offsets.append(len(out))
            out += s
        while len(out) % pad:
            out.append(END)
        return bytes(out), offsets


# -- the HUD font ---------------------------------------------------------------------------------------------------
HUD_VOICED, HUD_SEMI_VOICED, HUD_HALF_SPACE = 0x80, 0x81, 0x82
_MARK_OF = {HUD_VOICED: '゙', HUD_SEMI_VOICED: '゚'}     # the combining voiced and semi-voiced marks
_CODE_OF_MARK = dict((v, k) for k, v in _MARK_OF.items())


class HudEncoding(object):
    """the HUD font's encoding (text/hud_font.json; DrawHUDText and GetHUDTextWidth):
      - a byte below 0x80 is a glyph: the cell in row (byte & 0x70) >> 4, column byte & 0x0F;
      - 0x80 and 0x81 put the voiced and semi-voiced marks on the glyph before them (drawn from columns 15 and 16 of
        that glyph's row; only the rows the definition gives marks hold them);
      - 0x82 is a half space; 0xFF ends the string; the other bytes are unused.
    The source text (one string, the 0xFF implicit): the table's characters (a cell may draw more than one letter: Lv;
    ASCII stands for the fullwidth form too); a voiced or semi-voiced kana for its glyph and the mark (ガ, パ); a space
    for 0x82; {R,C} for a cell the table gives no character; {code XX} for any other byte, a mark the rules above
    cannot state included; {bytes XX ...} for raw bytes."""

    def __init__(self, path=None, data=None):
        if data is None:
            with open(path or HUD_DEFAULT, encoding='utf-8') as f:
                data = json.load(f)
        self.data = data
        self.mark_rows = set()          # the rows whose columns 15 and 16 hold the marks
        self.char_of = {}               # code -> character(s)
        self.cell_of = {}               # character(s) -> code
        for g in data['glyphs']:
            if g.get('mark'):
                self.mark_rows.add(g['row'])
            elif g.get('char') and g['column'] < 16:
                code = g['row'] << 4 | g['column']
                if g['char'] in self.cell_of:
                    raise TextError('%r is named by two cells' % g['char'])
                self.char_of[code] = g['char']
                self.cell_of[g['char']] = code
        for ch, code in list(self.cell_of.items()):
            if len(ch) == 1 and 0x21 <= ord(ch) <= 0x7E:
                self.cell_of.setdefault(chr(ord(ch) + 0xFEE0), code)     # the fullwidth form of an ASCII character
        self.longest = max(len(ch) for ch in self.cell_of)

    def decode(self, data, start=0):
        """the source text of the string at data[start:] and the offset after its 0xFF. Lossless: the text encodes back
        to the same bytes, or the string is written as {bytes ...}."""
        end = data.index(END, start) + 1
        raw = bytes(data[start:end])
        text = self._decode(raw)
        if self.encode(text) != raw:
            text = '{bytes ' + ' '.join('%02X' % b for b in raw[:-1]) + '}'
        return text, end

    def _decode(self, raw):
        out = []
        last = None                     # the last glyph's code, if a mark could follow it
        for b in raw[:-1]:
            if b < 0x80:
                ch = self.char_of.get(b)
                out.append(ch if ch else '{%d,%d}' % (b >> 4, b & 0x0F))
                last = b if ch and len(ch) == 1 else None
            elif b in _MARK_OF:
                c = None
                if last is not None and (last >> 4) in self.mark_rows:
                    c = unicodedata.normalize('NFC', out[-1] + _MARK_OF[b])
                if c is not None and len(c) == 1:
                    out[-1] = c
                else:
                    out.append('{code %02X}' % b)
                last = None
            elif b == HUD_HALF_SPACE:
                out.append(' ')
                last = None
            else:
                out.append('{code %02X}' % b)
                last = None
        return ''.join(out)

    def encode(self, text):
        """the bytes of one string's source text, 0xFF included"""
        out = bytearray()
        i, n = 0, len(text)
        while i < n:
            ch = text[i]
            if ch in '\r\n':
                i += 1
                continue
            if ch == '{':
                j = text.find('}', i)
                if j < 0:
                    raise TextError('an unclosed { at %d' % i)
                tok = text[i + 1:j].strip()
                i = j + 1
                m = re.match(r'^(\d+)\s*,\s*(\d+)$', tok)
                if m:
                    r, c = int(m.group(1)), int(m.group(2))
                    if not (0 <= r < 8 and 0 <= c < 16):
                        raise TextError('{%s}: a byte reaches rows 0 to 7 and columns 0 to 15' % tok)
                    out.append(r << 4 | c)
                    continue
                m = re.match(r'^code\s+([0-9A-Fa-f]{2})$', tok)
                if m and 0x80 <= int(m.group(1), 16) <= 0xFE:
                    out.append(int(m.group(1), 16))
                    continue
                m = re.match(r'^bytes((?:\s+[0-9A-Fa-f]{2})+)$', tok)
                if m:
                    out.extend(int(x, 16) for x in m.group(1).split())
                    continue
                raise TextError('unknown token {%s}' % tok)
            if ch == ' ':
                out.append(HUD_HALF_SPACE)
                i += 1
                continue
            for k in range(min(self.longest, n - i), 0, -1):      # the longest name first (Lv)
                if text[i:i + k] in self.cell_of:
                    out.append(self.cell_of[text[i:i + k]])
                    i += k
                    break
            else:
                d = unicodedata.normalize('NFD', ch)
                if len(d) == 2 and d[1] in _CODE_OF_MARK and d[0] in self.cell_of:
                    code = self.cell_of[d[0]]
                    if (code >> 4) not in self.mark_rows:
                        raise TextError('%r: the HUD font draws %r in row %d, which holds no voicing mark'
                                        % (ch, d[0], code >> 4))
                    out.append(code)
                    out.append(_CODE_OF_MARK[d[1]])
                    i += 1
                    continue
                raise TextError('%r (U+%04X) is not in the HUD font' % (ch, ord(ch)))
        if END in out:
            raise TextError('a 0xFF inside the string')
        out.append(END)
        return bytes(out)


# -- the source file ------------------------------------------------------------------------------------------------
_HEAD = re.compile(r'^@([A-Za-z_][A-Za-z0-9_]*)((?:\s+[a-z]+)*)\s*$')


def read_source(path):
    """[(name, [attributes], body text)] in file order; '#' lines are comments"""
    entries, cur = [], None
    with open(path, encoding='utf-8') as f:
        for k, line in enumerate(f, 1):
            line = line.rstrip('\n').rstrip('\r')
            if line.startswith('#'):    # no glyph is '#': a line that starts with one is a comment
                continue
            m = _HEAD.match(line)
            if m:
                cur = [m.group(1), m.group(2).split(), []]
                entries.append(cur)
                continue
            if line.startswith('@'):
                raise TextError('%s:%d: a string heading is @NAME [attributes]' % (path, k))
            if cur is None:
                if line.strip():
                    raise TextError('%s:%d: text before the first @NAME' % (path, k))
                continue
            cur[2].append(line)
    names = [e[0] for e in entries]
    dup = sorted(set(n for n in names if names.count(n) > 1))
    if dup:
        raise TextError('%s: names used twice: %s' % (path, ', '.join(dup)))
    return [(n, a, '\n'.join(b)) for n, a, b in entries]


def body_lines(text):
    """a decoded body laid out one line per \\n, \\f or \\e; a 0x7F at a line's end written {sp}"""
    lines, cur, i = [], '', 0
    while i < len(text):
        if text[i] == '\\' and i + 1 < len(text) and text[i + 1] in ESCAPES:
            cur += text[i:i + 2]
            lines.append(cur)
            cur = ''
            i += 2
            continue
        cur += text[i]
        i += 1
    if cur or not lines:
        lines.append(cur)
    return [(l[:-1] + '{sp}') if l.endswith('　') else l for l in lines]
