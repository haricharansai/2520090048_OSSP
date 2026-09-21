/**
 * ============================================================================
 * Module: Monitor-Based Task Queue with Condition Variables (Phase 6)
 * 
 * OS Concepts Demonstrated:
 * - CO-6 (Thread Coordination & Producer-Consumer Model):
 *   - pthread_cond_init(): Initializes condition variable.
 *   - pthread_cond_wait(): Atomically releases mutex and blocks thread on wait queue.
 *     Prevents CPU-intensive busy-waiting (spinning).
 *   - pthread_cond_signal(): Wakes up at least one waiting worker when task arrives.
 *   - pthread_cond_broadcast(): Wakes up all sleeping workers on shutdown.
 *   - pthread_cond_destroy(): Deallocates condition variable resources.
 *   - Spurious Wakeup Guard: Always uses `while (condition)` loop around `pthread_cond_wait`.
 * ============================================================================
 */

#include "task_queue.h"

int task_queue_init(task_queue_t *queue, int capacity) {
    if (!queue || capacity <= 0) {
        return -1;
    }

    /* Allocate dynamic memory for tasks on heap */
    queue->tasks = (Task *)malloc(sizeof(Task) * capacity);
    if (!queue->tasks) {
        perror("[Error] malloc failed for task_queue");
        return -1;
    }

    queue->capacity = capacity;
    queue->front = 0;
    queue->rear = 0;
    queue->count = 0;
    queue->shutdown = 0;

    /* Initialize the POSIX Mutex */
    if (pthread_mutex_init(&queue->lock, NULL) != 0) {
        perror("[Error] pthread_mutex_init failed");
        free(queue->tasks);
        queue->tasks = NULL;
        return -1;
    }

    /* Initialize the POSIX Condition Variable */
    if (pthread_cond_init(&queue->not_empty, NULL) != 0) {
        perror("[Error] pthread_cond_init failed");
        pthread_mutex_destroy(&queue->lock);
        free(queue->tasks);
        queue->tasks = NULL;
        return -1;
    }

    return 0;
}

int task_queue_push(task_queue_t *queue, Task task) {
    if (!queue || !queue->tasks) {
        return 0;
    }

    /* ENTER CRITICAL SECTION */
    pthread_mutex_lock(&queue->lock);

    if (queue->shutdown || queue->count >= queue->capacity) {
        /* Queue cannot accept more tasks */
        pthread_mutex_unlock(&queue->lock);
        return 0;
    }

    queue->tasks[queue->rear] = task;
    queue->rear = (queue->rear + 1) % queue->capacity;
    queue->count++;

    /* SIGNAL: Notify one waiting worker thread that a new task is available */
    pthread_cond_signal(&queue->not_empty);

    /* EXIT CRITICAL SECTION */
    pthread_mutex_unlock(&queue->lock);

    return 1;
}

int task_queue_pop(task_queue_t *queue, Task *task) {
    if (!queue || !queue->tasks) {
        return 0;
    }

    /* ENTER CRITICAL SECTION */
    pthread_mutex_lock(&queue->lock);

    /*
     * SPURIOUS WAKEUP GUARD:
     * While queue is empty and shutdown is not requested, sleep on condition variable.
     * pthread_cond_wait atomically unlocks &queue->lock and sleeps the thread.
     * Upon awakening, it re-acquires &queue->lock before returning.
     */
    while (queue->count == 0 && !queue->shutdown) {
        pthread_cond_wait(&queue->not_empty, &queue->lock);
    }

    /* If queue is empty and shutdown is in progress, exit gracefully */
    if (queue->count == 0 && queue->shutdown) {
        pthread_mutex_unlock(&queue->lock);
        return 0;
    }

    /* Dequeue task */
    if (task) {
        *task = queue->tasks[queue->front];
    }

    queue->front = (queue->front + 1) % queue->capacity;
    queue->count--;

    /* EXIT CRITICAL SECTION */
    pthread_mutex_unlock(&queue->lock);

    return 1;
}

void task_queue_set_shutdown(task_queue_t *queue) {
    if (!queue) return;

    pthread_mutex_lock(&queue->lock);
    queue->shutdown = 1;
    /* BROADCAST: Wake up all sleeping workers so they can observe shutdown and exit */
    pthread_cond_broadcast(&queue->not_empty);
    pthread_mutex_unlock(&queue->lock);
}

int task_queue_is_empty(task_queue_t *queue) {
    if (!queue) return 1;

    pthread_mutex_lock(&queue->lock);
    int empty = (queue->count == 0);
    pthread_mutex_unlock(&queue->lock);

    return empty;
}

int task_queue_size(task_queue_t *queue) {
    if (!queue) return 0;

    pthread_mutex_lock(&queue->lock);
    int size = queue->count;
    pthread_mutex_unlock(&queue->lock);

    return size;
}

void task_queue_destroy(task_queue_t *queue) {
    if (queue) {
        /* Destroy Condition Variable and Mutex */
        pthread_cond_destroy(&queue->not_empty);
        pthread_mutex_destroy(&queue->lock);

        if (queue->tasks) {
            free(queue->tasks);
            queue->tasks = NULL;
        }
        queue->capacity = 0;
        queue->front = 0;
        queue->rear = 0;
        queue->count = 0;
        queue->shutdown = 0;
    }
}
