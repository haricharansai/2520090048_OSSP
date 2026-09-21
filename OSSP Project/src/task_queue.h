#ifndef TASK_QUEUE_H
#define TASK_QUEUE_H

#include "common.h"

#define MAX_TASK_NAME_LEN 100
#define DEFAULT_QUEUE_CAPACITY 32

/**
 * Task Structure
 * Represents a unit of work submitted to the scheduler.
 */
typedef struct {
    int id;                             /* Unique task identifier (e.g., 101) */
    char name[MAX_TASK_NAME_LEN];       /* Descriptive task name */
    int duration_ms;                    /* Simulated task duration in milliseconds */
} Task;

/**
 * Task Queue Structure
 * Thread-safe FIFO monitor protected by POSIX Mutex and Condition Variable.
 */
typedef struct {
    Task *tasks;                /* Dynamically allocated array of tasks on heap */
    int capacity;               /* Maximum capacity of the queue buffer */
    int front;                  /* Index of the front item */
    int rear;                   /* Index of the rear item */
    int count;                  /* Current number of tasks in the queue */
    int shutdown;               /* Flag indicating shutdown has been requested */
    pthread_mutex_t lock;       /* POSIX Mutex protecting queue critical sections */
    pthread_cond_t not_empty;   /* POSIX Condition Variable signaled when task added */
} task_queue_t;

/**
 * Initialize the task queue, its mutex, and condition variable.
 * @param queue Pointer to task_queue_t
 * @param capacity Maximum number of tasks the queue can hold
 * @return 0 on success, -1 on failure
 */
int task_queue_init(task_queue_t *queue, int capacity);

/**
 * Push a new task into the task queue and signal waiting workers.
 * @param queue Pointer to task_queue_t
 * @param task The task to enqueue
 * @return 1 on success, 0 if queue is full or shut down
 */
int task_queue_push(task_queue_t *queue, Task task);

/**
 * Pop a task from the task queue. Blocks on condition variable if empty until
 * a new task arrives or shutdown is signaled.
 * @param queue Pointer to task_queue_t
 * @param task Pointer to store the dequeued task
 * @return 1 if task successfully retrieved, 0 if shutting down and queue is empty
 */
int task_queue_pop(task_queue_t *queue, Task *task);

/**
 * Signal the queue to shut down and wake up all waiting worker threads.
 * @param queue Pointer to task_queue_t
 */
void task_queue_set_shutdown(task_queue_t *queue);

/**
 * Check if the queue is empty (thread-safe).
 * @param queue Pointer to task_queue_t
 * @return 1 if empty, 0 otherwise
 */
int task_queue_is_empty(task_queue_t *queue);

/**
 * Return current number of tasks in queue (thread-safe).
 * @param queue Pointer to task_queue_t
 * @return Number of tasks
 */
int task_queue_size(task_queue_t *queue);

/**
 * Destroy the synchronization primitives and free queue dynamic memory.
 * @param queue Pointer to task_queue_t
 */
void task_queue_destroy(task_queue_t *queue);

#endif /* TASK_QUEUE_H */
