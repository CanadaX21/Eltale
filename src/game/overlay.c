#include "common.h"
#include "game/overlay.h"
#include "game/dma.h"
#include "segment_symbols.h"

/* the title overlay's image: its code, .data, art and .rodata (its .bss is not cleared) */
void LoadOvl_TitleScreen(void) {
    dma_write(title_overlay_ROM_START, gOverlaySlot, title_overlay_ROM_END - title_overlay_ROM_START);
}
/* the splash and intro overlay's image: its code, .data and .rodata (its .bss is not cleared) */
void LoadOvl_Splash_and_Intro(void) {
    dma_write(intro_overlay_ROM_START, gOverlaySlot, intro_overlay_ROM_END - intro_overlay_ROM_START);
}
/* the epilogue overlay's image (game mode 5: a narration over a painting of the castle in the clouds). Its segment
   keeps the historical name credits_overlay, which does not describe what it shows: the credits are game mode 7 */
void LoadOvl_Epilogue(void) {
    dma_write(credits_overlay_ROM_START, gOverlaySlot, credits_overlay_ROM_END - credits_overlay_ROM_START);
}
