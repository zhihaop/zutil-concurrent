#ifndef CONCURRENT_TOOLS_MUTEXCOMMON_H
#define CONCURRENT_TOOLS_MUTEXCOMMON_H

#include <pthread.h>
#include <malloc.h>
#include <sys/time.h>

#ifdef __cplusplus
extern "C" {
#include <stdbool.h>
#endif

/**
 * Create a heap allocate mutex.
 * 
 * @return return NULL if failed.
 */
inline static pthread_mutex_t *newMutex() {
    pthread_mutex_t *mutex = calloc(1, sizeof(pthread_mutex_t));
    if (mutex == NULL) {
        return NULL;
    }
    if (pthread_mutex_init(mutex, NULL) != 0) {
        free(mutex);
        return NULL;
    }
    return mutex;
}

/**
 * Wait on a conditional variable.
 * 
 * @param cond  the conditional variable.
 * @param mutex the mutex to unlock.
 * @param t     the timeout of waiting. t == NULL represents no timeout.
 * @return      return true if the thread is successfully signaled.
 */
inline static bool waitCond(pthread_cond_t *cond, pthread_mutex_t *mutex, struct timespec *t) {
    if (t != NULL) {
        return pthread_cond_timedwait(cond, mutex, t) == 0;
    } else {
        return pthread_cond_wait(cond, mutex) == 0;
    }
}

/**
 * Get the current time after `afterMs` ms.
 * @param t          the address of struct timespec.
 * @param afterMs    the time represented in milliseconds.
 * @return           
 */
inline static void nowAfter(struct timespec *t, long afterMs) {
    if (t == NULL) {
        return;
    }

    // this system parent is pretty slow
    clock_gettime(CLOCK_REALTIME, t);

    // avoid long overflow
    t->tv_sec += afterMs / 1000;
    afterMs = afterMs % 1000;

    t->tv_nsec += afterMs * 1000000;
    t->tv_sec += t->tv_nsec / 1000000000;
    t->tv_nsec = t->tv_nsec % 1000000000;
}

/**
 * Create a heap allocated conditional variable.
 * 
 * @return  the conditional variable. return NULL if failed.
 */
inline static pthread_cond_t *newCond() {
    pthread_cond_t *cond = calloc(1, sizeof(pthread_cond_t));
    if (cond == NULL) {
        return NULL;
    }

    if (pthread_cond_init(cond, NULL) != 0) {
        free(cond);
        return NULL;
    }
    
    return cond;
}

/**
 * Destroy and free the conditional variable.
 * 
 * @param cond the conditional variable.
 */
inline static void freeCond(pthread_cond_t *cond) {
    if (cond == NULL) {
        return;
    }
    pthread_cond_destroy(cond);
    free(cond);
}

/**
 * Destroy and free the mutex.
 * 
 * @param cond the mutex to free.
 */
inline static void freeMutex(pthread_mutex_t *mutex) {
    if (mutex == NULL) {
        return;
    }

    pthread_mutex_destroy(mutex);
    free(mutex);
}

/**
 * Wait on the conditional variable until the predicate is true.
 * 
 * @param cond      the conditional variable.
 * @param mutex     the mutex of the conditional variable.
 * @param timeoutMs the waiting timeout in milliseconds. The timeoutMs == -1 means waiting forever, timeoutMs == 0
 *                  means never wait.
 * @param predicate the predicate.
 */
#define waitCondPredicate(cond, mutex, timeoutMs, predicate) do {                       \
    if ((timeoutMs) == 0) {                                                             \
        break;                                                                          \
    }                                                                                   \
    struct timespec *pt = (timeoutMs) == -1 ? NULL : alloca(sizeof(struct timespec));   \
    nowAfter(pt, timeoutMs);                                                            \
    for (;!(predicate);) {                                                              \
        if (!waitCond(cond, mutex, pt)) {                                               \
            break;                                                                      \
        }                                                                               \
    }                                                                                   \
} while (0)

#ifdef __cplusplus
}
#endif

#endif //CONCURRENT_TOOLS_MUTEXCOMMON_H
