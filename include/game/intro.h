#ifndef GAME_INTRO_H
#define GAME_INTRO_H

/* The functions of the splash and intro overlay (src/intro_overlay/: intro.c, intro_screens.c), still assembly, declared once for every file that calls them. */

#include "common.h"
#include "scene2d.h"

s32 Intro_Update(void);   /* still assembly */

/* The state src/intro_overlay/intro.c defines (.bss), in its storage order: the order of these declarations is the
 * unit's .bss layout (include/bss_order.h). */
extern s32 D_802684B0_E22690;
extern s32 D_802684B4_E22694;
extern s32 D_802684B8_E22698;
extern s32 D_802684BC_E2269C;
extern s32 D_802684C0_E226A0;   /* this and the next five: their addresses are taken */
extern s32 D_802684C4_E226A4;
extern s32 D_802684C8_E226A8;
extern s32 D_802684CC_E226AC;
extern s32 D_802684D0_E226B0;
extern s32 D_802684D4_E226B4;
extern s32 D_802684D8_E226B8;
extern s32 D_802684DC_E226BC;
extern Scene2DSlot D_802684E0_E226C0[150];   /* the shared code indexes it by 0x50; 150 to the next object (inferred from the gap) */
extern u8 D_8026B3C0_E255A0[0xBB80];   /* Intro_Update takes its address with the Controller Pak scan; its size is the gap (inferred) */
extern s32 D_80276F40_E31120;
extern s32 D_80276F44_E31124;
extern u8 D_80276F48_E31128[0x12C];   /* never addressed by the code; its size is the gap (inferred) */
extern s32 D_80277074_E31254;
extern u8 D_80277078_E31258;

/* The data src/intro_overlay/intro.c defines. */
extern s32 D_80212230_DCC410;   /* its address is passed (Intro_Init, Intro_Update, intro_screens.c) */
extern s32 D_80212234_DCC414;
extern u8 D_80212238_DCC418;
extern u8 D_8021223C_DCC41C;
extern s32 D_80212240_DCC420;
extern s32 D_80212244_DCC424[3];   /* never read or written by the code */
extern s32 D_80212250_DCC430;   /* its address is passed */
extern s32 D_80212254_DCC434;   /* its address is passed */
extern u8 D_80212258_DCC438;   /* this and the next two: intro_screens.c's func_80210140 */
extern s8 D_8021225C_DCC43C;
extern s8 D_80212260_DCC440;
extern u8 D_80212264_DCC444;   /* this and the next two: intro_screens.c's func_80211634 */
extern s8 D_80212268_DCC448;
extern s8 D_8021226C_DCC44C;
extern Gfx D_80212270_DCC450[];
extern Gfx D_802122C8_DCC4A8[];
extern Gfx D_802122E0_DCC4C0[];
extern s32 D_802122F8_DCC4D8[2];
extern Scene2DCue D_80212300_DCC4E0[108];
extern Scene2DCueList D_80212810_DCC9F0[36];
extern Scene2DSprite D_80212930_DCCB10[459];
extern Scene2DHeader D_802170E8_DD12C8;
extern s32 D_802170F8_DD12D8[2];
extern const char D_80268480_E22660[];

/* The state src/intro_overlay/intro_screens.c defines (.bss), in its storage order: the order of these declarations is the
 * unit's .bss layout (include/bss_order.h). */
extern s8 D_80277080_E31260;   /* this and the next two: func_80210140 */
extern s8 D_80277081_E31261;
extern s8 D_80277082_E31262;
extern s8 D_80277083_E31263;   /* this and the next two: func_80211634 */
extern s8 D_80277084_E31264;
extern s8 D_80277085_E31265;

