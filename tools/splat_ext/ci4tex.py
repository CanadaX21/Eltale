"""ci4tex.py: splat's `ci4tex` segment type, a CI4 texture whose palette is elsewhere in the ROM.

The font sheet is CI4 drawn through one of the four 16-colour text palettes (gTextPalettes; a string's {pal N} picks
one at run time). The palettes are an image of their own (text_palettes.tlut.rgba16.png, one palette per row). splat's
ci4 type takes its palette from a sibling palette segment of the same name, which the sheet does not have. This type
writes the PNG with the palette read from the ROM offset given as its sixth argument ([start, ci4tex, name, width,
height, palette ROM offset]). The palette in the PNG is a copy for viewing. The build reads only the PNG's colour
indices (tools/image_converter.py ci4), so colours are changed in the palettes image, not in an image editor;
tools/asset_patches.py stops ./configure when a kept CI4 image's palette differs from the extracted one.
"""
from segtypes.n64.ci4 import N64SegCi4
from segtypes.n64.img import N64SegImg
from util import log
from util.color import unpack_color

PALETTE_BYTES = 0x20            # 16 colours, RGBA16 (gDPLoadTLUT_pal16)


class N64SegCi4tex(N64SegCi4):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        y = self.yaml
        self.palette_rom = y.get('palette_rom') if isinstance(y, dict) else (y[5] if len(y) > 5 else None)
        if not isinstance(self.palette_rom, int):
            log.error(f"Error: ci4tex {self.name} needs the palette's ROM offset as its sixth argument")

    def split(self, rom_bytes):
        data = rom_bytes[self.palette_rom:self.palette_rom + PALETTE_BYTES]
        if len(data) != PALETTE_BYTES:
            log.error(f"Error: ci4tex {self.name}: the palette at 0x{self.palette_rom:X} runs off the ROM")
        self.n64img.palette = [unpack_color([data[i], data[i + 1]]) for i in range(0, PALETTE_BYTES, 2)]
        N64SegImg.split(self, rom_bytes)
