#!/usr/bin/env python3
"""image_converter.py FORMAT IMAGE OUTPUT: a PNG image as N64 texels, for the build.

configure.py runs this for every image the extraction writes under assets/, with FORMAT taken from the file's name
(tex_000.rgba16.png converts as rgba16), and the build links OUTPUT in place of the ROM's texture. OUTPUT holds the
texels alone, in the formats the SDK's gbi.h names (G_IM_FMT_*, G_IM_SIZ_*): row after row from the top, each row from
the left, with no header and no padding.

    rgba16   16 bits, big-endian: red 5, green 5, blue 5, alpha 1 (as gbi.h's GPACK_RGBA5551)
    i8       8 bits of intensity
    i4       4 bits of intensity
    ia4      4 bits: intensity 3, alpha 1
    ci8      an 8-bit palette index
    ci4      a 4-bit palette index
    4-bit texels share a byte two by two, the left one in the high bits.

IMAGE is a PNG of any kind, decoded by pypng. For rgba16, i8, i4 and ia4 each pixel is read as 8-bit red, green, blue
and alpha, as the PNG defines it: palette entries looked up, tRNS transparency applied, other chunks (gamma and the
like) ignored. Intensity is a pixel's grey value, or a coloured pixel's Rec. 709 luma. For ci8 and ci4, IMAGE must be a
palette image and its indices are the texels: the PNG's palette is only a copy to view the image by, since the game's
palettes are data of their own (tools/splat_ext/ci4tex.py, ci8tex.py).

The rounding and thresholds below decide the bytes an edited image becomes; they are the ones the build's images have
always converted by, so an edit converts as it did before. An image this cannot convert as asked is refused, with a
message and exit status 1, and OUTPUT is then not written: 16-bit samples; for the direct formats, greyscale below 8
bits, an sBIT chunk below 8 bits or a palette index with no entry; for ci8 and ci4, an image that is not a palette
image; for ci4, an index above 15; for the 4-bit formats, an odd width.

This version was written for this repository from the N64 texel formats, not from the code of the earlier versions of
this file. Those came from pmret/papermario's image build script (tools/build/img/build.py) and remain in the
repository's history; the credits (CREDITS.md) record how they came here.
"""
import argparse
import sys
import zlib

import png  # pypng (requirements.txt): inflates, unfilters, de-interlaces and unpacks the samples

FORMATS = ('rgba16', 'ia4', 'i4', 'i8', 'ci4', 'ci8')
INDEXED = ('ci4', 'ci8')
NIBBLE_FORMATS = ('ia4', 'i4', 'ci4')     # two texels to a byte


class Refused(Exception):
    """IMAGE cannot be converted as asked; the text says why"""


def load_png(path):
    """(width, height, rows, info, sbit): pypng's description of the image, every row as a list of samples (unpacked to
    one value each), and the sBIT chunk's bytes or None"""
    try:
        with open(path, 'rb') as f:
            data = f.read()
    except OSError as ex:
        raise Refused('cannot read it (%s)' % (ex.strerror or ex))
    try:
        sbit = None
        for kind, content in png.Reader(bytes=data).chunks():
            if kind == b'sBIT':
                sbit = content
            if kind == b'IEND':
                break
        width, height, rows, info = png.Reader(bytes=data).read()
        rows = [list(row) for row in rows]
    except (png.Error, EOFError, zlib.error, ValueError) as ex:
        raise Refused('not a PNG that can be read (%s)' % ex)
    return width, height, rows, info, sbit


def is_palette_image(info):
    # colour type 3: one sample per pixel that is not grey. (pypng also reports a PLTE chunk in an RGB image, which
    # the PNG format allows as a suggested palette, so the presence of a palette does not decide it.)
    return info['planes'] == 1 and not info['greyscale']


def rgba_pixels(width, rows, info, sbit):
    """the image as rows of (red, green, blue, alpha), 8 bits each, as the PNG defines its pixels"""
    depth = info['bitdepth']
    if depth == 16:
        raise Refused('16-bit samples; save it with 8 bits per sample')
    if sbit is not None and min(sbit) < 8:
        raise Refused('an sBIT chunk of %s significant bits, which would change what its samples mean; save it '
                      'with full 8-bit samples and no sBIT chunk' % '/'.join(str(n) for n in sbit))
    if is_palette_image(info):
        entries = [tuple(e) + (255,) * (4 - len(e)) for e in info['palette']]   # tRNS alphas, else opaque
        out = []
        for y, row in enumerate(rows):
            for x, index in enumerate(row):
                if index >= len(entries):
                    raise Refused('pixel (%d, %d) has index %d, beyond its %d palette entries'
                                  % (x, y, index, len(entries)))
            out.append([entries[index] for index in row])
        return out
    if depth != 8:      # greyscale at 1, 2 or 4 bits: the only depths left that a PNG allows
        raise Refused('greyscale with %d bits per sample; save it with 8' % depth)
    planes = info['planes']
    key = info.get('transparent')       # tRNS of a greyscale or RGB image: the one value or colour that is clear
    key = tuple(key) if key is not None else None
    out = []
    for row in rows:
        line = []
        for start in range(0, width * planes, planes):
            sample = tuple(row[start:start + planes])
            if planes == 1:
                line.append(sample * 3 + ((0 if sample == key else 255),))
            elif planes == 2:
                line.append(sample[:1] * 3 + sample[1:])
            elif planes == 3:
                line.append(sample + ((0 if sample == key else 255),))
            else:
                line.append(sample)
        out.append(line)
    return out


