#ifndef _COMMON_H_
#define _COMMON_H_

#include "ultra64.h"
#include "structs.h"
#include "quest_types.h"
#include "variables.h"

/* The number of elements of an array. */
#define ARRAY_COUNT(arr) (s32)(sizeof(arr) / sizeof((arr)[0]))
/* The top of a stack array: a thread's stack grows down from its end. */
#define STACK_TOP(stack) ((stack) + ARRAY_COUNT(stack))

/* Matching hacks: a pad (an unused local that only fills a -g3 stack slot) is a last resort and sits inside
 * #ifdef MATCHING_HACK_IDO. Build with -DNO_MATCHING_HACKS to leave the pads out. */
#ifndef NO_MATCHING_HACKS
#define MATCHING_HACK_IDO
#endif

#endif
