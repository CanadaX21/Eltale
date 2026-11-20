"""asset_patches.py: authored changes to generated asset files, kept apart and put back after every extraction.

Extraction (./configure, which runs splat) writes assets/ from the ROM every time, so an edit made there would not
last. An authored change is kept apart from the generated file, under asset_patches/ at the same path, and put back
after each extraction:

    assets/boss_guilty/records.anim.inc.c  ->  asset_patches/boss_guilty/records.anim.inc.c.patch
    assets/boss_guilty/dl_00.gfx.inc.c     ->  asset_patches/boss_guilty/dl_00.gfx.inc.c.patch
    assets/boss_guilty/vtx_00.vtx.inc.c    ->  asset_patches/boss_guilty/vtx_00.vtx.inc.c.patch
    assets/boss_guilty/tex_00.rgba16.png   ->  asset_patches/boss_guilty/tex_00.rgba16.png (the whole edited image)
                                               asset_patches/boss_guilty/tex_00.rgba16.png.base (the image it replaces)

Each generated file has its extracted form beside it as FILE.pristine; a change is made against that.

Text (the generated C: records, display lists, vertex runs) is kept as a unified diff of its pristine text. A patch
applies exactly or not at all. Its first line, written by `save`, names the SHA-1 of the text it was made against and
its number of hunks:
  - every hunk has context (a hunk of added lines alone, which nothing could place, is refused), every line ends with
    its newline (a cut line is a cut patch), and the hunk count must match (a patch cut between hunks);
  - made against this very text: each hunk must match exactly at the line it names;
  - made against another text (the generator or its definitions have changed since): each hunk's context must occur
    exactly once after the previous hunk, wherever that is, and the step reports that it applied it by context;
  - otherwise ./configure stops and names the patch. Remake it from the current file with `save`.

An image is kept whole (an edited PNG; its size may change too, with the display list that loads it). Its .base file
names the image it replaces by a SHA-1 of the extracted image's content (size, format, palette and pixel rows, so not
of the PNG's compressed bytes, which another PNG writer could change). After an extraction the kept image is put back
only when the freshly extracted image is that one; otherwise ./configure stops and names it (the ROM, the definitions
or the extraction changed what it replaces: look at the new image and save the edit again). No delta: the kept image is
the developer's whole image. Two images with the same content compare equal whatever wrote them.

assets/.asset_state.tsv records, for each generated file, its SHA-1 and its pristine copy's as the last ./configure
left them. ./configure runs, in order:
  guard      before the split: a file as the last ./configure left it is no edit (kept changes added or removed since
             are simply applied or not by this extraction). A file that changed since must have the same content as
             its pristine copy with its kept change (as `save` and `restore` leave it), or it holds an edit not kept
             yet, which the split would overwrite: ./configure stops and names it (keep it with `save FILE` or `save
             --all`, or drop it with `restore FILE`). A file that is its pristine copy while it has a kept change (a
             revert by hand, or the split run outside ./configure) stops it too: `restore` puts the change back, a
             named `save` removes it (`save --all` leaves it). When its pristine copy changed too, or is gone, what the
             file holds cannot be told, and ./configure stops as well. It also stops when the last ./configure was cut
             off during its split (assets/.split_running is still there): see `resync`; `save` and `restore` refuse
             then. Passing, it writes the marker with each display list, vertex run and image's SHA-1 as it is.
  (splat)    the split; splat's anim, records and maprecords types (tools/splat_ext/) write the record files' pristine
             copies and apply their patches themselves. When the split fails, ./configure runs `resync` at once: the
             files that changed since the marker was written are the split's output (nothing else runs meanwhile).
  extracted  after it: the display lists, vertex runs and textures (*.gfx.inc.c, *.vtx.inc.c, *.png) get their pristine
             copies and their kept changes, and the state file is written. A kept change that does not apply, whose
             generated file the split no longer writes, or a file under asset_patches/ that is no kept change, stops
             ./configure; a file whose kept change does not apply is removed with its build objects (as the record
             types do), so that `ninja` run alone stops instead of building it without the change.
So a change survives re-extraction, and a clean clone with the original ROM and the asset_patches/ folder rebuilds the
same ROM. The folder holds the developer's own changes: the project ships none (the original build uses no patch).

    python3 tools/asset_patches.py save FILE...     keep each FILE's change (no change: the kept one is removed)
    python3 tools/asset_patches.py save --all       keep the change of every generated file whose edit is not kept
    python3 tools/asset_patches.py restore FILE...  drop an edit not kept: FILE becomes its pristine copy with its kept
                                                    change, as the next extraction would write it
    python3 tools/asset_patches.py check            apply every kept change to its pristine copy in memory and report
    python3 tools/asset_patches.py list             the kept changes and the generated files they change
    python3 tools/asset_patches.py resync           after a split that stopped: take each display list, vertex run and
                                                    image that changed since the guard wrote the marker as the split's
                                                    output (its new pristine copy), record the state, remove the marker.
                                                    ./configure runs it when its split fails; run it by hand only after
                                                    a ./configure cut off during its split, and only if no generated
                                                    file was edited since
    python3 tools/asset_patches.py guard [--begin-split] | extracted   ./configure's steps (above)

Run from the project's root. Python 3 standard library only.
"""
import difflib
import hashlib
import os
import re
import shutil
import struct
import sys
import zlib

