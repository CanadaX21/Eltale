#include "common.h"
/* split from startthread.c: the original was separate files (each IDO object pads its .rodata to 16;
   the zero words before this file's first literal mark the boundary). Declarations copied from startthread.c. */
extern s32 __osThreadTail;
extern OSThread *__osRunQueue;
extern void __osEnqueueThread(OSThread **, OSThread *);
extern OSThread *__osPopThread(OSThread **);
extern OSThread *__osRunningThread;
extern void __osDispatchThread(void);
extern void __osEnqueueAndYield(OSThread **);
extern OSDevMgr __osPiDevMgr;          /* __osPiDevMgr */
extern s32 __osPiAccessQueueEnabled;               /* __osPiAccessQueueEnabled */
extern OSThread piThread;          /* piThread */
extern char piThreadStack[];       /* piThreadStack */
extern OSMesgQueue piEventQueue;       /* piEventQueue */
extern OSMesg piEventBuf[];          /* piEventBuf */
extern OSMesgQueue __osPiAccessQueue;       /* __osPiAccessQueue */
void InitPiHandles(void);            /* copies the PI domain 1 / 2 timings into the cart and disk handles */
void __osPiCreateAccessQueue(void);            /* __osPiCreateAccessQueue */
void __osDevMgrMain(void* arg);       /* __osDevMgrMain */
void osCreatePiManager(OSPri pri, OSMesgQueue* cmdQ, OSMesg* cmdBuf, s32 cmdMsgCount) {
    u32 savedMask;
    OSPri oldPri;
    OSPri myPri;

    if (__osPiDevMgr.active) {
        return;
    }
    InitPiHandles();
    osCreateMesgQueue(cmdQ, cmdBuf, cmdMsgCount);
    osCreateMesgQueue(&piEventQueue, piEventBuf, 1);
    if (!__osPiAccessQueueEnabled) {
        __osPiCreateAccessQueue();
    }
    osSetEventMesg(OS_EVENT_PI, &piEventQueue, (OSMesg)0x22222222);
    oldPri = -1;
    myPri = osGetThreadPri(NULL);
    if (myPri < pri) {
        oldPri = myPri;
        osSetThreadPri(NULL, pri);
    }
    savedMask = __osDisableInt();
    __osPiDevMgr.active = 1;
    __osPiDevMgr.thread = &piThread;
    __osPiDevMgr.cmdQueue = cmdQ;
    __osPiDevMgr.evtQueue = &piEventQueue;
    __osPiDevMgr.acsQueue = &__osPiAccessQueue;
    __osPiDevMgr.dma = osPiRawStartDma;
    __osPiDevMgr.edma = osEPiRawStartDma;
    osCreateThread(&piThread, 0, __osDevMgrMain, &__osPiDevMgr, piThreadStack + OS_PIM_STACKSIZE, pri);
    osStartThread(&piThread);
    __osRestoreInt(savedMask);
    if (oldPri != -1) {
        osSetThreadPri(NULL, oldPri);
    }
}
extern OSPiHandle gCartHandle;        /* the cartridge (domain 1) handle */
extern OSPiHandle gDiskHandle;        /* the disk drive (domain 2) handle */
void InitPiHandles(void) {
    gCartHandle.latency = IO_READ(PI_BSD_DOM1_LAT_REG);
    gCartHandle.pulse = IO_READ(PI_BSD_DOM1_PWD_REG);
    gCartHandle.pageSize = IO_READ(PI_BSD_DOM1_PGS_REG);
    gCartHandle.relDuration = IO_READ(PI_BSD_DOM1_RLS_REG);
    gCartHandle.domain = PI_DOMAIN1;
    gDiskHandle.latency = IO_READ(PI_BSD_DOM2_LAT_REG);
    gDiskHandle.pulse = IO_READ(PI_BSD_DOM2_PWD_REG);
    gDiskHandle.pageSize = IO_READ(PI_BSD_DOM2_PGS_REG);
    gDiskHandle.relDuration = IO_READ(PI_BSD_DOM2_RLS_REG);
    gDiskHandle.domain = PI_DOMAIN2;
}
