#include "common.h"
#include "game/title.h"
#include "game/audio.h"

/*
 * The title screen (game mode 3): its logic. The overlay was built from three source files, this one (the entry, the
 * per-frame function, the state machine, the menu's input and the frames after a choice), title_menu.c (the drawing) and
 * title_fade.c (the fade): each of the overlay's sections is padded to 16 bytes at the same two points. LoadOvl_TitleScreen
 * copies the overlay to 0x8020C0D0, the slot the intro and the ending overlays also use; TitleLoop then calls the
 * slot's entry, TitleScreen_Init, and BuildFrame calls TitleScreen_Update once a frame. Its art (palettes, textures,
 * the menu's encoded text) lies after the three files' .data as one binary piece; the names into it are defined from
 * that piece's start (undefined_syms.jp.txt).
 *
 * Each entry starts afresh because the loader copies the whole image again: the copy restores every .data variable,
 * including TitleScreen_Step's and TitleScreen_DrawMenu's first-call flags, so both first-call blocks run again. The
 * entry alone does not reset the screen: it sets state 0, and only Step's first call moves it to state 3. A build
 * that keeps this overlay resident, or a port that initializes it once, must restore its .data on every entry;
 * otherwise Step stays in state 0 and the screen stays undrawn, ignoring the buttons.
 */

extern Gfx* gDisplayListHead;
extern u16 gButtonsPressed;
extern s8 gStickX;
extern s8 gStickY;
extern u8 D_8007A1CC;  /* main's selection byte: the chosen option in its high nibble */
extern s8 D_8006CC80;  /* main's: set as the splash and intro end; the screen clears it on its first frame */


/* title_menu.c: the drawing */
extern Gfx gTitleSetupDL[];
extern s8 gTitleOptionCursor;
extern s32 gTitleBlinkAlpha;

/* title_fade.c: the fade */
extern s32 gTitleFadeKind;
extern s32 gTitleFadeFrames;

/* ---- .data ---- */
/* 0 nothing under way (the step holds the timer); 1 a choice, B or the push-start screen's timeout under way (the
   timer runs down and the step acts at 0); 2 the fade that ends the screen after B or the menu's idle limit */
s32 gTitleTransition = 0;

/* ---- .bss (all three files'). The loader does not clear it: it keeps what the slot held before (after the intro, the intro's own
   bytes). Each variable is set before its first read on every entry: by TitleScreen_Init, by Step's or DrawMenu's
   first call, or each frame before it is read. The stick's previous direction is read first, but a stale value can
   only hide a press already held, in a state that ignores it. New state needs the same: an initializer (it then
   lives in .data, restored by every copy), a value set in TitleScreen_Init, or a first-call block. State kept from
   one entry to the next lives in main, as the menu's last choice does (the selection byte). ---- */
s32 gTitleState;
s32 gTitleTimer;
s32 D_80226D98_DC62B8;
s32 gTitlePulseAlpha;
s32 gTitlePulseStep;
s32 gTitleMenuState;

/* The overlay's entry (the slot's first function: main calls it after loading any mode overlay). */
void TitleScreen_Init(void) {
    gTitleState = 0;
    gTitleMenuState = 0;
    gTitleFadeFrames = 1;
    gTitleFadeKind = 0;
    gTitleTimer = 1;
    gTitlePulseAlpha = 0;
    gTitlePulseStep = 8;
    D_80226D98_DC62B8 = 0;
}

/* One frame of the title screen, called by BuildFrame: 0 once an option is chosen (main reads the selection byte),
   1 when the screen gives up and returns to the splash and intro, 2 to stay. */
s32 TitleScreen_Update(void) {
    if (TitleScreen_Step() == 0) {
        if (gTitleMenuState == 0) {
            return 1;
        }
        if (gTitleMenuState == 15) {
            gTitleMenuState = 0;
            return 0;
        }
    }
    return 2;
}

