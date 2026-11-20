"""maprecords.py: splat's `maprecords` segment type, a run of a map blob's records as typed C.

The writer is the `anim` type's (tools/splat_ext/anim.py: the project's C types are the one definition; a record is a
sized symbol declared with its type; raw records; authored changes kept as patches). This type gives a map's records
their own name and file: submaps, doors, entrances, model sets with their placed models and lists, geometry with its
collision faces and vertices, lights and their keys, the prop table, the camera setups, the wall navmeshes, the NPC
records and the map-object lists, written as assets/<dir>/<name>.maprecords.inc.c, which configure.py compiles as a C
unit. A display list that shares a unit with the records after it (no pad to 16 between them) is written in the same
file, as a Gfx record in gbi macros.
"""
import importlib.util
from pathlib import Path


def _anim():
    """anim.py beside this file, loaded by its path (as it loads tools/c_layout.py)"""
    path = Path(__file__).resolve().parent / "anim.py"
    spec = importlib.util.spec_from_file_location("eltale_splat_ext_anim_maps", path)
    mod = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)
    return mod


anim = _anim()


class N64SegMaprecords(anim.N64SegAnim):
    SUFFIX = "maprecords"
    WHAT = "map records"
    WRITER = "tools/splat_ext/maprecords.py (the anim type's writer)"
    TYPES = "Submap, MapDoor, MapModelData, ModelTransform, ModelGeometry, MapLight, CameraSetup, NavMeshData, NpcRecord and the rest; MapCameras in include/map_format.h"
    EXTRA_INCLUDES = '#include "dialogue.h"'    # an NPC record names its strings (DLG_NPC_<offset>)
