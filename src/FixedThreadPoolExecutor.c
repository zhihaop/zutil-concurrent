#include "FixedThreadPoolExecutor.h"

#include <stdatomic.h>
#include <pthread.h>
#include <malloc.h>

typedef struct FixedThreadPoolExecutor {
    BlockingQueue *taskQueue;

    size_t poolSize;
    bool shutdown;

    pthread_t threads[];
} FixedThreadPoolExecutor;

enum TaskState {
    RUNNING,
    STOP
};

/**
 * Represents the runnable closure and its context.
 */
typedef struct Task {
    void (*fn)(void *);

    void *arg;

    enum TaskState state;
} Task;


static void *start_routine(void *arg) {
    FixedThreadPoolExecutor *pool = arg;
    BlockingQueue *queue = pool->taskQueue;
    Task r;

    for (;;) {
        if (!queue->poll(queue, &r, -1)) {
            continue;
        }

        if (r.state == STOP) {
            return NULL;
        }
        r.fn(r.arg);
    }
}

FixedThreadPoolExecutor *newExecutor(size_t corePoolSize, size_t taskQueueSize, BlockingQueueBuilder builder) {
    FixedThreadPoolExecutor *pool = calloc(1, sizeof(FixedThreadPoolExecutor) + sizeof(pthread_t) * corePoolSize);
    if (pool == NULL) {
        return NULL;
    }

    pool->poolSize = 0;
    pool->taskQueue = builder(taskQueueSize, sizeof(Task));
    atomic_init(&pool->shutdown, true);

    if (pool->taskQueue == NULL) {
        executorFree(pool);
        return NULL;
    }

    atomic_store(&pool->shutdown, false);

    for (int i = 0; i < corePoolSize; ++i) {
        if (pthread_create(&pool->threads[i], NULL, start_routine, pool) != 0) {
            executorShutdown(pool);
            executorFree(pool);
            return NULL;
        }
        pool->poolSize += 1;
    }

    return pool;
}

bool executorSubmit(FixedThreadPoolExecutor *pool, void (*fn)(void *), void *arg) {
    if (atomic_load(&pool->shutdown)) {
        return false;
    }
    Task r = {.fn = fn, .arg = arg, .state = RUNNING};
    if (!pool->taskQueue->offer(pool->taskQueue, &r, 0)) {
        fn(arg);
    }
    return true;
}

void executorFree(FixedThreadPoolExecutor *pool) {
    executorShutdown(pool);
    pool->taskQueue->free(pool->taskQueue);
    free(pool);
}

void executorShutdown(FixedThreadPoolExecutor *pool) {
    bool legacy_state;
    atomic_init(&legacy_state, false);
    if (atomic_compare_exchange_strong(&pool->shutdown, &legacy_state, true)) {

        Task stop = {.fn = NULL, .arg = NULL, .state = STOP};
        for (int i = 0; i < pool->poolSize; ++i) {
            pool->taskQueue->offer(pool->taskQueue, &stop, -1);
        }

        for (int i = 0; i < pool->poolSize; ++i) {
            pthread_join(pool->threads[i], NULL);
        }
    }
}

