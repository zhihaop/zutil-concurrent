#ifndef ZUTIL_CONCURRENT_LINKEDBLOCKINGQUEUE_H
#define ZUTIL_CONCURRENT_LINKEDBLOCKINGQUEUE_H

#include "BlockingQueue.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * New a linked blocking queue with initial capacity.
 * 
 * @param capacity  the initial capacity of the blocking queue.
 * @param itemSize  the size of the item.
 * @return          return NULL if failed.
 */
BlockingQueue *newLinkedBlockingQueue(size_t capacity, size_t itemSize);

#ifdef __cplusplus
}
#endif

#endif //ZUTIL_CONCURRENT_LINKEDBLOCKINGQUEUE_H
