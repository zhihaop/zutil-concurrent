#include "FixedThreadPoolExecutor.h"
#include "LinkedBlockingQueue.h"
#include "ArrayBlockingQueue.h"

#include <stdatomic.h>
#include <stdio.h>

#include <sys/time.h>

void executorExample();
void arrayBlockingQueueExample();
void linkedBlockingQueueExample();
void benchmarkArrayBlockingQueue();
void benchmarkLinkedBlockingQueue();

void blockingQueueExample(BlockingQueue *queue, int queueSize);

int main() {
    executorExample();
    arrayBlockingQueueExample();
    linkedBlockingQueueExample();
    benchmarkArrayBlockingQueue();
    benchmarkLinkedBlockingQueue();
}

void foo(void *arg) {
    int *counter = arg;
    atomic_fetch_add(counter, 1);
}

void executorExample() {
    printf("> executor test\n");

    size_t corePoolSize = 16;
    size_t taskQueueSize = 32;
    int taskCount = 10000000;
    int taskFinish = 0;

    // LinkedBlockingQueue supports unbounded capacity, taskQueueSize == BLOCKING_QUEUE_UNBOUNDED means
    // blocking queue with unbounded size.
    ExecutorService *pool = newFixedThreadPoolExecutor(corePoolSize, taskQueueSize, "test-%d", newLinkedBlockingQueue);

    struct timeval tv0;
    gettimeofday(&tv0, NULL);

    for (int i = 0; i < taskCount; ++i) {
        if (!pool->submit(pool, foo, &taskFinish)) {
            // equivalent to CallerRunPolicy
            if (!pool->isShutdown(pool)) {
                foo(&taskFinish);
            }
        }
    }

    pool->shutdown(pool);

    struct timeval tv1;
    gettimeofday(&tv1, NULL);

    double diff = (double) (tv1.tv_sec - tv0.tv_sec) * 1000.0 + (double) (tv1.tv_usec - tv0.tv_usec) / 1000.0;

    printf("number of finished tasks = %d, elapsed time = %f ms\n", taskFinish, diff);

    pool->free(pool);
}

void linkedBlockingQueueExample() {
    printf("> linked blocking queue test\n");
    int queueSize = 12;
    BlockingQueue *queue = newLinkedBlockingQueue(queueSize, sizeof(int));
    blockingQueueExample(queue, queueSize);
    queue->free(queue);

    // ArrayBlockingQueue only support bounded capacity
    BlockingQueue *null = newArrayBlockingQueue(BLOCKING_QUEUE_UNBOUNDED, sizeof(int));
    if (null != NULL) {
        fprintf(stderr, "newArrayBlockingQueue(BLOCKING_QUEUE_UNBOUNDED, sizeof(int)) should return NULL\n");
    }
}

void arrayBlockingQueueExample() {
    printf("> array blocking queue test\n");
    int queueSize = 12;
    BlockingQueue *queue = newArrayBlockingQueue(queueSize, sizeof(int));
    blockingQueueExample(queue, queueSize);
    queue->free(queue);
}

void blockingQueueExample(BlockingQueue *queue, int queueSize) {
    // test offer
    for (int i = 0; i < queueSize; ++i) {
        printf("queue.offer(%d)\n", i);
        queue->offer(queue, &i, -1);
    }

    // test offer timeout
    int num = -1;
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