/* The screen's state machine, one step a frame. It returns the timer: 0 ends the screen (TitleScreen_Update then reads
   the menu state: 0 back to the splash and the intro, 15 an option chosen). Its first call on each entry sets state 3
   (the copy of the overlay re-arms sStarted). The states:
   3  one frame, then the push-start screen with a 5,400-frame wait and a 20-frame fade from black;
   4  the push-start screen: left for the whole wait, it fades to black and the screen ends; Start, then 60 frames,
      opens the menu;
   5  the menu: B, or 7,200 frames without a button or the stick, fades to black and ends the screen; a choice waits
      60 frames, then moves on;
   6  two frames, then 8 with a 60-frame fade to black, whichever option was chosen;
   7  the two-option prompts (menu states 7 and 8) and their messages (9 and 10): never entered, as 6 moves every
      choice to 8 and no code sets menu states 6 to 8;
   8  the screen drawn opaque until the timer runs out (TitleScreen_FinishSelection), the fade running. */
s32 TitleScreen_Step(void) {
    static u8 sChoiceDelay = 0;   /* 0x8020F1E4: state 6's frames */
    static u32 sIdleFrames = 0;   /* 0x8020F1E8: the menu's frames without a button or the stick */
    static u16 sHadInput = 0;     /* 0x8020F1EC: a button or the stick last frame: the idle count starts again */
    static u8 sStarted = 0;       /* 0x8020F1F0: set by the first call on each entry */

    if (sStarted == 0) {
        gTitleState = 3;
        gTitleTimer = 1;
        gTitleMenuState = 0;
        D_8006CC80 = 0;
        sStarted++;
    }
    switch (gTitleState) {
        case 3:
            if (--gTitleTimer == 0) {
                gTitleTimer = 5400;
                gTitleState++;
                gTitleFadeFrames = 20;
                gTitleFadeKind = 2;
            }
            break;
        case 4:
            TitleScreen_UpdateMenu();
            if (gTitleFadeKind == 2) {
                TitleScreen_UpdateFade();
            }
            if (gTitleTimer == 0 && gTitleMenuState == 0 && gTitleTransition == 0) {
                gTitleTransition = 1;
                gTitleFadeFrames = 60;
                gTitleFadeKind = 1;
                gTitleTimer = 60;
            }
            if (gTitleTransition == 1) {
                TitleScreen_UpdateFade();
                if (gTitleFadeKind == 0) {
                    gTitlePulseAlpha = 0;
                    gTitlePulseStep = 8;
                    gTitleTimer = 0;
                    gTitleState = 0;
                    gTitleTransition = 0;
                    gTitleMenuState = 0;
                }
            }
            if (gTitleTimer == 0 && gTitleMenuState == 1) {
                gTitleMenuState = 2;
                gTitleTimer = 2;
                gTitleState++;
            }
            break;
        case 5:
            TitleScreen_UpdateMenu();
            if (sHadInput == 0) {
                sIdleFrames++;
            } else {
                sIdleFrames = 0;
            }
            if (!(gButtonsPressed & (A_BUTTON | B_BUTTON | Z_TRIG | START_BUTTON)) && gStickX >= -19 && gStickX < 20 &&
                gStickY >= -19 && gStickY < 20) {
                sHadInput = 0;
            } else {
                sHadInput = 1;
            }
            if (sIdleFrames > 7200) {
                sHadInput = 0;
                gTitleMenuState = 4;
                gTitleTransition = 1;
                gTitleTimer = 2;
                sIdleFrames = 0;
            }
            if (gTitleTransition == 0) {
                gTitleTimer = 20;
            }
            if (gTitleTimer == 0 && (gTitleMenuState == 3 || gTitleMenuState == 4)) {
                if (gTitleMenuState == 4) {
                    gTitleTransition = 2;
                    gTitleFadeFrames = 60;
                    gTitleFadeKind = 1;
                }
                gTitleTimer = 60;
                if (gTitleMenuState == 3) {
                    gTitleMenuState = 5;
                    gTitleState++;
                    gTitlePulseAlpha = 255;
                    gTitlePulseStep = 0;
                    gTitleTransition = 0;
                }
            }
            if (gTitleTransition == 2) {
                TitleScreen_UpdateFade();
                if (gTitleFadeKind == 0) {
                    gTitlePulseAlpha = 0;
                    gTitlePulseStep = 8;
                    gTitleTimer = 0;
                    D_80226D98_DC62B8 = 0;
                    gTitleState = 3;
                    gTitleMenuState = 0;
                    gTitleTransition = 0;
                }
            }
            break;
        case 6:
            TitleScreen_UpdateMenu();
            if (sChoiceDelay < 2) {
                sChoiceDelay++;
            } else if ((D_8007A1CC & 0xF0) == 0x10) {
                /* NOTE: the original tests the chosen option, then does the same for both: every choice goes to
                   state 8, so state 7 is never entered */
                gTitleState = 8;
                gTitleFadeFrames = 60;
                gTitleFadeKind = 1;
            } else {
                gTitleState = 8;
                gTitleFadeFrames = 60;
                gTitleFadeKind = 1;
            }
            break;
        case 7:
            TitleScreen_UpdateMenu();
            if (gTitleTransition == 0) {
                gTitleTimer = 20;
            }
            if (gTitleMenuState == 2) {
                gTitleState = 5;
            }
            if (gTitleTimer == 0 && (gTitleMenuState == 11 || gTitleMenuState == 12 || gTitleMenuState == 13 ||
                                     gTitleMenuState == 14 || gTitleMenuState == 9 || gTitleMenuState == 10)) {
                if (gTitleMenuState == 9) {
                    gTitleTimer = 10;
                    gTitleState = 5;
                    gTitleTransition = 0;
                    gTitleMenuState = 2;
                }
                if (gTitleMenuState == 10) {
                    gTitleTimer = 10;
                    gTitleState = 5;
                    gTitleTransition = 0;
                    gTitleMenuState = 2;
                }
                if (gTitleMenuState == 12) {
                    gTitleTimer = 5;
                    gTitleMenuState = 9;
                    gTitleTransition = 0;
                }
                if (gTitleMenuState == 11) {
                    gTitleTimer = 60;
                    gTitleState++;
                    gTitleMenuState = 15;
                    gTitlePulseAlpha = 0;
                    gTitlePulseStep = 256;
                    gTitleFadeFrames = 60;
                    gTitleFadeKind = 1;
                    gTitleTransition = 0;
                }
                if (gTitleMenuState == 13) {
                    gTitleTimer = 5;
                    gTitleMenuState = 10;
                    gTitleTransition = 0;
                }
                if (gTitleMenuState == 14) {
                    gTitleTimer = 10;
                    gTitleState = 5;
                    gTitleMenuState = 2;
                    gTitleTransition = 0;
                }
            }
            break;
        case 8:
            TitleScreen_FinishSelection();
            TitleScreen_UpdateFade();
            break;
    }
    if (gTitleTimer == 0) {
        sHadInput = 0;
        sIdleFrames = 0;
    }
    return gTitleTimer;
}

