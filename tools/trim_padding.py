"""Copy an IDO/asm-processor object for objdiff's target side, with the trailing alignment
padding taken out of the last function's size.

splat cuts the original code at file boundaries, so in an object assembled from its output the
last function of each file carries the zero words that pad the file's code to 16 bytes
(`jr ra; <delay>; nop; nop`). In compiled C those words are outside the function: IDO's
assembler pads the end of .text itself, and the last function's symbol size stops before them.
objdiff would score that as "missing" instructions. Only the symbol size changes; bytes stay.

    python3 trim_padding.py IN.o OUT.o
Standard library only (ELF32 big-endian, MIPS).
"""
import struct, sys

JR_RA = 0x03E00008


def trim(data):
    data = bytearray(data)
    shoff, = struct.unpack_from('>I', data, 0x20)
    shentsize, shnum, shstrndx = struct.unpack_from('>HHH', data, 0x2E)
    secs = [struct.unpack_from('>IIIIIIIIII', data, shoff + i * shentsize) for i in range(shnum)]
    names = secs[shstrndx]
    def secname(s):
        off = names[4] + s[0]
        return bytes(data[off:data.index(0, off)]).decode()
    text_idx = [i for i, s in enumerate(secs) if secname(s) == '.text']
    if not text_idx:
        return bytes(data), 0
    ti = text_idx[0]
    toff, tsize = secs[ti][4], secs[ti][5]
    changed = 0
    for s in secs:
        if s[1] != 2:                       # SHT_SYMTAB
            continue
        off, size, entsize = s[4], s[5], s[9] or 16
        syms = []
        for k in range(size // entsize):
            p = off + k * entsize
            st_name, st_value, st_size, st_info, st_other, st_shndx = struct.unpack_from('>IIIBBH', data, p)
            if st_shndx == ti and (st_info & 0xF) == 2 and st_size:   # STT_FUNC in .text
                syms.append((st_value, st_size, p))
        if not syms:
            continue
        value, fsize, p = max(syms)          # the last function in the section
        words = [struct.unpack_from('>I', data, toff + value + i)[0] for i in range(0, fsize, 4)]
        # find the final jr ra; everything after its delay slot must be zero padding
        last = max((i for i, w in enumerate(words) if w == JR_RA), default=None)
        if last is None or last + 2 >= len(words) or any(words[last + 2:]):
            continue
        new = (last + 2) * 4
        struct.pack_into('>I', data, p + 8, new)
        changed += fsize - new
    return bytes(data), changed


if __name__ == '__main__':
    src, dst = sys.argv[1], sys.argv[2]
    out, n = trim(open(src, 'rb').read())
    open(dst, 'wb').write(out)
