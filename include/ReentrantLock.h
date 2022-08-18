#ifndef ZUTIL_CONCURRENT_REENTRANTLOCK_H
#define ZUTIL_CONCURRENT_REENTRANTLOCK_H

#ifdef __cplusplus
extern "C" {
#endif

#include <pthread.h>

typedef struct ReentrantLock ReentrantLock;

/**
 * Create a reentrant lock.
 * 
 * @return the reentrant lock. 
 */
ReentrantLock *newReentrantLock();

/**
 * Free the reentrant lock.
 * @param lock the reentrant lock. 
 */
void freeReentrantLock(ReentrantLock *lock);

/**
 * Lock the reentrant lock. 
 * @param lock the reentrant lock. 
 */
void lockReentrantLock(ReentrantLock *lock);

/**
 * Unlock the reentrant lock.
 * @param lock the reentrant lock.
 */
void unlockReentrantLock(ReentrantLock *lock);

/**
 * Get the native handle of the reentrant lock. 
 * @param lock the reentrant lock. 
 * @return the native pthread_mutex_t handle.
 */
pthread_mutex_t *nativeHandleReentrantLock(ReentrantLock *lock);

#ifdef __cplusplus
}
#endif
#endif //ZUTIL_CONCURRENT_REENTRANTLOCK_H
