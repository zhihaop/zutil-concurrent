#include "CountDownLatch.h"
#include "ReentrantLock.h"
#include "Condition.h"

#include <stdatomic.h>
#include <malloc.h>

struct CountDownLatch {
    ReentrantLock *lock;
    Condition *condition;
    int count;
};

CountDownLatch *newCountDownLatch(int count) {
    CountDownLatch *latch = calloc(1, sizeof(CountDownLatch));
    if (latch == NULL) {
        return NULL;
    }

    atomic_init(&latch->count, count);

    latch->lock = newReentrantLock();
    if (latch->lock == NULL) {
        freeCountDownLatch(latch);
        return NULL;
    }

    latch->condition = newCondition(latch->lock);
    if (latch->condition == NULL) {
        freeCountDownLatch(latch);
        return NULL;
    }
    return latch;
}

void freeCountDownLatch(CountDownLatch *latch) {
    if (latch->lock) {
        freeReentrantLock(latch->lock);
    }
    if (latch->condition) {
        freeCondition(latch->condition);
    }
    free(latch);
}

void decreaseCountDownLatch(CountDownLatch *latch) {
    if (atomic_fetch_sub(&latch->count, 1) == 1) {
        lockReentrantLock(latch->lock);
        signalAllCondition(latch->condition);
        unlockReentrantLock(latch->lock);
    }
}

bool awaitCountDownLatch(CountDownLatch *latch, long timeoutMs) {
    lockReentrantLock(latch->lock);
    while (atomic_load(&latch->count) != 0) {
        timeoutMs = awaitCondition(latch->condition, timeoutMs);
        if (timeoutMs == 0) {
            unlockReentrantLock(latch->lock);
            return false;
        }
    }
    unlockReentrantLock(latch->lock);
    return true;
}