/* Labels of the unit's own assembly (its data still written as assembly in the .c) that its C uses. */
extern u16 D_80217100_DD12E0[];   /* src/intro_overlay/intro.c's own assembly */
extern u8 D_80217300_DD14E0[];   /* src/intro_overlay/intro.c's own assembly */
extern u16 D_80217A80_DD1C60[];   /* src/intro_overlay/intro.c's own assembly */
extern u8 D_80217C80_DD1E60[];   /* src/intro_overlay/intro.c's own assembly */
extern u16 D_80218400_DD25E0[];   /* src/intro_overlay/intro.c's own assembly */
extern u16 D_80218600_DD27E0[];   /* src/intro_overlay/intro.c's own assembly */
extern u8 D_80218800_DD29E0[];   /* src/intro_overlay/intro.c's own assembly */
extern u16 D_8021C1A0_DD6380[];   /* src/intro_overlay/intro.c's own assembly */
extern u8 D_8021C3A0_DD6580[];   /* src/intro_overlay/intro.c's own assembly */
extern u16 D_8022CDA0_DE6F80[];   /* src/intro_overlay/intro.c's own assembly */
extern u8 D_8022CFA0_DE7180[];   /* src/intro_overlay/intro.c's own assembly */
extern u16 D_8022D330_DE7510[];   /* src/intro_overlay/intro.c's own assembly */
extern u8 D_8022D530_DE7710[];   /* src/intro_overlay/intro.c's own assembly */
extern u16 D_80232DE0_DECFC0[];   /* src/intro_overlay/intro.c's own assembly */
extern u8 D_80232FE0_DED1C0[];   /* src/intro_overlay/intro.c's own assembly */
extern u16 D_80238890_DF2A70[];   /* src/intro_overlay/intro.c's own assembly */
extern u8 D_80238A90_DF2C70[];   /* src/intro_overlay/intro.c's own assembly */
extern u16 D_8023D6D0_DF78B0[];   /* src/intro_overlay/intro.c's own assembly */
extern u8 D_8023D8D0_DF7AB0[];   /* src/intro_overlay/intro.c's own assembly */
extern u16 D_80243180_DFD360[];   /* src/intro_overlay/intro.c's own assembly */
extern u8 D_80243380_DFD560[];   /* src/intro_overlay/intro.c's own assembly */
extern u16 D_80248C30_E02E10[];   /* src/intro_overlay/intro.c's own assembly */
extern u8 D_80248E30_E03010[];   /* src/intro_overlay/intro.c's own assembly */
extern u8 D_8024B348_E05528[];   /* src/intro_overlay/intro.c's own assembly */
extern u16 D_8024BB30_E05D10[];   /* src/intro_overlay/intro.c's own assembly */
extern u8 D_8024BD38_E05F18[];   /* not referenced by the code or the sprites */
extern u16 D_8024C2E0_E064C0[];   /* not referenced by the code or the sprites */
extern u8 D_8024C4E8_E066C8[];   /* src/intro_overlay/intro.c's own assembly */
extern u16 D_8024CC70_E06E50[];   /* src/intro_overlay/intro.c's own assembly */
extern u8 D_8024CE78_E07058[];   /* src/intro_overlay/intro.c's own assembly */
extern u16 D_8024E050_E08230[];   /* src/intro_overlay/intro.c's own assembly */
extern u8 D_8024E258_E08438[];   /* src/intro_overlay/intro.c's own assembly */
extern u16 D_802510B0_E0B290[];   /* src/intro_overlay/intro.c's own assembly */
extern u8 D_802512B8_E0B498[];   /* src/intro_overlay/intro.c's own assembly */
extern u16 D_802531B0_E0D390[];   /* src/intro_overlay/intro.c's own assembly */
extern u8 D_802533B8_E0D598[];   /* src/intro_overlay/intro.c's own assembly */
extern u16 D_80254E90_E0F070[];   /* src/intro_overlay/intro.c's own assembly */
extern u8 D_80255098_E0F278[];   /* src/intro_overlay/intro.c's own assembly */
extern u16 D_802566F8_E108D8[];   /* src/intro_overlay/intro.c's own assembly */
extern u8 D_80256900_E10AE0[];   /* src/intro_overlay/intro.c's own assembly */
extern u16 D_80257538_E11718[];   /* src/intro_overlay/intro.c's own assembly */
extern u8 D_80257740_E11920[];   /* src/intro_overlay/intro.c's own assembly */
extern u16 D_80258378_E12558[];   /* src/intro_overlay/intro.c's own assembly */
extern u8 D_80258580_E12760[];   /* src/intro_overlay/intro.c's own assembly */
extern u16 D_80259048_E13228[];   /* src/intro_overlay/intro.c's own assembly */

#endif
