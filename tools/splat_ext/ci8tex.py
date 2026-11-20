"""ci8tex.py: splat's `ci8tex` segment type, a CI8 texture whose palette is elsewhere in the ROM.

The spell bank's projectile materials draw CI8 textures through one of four 256-colour palettes (gProjPalettes, chosen by
the material at run time); the palettes are records of their own in the bank. splat's ci8 type takes its palette from a
sibling palette segment of the same name, which these textures do not have. This type writes the PNG with the palette
read from the ROM offset given as its sixth argument ([start, ci8tex, name, width, height, palette ROM offset]): the
palette in the PNG is a copy for viewing. The build reads only the PNG's colour indices (tools/image_converter.py ci8),
so the colours are changed in the palette record, not in an image editor; tools/asset_patches.py stops ./configure when
a kept CI8 image's palette differs from the extracted one.
"""
from segtypes.n64.ci8 import N64SegCi8
from segtypes.n64.img import N64SegImg
from util import log
from util.color import unpack_color

PALETTE_BYTES = 0x200           # 256 colours, RGBA16 (gDPLoadTLUT_pal256)


class N64SegCi8tex(N64SegCi8):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        y = self.yaml
        self.palette_rom = y.get('palette_rom') if isinstance(y, dict) else (y[5] if len(y) > 5 else None)
        if not isinstance(self.palette_rom, int):
            log.error(f"Error: ci8tex {self.name} needs the palette's ROM offset as its sixth argument")

    def split(self, rom_bytes):
        data = rom_bytes[self.palette_rom:self.palette_rom + PALETTE_BYTES]
        if len(data) != PALETTE_BYTES:
            log.error(f"Error: ci8tex {self.name}: the palette at 0x{self.palette_rom:X} runs off the ROM")
        self.n64img.palette = [unpack_color([data[i], data[i + 1]]) for i in range(0, PALETTE_BYTES, 2)]
        N64SegImg.split(self, rom_bytes)