ASSETS, PATCHES = 'assets', 'asset_patches'
PRISTINE = '.pristine'
# the files this step keeps pristine copies of and puts kept changes back into (splat's own gfx, vtx and image types)
TEXT_KINDS = ('.gfx.inc.c', '.vtx.inc.c', '.dialogue.txt',     # the dialogue bank's source text too
              '.npcnames.txt')                                # and the NPC files' name pools
IMAGE_KIND = '.png'
# the record files, whose pristine copies and patches splat's anim, records and maprecords types handle
RECORD_KINDS = ('.anim.inc.c', '.records.inc.c', '.maprecords.inc.c', '.spellrecords.inc.c', '.npcrecords.inc.c',
                '.gfxrecords.inc.c')
BASE = '.base'
STATE = '.asset_state.tsv'      # under assets/: each generated file's SHA-1 and its pristine copy's, as last left
MARKER = '.split_running'       # under assets/: written before the split, removed by `extracted` or `resync`
BUILD = 'build'
_HEADER = re.compile(r'# asset_patches: made against ([0-9a-f]{40}), (\d+) hunks?$')
_HUNK = re.compile(r'@@ -(\d+)(?:,(\d+))? \+(\d+)(?:,(\d+))? @@')
_BASE = re.compile(r'# asset_patches: replaces the extracted image ([0-9a-f]{40})\b')


class PatchError(Exception):
    pass


def text_sha1(text):
    return hashlib.sha1(text.encode('utf-8')).hexdigest()


def _rel(asset_file, root='.'):
    rel = os.path.relpath(os.path.join(root, asset_file), os.path.join(root, ASSETS)).replace(os.sep, '/')
    if rel.startswith('../'):
        raise PatchError('%s is not under %s/' % (asset_file, ASSETS))
    return rel


def patch_path(asset_file, root='.'):
    """asset_patches/<path under assets/>.patch for a generated text file under assets/"""
    return os.path.normpath(os.path.join(root, PATCHES, _rel(asset_file, root) + '.patch'))


def image_path(asset_file, root='.'):
    """asset_patches/<path under assets/> (the kept image) for a generated image under assets/"""
    return os.path.normpath(os.path.join(root, PATCHES, _rel(asset_file, root)))


def is_image(path):
    return path.endswith(IMAGE_KIND)


# -- text ----------------------------------------------------------------------------------------------------------
def _lines(text):
    lines = text.splitlines(True)
    if lines and not lines[-1].endswith('\n'):
        lines[-1] += '\n'
    return lines


def make(pristine, edited, rel):
    """the patch of a generated file's text (pristine -> edited), with its header; '' when they are equal (a file's
    missing final newline is given one: a patch line always ends with its newline)"""
    diff = ''.join(difflib.unified_diff(_lines(pristine), _lines(edited), 'a/%s/%s' % (ASSETS, rel),
                                        'b/%s/%s' % (ASSETS, rel), n=3))
    if not diff:
        return ''
    hunks = sum(1 for l in diff.splitlines() if l.startswith('@@'))
    return '# asset_patches: made against %s, %d hunk%s\n%s' % (text_sha1(pristine), hunks, '' if hunks == 1 else 's', diff)


