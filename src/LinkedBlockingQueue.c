#include "LinkedBlockingQueue.h"

#include <stdatomic.h>
#include <string.h>

#include "ReentrantLock.h"
#include "Condition.h"

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

    ReentrantLock *putLock;
    ReentrantLock *takeLock;
    Condition *nonFull;
    Condition *nonEmpty;

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
    BlockingQueue parent = {
            .offer = (bool (*)(struct BlockingQueue *, void *, long)) queueOffer,
            .poll = (bool (*)(struct BlockingQueue *, void *, long)) queuePoll,
            .free = (void (*)(struct BlockingQueue *)) queueFree
    };
    memcpy(&queue->parent, &parent, sizeof(BlockingQueue));

    queue->itemSize = itemSize;
    queue->capacity = capacity;
    atomic_init(&queue->count, 0);

    queue->putLock = newReentrantLock();
    queue->takeLock = newReentrantLock();
    if (queue->putLock == NULL || queue->takeLock == NULL) {
        queueFree(queue);
        return NULL;
    }

    queue->nonEmpty = newCondition(queue->takeLock);
    queue->nonFull = newCondition(queue->putLock);
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
    if (queue->takeLock) {
        lockReentrantLock(queue->takeLock);
    }

    LinkedNode *node = queue->head;
    while (node != NULL) {
        LinkedNode *next = node->next;
        free(node);
        node = next;
    }

    if (queue->takeLock) {
        unlockReentrantLock(queue->takeLock);
    }
    if (queue->nonEmpty) {
        freeCondition(queue->nonEmpty);
    }
    if (queue->nonFull) {
        freeCondition(queue->nonFull);
    }
    if (queue->takeLock) {
        freeReentrantLock(queue->takeLock);
    }
    if (queue->putLock) {
        freeReentrantLock(queue->putLock);
    }
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
    LinkedNode *h = queue->head;
    LinkedNode *first = h->next;
    
    queue->head = first;
    memcpy(item, first->data, queue->itemSize);
    free(h);
 
    return atomic_fetch_add(&queue->count, -1);
}


static bool queuePoll(LinkedBlockingQueue *queue, void *item, long timeoutMs) {
    ReentrantLock *takeLock = queue->takeLock;
    Condition *nonEmpty = queue->nonEmpty;
    size_t capacity = queue->capacity;
    lockReentrantLock(takeLock);

    while (atomic_load(&queue->count) == 0) {
        timeoutMs = awaitCondition(nonEmpty, timeoutMs);
        
        if (timeoutMs == 0) {
            unlockReentrantLock(takeLock);
            return false;
        }
    }

    int before = dequeue(queue, item);
    if (before > 1) {
        signalCondition(nonEmpty);
    }

    unlockReentrantLock(takeLock);

    if (before == capacity) {
        lockReentrantLock(queue->putLock);
        signalCondition(queue->nonFull);
        unlockReentrantLock(queue->putLock);
    }
    return true;
}

static bool queueOffer(LinkedBlockingQueue *queue, void *item, long timeoutMs) {
    ReentrantLock* putLock = queue->putLock;
    Condition* nonFull = queue->nonFull;
    size_t capacity = queue->capacity;
    lockReentrantLock(putLock);

    while (atomic_load(&queue->count) == capacity) {
        timeoutMs = awaitCondition(nonFull, timeoutMs);
        
        if (timeoutMs == 0) {
            unlockReentrantLock(putLock);
            return false;
        }
    }
    
    int before = enqueue(queue, item);
    if (before + 1 < capacity) {
        signalCondition(nonFull);
    }

    unlockReentrantLock(putLock);

    if (before == 0) {
        lockReentrantLock(queue->takeLock);
        signalCondition(queue->nonEmpty);
        unlockReentrantLock(queue->takeLock);
    }
    return true;
}

