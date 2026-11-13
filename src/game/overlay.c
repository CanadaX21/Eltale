#include "common.h"
#include "game/overlay.h"
#include "game/dma.h"
extern u8 credits_overlay_ROM_END[];
extern u8 credits_overlay_ROM_START[];
extern u8 title_overlay_ROM_START[];
extern u8 title_overlay_ROM_END[];
extern u8 intro_overlay_ROM_START[];
extern u8 intro_overlay_ROM_END[];
extern u8 func_8020C0D0[];

/* the title overlay's image: its code, .data, art and .rodata (its .bss is not cleared) */
void LoadOvl_TitleScreen(void) {
    dma_write(title_overlay_ROM_START, func_8020C0D0, title_overlay_ROM_END - title_overlay_ROM_START);
}
/* the splash and intro overlay's image: its code, .data and .rodata (its .bss is not cleared) */
void LoadOvl_Splash_and_Intro(void) {
    dma_write(intro_overlay_ROM_START, func_8020C0D0, intro_overlay_ROM_END - intro_overlay_ROM_START);
}
void LoadOvl_Ending(void) {
    dma_write(credits_overlay_ROM_START, func_8020C0D0, credits_overlay_ROM_END - credits_overlay_ROM_START);
}