def apply(pristine, patch, name='patch', notes=None):
    """the pristine text with the patch applied, exactly (see the module's notes); raises PatchError. A note on how it
    was placed, when not at the lines it names in the text it was made against, is appended to `notes`"""
    patch = patch.replace('\r\n', '\n')
    src = pristine.splitlines(True)
    lines = patch.splitlines(True)
    made_against, want_hunks, i = None, None, 0
    while i < len(lines) and not lines[i].startswith('@@'):
        m = _HEADER.match(lines[i].rstrip('\n'))
        if m:
            made_against, want_hunks = m.group(1), int(m.group(2))
        i += 1
    hunks = []
    while i < len(lines):
        m = _HUNK.match(lines[i])
        if not m:
            raise PatchError('%s: line %d is not a hunk header' % (name, i + 1))
        old_start, old_n = int(m.group(1)), int(m.group(2) or 1)
        new_n = int(m.group(4) or 1)
        i += 1
        entries = []
        while i < len(lines) and not lines[i].startswith('@@'):
            l = lines[i]
            if l.startswith('\\'):                      # "\ No newline at end of file": the line before has none
                if not entries:
                    raise PatchError('%s: line %d: a no-newline mark with no line before it' % (name, i + 1))
                entries[-1] = (entries[-1][0], entries[-1][1].rstrip('\n'))
                i += 1
                continue
            tag, text = l[:1], l[1:]
            if tag not in (' ', '-', '+'):
                raise PatchError('%s: line %d is not part of a hunk' % (name, i + 1))
            if not text.endswith('\n'):
                raise PatchError('%s: line %d has no newline: the patch is cut' % (name, i + 1))
            entries.append((tag, text))
            i += 1
        old = [t for g, t in entries if g in (' ', '-')]
        new = [t for g, t in entries if g in (' ', '+')]
        if len(old) != old_n or len(new) != new_n:
            raise PatchError('%s: the hunk at line %d has %d/%d lines, its header says %d/%d: the patch is cut' % (
                name, old_start, len(old), len(new), old_n, new_n))
        if not old:
            raise PatchError('%s: the hunk at line %d has no context line, so nothing places it; make patches with '
                             '`asset_patches.py save`' % (name, old_start))
        hunks.append((old_start, old, new))
    if not hunks:
        raise PatchError('%s: no hunk' % name)
    if want_hunks is not None and want_hunks != len(hunks):
        raise PatchError('%s: %d hunks, its header says %d: the patch is cut' % (name, len(hunks), want_hunks))
    same_text = made_against is not None and made_against == text_sha1(pristine)
    if made_against is None:
        notes is not None and notes.append('%s has no asset_patches header: applied by its context' % name)
    elif not same_text:
        notes is not None and notes.append('%s was made against another generated text (%s, now %s): applied by its '
                                           'context' % (name, made_against[:12], text_sha1(pristine)[:12]))
    out, pos = [], 0
    for k, (old_start, old, new) in enumerate(hunks):
        want = old_start - 1
        if same_text:
            if want < pos or src[want:want + len(old)] != old:
                raise PatchError('%s: hunk %d does not match at line %d of the text it was made against: the patch was '
                                 'edited; remake it with `asset_patches.py save`' % (name, k + 1, old_start))
            at = want
        else:
            # another text: the context must occur exactly once after the previous hunk, wherever it is
            cands = [j for j in range(pos, len(src) - len(old) + 1) if src[j:j + len(old)] == old]
            if len(cands) != 1:
                raise PatchError('%s: hunk %d (line %d) matches %s; remake the patch from the current file with '
                                 '`asset_patches.py save`' % (name, k + 1, old_start,
                                                              'nowhere' if not cands else '%d places' % len(cands)))
            at = cands[0]
            if at != want:
                notes is not None and notes.append('%s: hunk %d placed at line %d, not %d' % (name, k + 1, at + 1, old_start))
        out += src[pos:at] + new
        pos = at + len(old)
    result = out + src[pos:]
    if any(not l.endswith('\n') for l in result[:-1]):
        raise PatchError('%s: a line without its newline lands before the end of the file' % name)
    return ''.join(result)


# -- images ----------------------------------------------------------------------------------------------------------
def _paeth(a, b, c):
    p = a + b - c
    pa, pb, pc = abs(p - a), abs(p - b), abs(p - c)
    return a if pa <= pb and pa <= pc else (b if pb <= pc else c)


