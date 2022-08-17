#ifndef ZUTIL_CONCURRENT_EXECUTORSERVICE_H
#define ZUTIL_CONCURRENT_EXECUTORSERVICE_H

#ifdef __cplusplus
extern "C" {
#else

#include <stdbool.h>

#endif

typedef struct ExecutorService {
    /**
     * Submit a Task to the executor service.
     * 
     * @param executor      the executor to submit.
     * @param fn            the function to submit.
     * @param arg           the parameter of the function.
     * @return              return false if failed.
     */
    bool (*const submit)(struct ExecutorService *executor, void (*fn)(void *), void *arg);

    /**
     * Wait all the tasks finished, and shutdown the executor service.
     * 
     * @param executor      the executor service.
     */
    void (*const shutdown)(struct ExecutorService *executor);

    /**
     * Shutdown and free the executor service.
     * 
     * @param executor      the executor service.
     */
    void (*const free)(struct ExecutorService *executor);

    /**
     * Check if the executor service is shutdown.
     * 
     * @param executor      the executor service.
     * @return              return true if the the executor service is shutdown.
     */
    bool (*const isShutdown)(struct ExecutorService *executor);
} ExecutorService;


#ifdef __cplusplus
}
#endif

#endif //ZUTIL_CONCURRENT_EXECUTORSERVICE_H
