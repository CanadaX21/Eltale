"""npcnames.py: splat's `npcnames` segment type, an NPC file's name pool as source text.

An NPC record names its speaker by a pointer into the pool of names at the end of its area's file (or of Shannon's
block): StartNPCText copies 0x20 bytes from there into gSpeakerName, so a name is at most 0x20 bytes, its 0xFF
included. This type writes the pool (yaml: {start, type: npcnames, name, unreferenced: [offsets]}) as
assets/<dir>/<name>.npcnames.txt: each string as @NAME and its text, through tools/text_codec.py and the definition
text/encoding.json (the glyph table the dialogue uses). NAME is the string's C symbol (symbol_addrs.txt names it by
its offset in the original file, gNpcArea0Name_5FB34, as the dialogue's strings are DLG_NPC_<offset>); at build time
tools/text_build.py encodes the file back into an object that defines each NAME at its string, so a name may change
length and every record that points at a name follows. The layout is the original's (the definition's `npc_names`
bank): each string on a 4-byte step with zero fill, the pool padded to 16 with zeros. `unreferenced` lists the file
offsets of strings no NPC record points at (marked in the text). Edits are kept like a display list's
(tools/asset_patches.py: the file is a text kind).
"""
import importlib.util
from pathlib import Path

from segtypes.common.codesubsegment import CommonSegCodeSubsegment
from util import log, options


def _codec():
    path = Path(__file__).resolve().parent.parent / "text_codec.py"
    spec = importlib.util.spec_from_file_location("eltale_text_codec_names", path)
    mod = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)
    return mod


text_codec = _codec()


def _fail(msg):
    log.error(f"npcnames: {msg}")


class N64SegNpcnames(CommonSegCodeSubsegment):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        y = self.yaml if isinstance(self.yaml, dict) else {}
        self.unreferenced = set(y.get("unreferenced", []) or [])

    def get_linker_section(self) -> str:
        return ".data"

    def out_path(self) -> Path:
        return options.opts.asset_path / self.dir / f"{self.name}.npcnames.txt"

    def should_scan(self) -> bool:
        return False

    def should_split(self) -> bool:
        return self.extract and options.opts.is_mode_active("npcnames")

    def split(self, rom_bytes: bytes):
        enc = text_codec.Encoding(str(options.opts.base_path / "text" / "encoding.json"))
        bank = enc.data["banks"]["npc_names"]
        step, limit = int(bank["align"]), int(bank["limit"], 0)
        seg = self.get_most_parent()
        names = {}
        for lst in seg.seg_symbols.values():
            for s in lst:
                if s.user_declared and self.vram_start <= s.vram_start < self.vram_start + self.rom_end - self.rom_start:
                    names.setdefault(s.vram_start, s.name)
        data = rom_bytes[self.rom_start:self.rom_end]
        base = self.rom_start - seg.rom_start                 # the pool's offset in its file
        entries, pos, marks = [], 0, set(self.unreferenced)
        while pos < len(data):
            if data[pos] == 0:
                pos += 1
                continue
            if pos % step:
                _fail(f"{self.name}: a string at +0x{pos:X} is not on the pool's {step}-byte steps")
            try:
                text, after = enc.decode(data, pos)
            except (text_codec.TextError, ValueError) as ex:
                _fail(f"{self.name}: +0x{pos:X}: {ex}")
            if after - pos > limit:
                _fail(f"{self.name}: the string at +0x{pos:X} is 0x{after - pos:X} bytes; the code reads 0x{limit:X}")
            name = names.get(self.vram_start + pos)
            if name is None:
                _fail(f"{self.name}: no symbol stands at the string at +0x{pos:X} (RAM 0x{self.vram_start + pos:08X})")
            mark = base + pos in marks
            marks.discard(base + pos)
            entries.append((name, mark, text))
            pos = after
        if marks:
            _fail(f"{self.name}: marked offsets that start no string: " + ", ".join(f"0x{o:X}" for o in sorted(marks)))
        lines = [f"# {bank['about']}",
                 f"# ROM 0x{self.rom_start:X}-0x{self.rom_end:X} of the original ({seg.name} +0x{base:X} to its end), written by",
                 "# tools/splat_ext/npcnames.py; tools/text_build.py encodes this file back at build time (the original's bytes",
                 "# for the original's text). Each string: @NAME [unreferenced], then its text. NAME is the string's C symbol, by",
                 f"# its offset in the original file; the NPC records point at it. {bank['limits']}",
                 "# The text (text/encoding.json, tools/text_codec.py): the glyph table's characters; {pal N} {font N} the codes",
                 "# a name starts with; {R,C} the cell in row R; a line break in this file is only layout.",
                 "# A string may change length: the strings after it move and every record follows. To keep a change:",
                 "# python3 tools/asset_patches.py save <this file> (./configure rewrites the file from the ROM)."]
        for name, mark, text in entries:
            lines.append("")
            lines.append(f"@{name}" + (" unreferenced" if mark else ""))
            lines.extend(text_codec.body_lines(text))
        out = self.out_path()
        out.parent.mkdir(parents=True, exist_ok=True)
        with open(out, "w", encoding="utf-8", newline="\n") as f:
            f.write("\n".join(lines) + "\n")