def image_key(data, name='image'):
    """(SHA-1 of a PNG's image, (width, height, bit depth, colour type)): the size, format, palette, transparency
    and pixel rows, decompressed and unfiltered, so the same image written by another PNG writer has the same key;
    raises PatchError for anything that is not a PNG this reads (interlaced PNGs are refused)"""
    if data[:8] != b'\x89PNG\r\n\x1a\n':
        raise PatchError('%s is not a PNG' % name)
    pos, ihdr, plte, trns, idat = 8, None, b'', b'', []
    while pos + 8 <= len(data):
        n, typ = struct.unpack('>I4s', data[pos:pos + 8])
        body = data[pos + 8:pos + 8 + n]
        if len(body) != n:
            raise PatchError('%s: a chunk is cut' % name)
        pos += 12 + n
        if typ == b'IHDR':
            ihdr = body
        elif typ == b'PLTE':
            plte = body
        elif typ == b'tRNS':
            trns = body
        elif typ == b'IDAT':
            idat.append(body)
        elif typ == b'IEND':
            break
    if ihdr is None or len(ihdr) != 13 or not idat:
        raise PatchError('%s: no image header or no image data' % name)
    w, h, depth, ctype, _comp, _filt, interlace = struct.unpack('>IIBBBBB', ihdr)
    channels = {0: 1, 2: 3, 3: 1, 4: 2, 6: 4}.get(ctype)
    if channels is None or interlace:
        raise PatchError('%s: colour type %d%s is not read here' % (name, ctype, ', interlaced' if interlace else ''))
    stride = (w * channels * depth + 7) // 8
    bpp = max(1, channels * depth // 8)
    try:
        raw = zlib.decompress(b''.join(idat))
    except zlib.error as ex:
        raise PatchError('%s: its image data does not decompress (%s)' % (name, ex))
    if len(raw) < h * (stride + 1):
        raise PatchError('%s: its image data is cut' % name)
    rows, prev, p = [], bytearray(stride), 0
    for _ in range(h):
        ft, line = raw[p], bytearray(raw[p + 1:p + 1 + stride])
        p += 1 + stride
        for x in range(stride):
            a = line[x - bpp] if x >= bpp else 0
            b = prev[x]
            c = prev[x - bpp] if x >= bpp else 0
            if ft == 1:
                line[x] = (line[x] + a) & 0xFF
            elif ft == 2:
                line[x] = (line[x] + b) & 0xFF
            elif ft == 3:
                line[x] = (line[x] + ((a + b) >> 1)) & 0xFF
            elif ft == 4:
                line[x] = (line[x] + _paeth(a, b, c)) & 0xFF
            elif ft != 0:
                raise PatchError('%s: filter type %d in a row' % (name, ft))
        rows.append(bytes(line))
        prev = line
    digest = hashlib.sha1(b'%d %d %d %d\n' % (w, h, depth, ctype))
    for part in (plte, trns):
        digest.update(b'%d\n' % len(part) + part)
    for r in rows:
        digest.update(r)
    return digest.hexdigest(), (w, h, depth, ctype)


CI8_KIND = '.ci8.png'           # the spell bank's CI8 textures (splat's ci8tex type)
CI4_KIND = '.ci4.png'           # the font sheet (splat's ci4tex type)


def png_palette(data, name='image'):
    """(colour type, PLTE bytes, tRNS bytes) of a PNG"""
    if data[:8] != b'\x89PNG\r\n\x1a\n':
        raise PatchError('%s is not a PNG' % name)
    pos, ctype, plte, trns = 8, None, b'', b''
    while pos + 8 <= len(data):
        n, typ = struct.unpack('>I4s', data[pos:pos + 8])
        body = data[pos + 8:pos + 8 + n]
        pos += 12 + n
        if typ == b'IHDR' and len(body) == 13:
            ctype = body[9]
        elif typ == b'PLTE':
            plte = body
        elif typ == b'tRNS':
            trns = body
        elif typ == b'IEND':
            break
    return ctype, plte, trns


def ci_palette_problem(f, kept_data, pristine_data):
    """a kept CI8 or CI4 image whose palette is not the extracted one's, or None. The build reads only a colour-indexed
    image's indices (tools/image_converter.py ci8, ci4); its colours come from palettes kept elsewhere (a CI8 texture's
    from the palette record its material names, the spell bank's four palette records; the font sheet's from the text
    palettes, font/text_palettes.tlut.rgba16.png, one palette per row), and the PNG's palette is a copy for viewing. A
    colour changed in an image editor would be lost without a word, so the kept image must carry the extracted palette."""
    if f.endswith(CI8_KIND):
        kind, why = 'CI8', ('its colours are the palette record its material names (the spell bank\'s palette records); '
                            'keep the extracted palette in the image (same colours, same order) and change colours in '
                            'the palette record')
    elif f.endswith(CI4_KIND):
        kind, why = 'CI4', ('its colours are the text palettes (font/text_palettes.tlut.rgba16.png, one palette per row, '
                            'the row a string\'s {pal N} picks); keep the extracted palette in the image (same colours, '
                            'same order) and change colours in the palettes image')
    else:
        return None
    if png_palette(kept_data, f) != png_palette(pristine_data, f + PRISTINE):
        return ('%s: a %s image whose palette is not the extracted one: the build reads only its colour indices, and %s'
                % (f, kind, why))
    return None


def _describe(fmt):
    w, h, depth, ctype = fmt
    return '%dx%d, colour type %d, depth %d' % (w, h, ctype, depth)


def _read_base(path):
    with open(path, encoding='utf-8') as f:
        first = f.readline().rstrip('\n')
    m = _BASE.match(first)
    if not m:
        raise PatchError('%s: its first line does not name the image it replaces (save the image again)' % path)
    return m.group(1)


# -- files, state ----------------------------------------------------------------------------------------------------
def _read(path):
    with open(path, encoding='utf-8') as f:            # universal newlines: a CRLF copy reads as LF
        return f.read()


def _read_bytes(path):
    with open(path, 'rb') as f:
        return f.read()


def _sha1_file(path):
    return hashlib.sha1(_read_bytes(path)).hexdigest()


def _norm(f, root='.'):
    return os.path.normpath(os.path.relpath(f, root)).replace(os.sep, '/')


def _state_path(root='.'):
    return os.path.join(root, ASSETS, STATE)


def _marker_path(root='.'):
    return os.path.join(root, ASSETS, MARKER)


def _read_state(root='.'):
    """{generated file: (its SHA-1, its pristine copy's SHA-1 or None)} as the last `extracted` or `resync` left them"""
    out = {}
    p = _state_path(root)
    if os.path.isfile(p):
        with open(p, encoding='utf-8') as f:
            for line in f:
                if line.startswith('#'):
                    continue
                parts = line.rstrip('\n').split('\t')
                if len(parts) >= 2:
                    out[parts[0]] = (parts[1], parts[2] if len(parts) > 2 else None)
    return out


def _generated(root='.'):
    """every generated file under assets/ this tool follows: those with a pristine copy beside them"""
    base = os.path.join(root, ASSETS)
    for d, _, files in os.walk(base):
        for name in sorted(files):
            if name.endswith(PRISTINE):
                yield os.path.normpath(os.path.join(d, name[:-len(PRISTINE)]))


def _write_state(root='.'):
    """record each generated file's SHA-1 and its pristine copy's (the guard's reference at the next ./configure)"""
    rows = []
    for f in _generated(root):
        if os.path.isfile(f):
            rows.append('%s\t%s\t%s\n' % (_norm(f, root), _sha1_file(f), _sha1_file(f + PRISTINE)))
    with open(_state_path(root), 'w', encoding='utf-8', newline='\n') as w:
        w.write('# written by tools/asset_patches.py: each generated file and its pristine copy, as the last '
                './configure left them\n')
        w.writelines(sorted(rows))


def expected(f, root='.', notes=None):
    """what the extraction leaves at generated file f: its pristine copy with its kept change (bytes for an image,
    text otherwise); raises PatchError when the kept change does not apply"""
    pf = f + PRISTINE
    if is_image(f):
        kept = image_path(f, root)
        if os.path.isfile(kept):
            if not os.path.isfile(kept + BASE):
                raise PatchError('%s has no %s naming the image it replaces: save the edit again' % (kept, kept + BASE))
            base = _read_base(kept + BASE)
            key, _ = image_key(_read_bytes(pf), pf)
            if key != base:
                raise PatchError('%s: the extracted image is not the one %s replaces (it was %s, it is now %s): look at '
                                 'the new image and save the edit again, or remove the kept image' % (
                                     f, kept, base[:12], key[:12]))
            image_key(_read_bytes(kept), kept)          # it must still be a PNG this reads
            problem = ci_palette_problem(f, _read_bytes(kept), _read_bytes(pf))
            if problem:
                raise PatchError(problem)
            return _read_bytes(kept)
        return _read_bytes(pf)
    text = _read(pf)
    p = patch_path(f, root)
    if os.path.isfile(p):
        text = apply(text, _read(p), p, notes)
    return text


def same_content(f, exp):
    """generated file f has the content `exp` (as `expected` gives it): an image by its image key, so a PNG written
    again with the same pixels compares equal; text with its final newline as a patch line has it"""
    if isinstance(exp, bytes):
        data = _read_bytes(f)
        if data == exp:
            return True
        try:
            return image_key(data, f)[0] == image_key(exp, 'the expected image')[0]
        except PatchError:
            return False
    return ''.join(_lines(_read(f))) == ''.join(_lines(exp))


def _has_kept(f, root='.'):
    return os.path.isfile(image_path(f, root)) if is_image(f) else os.path.isfile(patch_path(f, root))


def edits(root='.'):
    """(the generated files holding an edit not kept, those that are their extracted form without their kept change,
    error lines for files whose state cannot be told, files checked): the guard's rule, also `save --all`'s"""
    found, unapplied, errors, n = [], [], [], 0
    state = _read_state(root)
    seen = set()
    for f in list(_generated(root)) + [os.path.normpath(os.path.join(root, r)) for r in state]:
        if f in seen or not os.path.isfile(f):
            continue
        seen.add(f)
        n += 1
        entry = state.get(_norm(f, root))
        pf = f + PRISTINE
        try:
            if entry and entry[0] == _sha1_file(f):
                continue                                # as the last ./configure left it
            if entry and (not os.path.isfile(pf) or (entry[1] is not None and entry[1] != _sha1_file(pf))):
                errors.append('%s changed since the last ./configure, and so did its pristine copy %s (edited, '
                              'replaced or removed), so whether it holds an edit that is not kept cannot be told: if it '
                              'holds one you want, copy it somewhere safe; then delete %s and %s and run ./configure'
                              % (f, pf, f, pf))
                continue
            if not os.path.isfile(pf):
                continue                                # no record and no pristine copy: nothing to compare it with
            if same_content(f, expected(f, root)):
                continue                                # its pristine copy with its kept change (as save and restore leave it)
            pristine = _read_bytes(pf) if is_image(f) else _read(pf)
            if _has_kept(f, root) and same_content(f, pristine):
                unapplied.append(f)                     # a revert, or the split run outside ./configure
            else:
                found.append(f)
        except (PatchError, OSError) as ex:
            errors.append('%s changed since the last ./configure, and whether it holds an edit that is not kept cannot '
                          'be told: %s' % (f, ex))
    return found, unapplied, errors, n


def _remove_built(f, root='.'):
    """remove a generated file and its build objects, so that `ninja` run alone stops on it"""
    gone = []
    rel = _norm(f, root)
    for p in (f, os.path.join(root, BUILD, rel + '.o'), os.path.join(root, BUILD, rel + '.j'),
              os.path.join(root, BUILD, rel[:-len(IMAGE_KIND)] + '.j') if is_image(f) else None):
        if p and os.path.isfile(p):
            os.remove(p)
            gone.append(p)
    return gone


# -- the commands --------------------------------------------------------------------------------------------------
def save(f, root='.'):
    """keep generated file f's change; returns (ok, message)"""
    pf = f + PRISTINE
    if not os.path.isfile(pf):
        return False, ('%s: no %s: nothing puts a kept change back into this file (only the generated files with a '
                       'pristine copy beside them; run ./configure first)' % (f, pf))
    entry = _read_state(root).get(_norm(f, root))
    if entry and entry[1] is not None and entry[1] != _sha1_file(pf):
        return False, ('%s: not saved: its pristine copy %s changed since the last ./configure, so a change made '
                       'against it would not be the one made against the extracted file' % (f, pf))
    rel = _rel(f, root)
    if is_image(f):
        pdata, edata = _read_bytes(pf), _read_bytes(f)
        try:
            pkey, pfmt = image_key(pdata, pf)
            ekey, efmt = image_key(edata, f)
        except PatchError as ex:
            return False, '%s: not saved: %s' % (f, ex)
        kept = image_path(f, root)
        problem = ci_palette_problem(f, edata, pdata) if ekey != pkey else None
        if problem:
            return False, problem + ': not saved'
        if ekey == pkey:
            removed = [x for x in (kept, kept + BASE) if os.path.isfile(x)]
            for x in removed:
                os.remove(x)
            return True, ('%s: the extracted image%s' % (f, '; removed %s' % kept if removed else ''))
        os.makedirs(os.path.dirname(kept), exist_ok=True)
        with open(kept, 'wb') as w:
            w.write(edata)
        with open(kept + BASE, 'w', encoding='utf-8', newline='\n') as w:
            w.write('# asset_patches: replaces the extracted image %s (%s) of %s/%s\n' % (
                pkey, _describe(pfmt), ASSETS, rel))
        note = '' if efmt == pfmt else (' (it was %s, it is %s: the display list that loads it must load the new '
                                        'one)' % (_describe(pfmt), _describe(efmt)))
        return True, '%s: saved %s whole%s' % (f, kept, note)
    pristine, edited = _read(pf), _read(f)
    diff = make(pristine, edited, rel)
    out = patch_path(f, root)
    if not diff:
        if os.path.isfile(out):
            os.remove(out)
            return True, '%s: no change from the generated text; removed %s' % (f, out)
        return True, '%s: no change from the generated text' % f
    if apply(pristine, diff, out) != ''.join(_lines(edited)):   # the patch must rebuild the edited file
        return False, '%s: the patch would not rebuild the edited file: not saved' % f
    os.makedirs(os.path.dirname(out), exist_ok=True)
    with open(out, 'w', encoding='utf-8', newline='\n') as w:
        w.write(diff)
    body = diff.splitlines()[3:]
    return True, '%s: saved %s (+%d -%d lines)' % (f, out, sum(1 for l in body if l.startswith('+')),
                                                   sum(1 for l in body if l.startswith('-')))


def restore(f, root='.'):
    exp = expected(f, root)
    if isinstance(exp, bytes):
        with open(f, 'wb') as w:
            w.write(exp)
    else:
        with open(f, 'w', encoding='utf-8', newline='\n') as w:
            w.write(exp)


def guard(root='.', begin_split=False):
    """./configure's step before the split; returns the list of error lines. With begin_split (./configure's call), a
    passing guard leaves the marker that `extracted` or `resync` removes"""
    if os.path.isfile(_marker_path(root)):
        return [CUT_OFF % _marker_path(root)]
    found, unapplied, errors, n = edits(root)
    for f in found:
        errors.append('%s holds an edit that is not kept: keep it with `python3 tools/asset_patches.py save %s`, or '
                      'drop it with `python3 tools/asset_patches.py restore %s`' % (f, f, f))
    for f in unapplied:
        errors.append('%s is its extracted form without its kept change (reverted by hand, or split outside '
                      './configure): `python3 tools/asset_patches.py restore %s` puts the kept change back; `python3 '
                      'tools/asset_patches.py save %s` removes the kept change' % (f, f, f))
    if not errors:
        print('asset_patches: guard: %d generated files checked, none holds an edit that is not kept' % n)
        if begin_split:
            os.makedirs(os.path.join(root, ASSETS), exist_ok=True)
            rows = ['%s\t%s\n' % (_norm(f, root), _sha1_file(f)) for f in _split_written(root)]
            tmp = _marker_path(root) + '.tmp'           # written whole, then put in place: never a marker cut short
            with open(tmp, 'w', encoding='utf-8', newline='\n') as w:
                w.write('# asset_patches split snapshot: %d files (written by tools/asset_patches.py guard before the '
                        'split: each display list, vertex run and image as it was then; removed when the split is '
                        'followed, by extracted or resync)\n' % len(rows))
                w.writelines(sorted(rows))
            os.replace(tmp, _marker_path(root))
    return errors


CUT_OFF = ('the last ./configure was cut off during its split (%s is still there): a file that split rewrote cannot be '
           'told from an edit made since. If no generated file was edited since, run `python3 tools/asset_patches.py '
           'resync`, then ./configure; if one was, copy it somewhere safe first')


def _split_written(root='.'):
    """the generated files splat's own types write (display lists, vertex runs, images), whose pristine copies this
    tool keeps"""
    base = os.path.join(root, ASSETS)
    for d, _, files in os.walk(base):
        for name in sorted(files):
            if name.endswith(TEXT_KINDS) or name.endswith(IMAGE_KIND):
                yield os.path.normpath(os.path.join(d, name))


_SNAPSHOT = re.compile(r'# asset_patches split snapshot: (\d+) files\b')


def _read_marker(root='.'):
    """({file: SHA-1} as the guard found them just before the split, whether the snapshot is whole: its first line
    states how many files it holds, and it holds that many; a marker from an earlier version has no such line)"""
    out, want = {}, None
    with open(_marker_path(root), encoding='utf-8') as f:
        for k, line in enumerate(f):
            m = _SNAPSHOT.match(line) if k == 0 else None
            if m:
                want = int(m.group(1))
            elif not line.startswith('#'):
                parts = line.rstrip('\n').split('\t')
                if len(parts) == 2:
                    out[parts[0]] = parts[1]
    return out, want is not None and want == len(out)


def resync(root='.'):
    """after a split that stopped: each display list, vertex run and image that changed since the guard ran just
    before it (the marker holds them as they were then) is the split's output and becomes its new pristine copy; a
    file the split did not reach keeps its own (a record file's type writes its own). Writes the state, removes the
    marker. Returns False when there is no marker (no split to follow)"""
    if not os.path.isfile(_marker_path(root)):
        print('asset_patches: resync: no %s: no split was cut off, nothing to do' % _marker_path(root))
        return False
    before, whole = _read_marker(root)
    if not whole:
        print('asset_patches: resync: not done: %s holds no whole snapshot of the files before the split (it was '
              'written by an earlier version of this tool), so what the split rewrote cannot be told from kept changes. '
              'Copy any generated file you edited since the last ./configure somewhere safe, then remove the assets/ '
              'folder (kept changes are under asset_patches/ and come back) and run ./configure' % _marker_path(root))
        return False
    taken = 0
    for f in _split_written(root):
        if before.get(_norm(f, root)) != _sha1_file(f) or not os.path.isfile(f + PRISTINE):
            shutil.copyfile(f, f + PRISTINE)
            taken += 1
    _write_state(root)
    os.remove(_marker_path(root))
    print('asset_patches: resync: %d display lists, vertex runs and images the split rewrote taken as their pristine '
          'copies; the state recorded' % taken)
    return True


def extracted(root='.'):
    """./configure's step after the split; returns the list of error lines"""
    errors, copies, applied = [], 0, 0
    base = os.path.join(root, ASSETS)
    try:
        for d, _, files in os.walk(base):
            for name in files:
                if name.endswith(TEXT_KINDS) or name.endswith(IMAGE_KIND):
                    f = os.path.join(d, name)
                    shutil.copyfile(f, f + PRISTINE)
                    copies += 1
        for kind, p, f in _kept(root):
            if kind == 'other':
                errors.append('%s: not a kept change (a text patch ends in .patch, a kept image is a .png with its '
                              '.png.base): remove it or save the change again' % p)
                continue
            if not os.path.isfile(f):
                errors.append('%s: the split wrote no %s, the file it changes: remove it or remake it' % (p, f))
                continue
            if kind == 'text' and f.endswith(RECORD_KINDS):
                continue                                # applied by splat's anim, records and maprecords types
            if kind == 'text' and not f.endswith(TEXT_KINDS):
                errors.append('%s: %s is no generated text this step puts changes into' % (p, f))
                continue
            notes = []
            try:
                exp = expected(f, root, notes)
            except (PatchError, OSError) as ex:
                gone = _remove_built(f, root)
                errors.append('%s: the kept change does not apply: %s (removed %s, so that `ninja` alone stops; to '
                              'remake the change, copy %s to %s, make the edit again and save it)' % (
                                  f, ex, ', '.join(gone), f + PRISTINE, f))
                continue
            if isinstance(exp, bytes):
                with open(f, 'wb') as w:
                    w.write(exp)
            else:
                with open(f, 'w', encoding='utf-8', newline='\n') as w:
                    w.write(exp)
            applied += 1
            print('asset_patches: %s: applied %s' % (f, p))
            for note in notes:
                print('asset_patches: note: %s' % note)
    finally:
        if os.path.isdir(base):
            _write_state(root)                          # also after an error: the files are as this step left them
        if os.path.isfile(_marker_path(root)):
            os.remove(_marker_path(root))
    if not errors:
        print('asset_patches: extracted: %d pristine copies written, %d kept change(s) put back' % (copies, applied))
    return errors


def _kept(root='.'):
    """every kept change: (kind, kept file, generated file); kind 'text' or 'image'; anything else 'other'"""
    base = os.path.join(root, PATCHES)
    for d, _, files in os.walk(base):
        for name in sorted(files):
            p = os.path.normpath(os.path.join(d, name))
            rel = os.path.relpath(p, base).replace(os.sep, '/')
            if name.endswith('.patch'):
                yield 'text', p, os.path.normpath(os.path.join(root, ASSETS, rel[:-len('.patch')]))
            elif name.endswith(IMAGE_KIND):
                yield 'image', p, os.path.normpath(os.path.join(root, ASSETS, rel))
            elif name.endswith(IMAGE_KIND + BASE):
                if not os.path.isfile(p[:-len(BASE)]):
                    yield 'other', p, None          # a .base whose image is missing
            else:
                yield 'other', p, None


def main(argv):
    cmds = ('save', 'restore', 'check', 'list', 'guard', 'extracted', 'resync')
    if not argv or argv[0] not in cmds:
        print(__doc__)
        return 2
    cmd, rc = argv[0], 0
    if cmd in ('guard', 'extracted'):
        errors = guard(begin_split='--begin-split' in argv[1:]) if cmd == 'guard' else extracted()
        for e in errors:
            print('Error: asset_patches: %s' % e, file=sys.stderr)
        if errors and cmd == 'guard':
            print('Error: asset_patches: ./configure stopped before the split, which would overwrite what %d file(s) '
                  'hold' % len(errors), file=sys.stderr)
        return 1 if errors else 0
    if cmd == 'resync':
        return 0 if resync() else 1
    if cmd in ('save', 'restore') and os.path.isfile(_marker_path('.')):
        print('%s: not done: %s' % (cmd, CUT_OFF % _marker_path('.')))
        return 1
    if cmd == 'save':
        files = argv[1:]
        if files == ['--all']:
            files, unapplied, errors, _ = edits()
            for e in errors:
                print('not checked: %s' % e)
                rc = 1
            for f in unapplied:
                print('not saved: %s is its extracted form without its kept change; `restore %s` puts the change back, '
                      '`save %s` (named) removes it' % (f, f, f))
                rc = 1
            if not files and not errors and not unapplied:
                print('save --all: no generated file holds an edit that is not kept')
        elif not files:
            print('save: name the edited files under %s/, or --all' % ASSETS)
            return 2
        for f in files:
            try:
                ok, msg = save(f)
            except (PatchError, OSError) as ex:
                ok, msg = False, '%s: not saved: %s' % (f, ex)
            print(msg)
            rc = rc if ok else 1
    elif cmd == 'restore':
        if len(argv) < 2:
            print('restore: name the generated files under %s/' % ASSETS)
            return 2
        for f in argv[1:]:
            if not os.path.isfile(f + PRISTINE):
                print('%s: no %s: nothing to restore it from' % (f, f + PRISTINE))
                rc = 1
                continue
            try:
                restore(f)
                print('%s: restored (its pristine copy%s)' % (f, ' and its kept change' if (
                    os.path.isfile(image_path(f)) if is_image(f) else os.path.isfile(patch_path(f))) else ''))
            except (PatchError, OSError) as ex:
                print('%s: not restored: %s' % (f, ex))
                rc = 1
    elif cmd == 'check':
        n = 0
        for kind, p, f in _kept():
            n += 1
            if kind == 'other':
                print('FAIL %s: not a kept change' % p)
                rc = 1
                continue
            if not os.path.isfile(f + PRISTINE):
                print('FAIL %s: %s is missing (run ./configure)' % (p, f + PRISTINE))
                rc = 1
                continue
            notes = []
            try:
                exp = expected(f, '.', notes)
            except (PatchError, OSError) as ex:
                print('FAIL %s' % ex)
                rc = 1
                continue
            same = os.path.isfile(f) and same_content(f, exp)
            print('ok   %s -> %s%s' % (p, f, '' if same else ' (the file on disk differs: run ./configure, or save it)'))
            for note in notes:
                print('     note: %s' % note)
        print('%d kept change(s), %s' % (n, 'all apply' if rc == 0 else 'some do not apply'))
    else:
        for kind, p, f in _kept():
            print('%s -> %s' % (p, f if f else '(not a kept change)'))
    return rc


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
