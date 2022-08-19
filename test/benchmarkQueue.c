#include "FixedThreadPoolExecutor.h"
#include "LinkedBlockingQueue.h"
#include "ArrayBlockingQueue.h"
#include "CountDownLatch.h"

#include <stdatomic.h>
#include <sys/time.h>
#include <stdio.h>

static const int CONSUMERS = 16;
static const int PRODUCERS = 16;
static const size_t QUEUE_SIZE = 1024;
static const int TEST_SIZE = 1000000;

static void benchmarkQueue(BlockingQueue *queue);

struct BenchmarkContext {
    CountDownLatch *latch;
    BlockingQueue *queue;
    int producers;
    int consumers;
    int finished;
    int exits;
};

void benchmarkLinkedBlockingQueue() {
    printf("> linked blocking queue benchmark\n");
    BlockingQueue *queue = newLinkedBlockingQueue(QUEUE_SIZE, sizeof(long long));
    benchmarkQueue(queue);
    queue->free(queue);
}

void benchmarkArrayBlockingQueue() {
    printf("> array blocking queue benchmark\n");
    BlockingQueue *queue = newArrayBlockingQueue(QUEUE_SIZE, sizeof(long long));
    benchmarkQueue(queue);
    queue->free(queue);
}

static void consumerThread(void *arg) {
    struct BenchmarkContext *context = arg;
    BlockingQueue *queue = context->queue;
    CountDownLatch *latch = context->latch;
    for (;;) {
        long long x;
        queue->poll(queue, &x, -1);
        atomic_fetch_add(&context->finished, 1);
        if (x == -1) {
            atomic_fetch_add(&context->exits, 1);
            decreaseCountDownLatch(latch);
            return;
        }
    }
}

static void producerThread(void *arg) {
    struct BenchmarkContext *context = arg;
    BlockingQueue *queue = context->queue;
    CountDownLatch *latch = context->latch;
    for (int i = 0; i < TEST_SIZE; ++i) {
        long long x = i;
        queue->offer(queue, &x, -1);
        atomic_fetch_add(&context->finished, 1);
    }
    if (atomic_fetch_add(&context->exits, 1) + 1 == context->producers) {
        for (int i = 0; i < context->consumers; ++i) {
            long long stop = -1;
            queue->offer(queue, &stop, -1);
        }
    }
    decreaseCountDownLatch(latch);
}

static void showResult(long finished, struct timeval *s, struct timeval *t) {
    double dur = (double) (t->tv_sec - s->tv_sec) * 1000.0 + (double) (t->tv_usec - s->tv_usec) / 1000.0;
    printf("%f mops\n", (double) finished / dur / 1000.0);
}

static void benchmarkQueueMP(BlockingQueue *queue, int producers, int consumers) {
    
    CountDownLatch *latch = newCountDownLatch(producers + consumers);
    ExecutorService *producer = newFixedThreadPoolExecutor(producers, -1, "producer-%d", newLinkedBlockingQueue);
    ExecutorService *consumer = newFixedThreadPoolExecutor(consumers, -1, "consumer-%d", newLinkedBlockingQueue);

    struct BenchmarkContext producerContext = {
            .producers = producers,
            .consumers = consumers,
            .latch = latch,
            .finished = 0,
            .queue = queue,
            .exits = 0,
    };
    
    struct BenchmarkContext consumerContext = {
            .producers = producers,
            .consumers = consumers,
            .latch = latch,
            .finished = 0,
            .queue = queue,
            .exits = 0,
    };

    struct timeval s, t;
    gettimeofday(&s, NULL);
    for (int i = 0; i < producers; ++i) {
        producer->submit(producer, producerThread, &producerContext);

    }
    for (int i = 0; i < consumers; ++i) {
        consumer->submit(consumer, consumerThread, &consumerContext);
    }
    awaitCountDownLatch(latch, -1);
    gettimeofday(&t, NULL);

    showResult(producerContext.finished + consumerContext.finished, &s, &t);

    freeCountDownLatch(latch);
    consumer->free(consumer);
    producer->free(producer);
}

static void benchmarkQueue(BlockingQueue *queue) {
    printf("> spsc test: ");
    fflush(stdout);
    benchmarkQueueMP(queue, 1, 1);
    printf("> spmc test: ");
    fflush(stdout);
    benchmarkQueueMP(queue, 1, CONSUMERS);
    printf("> mpsc test: ");
    fflush(stdout);
    benchmarkQueueMP(queue, PRODUCERS, 1);
    printf("> mpmc test: ");
    fflush(stdout);
    benchmarkQueueMP(queue, PRODUCERS, CONSUMERS);
}