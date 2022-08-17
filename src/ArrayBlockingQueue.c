#include "ArrayBlockingQueue.h"
#include "MutexCommon.h"

#include <malloc.h>
#include <string.h>

/**
 * An Array BlockingQueue implementation.
 */
typedef struct ArrayBlockingQueue {
    BlockingQueue parent;

    pthread_mutex_t *mutex;
    pthread_cond_t *nonFull;
    pthread_cond_t *nonEmpty;

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

    queue->mutex = newMutex();
    queue->nonFull = newCond();
    queue->nonEmpty = newCond();

    if (queue->mutex == NULL || queue->nonFull == NULL || queue->nonEmpty == NULL) {
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
    if (queue == NULL) return;

    freeCond(queue->nonEmpty);
    freeCond(queue->nonFull);
    freeMutex(queue->mutex);

    free(queue);
}


static bool queuePoll(ArrayBlockingQueue *queue, void *item, long timeoutMs) {
    lockMutex(queue->mutex, NULL);

    waitCondPredicate(queue->nonEmpty, queue->mutex, timeoutMs, queue->size != 0);
    
    if (queue->size == 0) {
        pthread_mutex_unlock(queue->mutex);
        return false;
    }

    dequeue(queue, item);

    pthread_cond_broadcast(queue->nonFull);
    pthread_mutex_unlock(queue->mutex);
    return true;
}

static bool queueOffer(ArrayBlockingQueue *queue, void *item, long timeoutMs) {
    lockMutex(queue->mutex, NULL);

    waitCondPredicate(queue->nonFull, queue->mutex, timeoutMs, queue->size != queue->capacity);
    
    if (queue->size == queue->capacity) {
        pthread_mutex_unlock(queue->mutex);
        return false;
    }

    enqueue(queue, item);

    pthread_cond_broadcast(queue->nonEmpty);
    pthread_mutex_unlock(queue->mutex);
    return true;
}
