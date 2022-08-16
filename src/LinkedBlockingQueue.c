#include "LinkedBlockingQueue.h"
#include "MutexCommon.h"

#include <stdatomic.h>
#include <string.h>

/**
 * The linked node in Linked BlockingQueue.
 */
typedef struct LinkedNode {
    struct LinkedNode *next;
    char data[];
} LinkedNode;

/**
 * A Linked BlockingQueue implementation using two lock queue algorithm.
 */
typedef struct LinkedBlockingQueue {
    BlockingQueue parent;

    pthread_mutex_t *putLock;
    pthread_mutex_t *takeLock;
    pthread_cond_t *nonFull;
    pthread_cond_t *nonEmpty;

    size_t capacity;
    size_t count;
    size_t itemSize;

    LinkedNode *head;
    LinkedNode *tail;
} LinkedBlockingQueue;

/* member functions */
static void queueFree(LinkedBlockingQueue *queue);
static bool queuePoll(LinkedBlockingQueue *queue, void *item, long timeoutMs);
static bool queueOffer(LinkedBlockingQueue *queue, void *item, long timeoutMs);

/* private member functions */
inline static LinkedNode *newNode(void *item, size_t itemSize);
inline static int enqueue(LinkedBlockingQueue *queue, void *item);
inline static int dequeue(LinkedBlockingQueue *queue, void *item);


BlockingQueue *newLinkedBlockingQueue(size_t capacity, size_t itemSize) {
    LinkedBlockingQueue *queue = calloc(1, sizeof(LinkedBlockingQueue));
    if (queue == NULL) {
        return NULL;
    }

    // member function binding
    queue->parent.offer = (bool (*)(struct BlockingQueue *, void *, long)) queueOffer;
    queue->parent.poll = (bool (*)(struct BlockingQueue *, void *, long)) queuePoll;
    queue->parent.free = (void (*)(struct BlockingQueue *)) queueFree;

    queue->itemSize = itemSize;
    queue->capacity = capacity;
    atomic_init(&queue->count, 0);

    queue->putLock = newMutex();
    queue->takeLock = newMutex();
    if (queue->putLock == NULL || queue->takeLock == NULL) {
        queueFree(queue);
        return NULL;
    }
    
    queue->nonEmpty = newCond();
    queue->nonFull = newCond();
    if (queue->nonFull == NULL || queue->nonEmpty == NULL) {
        queueFree(queue);
        return NULL;
    }

    queue->head = newNode(NULL, itemSize);
    queue->tail = queue->head;
    if (queue->head == NULL) {
        queueFree(queue);
        return NULL;
    }
    return &queue->parent;
}

/**
 * Create a linked node.
 * 
 * @param item      the item in the linked node.
 * @param itemSize  the size of item.
 * @return 
 */
inline static LinkedNode *newNode(void *item, size_t itemSize) {
    if (item == NULL) {
        return calloc(1, sizeof(LinkedNode));
    }

    LinkedNode *node = malloc(sizeof(LinkedNode) + itemSize);
    node->next = NULL;
    memcpy(node->data, item, itemSize);
    return node;
}

static void queueFree(LinkedBlockingQueue *queue) {
    if (queue == NULL) {
        return;
    }

    if (queue->takeLock != NULL) {
        pthread_mutex_lock(queue->takeLock);
    }

    LinkedNode *node = queue->head;
    while (node != NULL) {
        LinkedNode *next = node->next;
        free(node);
        node = next;
    }

    if (queue->takeLock != NULL) {
        pthread_mutex_unlock(queue->takeLock);
    }

    freeCond(queue->nonEmpty);
    freeCond(queue->nonFull);
    freeMutex(queue->takeLock);
    freeMutex(queue->putLock);
    free(queue);

}

/**
 * Put an item from the queue.
 * 
 * @param queue     the blocking queue.
 * @param item      the item to be put.
 * @return          the number of item before enqueue.
 */
inline static int enqueue(LinkedBlockingQueue *queue, void *item) {
    LinkedNode *node = newNode(item, queue->itemSize);
    queue->tail->next = node;
    queue->tail = node;

    return atomic_fetch_add(&queue->count, 1);
}

/**
 * Take an item from the queue.
 * 
 * @param queue     the blocking queue.
 * @param item      the return item.
 * @return          the number of item before dequeue.
 */
inline static int dequeue(LinkedBlockingQueue *queue, void *item) {
    LinkedNode *first = queue->head->next;
    LinkedNode *legacy = queue->head;
    queue->head = first;

    memcpy(item, first->data, queue->itemSize);
    free(legacy);

    return atomic_fetch_add(&queue->count, -1);
}

static bool queuePoll(LinkedBlockingQueue *queue, void *item, long timeoutMs) {
    pthread_mutex_lock(queue->takeLock);

    waitCondPredicate(queue->nonEmpty, queue->takeLock, timeoutMs, atomic_load(&queue->count) != 0);

    if (atomic_load(&queue->count) == 0) {
        pthread_mutex_unlock(queue->takeLock);
        return false;
    }

    int before = dequeue(queue, item);
    if (before > 1) {
        pthread_cond_signal(queue->nonEmpty);
    }

    pthread_mutex_unlock(queue->takeLock);

    if (before == queue->capacity) {
        pthread_mutex_lock(queue->putLock);
        pthread_cond_signal(queue->nonFull);
        pthread_mutex_unlock(queue->putLock);
    }

    return true;
}

static bool queueOffer(LinkedBlockingQueue *queue, void *item, long timeoutMs) {
    pthread_mutex_lock(queue->putLock);

    waitCondPredicate(queue->nonFull, queue->putLock, timeoutMs, atomic_load(&queue->count) != queue->capacity);

    if (atomic_load(&queue->count) == queue->capacity) {
        pthread_mutex_unlock(queue->putLock);
        return false;
    }

    int before = enqueue(queue, item);

    if (before + 1 < queue->capacity) {
        pthread_cond_signal(queue->nonFull);
    }

    pthread_mutex_unlock(queue->putLock);

    if (before == 0) {
        pthread_mutex_lock(queue->takeLock);
        pthread_cond_signal(queue->nonEmpty);
        pthread_mutex_unlock(queue->takeLock);
    }

    return true;
}

