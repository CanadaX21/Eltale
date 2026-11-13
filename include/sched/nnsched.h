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

#endif
