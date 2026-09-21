#ifndef WORKER_H
#define WORKER_H

#include "common.h"
#include "task_queue.h"

/**
 * Worker thread context data passed via pthread_create()
 */
typedef struct {
    int id;                     /* Logical worker ID (1, 2, 3...) */
    pthread_t thread_id;        /* POSIX thread handle */
    task_queue_t *queue;        /* Pointer to shared task queue */
} worker_arg_t;

/**
 * Entry function for worker threads
 * @param arg Pointer to worker_arg_t structure
 * @return NULL on completion
 */
void *worker_function(void *arg);

#endif /* WORKER_H */
