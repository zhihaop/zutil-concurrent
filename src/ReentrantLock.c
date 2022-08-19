#include "ReentrantLock.h"

#include <malloc.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

struct ReentrantLock {
    pthread_mutexattr_t attr;
    pthread_mutex_t mutex;
};

/**
* Get the time after `afterMs` ms.
* @param t          the address of struct timespec.
* @param afterMs    the time represented in milliseconds.
* @return           
*/
inline static void timeAfter(struct timespec *t, long afterMs) {
    // avoid long overflow
    t->tv_sec += afterMs / 1000;
    afterMs = afterMs % 1000;

    t->tv_nsec += afterMs * 1000000;
    t->tv_sec += t->tv_nsec / 1000000000;
    t->tv_nsec = t->tv_nsec % 1000000000;
}

ReentrantLock *newReentrantLock() {
    pthread_mutexattr_t attr;
    if (pthread_mutexattr_init(&attr)) {
        return NULL;
    }

    ReentrantLock *lock = calloc(1, sizeof(ReentrantLock));
    if (lock == NULL) {
        return NULL;
    }

    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);

    if (pthread_mutex_init(&lock->mutex, &attr)) {
        free(lock);
        pthread_mutexattr_destroy(&attr);
        return NULL;
    }
    lock->attr = attr;
    return lock;
}

void freeReentrantLock(ReentrantLock *lock) {
    pthread_mutexattr_destroy(&lock->attr);
    pthread_mutex_destroy(&lock->mutex);
    free(lock);
}

void unlockReentrantLock(ReentrantLock *lock) {
    pthread_mutex_unlock(&lock->mutex);
}

void lockReentrantLock(ReentrantLock *lock) {
    pthread_mutex_lock(&lock->mutex);
}

pthread_mutex_t *nativeHandleReentrantLock(ReentrantLock *lock) {
    return &lock->mutex;
}

bool tryLockReentrantLock(ReentrantLock *lock) {
    return pthread_mutex_trylock(&lock->mutex) == 0;
}
