"""asset_patches.py: authored changes to generated asset files, kept as patches.

Extraction (./configure, which runs splat) writes assets/ from the ROM every time, so an edit made there does not last.
An authored change is kept apart from the generated file, as a unified diff of its generated (pristine) text, under
asset_patches/ at the same path:

    assets/boss_guilty/records.anim.inc.c  ->  asset_patches/boss_guilty/records.anim.inc.c.patch

The generator writes the pristine text to FILE.pristine, then applies FILE's patch, if there is one, and writes FILE.
A patch applies exactly or not at all. Its first line, written by `save`, names the SHA-1 of the text it was made
against and its number of hunks:
  - every hunk has context (a hunk of added lines alone, which nothing could place, is refused), every line ends with
    its newline (a cut line is a cut patch), and the hunk count must match (a patch cut between hunks);
  - made against this very text: each hunk must match exactly at the line it names;
  - made against another text (the generator or its definitions have changed since): each hunk must match at its line
    or at exactly one place after the previous hunk, and the generator reports that it applied it by context;
  - otherwise the extraction stops and names the patch. Remake it from the current file with `save`.
So a change survives re-extraction, and a clean clone with the original ROM and the asset_patches/ folder rebuilds the
same ROM. The folder holds the developer's own changes: the decomp's repositories ship none (the original build uses
no patch). Today splat's `anim` type (tools/splat_ext/anim.py) applies these patches.

Only the record files are covered (*.anim.inc.c, *.records.inc.c: the files with a .pristine beside them). The
extracted textures (.png), vertex files and display lists are rewritten from the ROM at every ./configure too, and
no step keeps a change to them: an edit there is lost. `save` refuses a file nothing applies a patch to.

    python3 tools/asset_patches.py save FILE...   write each FILE's change against FILE.pristine (no change: the patch
                                                  is removed)
    python3 tools/asset_patches.py check          apply every patch to its pristine file in memory and report
    python3 tools/asset_patches.py list           the patches and the generated files they change

Run from the project's root. Python 3 standard library only.
"""
import difflib
import hashlib
import os
import re
import sys

ASSETS, PATCHES = 'assets', 'asset_patches'
_HEADER = re.compile(r'# asset_patches: made against ([0-9a-f]{40}), (\d+) hunks?$')
_HUNK = re.compile(r'@@ -(\d+)(?:,(\d+))? \+(\d+)(?:,(\d+))? @@')


class PatchError(Exception):
    pass


def text_sha1(text):
    return hashlib.sha1(text.encode('utf-8')).hexdigest()


def patch_path(asset_file, root='.'):
    """asset_patches/<path under assets/>.patch for a generated file under assets/"""
    rel = os.path.relpath(os.path.join(root, asset_file), os.path.join(root, ASSETS)).replace(os.sep, '/')
    if rel.startswith('../'):
        raise PatchError('%s is not under %s/' % (asset_file, ASSETS))
    return os.path.normpath(os.path.join(root, PATCHES, rel + '.patch'))


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


def _all_patches(root='.'):
    base = os.path.join(root, PATCHES)
    for d, _, files in os.walk(base):
        for f in sorted(files):
            if f.endswith('.patch'):
                p = os.path.normpath(os.path.join(d, f))
                rel = os.path.relpath(p, base)[:-len('.patch')]
                yield p, os.path.normpath(os.path.join(root, ASSETS, rel))


def _read(path):
    with open(path, encoding='utf-8') as f:            # universal newlines: a CRLF copy reads as LF
        return f.read()


def main(argv):
    if not argv or argv[0] not in ('save', 'check', 'list'):
        print(__doc__)
        return 2
    cmd, rc = argv[0], 0
    if cmd == 'save':
        if len(argv) < 2:
            print('save: name the edited files under %s/' % ASSETS)
            return 2
        for f in argv[1:]:
            pf = f + '.pristine'
            if not os.path.isfile(pf):
                print('%s: no %s: nothing applies a patch to this file (only the files a generator keeps a pristine '
                      'copy of)' % (f, pf))
                rc = 1
                continue
            rel = os.path.relpath(f, ASSETS).replace(os.sep, '/')
            pristine, edited = _read(pf), _read(f)
            diff = make(pristine, edited, rel)
            out = patch_path(f)
            if not diff:
                if os.path.isfile(out):
                    os.remove(out)
                    print('%s: no change from the generated text; removed %s' % (f, out))
                else:
                    print('%s: no change from the generated text' % f)
                continue
            if apply(pristine, diff, out) != ''.join(_lines(edited)):   # the patch must rebuild the edited file
                print('%s: the patch would not rebuild the edited file: not saved' % f)
                rc = 1
                continue
            os.makedirs(os.path.dirname(out), exist_ok=True)
            with open(out, 'w', encoding='utf-8', newline='\n') as w:
                w.write(diff)
            body = diff.splitlines()[3:]
            print('%s: saved %s (+%d -%d lines)' % (f, out, sum(1 for l in body if l.startswith('+')),
                                                    sum(1 for l in body if l.startswith('-'))))
    elif cmd == 'check':
        n = 0
        for p, f in _all_patches():
            n += 1
            pf = f + '.pristine'
            if not os.path.isfile(pf):
                print('FAIL %s: %s is missing (run ./configure)' % (p, pf))
                rc = 1
                continue
            notes = []
            try:
                text = apply(_read(pf), _read(p), p, notes)
            except PatchError as ex:
                print('FAIL %s' % ex)
                rc = 1
                continue
            same = os.path.isfile(f) and _read(f) == text
            print('ok   %s -> %s%s' % (p, f, '' if same else ' (the file on disk differs: run ./configure)'))
            for note in notes:
                print('     note: %s' % note)
        print('%d patch(es), %s' % (n, 'all apply' if rc == 0 else 'some do not apply'))
    else:
        for p, f in _all_patches():
            print('%s -> %s' % (p, f))
    return rc


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
