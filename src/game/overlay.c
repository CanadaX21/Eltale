#include "common.h"
extern u8 credits_overlay_ROM_END[];
extern u8 credits_overlay_ROM_START[];
extern u8 title_overlay_ROM_START[];
extern u8 intro_overlay_ROM_START[];
extern u8 func_8020C0D0[];
void dma_write(u8* devAddr, void* vaddr, u32 nbytes);

void LoadOvl_TitleScreen(void) {
    dma_write(title_overlay_ROM_START, func_8020C0D0, intro_overlay_ROM_START - title_overlay_ROM_START);
}
void LoadOvl_Splash_and_Intro(void) {
    dma_write(intro_overlay_ROM_START, func_8020C0D0, credits_overlay_ROM_START - intro_overlay_ROM_START);
}
void LoadOvl_Ending(void) {
    dma_write(credits_overlay_ROM_START, func_8020C0D0, credits_overlay_ROM_END - credits_overlay_ROM_START);
}
