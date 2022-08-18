#include "Condition.h"

#include <stdatomic.h>

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
* Get the current time after `afterMs` ms.
* @param t          the address of struct timespec.
* @param afterMs    the time represented in milliseconds.
* @return           
*/
inline static void nowAfter(struct timespec *t, long afterMs) {
    if (t == NULL) {
        return;
    }

    // this system call is pretty slow
    struct timeval tv;

    // using gettimeofday instead of clock_gettime because it is faster
    gettimeofday(&tv, NULL);

    // avoid long overflow
    tv.tv_sec += afterMs / 1000;
    afterMs = afterMs % 1000;

    tv.tv_usec += afterMs * 1000;
    tv.tv_sec += tv.tv_usec / 1000000;
    tv.tv_usec = tv.tv_usec % 1000000;

    t->tv_sec = tv.tv_sec;
    t->tv_nsec = tv.tv_usec * 1000;
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
 * Remove the condition node in queue.
 * 
 * @param node the condition node.
 */
inline static void removeConditionNode(struct ConditionNode* head, struct ConditionNode *node) {
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

bool waitCondition(Condition *condition, long timeoutMs) {
    struct timespec t;
    struct timespec *pt = NULL;
    if (timeoutMs == 0) {
        return false;
    } else if (timeoutMs != -1) {
        pt = &t;
        nowAfter(pt, timeoutMs);
    }

    struct ConditionNode *waitNode = newConditionNode();
    if (waitNode == NULL) {
        return false;
    }
    retainConditionNode(waitNode);

    struct ConditionNode *prevNode = condition->waitTail;
    condition->waitTail = waitNode;
    prevNode->next = waitNode;

    int ret = 0;
    pthread_cond_t *cond = &waitNode->condition;
    pthread_mutex_t *mutex = nativeHandleReentrantLock(condition->lock);

    while (waitNode->state == WAITING) {
        if (pt != NULL) {
            ret = pthread_cond_timedwait(cond, mutex, pt);
        } else {
            ret = pthread_cond_wait(cond, mutex);
        }
        // condition await timeout
        if (ret != 0) {
            removeConditionNode(condition->waitHead, waitNode);
            break;
        }
    }

    waitNode->state = INVALID;
    releaseConditionNode(waitNode);
    return ret == 0;
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


