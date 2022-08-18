#ifndef ZUTIL_CONCURRENT_THREADPOOL_H
#define ZUTIL_CONCURRENT_THREADPOOL_H

#include "ExecutorService.h"
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
 * New a fixed thread pool. The number of thread is fixed in the thread pool. FixedThreadPoolExecutor owns a blocking queue
 * internal in order to handle new Task, taskQueueSize represents the maximum tasks in the queue.
 * 
 * @param threadSize        the number of thread.
 * @param taskQueueSize     the number of queue size.
 * @param format            the format of contexts.
 * @param builder           the builder of queue.
 * @return                  return NULL if failed.
 */
ExecutorService *
newFixedThreadPoolExecutor(size_t threadSize, size_t taskQueueSize, const char *format, BlockingQueueBuilder builder);

#ifdef __cplusplus
}
#endif
#endif //ZUTIL_CONCURRENT_THREADPOOL_H
