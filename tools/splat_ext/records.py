"""records.py: splat's `records` segment type, a run of the game's data records as typed C.

The writer is the `anim` type's (tools/splat_ext/anim.py: the project's C types are the one definition; a record is a
sized symbol declared with its type; raw records; authored changes kept as patches). This type only gives the records
that are not animation data their own name and file: a monster set's AI options, base records (the monsters' stats),
its table of animation tables, its name table and its names, written as assets/<dir>/<name>.records.inc.c, which
configure.py compiles as a C unit.
"""
import importlib.util
from pathlib import Path


def _anim():
    """anim.py beside this file, loaded by its path (as it loads tools/c_layout.py)"""
    path = Path(__file__).resolve().parent / "anim.py"
    spec = importlib.util.spec_from_file_location("eltale_splat_ext_anim", path)
    mod = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)
    return mod


anim = _anim()


class N64SegRecords(anim.N64SegAnim):
    SUFFIX = "records"
    WHAT = "data records"
    WRITER = "tools/splat_ext/records.py (the anim type's writer)"
    TYPES = "MonsterBaseData, EnemyAIOption"
