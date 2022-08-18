#ifndef ZUTIL_CONCURRENT_CONDITION_H
#define ZUTIL_CONCURRENT_CONDITION_H

#include <stdio.h>
#include <malloc.h>
#include <pthread.h>
#include <sys/time.h>

#ifdef __cplusplus
extern "C" {
#else

#include <stdbool.h>

#endif

#include "ReentrantLock.h"

typedef struct Condition Condition;

/**
 * Create a condition variable from the reentrant lock. 
 * @param condition the condition variable.
 */
Condition *newCondition(ReentrantLock *lock);

/**
 * Free the condition variable.
 * @param condition the condition variable.
 */
void freeCondition(Condition *condition);

/**
 * Signal a waiting thread of the condition variable.
 * @param condition the condition variable.
 */
void signalCondition(Condition *condition);

/**
 * Signal all waiting threads of the condition variable.
 * @param condition the condition variable.
 */
void signalAllCondition(Condition *condition);

/**
 * Wait on the condition variable.
 * @param condition the condition variable.
 * @param timeoutMs the waiting timeout (milliseconds). timeoutMs == -1 means waiting 
 *                  forever (always returns -1), timeoutMs == 0 means never wait (always returns 0).
 * @return          the leave time (milliseconds).
 */
long awaitCondition(Condition *condition, long timeoutMs);

#ifdef __cplusplus
}
#endif

#endif //ZUTIL_CONCURRENT_CONDITION_H
