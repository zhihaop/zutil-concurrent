#include "Condition.h"

#include <stdatomic.h>
#include <alloca.h>

enum ConditionNodeState {
    WAITING,
    NOTIFIED,
    INVALID
};

struct ConditionNode {
    struct ConditionNode *next;
    pthread_cond_t condition;

    int count;
    int state;
};

struct Condition {
    ReentrantLock *lock;
    struct ConditionNode *waitHead;
    struct ConditionNode *waitTail;
};

/**
* Get the time after `afterMs` ms.
* @param t          the address of struct timespec.
* @param afterMs    the time represented in milliseconds.
* @return           
*/
inline static void timeAfter(struct timespec *t, long afterMs) {
    // avoid long overflow
    t->tv_sec += afterMs / 1000;
    afterMs = afterMs % 1000;

    t->tv_nsec += afterMs * 1000000;
    t->tv_sec += t->tv_nsec / 1000000000;
    t->tv_nsec = t->tv_nsec % 1000000000;
}

/**
 * Create a condition node, which represents the thread is waiting for some resources.
 * 
 * @return the condition node.
 */
inline static struct ConditionNode *newConditionNode() {
    struct ConditionNode *node = calloc(1, sizeof(struct ConditionNode));

    node->next = NULL;
    atomic_store(&node->count, 0);
    atomic_store(&node->state, WAITING);

    if (pthread_cond_init(&node->condition, NULL)) {
        free(node);
        return NULL;
    }
    return node;
}

/**
 * Add the reference count of node by 1.
 * 
 * @param node the condition node.
 */
inline static void retainConditionNode(struct ConditionNode *node) {
    atomic_fetch_add(&node->count, 1);
}

/**
 * Remove the condition node form queue.
 * 
 * @param node the condition node.
 */
inline static void removeFromQueueConditionNode(struct ConditionNode *head, struct ConditionNode *node) {
    while (head) {
        if (head->next == node) {
            head->next = node->next;
            return;
        }
        head = head->next;
    }
}

/**
 * Decrease the reference count of node by 1. Once the reference count decrease to zero, the condition node will
 * be destroyed.
 * 
 * @param node the condition node.
 */
inline static void releaseConditionNode(struct ConditionNode *node) {
    int now = atomic_fetch_sub(&node->count, 1) - 1;
    if (now == 0) {
        pthread_cond_destroy(&node->condition);
        free(node);
    } else if (now < 0) {
        fprintf(stderr, "double free node %p, ref_count %d\n", node, atomic_load(&node->count));
        fflush(stderr);
    }
}

Condition *newCondition(ReentrantLock *lock) {
    Condition *condition = calloc(1, sizeof(Condition));
    if (condition == NULL) {
        return NULL;
    }

    condition->lock = lock;
    condition->waitHead = newConditionNode();
    condition->waitTail = condition->waitHead;

    if (condition->waitTail == NULL) {
        free(condition);
        return NULL;
    }

    retainConditionNode(condition->waitHead);
    return condition;
}

void signalAllCondition(Condition *condition) {
    while (condition->waitHead->next) {
        signalCondition(condition);
    }
}

void signalCondition(Condition *condition) {
    struct ConditionNode *waitHead = condition->waitHead;
    struct ConditionNode *firstNode = waitHead->next;

    if (firstNode) {
        retainConditionNode(firstNode);
        waitHead->next = firstNode->next;
        if (firstNode == condition->waitTail) {
            condition->waitTail = waitHead;
        }

        if (firstNode->state != INVALID) {
            firstNode->next = NULL;
            firstNode->state = NOTIFIED;
            pthread_cond_signal(&firstNode->condition);
        }
        releaseConditionNode(firstNode);
    }
}

long awaitCondition(Condition *condition, long timeoutMs) {
    struct timespec *current = NULL;
    struct timespec *timeout = NULL;
    if (timeoutMs == 0) {
        return 0;
    }
    
    if (timeoutMs != -1) {
        current = alloca(sizeof(struct timespec));
        timeout = alloca(sizeof(struct timespec));
        clock_gettime(CLOCK_REALTIME, current);

        timeout->tv_sec = current->tv_sec;
        timeout->tv_nsec = current->tv_nsec;
        timeAfter(timeout, timeoutMs);
    }

    struct ConditionNode *waitNode = newConditionNode();
    if (waitNode == NULL) {
        return 0;
    }

    retainConditionNode(waitNode);

    struct ConditionNode *prevNode = condition->waitTail;
    condition->waitTail = waitNode;
    prevNode->next = waitNode;

    pthread_cond_t *cond = &waitNode->condition;
    pthread_mutex_t *mutex = nativeHandleReentrantLock(condition->lock);

    while (waitNode->state == WAITING) {
        int state;
        if (timeout != NULL) {
            state = pthread_cond_timedwait(cond, mutex, timeout);
        } else {
            state = pthread_cond_wait(cond, mutex);
        }
        // condition await timeout
        if (state != 0) {
            removeFromQueueConditionNode(condition->waitHead, waitNode);
            break;
        }
    }

    waitNode->state = INVALID;
    releaseConditionNode(waitNode);

    if (timeoutMs == -1) {
        return -1;
    } 
    
    struct timespec latest;
    clock_gettime(CLOCK_REALTIME, &latest);

    long duration = (latest.tv_sec - current->tv_sec) * 1000;
    duration += (latest.tv_nsec - current->tv_nsec) / 1000000;
    
    long leave = timeoutMs - duration;
    return leave < 0 ? 0 : leave;
}

void freeCondition(Condition *condition) {
    if (condition->lock) {
        lockReentrantLock(condition->lock);
    }

    struct ConditionNode *node = condition->waitHead;
    while (node) {
        struct ConditionNode *next = node->next;
        releaseConditionNode(node);
        node = next;
    }
    if (condition->lock) {
        unlockReentrantLock(condition->lock);
    }
    free(condition);
}


