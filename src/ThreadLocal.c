#include "ThreadLocal.h"
#include <pthread.h>
#include <malloc.h>

struct ThreadLocal {
    pthread_key_t key;
};

struct ThreadLocalValue {
    void *item;

    void (*deleter)(void *);
};

static void freeThreadLocalValue(void *ptr) {
    if (ptr == NULL) {
        return;
    }

    struct ThreadLocalValue *value = ptr;
    if (value->item == NULL) {
        free(value);
        return;
    }

    if (value->deleter == NULL) {
        free(value);
        return;
    }

    value->deleter(value->item);
    free(value);
}

bool setThreadLocal(ThreadLocal *threadLocal, void *item, void (*deleter)(void *)) {
    struct ThreadLocalValue *value = pthread_getspecific(threadLocal->key);
    if (value == NULL) {
        return false;
    }

    if (value->item != NULL) {
        if (value->deleter != NULL) {
            value->deleter(value->item);
        }
    }

    value->item = item;
    value->deleter = deleter;
}

void *getThreadLocal(ThreadLocal *threadLocal) {
    struct ThreadLocalValue *value = pthread_getspecific(threadLocal->key);
    if (value == NULL) {
        return false;
    }
    return value->item;
}

bool initThreadLocal(ThreadLocal *threadLocal) {
    if (pthread_key_create(&threadLocal->key, freeThreadLocalValue)) {
        return false;
    }
    struct ThreadLocalValue *value = calloc(1, sizeof(struct ThreadLocalValue));
    if (value == NULL) {
        pthread_key_delete(threadLocal->key);
    }
    pthread_setspecific(threadLocal->key, value);
}