/* One frame of the menu: draws the screen through TitleScreen_DrawMenu with the text's alpha pulsing between 0 and
   255, then, while no fade runs, answers the buttons of the current menu state with a sound, the next menu state and
   the timer the step waits on, and sets gTitleTransition where the step must act once the timer runs out. The timer
   runs down every frame. */
void TitleScreen_UpdateMenu(void) {
    gSPDisplayList(gDisplayListHead++, gTitleSetupDL);
    gDPSetEnvColor(gDisplayListHead++, 0, 0, 0, gTitlePulseAlpha);
    TitleScreen_DrawMenu(gTitleMenuState);
    gTitlePulseAlpha += gTitlePulseStep;
    if (gTitlePulseAlpha >= 256) {
        gTitlePulseAlpha = 255;
        gTitlePulseStep = -gTitlePulseStep;
    } else if (gTitlePulseAlpha < 0) {
        gTitlePulseAlpha = 0;
        gTitlePulseStep = -gTitlePulseStep;
    }
    gTitleBlinkAlpha = gTitlePulseAlpha;
    if (gTitleFadeKind == 0) {
        if (gTitleMenuState == 0 && (gButtonsPressed & START_BUTTON)) {
            PushSoundQueue(1, 255, 64);
            gTitlePulseAlpha = 0;
            gTitlePulseStep = 128;
            gTitleMenuState = 1;
            gTitleTimer = 60;
        } else if (gTitleMenuState == 2) {
            if (gButtonsPressed & (A_BUTTON | Z_TRIG | START_BUTTON)) {
                PushSoundQueue(1, 255, 64);
                gTitlePulseAlpha = 0;
                gTitlePulseStep = 128;
                gTitleMenuState = 3;
                gTitleTimer = 60;
                gTitleTransition = 1;
            } else if (gButtonsPressed & B_BUTTON) {
                PushSoundQueue(2, 255, 64);
                gTitlePulseAlpha = 255;
                gTitlePulseStep = 128;
                gTitleMenuState = 4;
                gTitleTimer = 2;
                gTitleTransition = 1;
            }
        } else if (gTitleMenuState == 8 || gTitleMenuState == 7) {
            if (gButtonsPressed & (A_BUTTON | Z_TRIG | START_BUTTON)) {
                PushSoundQueue(1, 255, 64);
                gTitlePulseAlpha = 0;
                gTitlePulseStep = 128;
                gTitleTransition = 1;
                gTitleTimer = 60;
                if (gTitleOptionCursor == 0) {
                    if (gTitleMenuState == 8) {
                        gTitleMenuState = 11;
                    } else {
                        gTitleMenuState = 13;
                    }
                } else if (gTitleMenuState == 8) {
                    gTitleMenuState = 12;
                } else {
                    gTitleMenuState = 14;
                }
            } else if (gButtonsPressed & B_BUTTON) {
                PushSoundQueue(2, 255, 64);
                gTitlePulseAlpha = 255;
                gTitlePulseStep = 128;
                gTitleMenuState = 2;
                gTitleTimer = 2;
                gTitleTransition = 1;
            }
        } else if (gTitleMenuState == 6) {
            if (gButtonsPressed & (A_BUTTON | B_BUTTON | Z_TRIG | START_BUTTON)) {
                PushSoundQueue(2, 255, 64);
                gTitlePulseAlpha = 255;
                gTitlePulseStep = 128;
                gTitleMenuState = 2;
                gTitleTimer = 2;
            }
        } else if (gTitleMenuState == 9) {
            if (gButtonsPressed & (A_BUTTON | Z_TRIG | START_BUTTON)) {
                PushSoundQueue(1, 255, 64);
                gTitleTransition = 1;
            }
        } else if (gTitleMenuState == 10 && (gButtonsPressed & (A_BUTTON | Z_TRIG | START_BUTTON))) {
            PushSoundQueue(1, 255, 64);
            gTitleTransition = 1;
        }
    }
    gTitleTimer--;
}

/* The last frames after a choice: the screen drawn fully opaque until the timer runs out, then the menu state that
   makes TitleScreen_Update report the choice. */
void TitleScreen_FinishSelection(void) {
    gSPDisplayList(gDisplayListHead++, gTitleSetupDL);
    gDPSetEnvColor(gDisplayListHead++, 0, 0, 0, 255);
    TitleScreen_DrawMenu(gTitleMenuState);
    if (--gTitleTimer == 0) {
        gTitleMenuState = 15;
    }
}
