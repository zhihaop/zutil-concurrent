#include "ThreadLocal.h"
#include <stdatomic.h>
#include <pthread.h>
#include <malloc.h>
#include <stdio.h>

struct ThreadLocalValue {
    ThreadLocal *key;

    void *item;

    void (*deleter)(void *);
};


static void freeThreadLocalValue(void *ptr) {
    if (ptr == NULL) {
        return;
    }

    struct ThreadLocalValue *value = ptr;
    if (value->item) {
        if (value->deleter) {
            value->deleter(value->item);
        }
    }
    free(value);
}

inline static bool createThreadStorageIfAbsent(ThreadLocal *threadLocal) {
    if (!atomic_load(&threadLocal->initialized)) {
        pthread_mutex_lock(&threadLocal->mutex);
        if (!atomic_load(&threadLocal->initialized)) {
            if (pthread_key_create(&threadLocal->key, freeThreadLocalValue)) {
                pthread_mutex_unlock(&threadLocal->mutex);
                return false;
            }
        }
        atomic_store(&threadLocal->initialized, true);
        pthread_mutex_unlock(&threadLocal->mutex);
    }
    return true;
}

inline static struct ThreadLocalValue *computeIfAbsentThreadLocalValue(ThreadLocal *threadLocal) {
    struct ThreadLocalValue *value = pthread_getspecific(threadLocal->key);
    if (value == NULL) {
        value = calloc(1, sizeof(struct ThreadLocalValue));
        if (value == NULL) {
            return NULL;
        }

        if (pthread_setspecific(threadLocal->key, value)) {
            free(value);
            return NULL;
        }
        value->key = threadLocal;
    }
    return value;
}

inline static bool setThreadLocalFast(ThreadLocal *threadLocal, void *item, void (*deleter)(void *)) {
    struct ThreadLocalValue *value = computeIfAbsentThreadLocalValue(threadLocal);
    if (value == NULL) {
        return NULL;
    }

    if (value->item != NULL) {
        if (value->deleter != NULL) {
            value->deleter(value->item);
        }
    }

    value->item = item;
    value->deleter = deleter;
    return true;
}

bool setThreadLocal(ThreadLocal *threadLocal, void *item, void (*deleter)(void *)) {
    if (!createThreadStorageIfAbsent(threadLocal)) {
        return false;
    }
    return setThreadLocalFast(threadLocal, item, deleter);
}

inline static void *getThreadLocalFast(ThreadLocal *threadLocal) {
    struct ThreadLocalValue *value = computeIfAbsentThreadLocalValue(threadLocal);
    if (value == NULL) {
        return NULL;
    }
    return value->item;
}

void *getThreadLocal(ThreadLocal *threadLocal) {
    if (!createThreadStorageIfAbsent(threadLocal)) {
        return NULL;
    }
    return getThreadLocalFast(threadLocal);
}

void *
computeIfAbsentThreadLocal(ThreadLocal *threadLocal, void *(*builder)(void *), void *arg, void (*deleter)(void *)) {
    if (!createThreadStorageIfAbsent(threadLocal)) {
        return NULL;
    }

    void *item = getThreadLocalFast(threadLocal);
    if (item != NULL) {
        return item;
    }

    if (!builder) {
        return NULL;
    }

    item = builder(arg);
    if (item == NULL) {
        return NULL;
    }

    if (!setThreadLocalFast(threadLocal, item, deleter)) {
        if (deleter) {
            deleter(item);
            return NULL;
        }
    }
    return item;
}

void destroyThreadLocal(ThreadLocal *threadLocal) {
    bool initialized = true;
    if (atomic_compare_exchange_strong(&threadLocal->initialized, &initialized, false)) {
        pthread_key_delete(threadLocal->key);
        threadLocal->key = -1;
    }
}

