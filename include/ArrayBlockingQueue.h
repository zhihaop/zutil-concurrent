#ifndef CONCURRENT_TOOLS_ARRAYBLOCKINGQUEUE_H
#define CONCURRENT_TOOLS_ARRAYBLOCKINGQUEUE_H

#include "BlockingQueue.h"

#ifdef __cplusplus
extern "C" {
#endif
    
/**
 * New a array blocking taskQueue with initial capacity.
 * 
 * @param capacity  the initial capacity of the blocking taskQueue.
 * @param itemSize  the size of the item.
 * @return          return NULL if failed.
 */
BlockingQueue *newArrayBlockingQueue(size_t capacity, size_t itemSize);


#ifdef __cplusplus
}
#endif
#endif //CONCURRENT_TOOLS_ARRAYBLOCKINGQUEUE_H
