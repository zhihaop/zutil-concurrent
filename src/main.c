#include "FixedThreadPoolExecutor.h"
#include "LinkedBlockingQueue.h"
#include "ArrayBlockingQueue.h"

#include <stdatomic.h>
#include <stdio.h>

#include <sys/time.h>

void foo(void *arg);
void executorExample();
void arrayBlockingQueueExample();
void linkedBlockingQueueExample();
void blockingQueueExample(BlockingQueue *queue, int queueSize);

int main() {
    executorExample();
    linkedBlockingQueueExample();
    arrayBlockingQueueExample();
}

void foo(void *arg) {
    int *counter = arg;
    atomic_fetch_add(counter, 1);
}

void executorExample() {
    printf("> executor test\n");

    int corePoolSize = 16;
    int taskQueueSize = 32;
    int taskCount = 10000000;
    int finished = 0;
    FixedThreadPoolExecutor *pool = newExecutor(corePoolSize, taskQueueSize, newLinkedBlockingQueue);

    struct timeval tv0;
    gettimeofday(&tv0, NULL);

    for (int i = 0; i < taskCount; ++i) {
        executorSubmit(pool, foo, &finished);
    }

    executorShutdown(pool);

    struct timeval tv1;
    gettimeofday(&tv1, NULL);

    double diff = (tv1.tv_sec - tv0.tv_sec) * 1000.0 + (tv1.tv_usec - tv0.tv_usec) / 1000.0;

    printf("number of finished tasks = %d, elapsed time = %f ms\n", finished, diff);

    executorFree(pool);
}

void linkedBlockingQueueExample() {
    printf("> linked blocking queue test\n");
    int queueSize = 12;
    BlockingQueue *queue = newLinkedBlockingQueue(queueSize, sizeof(int));
    blockingQueueExample(queue, queueSize);
}

void arrayBlockingQueueExample() {
    printf("> array blocking queue test\n");
    int queueSize = 12;
    BlockingQueue *queue = newArrayBlockingQueue(queueSize, sizeof(int));
    blockingQueueExample(queue, queueSize);
}

void blockingQueueExample(BlockingQueue *queue, int queueSize) {
    // test offer
    for (int i = 0; i < queueSize; ++i) {
        printf("queue.offer(%d)\n", i);
        queue->offer(queue, &i, -1);
    }

    // test offer timeout
    int num = queueSize;
    if (!queue->offer(queue, &num, 1000)) {
        printf("timeout (1000 ms): queue->offer(%d)\n", num);
    }

    // test poll
    for (int i = 0; i < queueSize; ++i) {
        int x;
        queue->poll(queue, &x, -1);
        printf("queue.pool() = %d\n", x);
    }

    // test poll timeout
    if (!queue->poll(queue, &num, 1000)) {
        printf("timeout (1000 ms): queue->poll() = null\n");
    }
}