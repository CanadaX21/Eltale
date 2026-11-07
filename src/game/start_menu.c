#include "common.h"
/* The Start menu (InitStartMenu to MapCursorToWorld): its own file after the music functions of sound.c. */
extern Gfx* gDisplayListHead;
extern Gfx gMenuRenderDL[];
extern u16 gGameMode;               /* the game mode (u16 as in main.c) */
extern u16 gSaveMenuState;
typedef struct {
    u8 w;
    u8 h;
} SpriteSize;
extern SpriteSize gMenuSpriteSizes[];      /* the menu sprite sheet: size per kind */
extern u8* gMenuDigitTextures[];             /* its pixels per kind (CI8) */
extern Gfx gMenuSpriteSetup[];             /* the menu sprite render setup */
extern Gfx gMenuSpriteSetupEnvAlpha[];             /* the same, using the env colour's alpha */
extern u16 gMenuSheetPalette[];             /* the menu sheet's palette (256 RGBA16) */
typedef struct {
    s32 v[4];
} MenuBgRows;                        /* per background strip (four): a row count */
typedef struct {
    u8* p[4];
} MenuBgTextures;                    /* per background strip: its CI8 texture (64 wide) */
extern u16 gBackgroundPalette[];             /* the background's palette */
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
extern SliderKinds gSliderKinds;
extern SliderStop gMessageSpeedStops[5];     /* message speed: the knob's five stops */
extern SliderStop gMusicVolumeStops[5];     /* music volume: the knob's five stops */
extern s8 gMessageSpeed;                /* the message speed (the page shows 4 - it) */
extern s8 gArrowBobOffsets[];              /* arrow bob offsets */
extern BrianData1 gBrianStats;        /* Brian's stats */
extern u32 gSpiritExpTable[];             /* exp needed per level */
extern s8 gStickX;                /* stick x */
extern u16 gButtonsHeld;               /* buttons held */
void DrawMenuSprite(s32 sprite, s32 x, s32 y, s32 w, s32 h);
typedef struct {
    void* ram;
    u8* romStart;
    u8* romEnd;
} MapPageImage;                         /* a Map page image or card: where it loads, its ROM block */
extern MapPageImage gMapPageImages[];   /* the six region maps, then the four cards */
void DrawMenuNumber(s32 digits, s32 x, s32 y, u32 value, s32 font);   /* draw a number with the sheet's digits (font = kind of '0') */
void DrawElementGauge(s32 percent, s32 x);
extern s32 D_8007A9E0;               /* gBrianStats.exp: a link name, as DrawElementsPage loads this and the level on separate high halves (members share one; a pointer local takes a stack word the frame lacks) */
extern u8 D_8007AA04;                /* gBrianStats.spiritLevel: the same */
extern s8 gMapLocationTable[][2];
extern PlayerState gPlayerState;
extern u16 gXPTable[];               /* XP needed per stat level */
typedef struct {
    u8 c[5];
} DigitText;                          /* a one-glyph text: the glyph code at c[3] */
u64 __ull_div(u64 a, u64 b);      /* libultra __ull_div */
void DrawString(s32 x, s32 y, u8* text);
extern s8 gMusicVolumeSetting;             /* the music volume setting (sound.c) */
void PlaySound(u8 delay, s8 id, u8 level);  /* sound.c */

/* ---- this file's .bss (0x8008FB30-0x8008FB6F), in address order. IDO 5.3 lays a variable out by its first
   declaration, so nothing above declares these. DrawEnvironmentPage's knob (0x8008FB58) is a static in that
   function, and the map cursor values after it are defined after that function. ---- */
