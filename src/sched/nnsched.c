#include "common.h"
#include "sched/nnsched.h"

/* The scheduler's state, in this order in .bss: a stack for each of its three threads, and the retrace count. */
u64 nnScStack[NN_SC_STACKSIZE / sizeof(u64)];          /* the scheduler thread's stack */
u64 nnScAudioStack[NN_SC_STACKSIZE / sizeof(u64)];     /* the audio thread's stack */
u64 nnScGraphicsStack[NN_SC_STACKSIZE / sizeof(u64)];  /* the graphics thread's stack */
u16 gRetraceCount;   /* retraces since the last frame: the event handler counts them, main.c waits on it */

void nnScCreateScheduler(NNSched * sc, u8 numFields) {
    
    sc->curGraphicsTask = 0;
    sc->curAudioTask    = 0;
    sc->graphicsTaskSuspended = 0;
    sc->clientList      = 0;
    sc->firstTime       = 1; 
    sc->retraceMsg      = NN_SC_RETRACE_MSG;
    sc->prenmiMsg       = NN_SC_PRE_NMI_MSG;
    
    osCreateMesgQueue(&sc->retraceMQ, sc->retraceMsgBuf, NN_SC_MAX_MESGS);
    osCreateMesgQueue(&sc->rspMQ, sc->rspMsgBuf, NN_SC_MAX_MESGS);
    osCreateMesgQueue(&sc->rdpMQ, sc->rdpMsgBuf, NN_SC_MAX_MESGS);
    osCreateMesgQueue(&sc->graphicsRequestMQ, sc->graphicsRequestBuf, NN_SC_MAX_MESGS);
    osCreateMesgQueue(&sc->audioRequestMQ, sc->audioRequestBuf, NN_SC_MAX_MESGS);
    osCreateMesgQueue(&sc->waitMQ, sc->waitMsgBuf, NN_SC_MAX_MESGS);
    
    osViSetEvent(&sc->retraceMQ, (OSMesg)VIDEO_MSG, numFields);    
    osSetEventMesg(OS_EVENT_SP, &sc->rspMQ, (OSMesg)RSP_DONE_MSG);
    osSetEventMesg(OS_EVENT_DP, &sc->rdpMQ, (OSMesg)RDP_DONE_MSG);
    osSetEventMesg(OS_EVENT_PRENMI, &sc->retraceMQ, (OSMesg)PRE_NMI_MSG);
}
void nnScStartScheduler(NNSched* sc) {

    osCreateThread(&sc->schedulerThread, 0x13, (void (*)(void*)) nnScEventHandler, sc, STACK_TOP(nnScStack), NN_SC_PRI);
    osStartThread(&sc->schedulerThread);
    osCreateThread(&sc->audioThread, 0x12, (void (*)(void*)) nnScExecuteAudio, sc, STACK_TOP(nnScAudioStack), NN_SC_AUDIO_PRI);
    osStartThread(&sc->audioThread);
    osCreateThread(&sc->graphicsThread, 0x11, (void (*)(void*)) nnScExecuteGraphics, sc, STACK_TOP(nnScGraphicsStack), NN_SC_GRAPHICS_PRI);
    osStartThread(&sc->graphicsThread);
}
OSMesgQueue* nnScGetAudioMQ(NNSched* sc) {
    return &sc->audioRequestMQ;
}
OSMesgQueue* nnScGetGfxMQ(NNSched* sc) {
    return &sc->graphicsRequestMQ;
}
void nnScEventHandler(NNSched* sc) {
    OSMesg msg = (OSMesg)0;

    while (1) {
        osRecvMesg(&sc->retraceMQ, &msg, OS_MESG_BLOCK);
        switch ((int)msg) {
            case VIDEO_MSG:
                gRetraceCount++;
                nnScEventBroadcast(sc, &sc->retraceMsg);
                break;
            case PRE_NMI_MSG:
                nnScEventBroadcast(sc, &sc->prenmiMsg);
                osViBlack(1);
                break;
        }
    }
}
void nnScAddClient(NNSched *sc, NNScClient *c, OSMesgQueue *msgQ)
{
  OSIntMask mask;
  mask = osSetIntMask(OS_IM_NONE);
  c->msgQ = msgQ;
  c->next = sc->clientList;
  sc->clientList = c;
  osSetIntMask(mask);
}
void nnScRemoveClient(NNSched *sc, NNScClient *c)
{
  NNScClient *client = sc->clientList; 
  NNScClient *prev   = 0;
  OSIntMask  mask;
  mask = osSetIntMask(OS_IM_NONE);
    
  while (client != 0) {
    if (client == c) {
      if(prev)
	prev->next = c->next;
      else
	sc->clientList = c->next;
      break;
    }
    prev   = client;
    client = client->next;
  }
  osSetIntMask(mask);
}
void nnScEventBroadcast(NNSched *sc, NNScMsg *msg)
{
  NNScClient *client;

  for (client = sc->clientList; client != 0; client = client->next) {
    osSendMesg(client->msgQ, (OSMesg *)msg, OS_MESG_NOBLOCK);
  }
}
void nnScExecuteAudio(NNSched *sc)
{
  OSMesg msg = (OSMesg)0;
  NNScTask *task = (NNScTask *)0;
  NNScTask *gfxTask = (NNScTask *)0;
  u32 yieldFlag;

  while(1) {
    osRecvMesg(&sc->audioRequestMQ, (OSMesg *)&task, OS_MESG_BLOCK);
    osWritebackDCacheAll();
    yieldFlag = 0;
    gfxTask = sc->curGraphicsTask;
    if( gfxTask ) {
      osSpTaskYield();
      osRecvMesg(&sc->rspMQ, &msg, OS_MESG_BLOCK);
      if (osSpTaskYielded(&gfxTask->list)){
        yieldFlag =1;
      } else {
        yieldFlag =2;
      }
    }

    sc->curAudioTask = task;
    osSpTaskLoad(&task->list);
    osSpTaskStartGo(&task->list);
    osRecvMesg(&sc->rspMQ, &msg, OS_MESG_BLOCK);
    sc->curAudioTask = (NNScTask *)0;

    if( sc->graphicsTaskSuspended )
      osSendMesg( &sc->waitMQ, &msg, OS_MESG_BLOCK );
    if( yieldFlag == 1 ) {
      osSpTaskLoad(&gfxTask->list);
      osSpTaskStartGo(&gfxTask->list);
    } else if ( yieldFlag == 2 ) {
      osSendMesg(&sc->rspMQ, &msg, OS_MESG_BLOCK);
    }
    osSendMesg(task->msgQ, task->msg, OS_MESG_BLOCK);
  }
}
void nnScExecuteGraphics(NNSched *sc)
{
  OSMesg msg = (OSMesg)0;
  NNScTask *task;

  while(1) {
    osRecvMesg(&sc->graphicsRequestMQ, (OSMesg *)&task, OS_MESG_BLOCK);
    nnScWaitTaskReady(sc, task);
    if( sc->curAudioTask ) {
      sc->graphicsTaskSuspended = task;
      osRecvMesg( &sc->waitMQ, &msg, OS_MESG_BLOCK );
      sc->graphicsTaskSuspended = (NNScTask *)0;
    }

    sc->curGraphicsTask = task;
    osSpTaskLoad(&task->list);
    osSpTaskStartGo(&task->list);
    osRecvMesg(&sc->rspMQ, &msg, OS_MESG_BLOCK);
    sc->curGraphicsTask = (NNScTask *)0;

    osRecvMesg(&sc->rdpMQ, &msg, OS_MESG_BLOCK);

    if (sc->firstTime) {
      osViBlack(FALSE);
      sc->firstTime = 0;
    }
    if ( task->flags & NN_SC_SWAPBUFFER ){
      osViSwapBuffer(task->framebuffer);
    }
    osSendMesg(task->msgQ, task->msg, OS_MESG_BLOCK);
  }
}
void nnScWaitTaskReady(NNSched* sc, NNScTask* task) {
    
    OSMesg msg = (OSMesg)0;
    NNScClient client;

    void* fb = task->framebuffer;
    if (osViGetCurrentFramebuffer() == fb) {
        do {
            nnScAddClient(sc, &client, &sc->waitMQ);
            osRecvMesg(&sc->waitMQ, &msg, OS_MESG_BLOCK);
            nnScRemoveClient(sc, &client);
        } while (osViGetCurrentFramebuffer() == fb);
    }
}
