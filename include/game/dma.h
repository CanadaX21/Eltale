#ifndef GAME_DMA_H
#define GAME_DMA_H

/* The functions of src/game/dma.c, declared once for every file that calls them. */

#include "common.h"

void dma_write(u8* devAddr, void* vaddr, u32 nbytes);   /* DMA from ROM */
s32 count_digits(s32 value, u8 *str_ptr, u16 flags);

#endif
