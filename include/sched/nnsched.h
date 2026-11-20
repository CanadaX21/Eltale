#ifndef SCHED_NNSCHED_H
#define SCHED_NNSCHED_H

/* The functions of src/sched/nnsched.c, declared once for every file that calls them. */

#include "common.h"

void nnScCreateScheduler(NNSched * sc, u8 numFields);
void nnScStartScheduler(NNSched* sc);
OSMesgQueue* nnScGetAudioMQ(NNSched* sc);
OSMesgQueue* nnScGetGfxMQ(NNSched* sc);
void nnScEventHandler(NNSched* sc);
void nnScAddClient(NNSched *sc, NNScClient *c, OSMesgQueue *msgQ);
void nnScRemoveClient(NNSched *sc, NNScClient *c);
void nnScEventBroadcast(NNSched *sc, NNScMsg *msg);
void nnScExecuteAudio(NNSched *sc);
void nnScExecuteGraphics(NNSched *sc);
void nnScWaitTaskReady(NNSched* sc, NNScTask* task);

/* The state src/sched/nnsched.c defines (.bss), in its storage order: the order of these declarations is the
 * unit's .bss layout (include/bss_order.h). */
extern u64 nnScStack[NN_SC_STACKSIZE / sizeof(u64)];   /* the scheduler thread's stack */
extern u64 nnScAudioStack[NN_SC_STACKSIZE / sizeof(u64)];   /* the audio thread's stack */
extern u64 nnScGraphicsStack[NN_SC_STACKSIZE / sizeof(u64)];   /* the graphics thread's stack */
extern u16 gRetraceCount;   /* retraces since the last frame: the event handler counts them, main.c waits on it */

#endif