s32 gPageTurnLeftX;               /* page turn: the x of the page on the left */
s32 gPageTurnRightX;               /* and of the page on the right */
s32 gPageTurnFrames;
u16 gMapFreeCursor;
u16 gMenuState;
s8 gRespawnOnExit;
s32 gMenuFrame;               /* menu frame counter */
f32 gCompassTargetX;
f32 gCompassTargetZ;
s32 gMapPageImageId;
s32 gMapPageMarker;
void InitStartMenu(void) {
    gMenuState = 0x8000;
    gSaveMenuState = 0;
    gMapFreeCursor = 0;
}
void CloseStartMenu(void) {
    gMenuState = (gMenuState & 3) | 0x8000;
    gSaveMenuState = 0;
    gMapFreeCursor = 0;
}
extern u16 gButtonsPressed;               /* buttons pressed this frame */
void DrawMenuBackground(void);
void SaveMenu(void);
void TurnMenuPage(void);
void DrawStatusPage(s32 arg0);
void DrawMapPage(s32 arg0);
void DrawElementsPage(s32 x);
void DrawEnvironmentPage(s32 arg0);
void StartMenu(mkst* dyn) {
    gSPDisplayList(gDisplayListHead++, gMenuRenderDL);
    gDPSetTextureLUT(gDisplayListHead++, G_TT_RGBA16);
    DrawMenuBackground();
    gMenuFrame++;
    if (gMenuState & 0x4000) {
        SaveMenu();
    } else if (gMenuState & 0x2000) {
        TurnMenuPage();
    } else {
        switch ((u16)(gMenuState & 3)) {
            case 0:
                DrawStatusPage(0);
                break;
            case 1:
                DrawMapPage(0);
                break;
            case 2:
                DrawElementsPage(0);
                break;
            case 3:
                DrawEnvironmentPage(0);
                break;
        }
    }
    gMenuState &= ~0x10;
    gDPSetTextureLUT(gDisplayListHead++, G_TT_NONE);
    gDPSetBlendColor(gDisplayListHead++, 0, 0, 0, 1);
    if ((gButtonsPressed & (B_BUTTON | START_BUTTON)) && !(gMenuState & 0x6020)) {
        gMenuState &= ~0x8000;
        PlaySound(0, 1, 0xFF);
    }
    if (!(gMenuState & 0x8000)) {
        if (gMenuState & 0x4000) {
            switch ((u16)(gMenuState & 0xC)) {
                case 0:
                    if (gSaveMenuState & 0x80) {
                        gGameMode = MODE_GAMEPLAY;
                        gRespawnOnExit = 1;
                    } else {
                        gGameMode = MODE_TITLE;
                        gRespawnOnExit = 0;
                    }
                    break;
                case 8:
                    gGameMode = MODE_TITLE;
                    break;
                case 4:
                    gGameMode = MODE_GAMEPLAY;
                    break;
            }
        } else {
            gGameMode = MODE_GAMEPLAY;
        }
        CloseStartMenu();
    }
}
void TurnMenuPage(void) {
    s32 page;
    s32 dir;
    s32 off;
    s32 cur;
    s32 next;

    dir = 0;
    off = gPageTurnFrames * gPageTurnFrames * 0.3125;
    if (gMenuState & 0x1000) {
        gPageTurnLeftX = 320 - off;
        gPageTurnRightX = -off;
        cur = gPageTurnLeftX;
        next = gPageTurnRightX;
        if (gPageTurnFrames == 0) {
            dir = -1;
        }
    } else {
        gPageTurnLeftX = off;
        gPageTurnRightX = off - 320;
        cur = gPageTurnRightX;
        next = gPageTurnLeftX;
        if (gPageTurnFrames == 0) {
            dir = 1;
        }
    }
    page = gMenuState & 3;
    gPageTurnFrames--;
    switch (page) {
        case 0:
            DrawStatusPage(cur);
            if (gMenuState & 0x1000) {
                DrawEnvironmentPage(next);
            } else {
                DrawMapPage(next);
            }
            break;
        case 1:
            DrawMapPage(cur);
            if (gMenuState & 0x1000) {
                DrawStatusPage(next);
            } else {
                DrawElementsPage(next);
            }
            break;
        case 2:
            DrawElementsPage(cur);
            if (gMenuState & 0x1000) {
                DrawMapPage(next);
            } else {
                DrawEnvironmentPage(next);
            }
            break;
        case 3:
            DrawEnvironmentPage(cur);
            if (gMenuState & 0x1000) {
                DrawElementsPage(next);
            } else {
                DrawStatusPage(next);
            }
            break;
    }
    if (dir != 0) {
        gMenuState &= ~0x2000;
        gMenuState &= ~0x1000;
        page += dir;
        gMenuState &= ~3;
        gMenuState |= page & 3;
    }
}
void DrawStatusPage(s32 arg0) {
    s32 y;
    s32 n;
    f32 f;
    BrianData1* stats;
    PlayerState* state;

    stats = &gBrianStats;
    state = &gPlayerState;
    gDPPipeSync(gDisplayListHead++);
    gDPSetEnvColor(gDisplayListHead++, 0xFF, 0xFF, 0xFF, 0xFF);
    DrawMenuSprite(0, arg0 + 0x28, 0x1E, 0xF0, 0x98);
    DrawMenuSprite(0x37, arg0 + 0x2B, 0xB7, 0xF0, 3);
    DrawMenuSprite(0x37, arg0 + 0x119, 0x24, 2, 0x96);
    DrawMenuSprite(0x37, arg0 + 0x27, 0x1D, 0xF2, 1);
    DrawMenuSprite(0x37, arg0 + 0x27, 0xB6, 0xF2, 1);
    DrawMenuSprite(0x37, arg0 + 0x27, 0x1D, 1, 0x9A);
    DrawMenuSprite(0x37, arg0 + 0x118, 0x1D, 1, 0x9A);
    DrawMenuSprite(0x38, arg0 + 0x29, 0x1F, 0xED, 1);
    DrawMenuSprite(0x38, arg0 + 0x29, 0x1F, 1, 0x95);
    DrawMenuSprite(0x39, arg0 + 0x29, 0xB4, 0xEE, 1);
    DrawMenuSprite(0x39, arg0 + 0x116, 0x1F, 1, 0x96);
    DrawMenuSprite(0x3C, arg0 + 0x3A, 0x1E, 0x15, 0x98);
    y = 0x4B;
    for (n = 0; n < 3; n++) {
        DrawMenuSprite(0x3D, arg0 + 0x5D, y, 0xA7, 2);
        y += 0x21;
    }
    y = 0x40;
    for (n = 0; n < 4; n++) {
        DrawMenuSprite(0x32, arg0 + 0x63, y, 0x3C, 7);
        y += 0x21;
    }
    y = 0x3C;
    for (n = 0; n < 4; n++) {
        DrawMenuSprite(0x20, arg0 + 0xC3, y, 0x13, 0xB);
        y += 0x21;
    }
    DrawMenuSprite(0x1B, arg0 + 0x38, 0x34, 0x18, 0xE);
    DrawMenuSprite(0x1C, arg0 + 0x37, 0x54, 0x1C, 0xD);
    DrawMenuSprite(0x1D, arg0 + 0x35, 0x78, 0x24, 0xF);
    DrawMenuSprite(0x1E, arg0 + 0x35, 0x99, 0x1C, 0xE);
    DrawMenuSprite(5, arg0 + 0x45, 0x1C, 8, 8);
    DrawMenuSprite(5, arg0 + 0xF3, 0x1C, 8, 8);
    n = (gMenuFrame >> 2) & 0xF;
    n = gArrowBobOffsets[n];
    DrawMenuSprite(6, arg0 + n + 0x36, 0x1A, 0xE, 0xB);
    DrawMenuSprite(7, arg0 - n + 0xFB, 0x1A, 0xE, 0xB);
    DrawMenuSprite(0x14, arg0 + 0x79, 0x35, 8, 0xA);
    DrawMenuSprite(0x14, arg0 + 0x79, 0x56, 8, 0xA);
    n = stats->maxHP;
    DrawMenuNumber(3, arg0 + 0x80, 0x35, n, 0xA);
    n = stats->maxMP;
    DrawMenuNumber(3, arg0 + 0x80, 0x56, n, 0xA);
    n = stats->currHP;
    DrawMenuNumber(3, arg0 + 0x64, 0x35, n, 0xA);
    n = stats->currMP;
    DrawMenuNumber(3, arg0 + 0x64, 0x56, n, 0xA);
    n = stats->def;
    DrawMenuNumber(3, arg0 + 0x80, 0x77, n, 0xA);
    n = stats->agi;
    DrawMenuNumber(3, arg0 + 0x80, 0x98, n, 0xA);
    n = gXPTable[stats->HPLevel];
    DrawMenuNumber(3, arg0 + 0xDA, 0x3B, stats->HPXP * 100 / n, 0xA);
    n = gXPTable[stats->MPLevel];
    if (state->flags & 0x4000) {
        n *= 3;
    } else {
        n *= 4;
    }
    DrawMenuNumber(3, arg0 + 0xDA, 0x5C, stats->MPXP * 100 / n, 0xA);
    n = gXPTable[stats->defLevel];
    if (state->flags & 0x2000) {
        n = (f32)n * 0.8;
    }
    DrawMenuNumber(3, arg0 + 0xDA, 0x7D, stats->defXP * 100 / n, 0xA);
    n = gXPTable[stats->agiLevel];
    if (state->flags & 0x8000) {
        n = (f32)n * 0.8;
    }
    DrawMenuNumber(3, arg0 + 0xDA, 0x9E, stats->agiXP * 100 / n, 0xA);
    f = stats->currHP;
    f /= stats->maxHP;
    f *= 50.0f;
    DrawMenuSprite(0x33, arg0 + 0x64, 0x41, f, 3);
    f = stats->currMP;
    f /= stats->maxMP;
    f *= 50.0f;
    DrawMenuSprite(0x33, arg0 + 0x64, 0x62, f, 3);
    f = stats->def;
    f /= 255.0f;
    f *= 50.0f;
    DrawMenuSprite(0x33, arg0 + 0x64, 0x83, f, 3);
    f = stats->agi;
    f /= 255.0f;
    f *= 50.0f;
    DrawMenuSprite(0x33, arg0 + 0x64, 0xA4, f, 3);
    DrawMenuSprite(0x15, arg0 + 0xF6, 0x3B, 8, 0xA);
    DrawMenuSprite(0x15, arg0 + 0xF6, 0x5C, 8, 0xA);
    DrawMenuSprite(0x15, arg0 + 0xF6, 0x7D, 8, 0xA);
    DrawMenuSprite(0x15, arg0 + 0xF6, 0x9E, 8, 0xA);
    DrawMenuSprite(0x1A, arg0 + 0x77, 0x19, 0x52, 0xD);
    if (!(gMenuState & 0x2000)) {
        if ((gStickX > 20) || (gButtonsHeld & R_TRIG)) {
            gMenuState |= 0x2000;
            gPageTurnFrames = 0x1F;
            PlaySound(0, 0, 0xFF);
        } else if ((gStickX < -20) || (gButtonsHeld & Z_TRIG)) {
            gMenuState |= 0x3000;
            gPageTurnFrames = 0x1F;
            PlaySound(0, 0, 0xFF);
        }
    }
}

