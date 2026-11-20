"""dialogue.py: splat's `dialogue` segment type, a run of the dialogue bank's strings as source text.

The dialogue bank (StartNPCText's and LoadSignText's) is read from the ROM a string at a time, by u16 offsets from
its two bases: the NPC records' message and altMessage from the bank's start, the sign records' name and text from
the sign part's start. This type writes one part (yaml: {start, type: dialogue, name}) as
assets/<dir>/<name>.dialogue.txt: each string as @NAME and its text, through tools/text_codec.py and the definition
text/encoding.json (the encoding and the glyph table). NAME is the string's offset in the original, as the C name
the records use (DLG_NPC_<offset>, DLG_SIGN_<offset>); tools/text_build.py encodes the file back at build time and
writes the names' values, so a string may change length and every reference follows. Optional yaml keys: `names`
and `unreferenced`, lists of original offsets the writer marks (a sign's name, 0x20 bytes at most; a string no
reference reaches). Edits are kept like a display list's (tools/asset_patches.py: the file is a text kind).
"""
import importlib.util
from pathlib import Path

from segtypes.common.codesubsegment import CommonSegCodeSubsegment
from util import log, options


def _codec():
    path = Path(__file__).resolve().parent.parent / "text_codec.py"
    spec = importlib.util.spec_from_file_location("eltale_text_codec", path)
    mod = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)
    return mod


text_codec = _codec()


def _fail(msg, seg=None):
    log.error(f"dialogue: {msg}")


class N64SegDialogue(CommonSegCodeSubsegment):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        y = self.yaml if isinstance(self.yaml, dict) else {}
        self.marks = {}
        for key in ("names", "unreferenced"):
            for off in y.get(key, []) or []:
                self.marks.setdefault(off, []).append("name" if key == "names" else "unreferenced")

    def get_linker_section(self) -> str:
        return ".data"

    def out_path(self) -> Path:
        return options.opts.asset_path / self.dir / f"{self.name}.dialogue.txt"

    def should_scan(self) -> bool:
        return False

    def should_split(self) -> bool:
        return self.extract and options.opts.is_mode_active("dialogue")

    def split(self, rom_bytes: bytes):
        enc = text_codec.Encoding(str(options.opts.base_path / "text" / "encoding.json"))
        parts = enc.data["banks"]["dialogue"]["parts"]
        part = next((p for p in parts if p["name"] == self.name), None)
        if part is None:
            _fail(f"{self.name}: no part of that name in text/encoding.json")
        prefix = part["prefix"]
        data = rom_bytes[self.rom_start:self.rom_end]
        try:
            strings = enc.split_part(data, enc.data["banks"]["dialogue"]["align"])
        except text_codec.TextError as ex:
            _fail(f"{self.name}: {ex}")
        lines = [f"# {part['about']}",
                 f"# ROM 0x{self.rom_start:X}-0x{self.rom_end:X} of the original, written by tools/splat_ext/dialogue.py;",
                 "# tools/text_build.py encodes this file back at build time (the original's bytes for the original's text).",
                 f"# Each string: @NAME [marks], then its text. NAME is its offset in the original ({prefix}_<offset>), and",
                 f"# the C name its references use (include/dialogue.h). {part['limits']}",
                 "# The text (text/encoding.json, tools/text_codec.py): the glyph table's characters; U+3000 or {sp} a space;",
                 "# \\n a new line, \\f a new page after a button, \\e the save prompt; {pal N} {font N} {spacing N} {speed N}",
                 "# {menu} {row N} {code XX} the other codes; {R,C} the cell in row R; a line break in this file is only layout.",
                 "# A string may change length: the strings after it move and every reference follows. To keep a change:",
                 "# python3 tools/asset_patches.py save <this file> (./configure rewrites the file from the ROM)."]
        for off, text in strings:
            marks = self.marks.pop(off, [])
            lines.append("")
            lines.append(f"@{prefix}_{off:04X}" + "".join(" " + m for m in marks))
            lines.extend(text_codec.body_lines(text))
        if self.marks:
            _fail(f"{self.name}: marked offsets that start no string: "
                  + ", ".join(f"0x{o:X}" for o in sorted(self.marks)))
        out = self.out_path()
        out.parent.mkdir(parents=True, exist_ok=True)
        with open(out, "w", encoding="utf-8", newline="\n") as f:
            f.write("\n".join(lines) + "\n")
