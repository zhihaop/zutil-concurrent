#include "ReentrantLock.h"

#include <malloc.h>
#include <pthread.h>
#include <stdlib.h>

struct ReentrantLock {
    pthread_mutexattr_t attr;
    pthread_mutex_t mutex;
};

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
