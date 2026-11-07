#include "common.h"
extern OSMesgQueue gPiMesgQueue;     /* the PI manager's command queue */
extern OSMesg   gPiMesgBuf[50];      /* its messages */
extern void		osCreatePiManager(OSPri, OSMesgQueue *, OSMesg *, s32);
extern OSThread gMainThread;         /* the main thread (mainproc) */
void mainproc(s32 arg);

extern OSThread gIdleThread;         /* the idle thread (also the boot stack's top) */
extern u64 gIdleStack[];             /* its stack */
extern u64 gMainStack[];             /* the main thread's stack */
void idle(void* arg);
void boot(void* arg) {
    osInitialize();
    osCreateThread(&gIdleThread, 1, idle, NULL, gIdleStack + 0x1000 / sizeof(u64), 1);
    osStartThread(&gIdleThread);
}
void idle(void *arg) {
    osCreatePiManager(150, &gPiMesgQueue, gPiMesgBuf, 50);
    osCreateThread(&gMainThread, 6, (void*)mainproc, arg, gMainStack + 0x3000 / sizeof(u64), 5);
    osStartThread(&gMainThread);
    do {} while (TRUE);
}
