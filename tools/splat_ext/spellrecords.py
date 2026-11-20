"""spellrecords.py: splat's `spellrecords` segment type, a run of the spell bank as typed C.

The writer is the `anim` type's (tools/splat_ext/anim.py: the project's C types are the one definition; a record is a
sized symbol declared with its type; raw records; authored changes kept as patches). The spell bank is packed on 8-byte
boundaries, so one unit holds whatever lies between two images or raw pieces: the spell names, the spell and effect
records and their tables, display lists (Gfx, in gbi macros), vertex runs (Vtx), the projectile animations, materials
and palettes, and one texture written as bytes (the burst's, which no unit boundary can leave outside). A unit is a
multiple of 16 bytes (IDO pads its .data to 16) and may start on any 8-byte boundary (the segment's subalign: 8).
Written as assets/spell_bank/<name>.spellrecords.inc.c, which configure.py compiles as a C unit.
"""
import importlib.util
from pathlib import Path


def _anim():
    """anim.py beside this file, loaded by its path (as it loads tools/c_layout.py)"""
    path = Path(__file__).resolve().parent / "anim.py"
    spec = importlib.util.spec_from_file_location("eltale_splat_ext_anim_spells", path)
    mod = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)
    return mod


anim = _anim()


class N64SegSpellrecords(anim.N64SegAnim):
    SUFFIX = "spellrecords"
    WHAT = "spell bank records"
    WRITER = "tools/splat_ext/spellrecords.py (the anim type's writer)"
    TYPES = ("SpellDef, SpellAnimDef, EffectSpawnDef, AnimEntry, ModelPart, ModelKeyFrame, Vec3f; SpellBurstDef in "
             "include/spell_burst.h, ProjModelDef in include/proj_material.h")
    EXTRA_INCLUDES = '#include "spell_burst.h"\n#include "proj_material.h"\n#include "spell_bank.h"'
