/**
 * ============================================================================
 * Module: Worker Thread Implementation (Phase 7)
 * 
 * OS Concepts Demonstrated:
 * - Event-Driven Concurrency: Condition-variable-based task acquisition.
 * - CO-5 & CO-6: Concurrent threads writing safely to the shared log file.
 * ============================================================================
 */

#include "worker.h"
#include "logger.h"

void *worker_function(void *arg) {
    worker_arg_t *worker = (worker_arg_t *)arg;
    task_queue_t *queue = worker->queue;
    Task task;

    logger_log("WORKER", "[Worker %d] Online | TID: %lu | Waiting on condition variable...",
               worker->id, (unsigned long)pthread_self());

    while (1) {
        /* Condition variable wait happens inside task_queue_pop */
        int has_task = task_queue_pop(queue, &task);
        if (!has_task) {
            /* Shutdown requested and queue drained */
            break;
        }

        logger_log("WORKER", "[Worker %d] Picked Task %d: \"%s\" (duration: %d ms)",
                   worker->id, task.id, task.name, task.duration_ms);

        /* Execute simulated workload outside critical section */
        usleep(task.duration_ms * 1000);

        logger_log("WORKER", "[Worker %d] Completed Task %d: \"%s\"",
                   worker->id, task.id, task.name);
    }

    logger_log("WORKER", "[Worker %d] Received shutdown notification. Terminating.", worker->id);
    pthread_exit(NULL);
}
