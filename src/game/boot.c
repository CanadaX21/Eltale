#include "common.h"
#include "game/boot.h"
#include "game/main.h"

/* The boot code's state, in this order in .bss. The boot entry (asm/entry.s) clears main's .bss from gBootStack
 * on and starts the stack pointer at its top; boot starts the idle thread, and idle the main thread, each on a
 * stack of its own. */
u64 gBootStack[0x2000 / sizeof(u64)];  /* the stack boot runs on */
OSThread gIdleThread;                  /* the idle thread */
OSThread gMainThread;                  /* the main thread (mainproc) */
u64 gIdleStack[0x1000 / sizeof(u64)];  /* the idle thread's stack */
u64 gMainStack[0x3000 / sizeof(u64)];  /* the main thread's stack */
OSMesgQueue gPiMesgQueue;              /* the PI manager's command queue */
OSMesg gPiMesgBuf[50];                 /* its messages */

void boot(void* arg) {
    osInitialize();
    osCreateThread(&gIdleThread, 1, idle, NULL, STACK_TOP(gIdleStack), 1);
    osStartThread(&gIdleThread);
}
void idle(void *arg) {
    osCreatePiManager(150, &gPiMesgQueue, gPiMesgBuf, ARRAY_COUNT(gPiMesgBuf));
    osCreateThread(&gMainThread, 6, (void*)mainproc, arg, STACK_TOP(gMainStack), 5);
    osStartThread(&gMainThread);
    do {} while (TRUE);
}
