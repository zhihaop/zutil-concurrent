#ifndef ZUTIL_CONCURRENT_COUNTDOWNLATCH_H
#define ZUTIL_CONCURRENT_COUNTDOWNLATCH_H

#ifdef __cplusplus
extern "C" {
#else

#include <stdbool.h>

#endif

typedef struct CountDownLatch CountDownLatch;

/**
 * Create a count down latch.
 * 
 * @param count     the initial count.
 * @return          the count down latch.
 */
CountDownLatch *newCountDownLatch(int count);

/**
 * Waiting the count decrease to zero.
 * 
 * @param latch     the count down latch.
 * @param timeoutMs the waiting timeout (milliseconds). timeoutMs == -1 means waiting 
 *                  forever (always returns true), timeoutMs == 0 means never wait;
 *                  
 * @return return true if success.
 */
bool awaitCountDownLatch(CountDownLatch *latch, long timeoutMs);

/**
 * Decrease the count by one.
 * 
 * @param latch the count down latch.
 */
void decreaseCountDownLatch(CountDownLatch *latch);

/**
 * Free the count down latch.
 * 
 * @param latch the count down latch.
 */
void freeCountDownLatch(CountDownLatch *latch);

#ifdef __cplusplus
}
#endif
#endif //ZUTIL_CONCURRENT_COUNTDOWNLATCH_H
