"""n64crc.py ROM [--check]: the cartridge header's boot checksum, for a modified build.

The console's boot code checksums the megabyte after the header and boot code (ROM 0x1000-0x101000) and compares the
result with the two words at 0x10; if they differ, the game does not start. This game's cartridge has the CIC-6102 boot
chip, whose boot code and seed this follows. A modified build whose change reaches that range (any change to main's code
or data, or one that moves them) needs the two words computed again. `ninja build/eltail.jp.z64` writes the ROM and then
checks it against the original's SHA-1, which a modified ROM fails by design; run this on the ROM it wrote.

    python3 tools/n64crc.py ROM          compute the two words and write them where they differ (on the original ROM
                                         they are already right, and the file is left as it is)
    python3 tools/n64crc.py ROM --check  only compare them (exit status 1 on a mismatch)

It refuses a file that is not a big-endian ROM (.z64, first word 0x80371240) or whose boot code is not the CIC-6102
one (CRC-32 of ROM 0x40-0x1000: 0x90BB6CB5), since the seed below holds for that chip only. Python 3 standard library
only.
"""
import struct
import sys
import zlib

SEED = 0xF8CA4DDC               # CIC-6102
BOOT_CRC32 = 0x90BB6CB5         # CRC-32 of the CIC-6102 boot code, ROM 0x40-0x1000
START, LENGTH = 0x1000, 0x100000
M = 0xFFFFFFFF


def checksum(rom):
    """the two header words the boot code expects, over ROM 0x1000-0x101000 (a shorter ROM reads as zero beyond its
    end)"""
    t1 = t2 = t3 = t4 = t5 = t6 = SEED
    data = bytes(rom[START:START + LENGTH]).ljust(LENGTH, b'\0')
    for (d,) in struct.iter_unpack('>I', data):
        if (t6 + d) & M < t6:
            t4 = (t4 + 1) & M
        t6 = (t6 + d) & M
        t3 ^= d
        s = d & 0x1F
        r = ((d << s) | (d >> (32 - s))) & M if s else d
        t5 = (t5 + r) & M
        t2 = t2 ^ r if t2 > d else t2 ^ t6 ^ d
        t1 = (t1 + (t5 ^ d)) & M
    return (t6 ^ t4 ^ t3) & M, (t5 ^ t2 ^ t1) & M


def problem(rom):
    """why this file is not a ROM this tool computes, or None"""
    if len(rom) < START or struct.unpack('>I', rom[:4])[0] != 0x80371240:
        return 'not a big-endian N64 ROM (.z64: the first word is 0x80371240)'
    crc = zlib.crc32(bytes(rom[0x40:START])) & M
    if crc != BOOT_CRC32:
        return 'its boot code is not the CIC-6102 one (CRC-32 %08X, not %08X): this seed does not hold' % (crc, BOOT_CRC32)
    return None


def main(argv):
    args = [a for a in argv if a != '--check']
    if len(args) != 1:
        print(__doc__)
        return 2
    path, only_check = args[0], '--check' in argv
    with open(path, 'rb') as f:
        rom = bytearray(f.read())
    why = problem(rom)
    if why:
        print('%s: %s' % (path, why))
        return 1
    want = checksum(rom)
    have = struct.unpack('>II', rom[0x10:0x18])
    if have == want:
        print('%s: header checksum %08X %08X: OK' % (path, have[0], have[1]))
        return 0
    if only_check:
        print('%s: header checksum %08X %08X, computed %08X %08X: MISMATCH' % (path, have[0], have[1], want[0], want[1]))
        return 1
    rom[0x10:0x18] = struct.pack('>II', *want)
    with open(path, 'wb') as f:
        f.write(rom)
    print('%s: header checksum %08X %08X -> %08X %08X (written)' % (path, have[0], have[1], want[0], want[1]))
    return 0


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
