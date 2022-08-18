#ifndef ZUTIL_CONCURRENT_THREADLOCAL_H
#define ZUTIL_CONCURRENT_THREADLOCAL_H

#ifdef __cplusplus
extern "C" {
#else

#include <stdbool.h>
#include <stddef.h>

#endif

typedef struct ThreadLocal ThreadLocal;

/**
 * Initialize the thread local variable.
 * @param threadLocal the thread local variable.
 * @return return true if success.
 */
bool initThreadLocal(ThreadLocal *threadLocal);

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

#ifdef __cplusplus
}
#endif
#endif //ZUTIL_CONCURRENT_THREADLOCAL_H
