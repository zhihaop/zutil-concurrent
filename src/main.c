#include "FixedThreadPoolExecutor.h"
#include "LinkedBlockingQueue.h"

#include <stdatomic.h>
#include <stdio.h>

#include <sys/time.h>

static int counter;

void foo(void *arg) {
    atomic_fetch_add(&counter, 1);
}

void executorExample() {
    int corePoolSize = 16;
    int taskQueueSize = 32;
    int taskCount = 10000000;

    FixedThreadPoolExecutor *pool = newExecutor(corePoolSize, taskQueueSize, newLinkedBlockingQueue);

    struct timeval tv0;
    gettimeofday(&tv0, NULL);

    for (int i = 0; i < taskCount; ++i) {
        executorSubmit(pool, foo, NULL);
    }

    executorShutdown(pool);

    struct timeval tv1;
    gettimeofday(&tv1, NULL);

    double diff = (tv1.tv_sec - tv0.tv_sec) * 1000.0 + (tv1.tv_usec - tv0.tv_usec) / 1000.0;

    printf("number of tasks=%d, elapsed time=%f ms\n", taskCount, diff);

    executorFree(pool);
}

void blockingQueueExample() {
    BlockingQueue *queue = newLinkedBlockingQueue(12, sizeof(int));

    for (int i = 0; i < 12; ++i) {
        printf("queue.offer(%d)\n", i);
        queue->offer(queue, &i, -1);
    }

    for (int i = 0; i < 12; ++i) {
        int x;
        queue->poll(queue, &x, -1);
        printf("queue.pool() = %d\n", x);
    }
}

int main() {
    executorExample();
    blockingQueueExample();
}