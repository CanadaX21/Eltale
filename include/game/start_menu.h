#ifndef GAME_START_MENU_H
#define GAME_START_MENU_H

/* The functions of src/game/start_menu.c, declared once for every file that calls them. */

#include "common.h"

void InitStartMenu(void);
void CloseStartMenu(void);
void StartMenu(mkst* dyn);
void TurnMenuPage(void);
void DrawStatusPage(s32 arg0);
void DrawEnvironmentPage(s32 arg0);
void DrawMapPage(s32 arg0);
void DrawElementsPage(s32 x);
void DrawMenuSprite(s32 sprite, s32 x, s32 y, s32 w, s32 h);
void DrawElementGauge(s32 percent, s32 x);
void DrawMenuBackground(void);
void DrawMenuNumber(s32 digits, s32 x, s32 y, u32 value, s32 font);   /* draw a number with the sheet's digits (font = kind of '0') */
void DrawMenuNumberZeros(s32 digits, s32 x, s32 y, u32 value, s32 font);
void DrawMenuDigits(s32 digits, s32 x, s32 y, u32 value);
void MapCursorToWorld(void);

/* gMenuState's bits where game mode 2 (MODE_MENU) begins and ends; the rest of the menu's state stays numeric */
#define MENU_TURNING_PAGE  0x2000  /* a page turns (TurnMenuPage) */
#define MENU_SAVE          0x4000  /* the save pages (SaveMenu): EnterSaveMode sets it with the save mode (SAVE_MODE_) */
#define MENU_OPEN          0x8000  /* the menu stays open; StartMenu clears it on B or Start and leaves for MODE_GAMEPLAY
                                      (after a load from the title too), or for MODE_TITLE when the title's load ends without one or
                                      after a delete */

typedef struct {
    u8 w;
    u8 h;
} SpriteSize;
typedef struct {
    s32 v[4];
} MenuBgRows;                        /* per background strip (four): a row count */
typedef struct {
    u8* p[4];
} MenuBgTextures;                    /* per background strip: its CI8 texture (64 wide) */
typedef struct {
    u16* p[4];
} GaugePalettes;                     /* the Elements gauge: a palette per layer */
typedef struct {
    u8* p[4][2];
} GaugeTextures;                     /* its two halves per layer */
typedef struct {
    u8 k[3];
} SliderKinds;                       /* the Environment page: the knob's sprite kinds */
typedef struct {
    s16 x;
    s16 kind;                        /* index into SliderKinds */
} SliderStop;
typedef struct {
    void* ram;
    u8* romStart;
    u8* romEnd;
} MapPageImage;                         /* a Map page image or card: where it loads, its ROM block */
typedef struct {
    u8 c[5];
} DigitText;                          /* a one-glyph text: the glyph code at c[3] */

/* The state src/game/start_menu.c defines (.bss), in its storage order: the order of these declarations is the
 * unit's .bss layout (include/bss_order.h). */
extern s32 gPageTurnLeftX;   /* page turn: the x of the page on the left */
extern s32 gPageTurnRightX;   /* and of the page on the right */
extern s32 gPageTurnFrames;
extern u16 gMapFreeCursor;
extern u16 gMenuState;
extern s8 gRespawnOnExit;
extern s32 gMenuFrame;   /* menu frame counter */
extern f32 gCompassTargetX;   /* compass target x */
extern f32 gCompassTargetZ;   /* compass target z */
extern s32 gMapPageImageId;
extern s32 gMapPageMarker;
extern f32 gMapCursorSavedX;
extern f32 gMapCursorSavedY;

/* The data src/game/start_menu.c defines. */
extern s8 gMusicVolumeSetting;   /* the music volume setting: an index into gMusicVolumeLevels (sound.c) */
extern SpriteSize gMenuSpriteSizes[88];   /* the menu sprite sheet: size per kind */
extern s8 gArrowBobOffsets[16];   /* the arrows' bob, by (gMenuFrame >> 2) & 0xF */
extern u8* gMenuDigitTextures[89];   /* its pixels per kind (CI8) */
extern Gfx gMenuSpriteSetup[3];   /* the menu sprite render setup */
extern Gfx gMenuSpriteSetupEnvAlpha[3];   /* the same, using the env colour's alpha */
extern Gfx D_80060F08[4];
extern f32 gMapCursorX;   /* the Map page's free cursor; -1 = not placed yet */
extern f32 gMapCursorY;
extern s8 gMapLocationTable[298][2];
extern s16 gMapPageMarkers[6][9][2];
extern u16* gMapPagePalettes[6];
extern u8* gMapPageStrips[6][24];
extern s16 gMapPageScale[2][6];
extern s16 gMapPageOrigin[2][6];
extern MapPageImage gMapPageImages[10];   /* the six region maps, then the four cards */
extern u16* gMapCardPalettes[4];   /* the cards' palettes, as loaded */
extern u8* gMapCardImages[4][2];   /* their pixels, as loaded */
extern u8 gMapCardSizes[4][3];   /* their width, then the rows of each half */
extern s16 gMapCardPositions[4][2];   /* their x, y */
extern SliderKinds gSliderKinds;
extern SliderStop gMessageSpeedStops[5];   /* message speed: the knob's five stops */
extern SliderStop D_800615C0[3];   /* unreferenced: zero */
extern SliderStop gMusicVolumeStops[5];   /* music volume: the knob's five stops */
extern GaugePalettes gGaugePalettes;
extern GaugeTextures gGaugeTextures;
extern MenuBgRows gStripTexHeights;   /* the strips' texture heights */
extern MenuBgRows gStripHeights;   /* the strips' heights on screen */
extern MenuBgTextures gMenuBgTextures;
extern DigitText gZeroDigitText;

#endif
