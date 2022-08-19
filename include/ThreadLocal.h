#ifndef ZUTIL_CONCURRENT_THREADLOCAL_H
#define ZUTIL_CONCURRENT_THREADLOCAL_H

#ifdef __cplusplus
extern "C" {
#else

#include <stdbool.h>
#include <stddef.h>

#endif

#include <pthread.h>

typedef struct {
    pthread_key_t key;
    bool initialized;
} ThreadLocal;

#define THREAD_LOCAL_INITIALIZER  {.key = -1, .initialized = false};
    
/**
* Init the thread local variable. It is same as marco THREAD_LOCAL_INITIALIZER.
 * 
* @param threadLocal   the thread local variable.
*/
inline static void initThreadLocal(ThreadLocal *threadLocal) {
    threadLocal->key = -1;
    threadLocal->initialized = false;
}

/**
 * Set the thread local variable.
 * @param threadLocal   the thread local variable.
 * @param item          the item to set.
 * @param deleter       the deleter of the item.
 * @return              return true if success.
 */
bool setThreadLocal(ThreadLocal *threadLocal, void *item, void (*deleter)(void *));

/**
 * Get the thread local variable.
 * 
 * @param threadLocal the thread local variable.
 * @return            the item (may be NULL).
 */
void *getThreadLocal(ThreadLocal *threadLocal);

/**
 * Set the thread local variable if absent.
 * 
 * @param threadLocal   the thread local variable.
 * @param builder       the builder of the variable.
 * @param deleter       the deleter of the variable.
 * @param arg           the argument of the builder.
 * @return              the thread local value (may be NULL if failed).
 */
void *
computeIfAbsentThreadLocal(ThreadLocal *threadLocal, void *(*builder)(void *), void *arg, void (*deleter)(void *));

/**
 * Destroy the thread local variable and free all the allocated memory.
 * 
 * @param threadLocal the thread local variable.
 */
void destroyThreadLocal(ThreadLocal *threadLocal);

#ifdef __cplusplus
}
#endif
#endif //ZUTIL_CONCURRENT_THREADLOCAL_H
