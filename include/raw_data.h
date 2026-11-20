#ifndef RAW_DATA_H
#define RAW_DATA_H
/* Raw data: bytes that nothing the game's C code is known to do reads as a type, kept as they are in the ROM.
 * No type is claimed for them. A RawWord that is the address of a named record is written as that name. */
typedef u32 RawWord;
typedef u8 RawByte;
/* A display-list command kept raw: no command is claimed as drawn (no gbi macro), but where its opcode makes its
 * second word an address, that word is written as the name of what it points at, so it follows a change of layout. */
typedef struct { RawWord w0, w1; } RawGfx;
/* A vertex kept raw: four words in hex (a vertex holds no address). */
typedef struct { RawWord w[4]; } RawVtx;
#endif
