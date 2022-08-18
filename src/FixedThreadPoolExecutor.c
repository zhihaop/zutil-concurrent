#include "FixedThreadPoolExecutor.h"

#include <stdatomic.h>
#include <pthread.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>

#define THREAD_NAME_MAX_LENGTH 64

// forward declaration
struct FixedThreadPoolExecutor;

/**
 * The state of the task and the executor.
 */
enum TaskState {
    TASK_STATE_RUNNING,
    TASK_STATE_SHUTDOWN
};

typedef struct ThreadContext {
    struct FixedThreadPoolExecutor *executor;
    char name[THREAD_NAME_MAX_LENGTH];
    pthread_t thread;
    size_t thread_id;
} ThreadContext;

/**
 * Represents the runnable closure and its context.
 */
typedef struct Task {
    void (*fn)(void *);
    void *arg;
    enum TaskState state;
} Task;

/**
 * An implementation of FixedThreadPoolExecutor.
 */
typedef struct FixedThreadPoolExecutor {
    ExecutorService parent;
    BlockingQueue *queue;
    size_t threadSize;
    enum TaskState s;

    ThreadContext contexts[];
} FixedThreadPoolExecutor;

/* member functions */
static void *executorThread(void *arg);
static void executorFree(FixedThreadPoolExecutor *executor);
static void executorShutdown(FixedThreadPoolExecutor *executor);
static bool executorGetShutdown(FixedThreadPoolExecutor *executor);
static bool executorSubmit(FixedThreadPoolExecutor *executor, void (*fn)(void *), void *arg);

ExecutorService
*newFixedThreadPoolExecutor(size_t threadSize,
                            size_t taskQueueSize,
                            const char *format,
                            BlockingQueueBuilder builder) {

    FixedThreadPoolExecutor *executor = calloc(1, sizeof(FixedThreadPoolExecutor) + sizeof(ThreadContext) * threadSize);
    if (executor == NULL) {
        return NULL;
    }

    // member function binding
    ExecutorService parent = {
            .free = (void (*)(struct ExecutorService *)) executorFree,
            .shutdown = (void (*)(struct ExecutorService *)) executorShutdown,
            .submit = (bool (*)(struct ExecutorService *, void (*)(void *), void *)) executorSubmit,
            .isShutdown = (bool (*)(struct ExecutorService *)) executorGetShutdown
    };
    memcpy(&executor->parent, &parent, sizeof(ExecutorService));
    
    executor->threadSize = 0;
    executor->queue = builder(taskQueueSize, sizeof(Task));
    atomic_init(&executor->s, TASK_STATE_SHUTDOWN);

    if (executor->queue == NULL) {
        executorFree(executor);
        return NULL;
    }

    atomic_store(&executor->s, TASK_STATE_RUNNING);

    for (int i = 0; i < threadSize; ++i) {
        ThreadContext *context = &executor->contexts[i];

        context->thread_id = i;
        context->executor = executor;

        if (strstr(format, "%d") != NULL) {
            sprintf(context->name, format, context->thread_id);
        } else {
            strcpy(context->name, format);
        }

        if (pthread_create(&context->thread, NULL, executorThread, context) != 0) {
            executorFree(executor);
            return NULL;
        }
        executor->threadSize += 1;
    }

    return &executor->parent;
}

static void *executorThread(void *arg) {
    ThreadContext *context = arg;
    FixedThreadPoolExecutor *executor = context->executor;
    BlockingQueue *queue = executor->queue;
    Task r;
    
#ifdef _GNU_SOURCE
    pthread_setname_np(pthread_self(), context->name);
#endif
    
    for (;;) {
        if (!queue->poll(queue, &r, -1)) {
            continue;
        }

        if (r.state == TASK_STATE_SHUTDOWN) {
            return NULL;
        }
        r.fn(r.arg);
    }
}

static bool executorSubmit(FixedThreadPoolExecutor *executor, void (*fn)(void *), void *arg) {
    if (atomic_load(&executor->s) == TASK_STATE_SHUTDOWN) {
        return false;
    }
    Task r = {.fn = fn, .arg = arg, .state = TASK_STATE_RUNNING};
    return executor->queue->offer(executor->queue, &r, 0);
}

static void executorFree(FixedThreadPoolExecutor *executor) {
    executorShutdown(executor);
    if (executor->queue) {
        executor->queue->free(executor->queue);
    }
    free(executor);
}

static void executorShutdown(FixedThreadPoolExecutor *executor) {
    enum TaskState state = TASK_STATE_RUNNING;
    if (atomic_compare_exchange_strong(&executor->s, &state, TASK_STATE_SHUTDOWN)) {
        BlockingQueue *queue = executor->queue;

        Task stop = {.fn = NULL, .arg = NULL, .state = TASK_STATE_SHUTDOWN};
        for (int i = 0; i < executor->threadSize; ++i) {
            queue->offer(queue, &stop, -1);
        }

        for (int i = 0; i < executor->threadSize; ++i) {
            pthread_join(executor->contexts[i].thread, NULL);
        }
    }
}

static bool executorGetShutdown(FixedThreadPoolExecutor *executor) {
    return atomic_load(&executor->s) == TASK_STATE_SHUTDOWN;
}

