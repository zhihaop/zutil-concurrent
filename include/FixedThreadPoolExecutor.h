#ifndef CONCURRENT_TOOLS_THREADPOOL_H
#define CONCURRENT_TOOLS_THREADPOOL_H

#include "BlockingQueue.h"

#ifdef __cplusplus
extern "C" {
#else

#include <stddef.h>
#include <stdbool.h>
    
#endif

typedef struct FixedThreadPoolExecutor FixedThreadPoolExecutor;

typedef BlockingQueue *(*BlockingQueueBuilder)(size_t, size_t);

/**
 * New a fixed thread pool. The number of thread is fixed in the thread pool. FixedThreadPoolExecutor owns a blocking taskQueue
 * internal in order to handle new Task, taskQueueSize represents the maximum tasks in the taskQueue.
 * 
 * @param corePoolSize      the number of thread.
 * @param taskQueueSize     the number of taskQueue size.
 * @param builder           the builder of taskQueue.
 * @return                  return NULL if failed.
 */
FixedThreadPoolExecutor *newExecutor(size_t corePoolSize, size_t taskQueueSize, BlockingQueueBuilder builder);

/**
 * Submit a Task to the thread pool.
 * 
 * @param pool      the thread pool to submit.
 * @param fn        the function to submit.
 * @param arg       the parameter of the function.
 * @return          return false if failed.
 */
bool executorSubmit(FixedThreadPoolExecutor *pool, void (*fn)(void *), void *arg);

/**
 * Wait all the tasks finished, and state the thread pool.
 * 
 * @param pool the thread pool.
 */
void executorShutdown(FixedThreadPoolExecutor *pool);

/**
 * Shutdown the thread pool, and free the thread pool.
 * 
 * @param pool the thread pool.
 */
void executorFree(FixedThreadPoolExecutor *pool);

#ifdef __cplusplus
}
#endif
#endif //CONCURRENT_TOOLS_THREADPOOL_H
