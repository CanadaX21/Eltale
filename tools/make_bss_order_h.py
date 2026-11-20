#!/usr/bin/env python3
"""make_bss_order_h.py OUT: writes include/bss_order.h, the padding macro that keeps IDO 5.3's .bss order
(docs/DECLARATIONS.md).

    python3 tools/make_bss_order_h.py include/bss_order.h

BSS_ORDER_PAD(n), n a decimal literal 0-255, expands on its own line into n forward struct declarations (256 when n
is 0: a whole turn of IDO's mod-256 order, so the slot always shows in the front end's symbol table). Each tag is
unique: BssOrderPad_<line>_<bits>. The table maps each n to its binary composition."""
import sys

LETTERS = 'abcdefgh'          # bit 0 .. bit 7 (1 .. 128 declarations)

def body():
    out = []
    out.append('#ifndef BSS_ORDER_H')
    out.append('#define BSS_ORDER_H')
    out.append('')
    out.append('/*')
    out.append(' * IDO 5.3 lays out a compilation unit\'s .bss in the order of each variable\'s block number modulo 256')
    out.append(' * (rules in tools/bss_order.py). With -g, every new declaration takes a block number, headers')
    out.append(' * included: a header that gains or loses declarations moves the .bss of the units that include it.')
    out.append(' * BSS_ORDER_PAD(n) takes n more block numbers (n forward struct declarations, nothing else: no code,')
    out.append(' * no data), so a unit keeps the original game\'s .bss order when the declarations before its state')
    out.append(' * change. The values are computed and kept by tools/bss_order.py; nobody writes them by hand. 0 means')
    out.append(' * a whole turn (256 declarations): the same order, and the slot stays visible to the tool. What')
    out.append(' * the slots cannot absorb (a unit\'s .bss declared out of storage order; the stretches where a unit\'s')
    out.append(' * function statics sit among its other .bss) is in docs/DECLARATIONS.md, and the tool refuses it.')
    out.append(' * A build that does not need the original layout can define NO_BSS_ORDER_PADS to drop them.')
    out.append(' * Written by tools/make_bss_order_h.py; change that, not this file.')
    out.append(' */')
    out.append('')
    out.append('#ifdef NO_BSS_ORDER_PADS')
    out.append('#define BSS_ORDER_PAD(n)')
    out.append('#else')
    out.append('#define BSS_ORDER_PAD(n) BSS_PAD_LINE_(n, __LINE__)')
    out.append('#define BSS_PAD_LINE_(n, line) BSS_PAD_LINE__(n, line)')
    out.append('#define BSS_PAD_LINE__(n, line) BSS_PAD_##n(BssOrderPad_##line##_)')
    out.append('')
    out.append('/* 2^k forward declarations, named by the bits they stand for */')
    out.append('#define BSS_PAD_X1(p, s) struct p##s;')
    for k in range(1, 9):
        n, h = 1 << k, 1 << (k - 1)
        out.append(f'#define BSS_PAD_X{n}(p, s) BSS_PAD_X{h}(p, s##0) BSS_PAD_X{h}(p, s##1)')
    out.append('')
    out.append('/* n = the sum of its bits; 0 = a whole turn */')
    out.append('#define BSS_PAD_0(p) BSS_PAD_X256(p, z)')
    for n in range(1, 256):
        parts = []
        for k in reversed(range(8)):
            if n & (1 << k):
                parts.append(f'BSS_PAD_X{1 << k}(p, {LETTERS[k]})')
        out.append(f'#define BSS_PAD_{n}(p) ' + ' '.join(parts))
    out.append('#endif')
    out.append('')
    out.append('#endif')
    return '\n'.join(out) + '\n'

if __name__ == '__main__':
    open(sys.argv[1], 'w', newline='\n').write(body())
