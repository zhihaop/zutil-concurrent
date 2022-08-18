#include "ArrayBlockingQueue.h"
#include "ReentrantLock.h"
#include "Condition.h"
#include <malloc.h>
#include <string.h>

/**
 * An Array BlockingQueue implementation.
 */
typedef struct ArrayBlockingQueue {
    BlockingQueue parent;

    ReentrantLock *lock;
    Condition *nonFull;
    Condition *nonEmpty;

    size_t capacity;
    size_t itemSize;
    size_t size;
    size_t head;
    size_t tail;

    char data[];
} ArrayBlockingQueue;

/* member functions */
static void queueFree(ArrayBlockingQueue *queue);

static bool queuePoll(ArrayBlockingQueue *queue, void *item, long timeoutMs);

static bool queueOffer(ArrayBlockingQueue *queue, void *item, long timeoutMs);

/* private member functions */
inline static void enqueue(ArrayBlockingQueue *queue, void *item);

inline static void dequeue(ArrayBlockingQueue *queue, void *item);

BlockingQueue *newArrayBlockingQueue(size_t capacity, size_t itemSize) {
    if (BLOCKING_QUEUE_UNBOUNDED - capacity == 0) {
        return NULL;
    }

    ArrayBlockingQueue *queue = calloc(1, sizeof(ArrayBlockingQueue) + capacity * itemSize * sizeof(void *));
    if (queue == NULL) {
        return NULL;
    }

    // member function binding
    BlockingQueue parent = {
            .offer = (bool (*)(struct BlockingQueue *, void *, long)) queueOffer,
            .free = (void (*)(struct BlockingQueue *)) queueFree,
            .poll = (bool (*)(struct BlockingQueue *, void *, long)) queuePoll
    };
    memcpy(&queue->parent, &parent, sizeof(BlockingQueue));

    queue->head = 0;
    queue->tail = 0;
    queue->size = 0;
    queue->itemSize = itemSize;
    queue->capacity = capacity;

    queue->lock = newReentrantLock();
    if (queue->lock == NULL) {
        queueFree(queue);
        return NULL;
    }

    queue->nonFull = newCondition(queue->lock);
    queue->nonEmpty = newCondition(queue->lock);
    if (queue->nonFull == NULL || queue->nonEmpty == NULL) {
        queueFree(queue);
        return NULL;
    }

    return &queue->parent;
}

/**
 * Put an item from the queue.
 * 
 * @param queue     the blocking queue.
 * @param item      the item to be put.
 */
inline static void enqueue(ArrayBlockingQueue *queue, void *item) {
    memcpy(queue->data + queue->tail * queue->itemSize, item, queue->itemSize);
    queue->tail = queue->tail + 1 >= queue->capacity ? 0 : queue->tail + 1;
    queue->size += 1;
}

/**
 * Take an item from the queue.
 * 
 * @param queue     the blocking queue.
 * @param item      the return item.
 */
inline static void dequeue(ArrayBlockingQueue *queue, void *item) {
    memcpy(item, queue->data + queue->head * queue->itemSize, queue->itemSize);
    queue->head = queue->head + 1 >= queue->capacity ? 0 : queue->head + 1;
    queue->size -= 1;
}

static void queueFree(ArrayBlockingQueue *queue) {
    if (queue->nonEmpty) {
        freeCondition(queue->nonEmpty);
    }
    if (queue->nonFull) {
        freeCondition(queue->nonFull);
    }
    if (queue->lock) {
        freeReentrantLock(queue->lock);
    }
    free(queue);
}


static bool queuePoll(ArrayBlockingQueue *queue, void *item, long timeoutMs) {
    lockReentrantLock(queue->lock);

    while (queue->size == 0) {
        if (!waitCondition(queue->nonEmpty, timeoutMs)) {
            unlockReentrantLock(queue->lock);
            return false;
        }
    }

    dequeue(queue, item);
    signalAllCondition(queue->nonFull);
    unlockReentrantLock(queue->lock);
    return true;
}

static bool queueOffer(ArrayBlockingQueue *queue, void *item, long timeoutMs) {
    lockReentrantLock(queue->lock);

    while (queue->size == queue->capacity) {
        if (!waitCondition(queue->nonFull, timeoutMs)) {
            unlockReentrantLock(queue->lock);
            return false;
        }
    }

    if (queue->size == queue->capacity) {
        unlockReentrantLock(queue->lock);
        return false;
    }

    enqueue(queue, item);

    signalAllCondition(queue->nonEmpty);
    unlockReentrantLock(queue->lock);
    return true;
}
