#ifndef GAME_SCREEN_FADE_H
#define GAME_SCREEN_FADE_H

/* The functions of src/game/screen_fade.c, declared once for every file that calls them. */

#include "common.h"

void BuildColourRamp(void);   /* build the RGBA ramp (screen_fade.c) */
void DrawScreenFade(mkst* arg0);
void StartScreenFade(s32 frames, u16 flags, void* ramp);

/* gScreenFadeFlags, and the flags StartScreenFade takes (it adds FADE_ON and keeps FADE_SETUP). Quest 64's screen_dim.h
   calls the first three SCREEN_DIMMABLE, USE_TIMER and SCREEN_BRIGHTENING. */
#define FADE_ON        0x1  /* the fade is drawn (DrawScreenFade); it stops when its frames run out, unless FADE_HOLD */
#define FADE_HOLD      0x2  /* the fade stays at its last step when its frames run out: FadeScreenOut leaves the screen
                               dark or white until FadeScreenIn */
#define FADE_BRIGHTEN  0x4  /* the fade steps back through its colour ramp: the screen clears (FadeScreenIn) */
#define FADE_SETUP     0x8  /* DrawScreenFade adds the fade's render setup (the mode loops set it) */

typedef struct {
    u8 r, g, b, a;
} FadeRGBA;                         /* one fade colour */

/* The state src/game/screen_fade.c defines (.bss), in its storage order: the order of these declarations is the
 * unit's .bss layout (include/bss_order.h). */
extern u16 gScreenFadeFlags;   /* screen fade flags (screen_fade.c) */
extern u8 gFadeRamp[256][4];   /* RGBA: grey 0xDC, alpha ramping 0 -> 0xFE */
extern s32 gScreenFadeFrames;
extern FadeRGBA* gFadeRampEntry;   /* the fade's current colour ramp entry */

#endif
