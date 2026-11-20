"""gfxrecords.py: splat's `gfxrecords` segment type, several display lists in one C unit as typed C.

splat's `gfx` type writes one list a unit, and every C unit pads its .data to 16 bytes: where lists follow one another
with no 16-byte boundary between them (Brian's model: lists 0, 23 and 24, 0x40 bytes each, each starting 8 bytes off a 16-byte
boundary, after the list they branch to), they are one unit of named lists. The writer is the `anim` type's (tools/splat_ext/anim.py: a record is a sized
symbol declared with its type, here `Gfx`; each command a gbi macro through pygfxd, the microcode splat's gfx type uses;
every address the name of the record it leads to, a vertex load the run it lies in at its index; authored changes kept
as patches). Written as assets/<dir>/<name>.gfxrecords.inc.c, which configure.py compiles as a C unit.
"""
import importlib.util
from pathlib import Path


def _anim():
    """anim.py beside this file, loaded by its path (as it loads tools/c_layout.py)"""
    path = Path(__file__).resolve().parent / "anim.py"
    spec = importlib.util.spec_from_file_location("eltale_splat_ext_anim_gfx", path)
    mod = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)
    return mod


anim = _anim()


class N64SegGfxrecords(anim.N64SegAnim):
    SUFFIX = "gfxrecords"
    WHAT = "display lists"
    WRITER = "tools/splat_ext/gfxrecords.py (the anim type's writer)"
    TYPES = "none for these: Gfx is the SDK's gbi.h type"
