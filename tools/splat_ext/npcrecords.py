"""npcrecords.py: splat's `npcrecords` segment type, a binary file's NPC records as typed C.

A record-only window: the file stays a binary piece but for its NPC records (SetupNPCs reads a map's through the
map's NPC list, LoadEndingScene the ending's through its scene table, both binary with the rest; now
every map is typed whole, and the ending's data, event_blob_a, is the one window). The records are typed C so that each names its strings in the dialogue bank
(DLG_NPC_<offset>, include/dialogue.h): a string that changes length moves the strings after it, and the record
follows, and so does a record's model and name in an NPC file. The window runs from the records to the next
16-byte boundary (a C unit's .data is a multiple of 16; the
bytes after the records there are a raw record, or IDO's zero pad when they are zeros), so the map's layout and its
other bytes stay as they are. The writer is the `anim` type's (tools/splat_ext/anim.py: the project's C types are the one
definition; a pointer is a name: a record of an NPC area's file or of Shannon's block, include/npcs.h). Written as
assets/<dir>/<name>.npcrecords.inc.c, which configure.py compiles as a C unit.
"""
import importlib.util
from pathlib import Path


def _anim():
    """anim.py beside this file, loaded by its path (as it loads tools/c_layout.py)"""
    path = Path(__file__).resolve().parent / "anim.py"
    spec = importlib.util.spec_from_file_location("eltale_splat_ext_anim_npcs", path)
    mod = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)
    return mod


anim = _anim()


class N64SegNpcrecords(anim.N64SegAnim):
    SUFFIX = "npcrecords"
    WHAT = "NPC records (a window in a binary file)"
    WRITER = "tools/splat_ext/npcrecords.py (the anim type's writer)"
    TYPES = "NpcRecord"
    EXTRA_INCLUDES = '#include "dialogue.h"'    # each record names its strings (DLG_NPC_<offset>)
