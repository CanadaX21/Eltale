#include "common.h"

/* ---- initialize.c's .data (0x8006CD00-0x8006CD20, ROM 0x6D900): libultra initialize.c ---- */
u64 D_8006CD00 = 62500000;           /* osClockRate (OS_CLOCK_RATE) */
s32 osViClock = 48681812;           /* osViClock (VI_NTSC_CLOCK) */
u32 D_8006CD0C = 0;                  /* __osShutdown */
u32 __OSGlobalIntMask = 0x003FFF01;         /* __OSGlobalIntMask (OS_IM_ALL) */

extern s32 D_80092FF0;               /* __osFinalrom */
extern s32 osTvType;               /* osTvType */
extern s32 D_8000030C;               /* osResetType */
extern u8 D_8000031C[];              /* osAppNMIBuffer */
extern __osExceptionVector __osExceptionPreamble;   /* __osExceptionPreamble (code, copied as data) */
void osUnmapTLBAll(void);
void osMapTLBRdb(void);
void bzero(void* addr, int len);     /* bzero */

void osInitialize(void) {
    u32 pifdata;
    u32 clock = 0;

    D_80092FF0 = TRUE;
    __osSetSR(__osGetSR() | SR_CU1);
    __osSetFpcCsr(FPCSR_FS | FPCSR_EV);
    while (__osSiRawReadIo(PIF_RAM_END - 3, &pifdata)) {
        ;
    }
    while (__osSiRawWriteIo(PIF_RAM_END - 3, pifdata | 8)) {
        ;
    }
    *(__osExceptionVector*)UT_VEC = __osExceptionPreamble;
    *(__osExceptionVector*)XUT_VEC = __osExceptionPreamble;
    *(__osExceptionVector*)ECC_VEC = __osExceptionPreamble;
    *(__osExceptionVector*)E_VEC = __osExceptionPreamble;
    osWritebackDCache((void*)UT_VEC, E_VEC - UT_VEC + sizeof(__osExceptionVector));
    osInvalICache((void*)UT_VEC, E_VEC - UT_VEC + sizeof(__osExceptionVector));
    osUnmapTLBAll();
    osMapTLBRdb();
    osPiRawReadIo(4, &clock);
    clock &= ~0xF;
    if (clock != 0) {
        D_8006CD00 = clock;
    }
    D_8006CD00 = D_8006CD00 * 3 / 4;
    if (D_8000030C == 0) {
        bzero(D_8000031C, OS_APP_NMI_BUFSIZE);
    }
    if (osTvType == OS_TV_PAL) {
        osViClock = VI_PAL_CLOCK;
    } else if (osTvType == OS_TV_MPAL) {
        osViClock = VI_MPAL_CLOCK;
    } else {
        osViClock = VI_NTSC_CLOCK;
    }
    IO_WRITE(AI_CONTROL_REG, AI_CONTROL_DMA_ON);
    IO_WRITE(AI_DACRATE_REG, AI_MAX_DAC_RATE - 1);
    IO_WRITE(AI_BITRATE_REG, AI_MAX_BIT_RATE - 1);
}
