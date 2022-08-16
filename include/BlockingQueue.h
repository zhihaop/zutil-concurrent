#ifndef CONCURRENT_TOOLS_BLOCKINGQUEUE_H
#define CONCURRENT_TOOLS_BLOCKINGQUEUE_H

#ifdef __cplusplus
extern "C" {
#else

#include <stdbool.h>
#include <stddef.h>

#endif

typedef struct BlockingQueue {

    /**
     * Free the blocking queue.
     * 
     * @param queue        the blocking queue to free.
     */
    void (*free)(struct BlockingQueue *queue);

    /**
     * Poll an item from the blocking queue. If the queue is empty, the function will be blocked until the queue is not empty or reaches its timeout.
     * 
     * @param queue         the blocking queue to poll from.
     * @param timeoutMs     the timeout represented in milliseconds.
     * @param item          the writer buffer.
     * @return              return false if failed.
     */
    bool (*poll)(struct BlockingQueue *queue, void *item, long timeoutMs);

    /**
     * Offer an item to the blocking queue. If the queue is full, the function will be blocked until the queue is not full or reaches its timeout.
     * 
     * @param queue         the blocking queue to queueOffer.
     * @param item          the address of the item to be queueOffer.
     * @param timeoutMs     the timeout represented in milliseconds. The timeoutMs == -1 means wait
     *                      forever. The timeoutMs == 0 means never wait.
     * @return              return true if success.
     */
    bool (*offer)(struct BlockingQueue *queue, void *item, long timeoutMs);
} BlockingQueue;

#ifdef __cplusplus
}
#endif

#endif //CONCURRENT_TOOLS_BLOCKINGQUEUE_H