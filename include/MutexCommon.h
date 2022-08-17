#ifndef ZUTIL_CONCURRENT_MUTEXCOMMON_H
#define ZUTIL_CONCURRENT_MUTEXCOMMON_H

#include <stdio.h>
#include <malloc.h>
#include <pthread.h>
#include <sys/time.h>

#ifdef __cplusplus
extern "C" {
#else

#include <stdbool.h>
#include <stdlib.h>

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
 * Get the current time after `afterMs` ms.
 * @param t          the address of struct timespec.
 * @param afterMs    the time represented in milliseconds.
 * @return           
 */
inline static void nowAfter(struct timespec *t, long afterMs) {
    if (t == NULL) {
        return;
    }

    // this system call is pretty slow
    struct timeval tv;
    
    // using gettimeofday instead of clock_gettime because it is faster
    gettimeofday(&tv, NULL);

    // avoid long overflow
    tv.tv_sec += afterMs / 1000;
    afterMs = afterMs % 1000;

    tv.tv_usec += afterMs * 1000;
    tv.tv_sec += tv.tv_usec / 1000000;
    tv.tv_usec = tv.tv_usec % 1000000;

    t->tv_sec = tv.tv_sec;
    t->tv_nsec = tv.tv_usec * 1000;
}

/**
 * Wait on a conditional variable.
 * 
 * @param cond  the conditional variable.
 * @param mutex the mutex to unlock.
 * @param t     the timeout of waiting. t == NULL represents no timeout.
 * @return      return true if the thread is successfully signaled.
 */
#ifdef ZUTIL_WAIT_CHECK
#define waitCond(cond, mutex, t) doWaitCondCheck(cond, mutex, t, __FILE__, __LINE__)
#else
#define waitCond(cond, mutex, t) doWaitCond(cond, mutex, t)
#endif
inline static bool doWaitCond(pthread_cond_t *cond, pthread_mutex_t *mutex, struct timespec *t) {
    if (t != NULL) {
        return pthread_cond_timedwait(cond, mutex, t) == 0;
    } else {
        return pthread_cond_wait(cond, mutex) == 0;
    }
}

inline static bool doWaitCondCheck(pthread_cond_t *cond, pthread_mutex_t *mutex, struct timespec *t, const char* file, int line) {
    if (t != NULL) {
        return pthread_cond_timedwait(cond, mutex, t) == 0;
    } else {
        struct timespec ts;
        nowAfter(&ts, 1000);
        for (;;) {
            if (pthread_cond_timedwait(cond, mutex, &ts) == 0) {
                return true;
            }
            fprintf(stderr, "long waiting detected on: %s:%d\n", file, line);
        }
    }
}


/**
 * Lock on a pthread mutex.
 * 
 * @param mutex the mutex to lock.
 * @param t     the timeout of waiting. t == NULL represents no timeout.
 * @return      return true if the thread is successfully signaled.
 */
#ifdef ZUTIL_WAIT_CHECK
#define lockMutex(mutex, t) doLockMutexCheck(mutex, t, __FILE__, __LINE__)
#else
#define lockMutex(mutex, t) doLockMutex(mutex, t)
#endif
inline static bool doLockMutex(pthread_mutex_t *mutex, struct timespec *t) {
    if (t != NULL) {
        return pthread_mutex_timedlock(mutex, t) == 0;
    } else {
        return pthread_mutex_lock(mutex) == 0;
    }
}

inline static bool doLockMutexCheck(pthread_mutex_t *mutex, struct timespec *t, const char* file, int line) {
    if (t != NULL) {
        return pthread_mutex_timedlock(mutex, t) == 0;
    } else {
        struct timespec ts;
        nowAfter(&ts, 1000);
        for (;;) {
            if (pthread_mutex_timedlock(mutex, &ts) == 0) {
                return true;
            }
            fprintf(stderr, "long waiting detected on %s:%d\n", file, line);
        }
    }
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

#endif //ZUTIL_CONCURRENT_MUTEXCOMMON_H