extern s8 gStickY;
void DrawEnvironmentPage(s32 arg0) {
    static s32 gEnvKnobX;       /* the moving knob's x */
    s32 bob;
    s32 sel;
    s32 y;
    SliderKinds kinds;

    kinds = gSliderKinds;
    gMessageSpeed = 4 - gMessageSpeed;
    DrawMenuSprite(0, arg0 + 0x28, 0x1E, 0xF0, 0x6C);
    DrawMenuSprite(0x37, arg0 + 0x2B, 0x8B, 0xF0, 3);
    DrawMenuSprite(0x37, arg0 + 0x119, 0x24, 2, 0x68);
    DrawMenuSprite(0x37, arg0 + 0x27, 0x1D, 0xF2, 1);
    DrawMenuSprite(0x37, arg0 + 0x27, 0x8A, 0xF2, 1);
    DrawMenuSprite(0x37, arg0 + 0x27, 0x1E, 1, 0x6D);
    DrawMenuSprite(0x37, arg0 + 0x118, 0x1E, 1, 0x6D);
    DrawMenuSprite(0x38, arg0 + 0x29, 0x1F, 0xED, 1);
    DrawMenuSprite(0x38, arg0 + 0x29, 0x20, 1, 0x68);
    DrawMenuSprite(0x39, arg0 + 0x29, 0x88, 0xEE, 1);
    DrawMenuSprite(0x39, arg0 + 0x116, 0x1F, 1, 0x69);
    DrawMenuSprite(0x3C, arg0 + 0x3A, 0x1E, 0x15, 0x6C);
    DrawMenuSprite(0x3D, arg0 + 0x5D, 0x56, 0xA7, 2);
    DrawMenuSprite(0x24, arg0 + 0x8D, 0x3E, 0x54, 7);
    DrawMenuSprite(0x24, arg0 + 0x8D, 0x6C, 0x54, 7);
    DrawMenuSprite(0x57, arg0 + 0xE9, 0x3C, 0x1A, 9);
    DrawMenuSprite(0x56, arg0 + 0x6C, 0x3C, 0x1A, 9);
    DrawMenuSprite(0x29, arg0 + 0xE9, 0x6B, 0x1A, 9);
    DrawMenuSprite(0x2A, arg0 + 0x6C, 0x6B, 0x1A, 9);
    DrawMenuSprite(0x22, arg0 + 0x31, 0x3B, 0x36, 0xC);
    DrawMenuSprite(0x23, arg0 + 0x35, 0x68, 0x2A, 0xC);
    DrawMenuSprite(5, arg0 + 0x45, 0x1C, 7, 7);
    DrawMenuSprite(5, arg0 + 0xF3, 0x1C, 7, 7);
    bob = gMenuFrame;
    bob >>= 2;
    bob &= 0xF;
    bob = gArrowBobOffsets[bob];
    DrawMenuSprite(6, arg0 + bob + 0x36, 0x1A, 0xE, 0xB);
    DrawMenuSprite(7, arg0 - bob + 0xFB, 0x1A, 0xE, 0xB);
    DrawMenuSprite(0x21, arg0 + 0x75, 0x18, 0x56, 0xE);
    DrawMenuSprite(0x25, arg0 + 0x8E, 0x3F, 0x50, 3);
    DrawMenuSprite(0x25, arg0 + 0x8E, 0x6D, 0x50, 3);
    DrawMenuSprite(0x27, arg0 + 0xB3, 0x39, 8, 0xB);
    DrawMenuSprite(0x27, arg0 + 0xB3, 0x67, 8, 0xB);
    if (gMenuState & 0x200) {
        switch (gMenuState & 0x100) {
            case 0:
                DrawMenuSprite(kinds.k[gMusicVolumeStops[gMusicVolumeSetting].kind], gMusicVolumeStops[gMusicVolumeSetting].x + arg0, 0x67, 8, 0xB);
                break;
            case 0x100:
                DrawMenuSprite(kinds.k[gMessageSpeedStops[gMessageSpeed].kind], gMessageSpeedStops[gMessageSpeed].x + arg0, 0x39, 8, 0xB);
                break;
        }
    } else {
        DrawMenuSprite(kinds.k[gMusicVolumeStops[gMusicVolumeSetting].kind], gMusicVolumeStops[gMusicVolumeSetting].x + arg0, 0x67, 8, 0xB);
        DrawMenuSprite(kinds.k[gMessageSpeedStops[gMessageSpeed].kind], gMessageSpeedStops[gMessageSpeed].x + arg0, 0x39, 8, 0xB);
    }
    if (gMenuState & 0x2000) {
        gMessageSpeed = 4 - gMessageSpeed;
        return;
    }
    if (!(gMenuState & 0x800)) {
        if ((gStickX > 20) || (gButtonsHeld & 0x10)) {
            gMenuState |= 0x2000;
            gPageTurnFrames = 0x1F;
            PlaySound(0, 0, 0xFF);
        } else if ((gStickX < -20) || (gButtonsHeld & 0x2000)) {
            gMenuState |= 0x3000;
            gPageTurnFrames = 0x1F;
            PlaySound(0, 0, 0xFF);
        }
        if (gButtonsPressed & 0x8000) {
            gMenuState |= 0x820;
            PlaySound(0, 1, 0xFF);
        }
    } else {
        if ((gButtonsPressed & 0x8000) && !(gMenuState & 0x200)) {
            gMenuState &= ~0x920;
            PlaySound(0, 2, 0xFF);
            gMessageSpeed = 4 - gMessageSpeed;
            return;
        }
        sel = gMenuState & 0x100;
        bob = gMenuFrame;
        bob >>= 1;
        bob &= 0xF;
        bob = gArrowBobOffsets[bob];
        if (gMenuState & 0x200) {
            switch (sel) {
                case 0:
                    if (gEnvKnobX >= gMessageSpeedStops[gMessageSpeed].x && gEnvKnobX <= gMessageSpeedStops[gMessageSpeed].x) {
                        gMenuState &= ~0x200;
                        gMenuState &= ~0x400;
                        sel = kinds.k[gMessageSpeedStops[gMessageSpeed].kind];
                        if (gStickX < 20 && gStickX > -20) {
                            DrawMenuSprite(sel, gEnvKnobX + arg0, 0x39, 8, 0xB);
                        }
                        y = 0x3A;
                        sel = 0;
                    }
                    break;
                case 0x100:
                    if (gEnvKnobX >= gMusicVolumeStops[gMusicVolumeSetting].x && gEnvKnobX <= gMusicVolumeStops[gMusicVolumeSetting].x) {
                        gMenuState &= ~0x200;
                        gMenuState &= ~0x400;
                        sel = kinds.k[gMusicVolumeStops[gMusicVolumeSetting].kind];
                        if (gStickX < 20 && gStickX > -20) {
                            DrawMenuSprite(sel, gEnvKnobX + arg0, 0x67, 8, 0xB);
                        }
                        y = 0x65;
                        sel = 0x100;
                    }
                    break;
            }
        }
        if (!(gMenuState & 0x200)) {
            switch (sel) {
                case 0:
                    y = 0x3A;
                    if (gStickY < -30 || gStickY > 30) {
                        if (!(gMenuState & 0x400)) {
                            gMenuState &= ~0x100;
                            gMenuState |= 0x100;
                            gMenuState |= 0x400;
                            PlaySound(0, 0, 0xFF);
                        }
                    } else if (gStickX < -20) {
                        if (!(gMenuState & 0x400)) {
                            gEnvKnobX = gMessageSpeedStops[gMessageSpeed--].x;
                            if (gMessageSpeed < 0) {
                                gMessageSpeed = 0;
                            }
                            gMenuState |= 0x600;
                        }
                    } else if (gStickX > 20) {
                        if (!(gMenuState & 0x400)) {
                            gEnvKnobX = gMessageSpeedStops[gMessageSpeed++].x;
                            if (gMessageSpeed > 4) {
                                gMessageSpeed = 4;
                            }
                            gMenuState |= 0x600;
                        }
                    } else {
                        gMenuState &= ~0x400;
                    }
                    break;
                case 0x100:
                    y = 0x65;
                    if (gStickY < -30 || gStickY > 30) {
                        if (!(gMenuState & 0x400)) {
                            gMenuState &= ~0x100;
                            gMenuState |= 0;    /* the row field set to the speed row (0), as the other case sets 0x100: the target stores twice */
                            gMenuState |= 0x400;
                            PlaySound(0, 0, 0xFF);
                        }
                    } else if (gStickX < -20) {
                        if (!(gMenuState & 0x400)) {
                            gEnvKnobX = gMusicVolumeStops[gMusicVolumeSetting--].x;
                            if (gMusicVolumeSetting < 0) {
                                gMusicVolumeSetting = 0;
                            }
                            gMenuState |= 0x400;
                            gMenuState |= 0x200;
                        }
                    } else if (gStickX > 20) {
                        if (!(gMenuState & 0x400)) {
                            gEnvKnobX = gMusicVolumeStops[gMusicVolumeSetting++].x;
                            if (gMusicVolumeSetting > 4) {
                                gMusicVolumeSetting = 4;
                            }
                            gMenuState |= 0x400;
                            gMenuState |= 0x200;
                        }
                    } else {
                        gMenuState &= ~0x400;
                    }
                    break;
            }
        }
        if (gMenuState & 0x200) {
            switch (sel) {
                case 0:
                if (gEnvKnobX < gMessageSpeedStops[gMessageSpeed].x) {
                    gEnvKnobX++;
                    sel = kinds.k[gMessageSpeedStops[gMessageSpeed - 1].kind];
                } else if (gEnvKnobX > gMessageSpeedStops[gMessageSpeed].x) {
                    gEnvKnobX--;
                    sel = kinds.k[gMessageSpeedStops[gMessageSpeed + 1].kind];
                } else {
                    gMenuState &= ~0x400;
                    sel = kinds.k[gMessageSpeedStops[gMessageSpeed].kind];
                }
                y = 0x3A;
                DrawMenuSprite(sel, gEnvKnobX + arg0, 0x39, 8, 0xB);
                break;
                case 0x100:
                if (gEnvKnobX < gMusicVolumeStops[gMusicVolumeSetting].x) {
                    gEnvKnobX++;
                    sel = kinds.k[gMusicVolumeStops[gMusicVolumeSetting - 1].kind];
                } else if (gEnvKnobX > gMusicVolumeStops[gMusicVolumeSetting].x) {
                    gEnvKnobX--;
                    sel = kinds.k[gMusicVolumeStops[gMusicVolumeSetting + 1].kind];
                } else {
                    gMenuState &= ~0x400;
                    sel = kinds.k[gMusicVolumeStops[gMusicVolumeSetting].kind];
                }
                y = 0x65;
                DrawMenuSprite(sel, gEnvKnobX + arg0, 0x67, 8, 0xB);
                break;
            }
        }
        DrawMenuSprite(1, arg0 - bob + 0x1E, y, 0x10, 0xD);
    }
    gMessageSpeed = 4 - gMessageSpeed;
}
void EndMenuDraw(void);
void dma_write(u8* devAddr, void* vaddr, u32 nbytes);
extern s16 gMapPageOrigin[2][6];
extern f32 gMapCursorX;
extern f32 gMapCursorY;
extern s16 gMapPageMarkers[][9][2];
extern u16* gMapPagePalettes[];
extern u8* gMapPageStrips[][24];
extern s16 gMapPageScale[2][6];
extern u16* gMapCardPalettes[];
extern u8* gMapCardImages[][2];
extern u8 gMapCardSizes[][3];
extern s16 gMapCardPositions[][2];
/* the Start menu .bss after DrawEnvironmentPage's static (0x8008FB58): the map cursor saved by DrawMapPage */
f32 gMapCursorSavedX;
f32 gMapCursorSavedY;
void DrawMapPage(s32 arg0) {
    static s32 sLoadedMapImage = -1;
    s32 mx;
    s32 my;
    s32 x;
    s32 rows;
    s32 s;
    s32 n;
    s32 y;
    f32 d;

    s = 0;
    if (gMapPageImageId < 6) {
        if (gMapPageImageId != sLoadedMapImage) {
            dma_write(gMapPageImages[gMapPageImageId].romStart, gMapPageImages[gMapPageImageId].ram,
                      gMapPageImages[gMapPageImageId].romEnd - gMapPageImages[gMapPageImageId].romStart);
            sLoadedMapImage = gMapPageImageId;
        }
        mx = 244;
        rows = 8;
        x = arg0 + 39;
        y = 29;
        if (x < 0) {
            if (x + mx < 0) {
                rows = 0;
            } else {
                s = -x;
                x = 0;
                mx -= s;
                s <<= 5;
            }
        }
        if (x > 320) {
            rows = 0;
        } else if (x + mx > 320) {
            mx -= x + mx - 320;
            s = 0;
        }
        if (rows) {
            gDPPipeSync(gDisplayListHead++);
            gDPLoadTLUT_pal256(gDisplayListHead++, gMapPagePalettes[gMapPageImageId]);
            for (n = 0; n < 24; n++) {
                if (n == 23) {
                    rows = 1;
                }
                gDPLoadTextureTile(gDisplayListHead++, gMapPageStrips[gMapPageImageId][n], G_IM_FMT_CI, G_IM_SIZ_8b, 244,
                                   rows, 0, 0, 243, rows - 1, 0, G_TX_WRAP, G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK,
                                   G_TX_NOLOD, G_TX_NOLOD);
                gSPTextureRectangle(gDisplayListHead++, x << 2, y << 2, (x + mx) << 2, (y + rows) << 2,
                                    G_TX_RENDERTILE, s, 0, 1 << 10, 1 << 10);
                gDPPipeSync(gDisplayListHead++);
                y += rows;
            }
            EndMenuDraw();
        }
    } else {
        DrawMenuSprite(0, arg0 + 0x28, 0x1E, 0xF0, 0xB4);
        DrawMenuSprite(0x38, arg0 + 0x29, 0x1F, 0xED, 1);
        DrawMenuSprite(0x38, arg0 + 0x29, 0x20, 1, 0xB1);
        DrawMenuSprite(0x39, arg0 + 0x29, 0xD0, 0xEE, 1);
        DrawMenuSprite(0x39, arg0 + 0x117, 0x1F, 1, 0xB2);
        DrawMenuSprite(0x37, arg0 + 0x2C, 0xD4, 0xF0, 3);
        DrawMenuSprite(0x37, arg0 + 0x11A, 0x25, 2, 0xB2);
        DrawMenuSprite(0x37, arg0 + 0x27, 0x1D, 0xF2, 1);
        DrawMenuSprite(0x37, arg0 + 0x27, 0xD3, 0xF2, 1);
        DrawMenuSprite(0x37, arg0 + 0x27, 0x1E, 1, 0xB3);
        DrawMenuSprite(0x37, arg0 + 0x119, 0x1E, 1, 0xB3);
        if (gMapPageImageId != sLoadedMapImage) {
            dma_write(gMapPageImages[gMapPageImageId].romStart, gMapPageImages[gMapPageImageId].ram,
                      gMapPageImages[gMapPageImageId].romEnd - gMapPageImages[gMapPageImageId].romStart);
            sLoadedMapImage = gMapPageImageId;
        }
        n = gMapPageImageId - 6;
        mx = gMapCardSizes[n][0];
        rows = gMapCardSizes[n][1];
        x = gMapCardPositions[n][0] + arg0;
        y = gMapCardPositions[n][1];
        if (x < 0) {
            if (x + mx < 0) {
                rows = 0;
            } else {
                s = -x;
                x = 0;
                mx -= s;
                s <<= 5;
            }
        }
        if (x > 320) {
            rows = 0;
        } else if (x + mx > 320) {
            mx -= x + mx - 320;
            s = 0;
        }
        if (rows) {
            gDPLoadTLUT_pal256(gDisplayListHead++, gMapCardPalettes[n]);
            gMenuState &= ~0x10;
            gDPLoadTextureTile(gDisplayListHead++, gMapCardImages[n][0], G_IM_FMT_CI, G_IM_SIZ_8b, gMapCardSizes[n][0],
                               gMapCardSizes[n][1], 0, 0, gMapCardSizes[n][0] - 1, gMapCardSizes[n][1] - 1, 0, G_TX_WRAP,
                               G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
            gDPPipeSync(gDisplayListHead++);
            gSPTextureRectangle(gDisplayListHead++, x << 2, y << 2, (x + mx) << 2, (y + rows) << 2, G_TX_RENDERTILE,
                                s, 0, 1 << 10, 1 << 10);
            gDPPipeSync(gDisplayListHead++);
            gDPLoadTextureTile(gDisplayListHead++, gMapCardImages[n][1], G_IM_FMT_CI, G_IM_SIZ_8b, gMapCardSizes[n][0],
                               gMapCardSizes[n][2], 0, 0, gMapCardSizes[n][0] - 1, gMapCardSizes[n][2] - 1, 0, G_TX_WRAP,
                               G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
            gDPPipeSync(gDisplayListHead++);
            gSPTextureRectangle(gDisplayListHead++, x << 2, (y + gMapCardSizes[n][1]) << 2, (x + mx) << 2,
                                (y + rows + gMapCardSizes[n][2]) << 2, G_TX_RENDERTILE, s, 0, 1 << 10, 1 << 10);
            EndMenuDraw();
        }
    }
    DrawMenuSprite(5, arg0 + 0x45, 0x1C, 7, 7);
    DrawMenuSprite(5, arg0 + 0xF3, 0x1C, 7, 7);
    n = (gMenuFrame >> 2) & 0xF;
    n = gArrowBobOffsets[n];
    DrawMenuSprite(6, arg0 + n + 0x36, 0x1A, 0xE, 0xB);
    DrawMenuSprite(7, arg0 - n + 0xFB, 0x1A, 0xE, 0xB);
    DrawMenuSprite(0x2B, arg0 + 0x83, 0x18, 0x3A, 0x10);
    if (gMapPageImageId < 6) {
        if (gMapPageMarker < 0) {
            mx = gPlayerState.brian.transform.pos.x;
            mx /= gMapPageScale[0][gMapPageImageId];
            mx += gMapPageOrigin[0][gMapPageImageId];
            my = gPlayerState.brian.transform.pos.z;
            my /= gMapPageScale[1][gMapPageImageId];
            my += gMapPageOrigin[1][gMapPageImageId];
        } else {
            mx = gMapPageMarkers[gMapPageImageId][gMapPageMarker][0];
            my = gMapPageMarkers[gMapPageImageId][gMapPageMarker][1];
        }
        if (gMenuFrame & 0x20) {
            DrawMenuSprite(0x2C, arg0 + mx + 0x23, my + 0xD, 0xC, 0x10);
        }
        if (gMapPageMarker < 0) {
            if (gMapCursorX == -1.0f) {
                gMapCursorX = mx;
                gMapCursorY = my;
            }
            if (gMapFreeCursor) {
                d = gStickX;
                if (d > 0.0f) {
                    d -= 5.0f;
                    if (d < 0.0f) {
                        d = 0.0f;
                    }
                } else {
                    d += 5.0f;
                    if (d > 0.0f) {
                        d = 0.0f;
                    }
                }
                gMapCursorX += d * 0.08;
                d = gStickY;
                if (d > 0.0f) {
                    d -= 5.0f;
                    if (d < 0.0f) {
                        d = 0.0f;
                    }
                } else {
                    d += 5.0f;
                    if (d > 0.0f) {
                        d = 0.0f;
                    }
                }
                gMapCursorY -= d * 0.08;
                if (gMapCursorX < 6.0f) {
                    gMapCursorX = 6.0f;
                } else if (gMapCursorX > 236.0f) {
                    gMapCursorX = 236.0f;
                }
                if (gMapCursorY < 6.0f) {
                    gMapCursorY = 6.0f;
                } else if (gMapCursorY > 178.0f) {
                    gMapCursorY = 178.0f;
                }
            }
            mx = gMapCursorX;
            my = gMapCursorY;
            mx -= gMapPageOrigin[0][gMapPageImageId];
            my -= gMapPageOrigin[1][gMapPageImageId];
            gCompassTargetX = gMapPageScale[0][gMapPageImageId] * mx;
            gCompassTargetZ = gMapPageScale[1][gMapPageImageId] * my;
            x = -2;
            if (gMapFreeCursor) {
                y = -2;
                my = 6;
            } else {
                y = 0;
                my = 5;
            }
            DrawMenuSprite(0x55, arg0 + gMapCursorX + 39.0f + x, gMapCursorY + 29.0f - 1.0f, my, 1);
            DrawMenuSprite(4, arg0 + gMapCursorX + 39.0f - 2.0f, gMapCursorY + 29.0f - 12.0f + y, 0xC, 0xC);
        }
    }
    if (!(gMenuState & 0x2000)) {
        if (!gMapFreeCursor) {
            if ((gStickX > 20) || (gButtonsHeld & R_TRIG)) {
                gMenuState |= 0x2000;
                gPageTurnFrames = 0x1F;
                PlaySound(0, 0, 0xFF);
            } else if ((gStickX < -20) || (gButtonsHeld & Z_TRIG)) {
                gMenuState |= 0x3000;
                gPageTurnFrames = 0x1F;
                PlaySound(0, 0, 0xFF);
            }
        }
        if ((gMapPageImageId < 6) && (gMapPageMarker < 0)) {
            if (!gMapFreeCursor) {
                gMapCursorSavedX = gMapCursorX;
                gMapCursorSavedY = gMapCursorY;
            }
            if (gButtonsPressed & A_BUTTON) {
                gMapFreeCursor ^= 1;
                gMenuState ^= 0x20;
                PlaySound(0, 0x17, 0xFF);
            } else if ((gButtonsPressed & B_BUTTON) && gMapFreeCursor) {
                gMapCursorX = gMapCursorSavedX;
                gMapCursorY = gMapCursorSavedY;
                PlaySound(0, 0x13, 0xFF);
            }
        }
    }
}

/* ---- sound.c's .data from here (0x800615E4-0x80061650, ROM 0x621E4), after DrawMapPage's
   static at 0x800615E0: the definitions keep this place in the text, so nothing above declares them. */
extern u16 D_80068EB0[];
extern u16 D_8006A8F0[];
extern u16 D_8006B4D8[];
extern u16 D_80069A98[];
extern u8 D_800690B8[];
extern u8 D_800695A8[];
extern u8 D_8006AAF8[];
extern u8 D_8006AFE8[];
extern u8 D_8006B6E0[];
extern u8 D_8006BBD0[];
extern u8 D_80069CA0[];
extern u8 D_8006A2C8[];
extern u8 D_80067090[];
extern u8 D_80067898[];
extern u8 D_800680A0[];
extern u8 D_800688A8[];
GaugePalettes gGaugePalettes = { { D_80068EB0, D_8006A8F0, D_8006B4D8, D_80069A98 } };
GaugeTextures gGaugeTextures = { { { D_800690B8, D_800695A8 }, { D_8006AAF8, D_8006AFE8 }, { D_8006B6E0, D_8006BBD0 }, { D_80069CA0, D_8006A2C8 } } };
MenuBgRows gStripTexHeights = { { 32, 32, 32, 24 } };        /* the strips' texture heights */
MenuBgRows gStripHeights = { { 26, 26, 26, 20 } };        /* the strips' heights on screen */
MenuBgTextures gMenuBgTextures = { { D_80067090, D_80067898, D_800680A0, D_800688A8 } };
DigitText gZeroDigitText = { { 0xA0, 0xC0, 0x80, 0x00, 0xFF } };

void DrawElementsPage(s32 x) {
    s32 n;
    s32 stick;
    f32 bar;

    DrawMenuSprite(0, x + 0x28, 0x1E, 0xF0, 0x9F);
    DrawMenuSprite(0x37, x + 0x2B, 0xBE, 0xF0, 3);
    DrawMenuSprite(0x37, x + 0x119, 0x24, 2, 0x9A);
    DrawMenuSprite(0x37, x + 0x27, 0x1D, 0xF2, 1);
    DrawMenuSprite(0x37, x + 0x27, 0xBD, 0xF2, 1);
    DrawMenuSprite(0x37, x + 0x27, 0x1E, 1, 0x9F);
    DrawMenuSprite(0x37, x + 0x118, 0x1E, 1, 0x9F);
    DrawMenuSprite(0x38, x + 0x29, 0x1F, 0xED, 1);
    DrawMenuSprite(0x38, x + 0x29, 0x20, 1, 0x9B);
    DrawMenuSprite(0x39, x + 0x29, 0xBB, 0xEE, 1);
    DrawMenuSprite(0x39, x + 0x116, 0x1F, 1, 0x9C);
    DrawMenuSprite(5, x + 0x45, 0x1C, 7, 7);
    DrawMenuSprite(5, x + 0xF3, 0x1C, 7, 7);

    n = (gMenuFrame >> 2) & 0xF;
    n = gArrowBobOffsets[n];

    DrawMenuSprite(6, x + n + 0x36, 0x1A, 0xE, 0xB);
    DrawMenuSprite(7, (x - n) + 0xFB, 0x1A, 0xE, 0xB);
    DrawMenuSprite(0x2D, x + 0x75, 0x19, 0x56, 0xD);

    DrawMenuSprite(0x32, x + 0xA5, 0x35, 0x35, 6);
    DrawMenuSprite(0x32, x + 0x31, 0x78, 0x35, 6);
    DrawMenuSprite(0x32, x + 0xD9, 0x78, 0x35, 6);
    DrawMenuSprite(0x32, x + 0xA5, 0xAE, 0x35, 6);

    DrawMenuSprite(0x14, x + 0x86, 0x34, 8, 0xA);
    DrawMenuSprite(0x14, x + 0x47, 0x6B, 8, 0xA);
    DrawMenuSprite(0x14, x + 0xF0, 0x6B, 8, 0xA);
    DrawMenuSprite(0x14, x + 0x86, 0xAD, 8, 0xA);

    DrawMenuSprite(0x2E, x + 0x9A, 0x3D, 0xC, 0xC);
    DrawMenuSprite(0x2F, x + 0x67, 0x6F, 0xC, 0xC);
    DrawMenuSprite(0x30, x + 0xCC, 0x6F, 0xC, 0xC);
    DrawMenuSprite(0x31, x + 0x9A, 0xA1, 0xC, 0xC);
    DrawMenuSprite(0x15, x + 0xA7, 0x92, 8, 0xA);

    DrawMenuNumber(2, x + 0x8D, 0x34, 0x32U, 0xA);
    DrawMenuNumber(2, x + 0x4E, 0x6B, 0x32U, 0xA);
    DrawMenuNumber(2, x + 0xF7, 0x6B, 0x32U, 0xA);
    DrawMenuNumber(2, x + 0x8D, 0xAD, 0x32U, 0xA);
    n = gBrianStats.spirits[0];
    DrawMenuNumber(2, x + 0x78, 0x34, n, 0xA);
    n = gBrianStats.spirits[1];
    DrawMenuNumber(2, x + 0x39, 0x6B, n, 0xA);
    n = gBrianStats.spirits[3];
    DrawMenuNumber(2, x + 0xE2, 0x6B, n, 0xA);
    n = gBrianStats.spirits[2];
    DrawMenuNumber(2, x + 0x78, 0xAD, n, 0xA);

    bar = gBrianStats.spirits[0];
    bar /= 50.0f;
    bar *= 50.0f;
    DrawMenuSprite(0x33, x + 0xA6, 0x36, bar, 3);
    bar = gBrianStats.spirits[1];
    bar /= 50.0f;
    bar *= 50.0f;
    DrawMenuSprite(0x33, x + 0x32, 0x79, bar, 3);
    bar = gBrianStats.spirits[3];
    bar /= 50.0f;
    bar *= 50.0f;
    DrawMenuSprite(0x33, x + 0xDA, 0x79, bar, 3);
    bar = gBrianStats.spirits[2];
    bar /= 50.0f;
    bar *= 50.0f;
    DrawMenuSprite(0x33, x + 0xA6, 0xAF, bar, 3);

    n = (D_8007A9E0 * 100) / gSpiritExpTable[D_8007AA04];
    if (n > 100) {
        n = 100;
    }
    DrawMenuNumber(3, x + 0x91, 0x92, n, 0xA);
    DrawElementGauge(n, x);
    if (!(gMenuState & 0x2000)) {
        stick = gStickX;
        if ((stick > 20) || (gButtonsHeld & R_TRIG)) {
            gMenuState |= 0x2000;
            gPageTurnFrames = 0x1F;
            PlaySound(0, 0, 0xFF);
        } else if ((stick < -20) || (gButtonsHeld & Z_TRIG)) {
            gMenuState |= 0x3000;
            gPageTurnFrames = 0x1F;
            PlaySound(0, 0, 0xFF);
        }
    }
}
void DrawMenuSprite(s32 sprite, s32 x, s32 y, s32 w, s32 h) {
    s32 pal;
    s32 cms;
    s32 cmt;
    s32 masks;
    s32 maskt;
    s32 shifts;
    s32 shiftt;
    s32 s;
    s32 t;
    s32 dsdx;
    s32 dtdy;
    s32 uls;
    s32 lrx;
    s32 lrs;
    s32 lrt;
    u8* image;
    s32 width;
    s32 alpha;
    s32 skip;

    alpha = 0;
    skip = 0;
    image = gMenuDigitTextures[sprite];
    lrs = gMenuSpriteSizes[sprite].w - 1;
    lrt = gMenuSpriteSizes[sprite].h - 1;
    width = gMenuSpriteSizes[sprite].w;
    maskt = 0;
    masks = 0;
    t = 0;
    s = 0;
    dsdx = dtdy = 0x400;
    pal = 0;
    cms = 0;
    cmt = 0;
    shifts = 0;
    shiftt = 0;
    uls = 0;
    switch (sprite) {
        case 0:
            maskt = 4;
            masks = 4;
            s = (x & 0xF) << 5;
            t = (y & 0xF) << 5;
            break;
        case 0x3C:
            alpha = -0x80;
            dtdy = 0;
            dsdx = 0;
            break;
        case 0x3E:
            alpha = 0x50;
            break;
        case 0x37:
        case 0x38:
        case 0x39:
            dtdy = 0;
            dsdx = 0;
            break;
        case 0x3D:
            dsdx = 0;
            break;
    }
    if (x < 0) {
        if (x + w < 0) {
            skip = -1;
        } else {
            lrx = -x; // RULE-EXCEPTION: R30, one local for the left overhang and the right edge: a second one makes the frame a word too big
            x = 0;
            w -= lrx;
            s += lrx << 5;
        }
    }
    if (x > 320) {
        skip = -1;
    } else {
        lrx = x + w;
        if (lrx > 320) {
            w = w - lrx + 320;
        }
    }
    if (w <= 0 || h <= 0) {
        skip = -1;
    }
    if (skip >= 0) {
        lrx = x + w;
        gDPPipeSync(gDisplayListHead++);
        gDPSetRenderMode(gDisplayListHead++, G_RM_XLU_SURF, G_RM_XLU_SURF2);
        gDPSetTextureFilter(gDisplayListHead++, G_TF_BILERP);
        gDPPipeSync(gDisplayListHead++);
        if (alpha == 0) {
            gSPDisplayList(gDisplayListHead++, gMenuSpriteSetup);
        } else {
            gSPDisplayList(gDisplayListHead++, gMenuSpriteSetupEnvAlpha);
            gDPPipeSync(gDisplayListHead++);
            gDPSetEnvColor(gDisplayListHead++, 0xFF, 0xFF, 0xFF, alpha);
        }
        if (!(gMenuState & 0x10)) {
            gDPPipeSync(gDisplayListHead++);
            gDPLoadTLUT_pal256(gDisplayListHead++, gMenuSheetPalette);
            gMenuState |= 0x10;
        }
        gDPLoadTextureTile(gDisplayListHead++, image, G_IM_FMT_CI, G_IM_SIZ_8b, width, 0, uls, 0, lrs, lrt, pal,
                           cms, cmt, masks, maskt, shifts, shiftt);
        gSPTextureRectangle(gDisplayListHead++, x << 2, y << 2, lrx << 2, (y + h) << 2, G_TX_RENDERTILE, s, t, dsdx,
                            dtdy);
        gDPPipeSync(gDisplayListHead++);
    }
}

void DrawElementGauge(s32 percent, s32 x) {
    s32 layer;
    s32 half;
    s32 clip;       /* width clipped off the left, then negative when this half is off screen */
    GaugePalettes pals;
    GaugeTextures tex;
    s32 top;
    s32 y;
    s32 w;
    s32 h = 50;     /* never read: every case below sets h */
    s32 tw;
    s32 th;
    s32 xx;
    s32 right;
    s32 s;
    s32 t;
    s32 x0;

    x0 = x;
    pals = gGaugePalettes;
    tex = gGaugeTextures;
    for (layer = 0; layer != 4; layer++) {
        gDPPipeSync(gDisplayListHead++);
        gDPSetRenderMode(gDisplayListHead++, G_RM_XLU_SURF, G_RM_XLU_SURF2);
        gDPSetTextureFilter(gDisplayListHead++, G_TF_BILERP);
        gDPPipeSync(gDisplayListHead++);
        if (layer != 1) {
            gSPDisplayList(gDisplayListHead++, gMenuSpriteSetup);
        } else {
            gSPDisplayList(gDisplayListHead++, gMenuSpriteSetupEnvAlpha);
            gDPSetEnvColor(gDisplayListHead++, 0xFF, 0xFF, 0xFF, 0x80);
        }
        gDPPipeSync(gDisplayListHead++);
        gDPLoadTLUT_pal256(gDisplayListHead++, pals.p[layer]);
        for (half = 0; half != 2; half++) {
            s = 0;
            switch (layer) {
                case 2:
                    xx = x0 + 0x87;
                    top = 50 - percent / 2;
                    y = top + 0x52;
                    w = 25;
                    h = 50;
                    tw = 25;
                    th = 50;
                    t = top << 5;
                    break;
                case 3:
                    top = 0;
                    t = 0;
                    y = 0x4F;
                    w = 28;
                    h = 56;
                    tw = 28;
                    th = 56;
                    xx = x0 + 0x84;
                    break;
                default:
                    top = 0;
                    t = 0;
                    y = 0x52;
                    w = 25;
                    h = 50;
                    tw = 25;
                    th = 50;
                    xx = x0 + 0x87;
                    break;
            }
            xx += w * half;
            if (xx < 0 && xx + w >= 0) {
                clip = -xx; // RULE-EXCEPTION: R30, one local for both uses: a second one makes the frame a word too big
                xx = 0;
                w -= clip;
                s = clip << 5;
            }
            if (xx > 320) {
                clip = -1;
            } else {
                clip = 0;
                right = xx + w;
                if (right > 320) {
                    w = w - right + 320;
                }
            }
            if (w <= 0) {
                clip = -1;
            }
            if (clip >= 0) {
                gDPLoadTextureTile(gDisplayListHead++, tex.p[layer][half], G_IM_FMT_CI, G_IM_SIZ_8b, tw, th, 0, top, tw - 1,
                                   th - 1, 0, G_TX_WRAP, G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
                gSPTextureRectangle(gDisplayListHead++, xx << 2, y << 2, (xx + w) << 2, (y + h - top) << 2,
                                    G_TX_RENDERTILE, s, t, 1 << 10, 1 << 10);
                gDPPipeSync(gDisplayListHead++);
            }
        }
    }
    gMenuState &= ~0x10;
}

void DrawMenuBackground(void) {
    s32 i;
    s32 y;
    MenuBgRows rows;
    MenuBgRows spans;
    MenuBgTextures tex;

    rows = gStripTexHeights;
    spans = gStripHeights;
    tex = gMenuBgTextures;
    gDPPipeSync(gDisplayListHead++);
    gDPLoadTLUT_pal256(gDisplayListHead++, gBackgroundPalette);
    gMenuState &= ~0x10;
    y = 22;
    for (i = 0; i < 4; i++) {
        gDPLoadTextureTile(gDisplayListHead++, tex.p[i], G_IM_FMT_CI, G_IM_SIZ_8b, 64, rows.v[i], 0, 0, 63, rows.v[i] - 1,
                           0, G_TX_MIRROR, G_TX_WRAP, 6, 5, G_TX_NOLOD, G_TX_NOLOD);
        gSPTextureRectangle(gDisplayListHead++, 23 << 2, y << 2, 297 << 2, (y + spans.v[i]) << 2, G_TX_RENDERTILE, 0, 0,
                            0x4A8, 0x4E6);
        gDPPipeSync(gDisplayListHead++);
        y += spans.v[i];
    }
    y = 120;
    for (i = 3; i >= 0; i--) {
        gDPLoadTextureTile(gDisplayListHead++, tex.p[i], G_IM_FMT_CI, G_IM_SIZ_8b, 64, rows.v[i], 0, 0, 63, rows.v[i] - 1,
                           0, G_TX_MIRROR, G_TX_WRAP, 6, 5, G_TX_NOLOD, G_TX_NOLOD);
        gSPTextureRectangle(gDisplayListHead++, 23 << 2, y << 2, 297 << 2, (y + spans.v[i]) << 2, G_TX_RENDERTILE, 0,
                            (rows.v[i] - 1) << 5, 0x4A8, -0x4E6);
        gDPPipeSync(gDisplayListHead++);
        y += spans.v[i];
    }
}
void DrawMenuNumber(s32 digits, s32 x, s32 y, u32 value, s32 font) {
    s32 j;
    u64 div;
    s64 v;
    s8 buf[10];                         /* the decimal digits, units first */
    s32 n;                              /* the digit being counted, then the index of the digit being drawn */
    s32 i;                              /* the digit loop's counter, then the sum of the digits drawn so far */
    s32 xoff;

    div = 1000000000;
    v = value;
    xoff = 0;
    j = 9;
    if (digits >= 10) {
        digits = 9;
    }
    /* RULE-EXCEPTION R30: i is reused as the running digit sum and n as the drawing index; a separate sum (118
       declaration orders) or a separate index (three positions) gives other registers */
    for (i = 0; i < 9; i++) {
        n = 0;
        for (;;) {
            v -= div;
            if (v < 0) {
                v += div;
                break;
            }
            n++;
        }
        buf[j] = n;
        j--;
        div = __ull_div(div, 10);
    }
    buf[0] = v;
    i = 0;
    if (!(gMenuState & 0x10)) {
        gDPPipeSync(gDisplayListHead++);
        gDPLoadTLUT_pal256(gDisplayListHead++, gMenuSheetPalette);
        gMenuState |= 0x10;
    }
    for (n = digits - 1; n >= 0; n--) {
        i += buf[n];
        if (i != 0 || n == 0) {
            gDPLoadTextureTile(gDisplayListHead++, gMenuDigitTextures[buf[n] + font], G_IM_FMT_CI, G_IM_SIZ_8b, 8, 10, 0, 0, 7, 9,
                               0, G_TX_WRAP, G_TX_WRAP, 3, 4, G_TX_NOLOD, G_TX_NOLOD);
            gSPTextureRectangle(gDisplayListHead++, (x + xoff) << 2, y << 2, (x + xoff + 8) << 2, (y + 10) << 2,
                                G_TX_RENDERTILE, 0, 0, 1 << 10, 1 << 10);
            gDPPipeSync(gDisplayListHead++);
        }
        xoff += 7;
    }
}
void DrawMenuNumberZeros(s32 digits, s32 x, s32 y, u32 value, s32 font) {
    s32 i;
    s32 j;
    u64 div;
    s64 v;
    s8 buf[10];                         /* the decimal digits, units first */
    s32 n;                              /* the digit being counted, then the index of the digit being drawn */
    s32 xoff;

    div = 1000000000;
    v = value;
    xoff = 0;
    j = 9;
    if (digits >= 10) {
        digits = 9;
    }
    for (i = 0; i < 9; i++) {
        n = 0;
        for (;;) {
            v -= div;
            if (v < 0) {
                v += div;
                break;
            }
            n++;
        }
        buf[j] = n;
        j--;
        div = __ull_div(div, 10);
    }
    buf[0] = v;
    if (!(gMenuState & 0x10)) {
        gDPPipeSync(gDisplayListHead++);
        gDPLoadTLUT_pal256(gDisplayListHead++, gMenuSheetPalette);
        gMenuState |= 0x10;
    }
    /* RULE-EXCEPTION R30: n is reused as the index; a separate index local (tried at every declaration position:
       above buf it grows the frame, below it the registers differ) or i / j does not match */
    for (n = digits - 1; n >= 0; n--) {
        gDPLoadTextureTile(gDisplayListHead++, gMenuDigitTextures[buf[n] + font], G_IM_FMT_CI, G_IM_SIZ_8b, 8, 10, 0, 0, 7, 9, 0,
                           G_TX_WRAP, G_TX_WRAP, 3, 4, G_TX_NOLOD, G_TX_NOLOD);
        gSPTextureRectangle(gDisplayListHead++, (x + xoff) << 2, y << 2, (x + xoff + 8) << 2, (y + 10) << 2,
                            G_TX_RENDERTILE, 0, 0, 1 << 10, 1 << 10);
        gDPPipeSync(gDisplayListHead++);
        xoff += 7;
    }
}
void DrawMenuDigits(s32 digits, s32 x, s32 y, u32 value) {
    s32 j;
    u64 div;
    s64 v;
    s8 buf[10];                         /* the decimal digits, units first */
    s32 n;                              /* the digit being counted, then the index of the digit being drawn */
    DigitText text;
    s32 i;                              /* the digit loop's counter, then the sum of the digits drawn so far */

    div = 1000000000;
    v = value;
    text = gZeroDigitText;
    j = 9;
    if (digits >= 10) {
        digits = 9;
    }
    /* RULE-EXCEPTION R30: i is reused as the running digit sum and n as the drawing index (as in DrawMenuNumber);
       separate locals or j as the index give other registers */
    for (i = 0; i < 9; i++) {
        n = 0;
        for (;;) {
            v -= div;
            if (v < 0) {
                v += div;
                break;
            }
            n++;
        }
        buf[j] = n;
        j--;
        div = __ull_div(div, 10);
    }
    buf[0] = v;
    i = 0;
    for (n = digits - 1; n >= 0; n--) {
        i += buf[n];
        if (i != 0 || n == 0) {
            text.c[3] = buf[n];
            DrawString(x, y, (u8*)&text);
        }
        x += 10;
    }
}
extern s32 gCurrentMap;
extern s32 gCurrentSubmap;
void MapCursorToWorld(void) {
    f32 dx;
    f32 dz;
    s32 i = 0;
    s32 image = 0;
    s32 marker = -1;

    while (gMapLocationTable[i][0] != -8) {
        switch (gMapLocationTable[i][0]) {
            case -2:
                i++;
                marker++;
                break;
            case -1:
                i++;
                image++;
                marker = -1;
                break;
        }
        if (gCurrentMap == gMapLocationTable[i][0] && gCurrentSubmap == gMapLocationTable[i][1]) {
            break;
        }
        i++;
    }
    gMapPageImageId = image;
    gMapPageMarker = marker;
    if (gMapPageImageId < 6 && gMapPageMarker < 0) {
        if (gMapCursorX == -1.0f) {
            gMapCursorX = 115.0f;
            gMapCursorY = 86.0f;
        }
        dx = gMapCursorX;
        dz = gMapCursorY;
        dx -= gMapPageOrigin[0][gMapPageImageId];
        dz -= gMapPageOrigin[1][gMapPageImageId];
        gCompassTargetX = gMapPageScale[0][gMapPageImageId] * dx;
        gCompassTargetZ = gMapPageScale[1][gMapPageImageId] * dz;
    }
}