def intensity(red, green, blue):
    """a pixel's intensity: its grey value, or a coloured pixel's Rec. 709 luma, rounded to the nearest whole number
    (an exact half to the even one). The products are summed in this order in double precision: the rounding of the
    rare colours whose luma falls on a half depends on it."""
    if red == green == blue:
        return red
    return round(0.2126 * red + 0.7152 * green + 0.0722 * blue)


def rgba16_texel(red, green, blue, alpha):
    return (red >> 3) << 11 | (green >> 3) << 6 | (blue >> 3) << 1 | (1 if alpha >= 128 else 0)


def ia4_texel(red, green, blue, alpha):
    return (intensity(red, green, blue) >> 5) << 1 | (1 if alpha > 128 else 0)


def i4_texel(red, green, blue, alpha):
    return intensity(red, green, blue) // 17


def i8_texel(red, green, blue, alpha):
    return intensity(red, green, blue)


TEXEL = {'rgba16': rgba16_texel, 'ia4': ia4_texel, 'i4': i4_texel, 'i8': i8_texel}


def two_per_byte(values):
    """4-bit values, two to a byte, the first in the high bits (an even count: the widths are checked first)"""
    return bytes(values[i] << 4 | values[i + 1] for i in range(0, len(values), 2))


def direct_warnings(fmt, pixels):
    notes = []
    if fmt in ('rgba16', 'ia4'):
        partial = sum(1 for p in pixels if 0 < p[3] < 255)
        if partial:
            notes.append('%d pixel(s) partly transparent: %s keeps one alpha bit, set for alpha %s'
                         % (partial, fmt, 'from 128' if fmt == 'rgba16' else 'above 128'))
    if fmt in ('i8', 'i4', 'ia4'):
        coloured = sum(1 for p in pixels if not p[0] == p[1] == p[2])
        if coloured:
            notes.append('%d pixel(s) in colour, converted to their luma (0.2126 R + 0.7152 G + 0.0722 B)' % coloured)
    if fmt in ('i8', 'i4'):
        clear = sum(1 for p in pixels if p[3] < 255)
        if clear:
            notes.append('%d pixel(s) not opaque: %s stores no alpha, so their transparency is lost' % (clear, fmt))
    return notes


def convert(fmt, path):
    """(the texels as bytes, a list of warnings) for the PNG at path in the format fmt; raises Refused"""
    width, height, rows, info, sbit = load_png(path)
    if fmt in NIBBLE_FORMATS and width % 2:
        raise Refused('%d pixels wide; %s puts two texels in a byte, so the width must be even' % (width, fmt))
    if fmt in INDEXED:
        if not is_palette_image(info):
            raise Refused('not a palette image; %s takes its texels from the palette indices' % fmt)
        if fmt == 'ci8':
            return bytes(index for row in rows for index in row), []
        for y, row in enumerate(rows):
            for x, index in enumerate(row):
                if index > 15:
                    raise Refused('pixel (%d, %d) has index %d; ci4 holds 0 to 15' % (x, y, index))
        return two_per_byte([index for row in rows for index in row]), []
    pixels = [p for row in rgba_pixels(width, rows, info, sbit) for p in row]
    texel = TEXEL[fmt]
    values = [texel(*p) for p in pixels]
    if fmt == 'rgba16':
        data = b''.join(v.to_bytes(2, 'big') for v in values)
    elif fmt == 'i8':
        data = bytes(values)
    else:
        data = two_per_byte(values)
    return data, direct_warnings(fmt, pixels)


def main(argv=None):
    parser = argparse.ArgumentParser(
        description='Write a PNG image as N64 texels for the build: the texels alone, row by row from the top.')
    parser.add_argument('format', choices=FORMATS, help='the texel format to write')
    parser.add_argument('image', help='the PNG to convert')
    parser.add_argument('output', help='where to write the texels')
    args = parser.parse_args(argv)
    try:
        data, warnings = convert(args.format, args.image)
    except Refused as ex:
        sys.stderr.write('image_converter.py: %s: %s\n' % (args.image, ex))
        return 1
    for note in warnings:
        sys.stderr.write('image_converter.py: %s: warning: %s\n' % (args.image, note))
    try:
        with open(args.output, 'wb') as f:
            f.write(data)
    except OSError as ex:
        sys.stderr.write('image_converter.py: %s: cannot write it (%s)\n' % (args.output, ex.strerror or ex))
        return 1
    return 0


if __name__ == '__main__':
    sys.exit(main())
