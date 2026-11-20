#ifndef GAME_BOOT_H
#define GAME_BOOT_H

/* The functions of src/game/boot.c, declared once for every file that calls them. */

#include "common.h"

void boot(void* arg);
void idle(void *arg);

/* The state src/game/boot.c defines (.bss), in its storage order: the order of these declarations is the
 * unit's .bss layout (include/bss_order.h). */
extern u64 gBootStack[0x2000 / sizeof(u64)];   /* the stack boot runs on */
extern OSThread gIdleThread;   /* the idle thread */
extern OSThread gMainThread;   /* the main thread (mainproc) */
extern u64 gIdleStack[0x1000 / sizeof(u64)];   /* the idle thread's stack */
extern u64 gMainStack[0x3000 / sizeof(u64)];   /* the main thread's stack */
extern OSMesgQueue gPiMesgQueue;   /* the PI manager's command queue */
extern OSMesg gPiMesgBuf[50];   /* its messages */

#endif
