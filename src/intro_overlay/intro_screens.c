#include "common.h"
#include "game/intro.h"

/*
 * The splash and intro (game mode 4): the second of the overlay's two source files (intro.c describes the overlay).
 * Ten functions Intro_Update calls: three that draw an image each through a helper of their own, and four larger ones
 * that read the buttons and the stick. Its .data is the images those functions draw, spans of the ROM read at build time
 * (as intro.c's); its .bss six bytes. The .bss is not cleared by the loader.
 */

/* ---- .data ---- */
GLOBAL_ASM(
.section .data
glabel D_80259250_E13430
.incbin "baserom.jp.z64", 0xE13430, 0x2D88
glabel D_8025BFD8_E161B8
.incbin "baserom.jp.z64", 0xE161B8, 0x208
glabel D_8025C1E0_E163C0
.incbin "baserom.jp.z64", 0xE163C0, 0x1118
glabel D_8025D2F8_E174D8
.incbin "baserom.jp.z64", 0xE174D8, 0x208
glabel D_8025D500_E176E0
.incbin "baserom.jp.z64", 0xE176E0, 0x2D88
glabel D_80260288_E1A468
.incbin "baserom.jp.z64", 0xE1A468, 0x208
glabel D_80260490_E1A670
.incbin "baserom.jp.z64", 0xE1A670, 0x1118
glabel D_802615A8_E1B788
.incbin "baserom.jp.z64", 0xE1B788, 0x208
glabel D_802617B0_E1B990
.incbin "baserom.jp.z64", 0xE1B990, 0x2D88
glabel D_80264538_E1E718
.incbin "baserom.jp.z64", 0xE1E718, 0x208
glabel D_80264740_E1E920
.incbin "baserom.jp.z64", 0xE1E920, 0x758
glabel D_80264E98_E1F078
.incbin "baserom.jp.z64", 0xE1F078, 0x208
glabel D_802650A0_E1F280
.incbin "baserom.jp.z64", 0xE1F280, 0x1FA8
glabel D_80267048_E21228
.incbin "baserom.jp.z64", 0xE21228, 0x208
glabel D_80267250_E21430
.incbin "baserom.jp.z64", 0xE21430, 0xDC8
glabel D_80268018_E221F8
.incbin "baserom.jp.z64", 0xE221F8, 0x208
glabel D_80268220_E22400
.incbin "baserom.jp.z64", 0xE22400, 0x60
glabel D_80268280_E22460
.incbin "baserom.jp.z64", 0xE22460, 0x200
)

/* ---- .bss (not cleared by the loader) ---- */
s8 D_80277080_E31260;            /* this and the next two: func_80210140 */
s8 D_80277081_E31261;
s8 D_80277082_E31262;
s8 D_80277083_E31263;            /* this and the next two: func_80211634 */
s8 D_80277084_E31264;
s8 D_80277085_E31265;

#pragma GLOBAL_ASM("asm/nonmatchings/intro_overlay/intro_screens/func_8020F170_DC9350.s")
#pragma GLOBAL_ASM("asm/nonmatchings/intro_overlay/intro_screens/func_8020F2DC_DC94BC.s")
#pragma GLOBAL_ASM("asm/nonmatchings/intro_overlay/intro_screens/func_8020F448_DC9628.s")
#pragma GLOBAL_ASM("asm/nonmatchings/intro_overlay/intro_screens/func_8020F5B4_DC9794.s")
#pragma GLOBAL_ASM("asm/nonmatchings/intro_overlay/intro_screens/func_8020F878_DC9A58.s")
#pragma GLOBAL_ASM("asm/nonmatchings/intro_overlay/intro_screens/func_8020FCDC_DC9EBC.s")
#pragma GLOBAL_ASM("asm/nonmatchings/intro_overlay/intro_screens/func_80210140_DCA320.s")
#pragma GLOBAL_ASM("asm/nonmatchings/intro_overlay/intro_screens/func_80210D38_DCAF18.s")
#pragma GLOBAL_ASM("asm/nonmatchings/intro_overlay/intro_screens/func_802111A4_DCB384.s")
#pragma GLOBAL_ASM("asm/nonmatchings/intro_overlay/intro_screens/func_80211634_DCB814.s")
