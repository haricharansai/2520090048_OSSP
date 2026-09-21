/**
 * ============================================================================
 * Project: Multithreaded Linux Task Processing System
 * Module:  Main Scheduler Process Entry Point (Phase 8 - Graceful Shutdown)
 * 
 * OS Concepts Demonstrated:
 * - CO-1 (OS as a Service Layer): POSIX Signal Handling (sigaction).
 * - CO-2 (Process Control & Lifecycle): Controlled process termination.
 * - CO-4 (Memory Management): Complete deallocation of process heap allocations.
 * - CO-5 (File Systems & File I/O): Flushing and safely closing file streams.
 * - CO-6 (Concurrency & Synchronization):
 *   - Graceful thread shutdown via broadcast signaling.
 *   - Deadlock-free thread resource reclamation using pthread_join().
 *   - Safe destruction of mutex and condition variable synchronization primitives.
 * ============================================================================
 */

#include "common.h"
#include "task_queue.h"
#include "worker.h"
#include "logger.h"
#include "race_demo.h"

/* Global volatile flag modified by Linux asynchronous signal handler */
static volatile sig_atomic_t g_shutdown_signal = 0;
static task_queue_t *g_queue_ptr = NULL;

/**
 * POSIX Signal Handler for SIGINT (Ctrl+C) and SIGTERM.
 * Ensures the application shuts down safely without abrupt thread abortion.
 */
static void handle_signal(int sig) {
    (void)sig;
    g_shutdown_signal = 1;
    if (g_queue_ptr) {
        task_queue_set_shutdown(g_queue_ptr);
    }
}

static void setup_signal_handlers(void) {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_signal;
    sigemptyset(&sa.sa_mask);

    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("[Warning] Could not set up SIGINT handler");
    }
    if (sigaction(SIGTERM, &sa, NULL) == -1) {
        perror("[Warning] Could not set up SIGTERM handler");
    }
}

static void print_banner(void) {
    printf("========================================================\n");
    printf("  %s\n", PROJECT_TITLE);
    printf("  Version: %s\n", PROJECT_VERSION);
    printf("========================================================\n");
}

static void print_process_info(int num_workers, int num_tasks) {
    printf("[System Init] Process initialized successfully.\n");
    printf("[System Info] Current Process ID (PID)        : %d\n", getpid());
    printf("[System Info] Parent Process ID (PPID)        : %d\n", getppid());
    printf("[System Info] Active Worker Threads           : %d\n", num_workers);
    printf("[System Info] Total Tasks to Process          : %d\n", num_tasks);
    printf("[System Info] Persistent Log Output           : %s\n", LOG_FILE_PATH);
    printf("--------------------------------------------------------\n");
}

static void print_usage(const char *prog_name) {
    printf("Usage:\n");
    printf("  %s                          Run standard task scheduler (3 workers, 6 tasks)\n", prog_name);
    printf("  %s --workers <N> --tasks <M> Run with custom worker and task counts\n", prog_name);
    printf("  %s --race-demo              Run controlled race condition comparison\n", prog_name);
    printf("  %s --help                   Show this help message\n\n", prog_name);
}

static int run_scheduler(int num_workers, int num_custom_tasks) {
    /* 1. Initialize Thread-Safe Persistent Logger (CO-5) */
    if (logger_init(LOG_FILE_PATH) != 0) {
        fprintf(stderr, "[Warning] Could not initialize logger at %s\n", LOG_FILE_PATH);
    }

    /* 2. Initialize the Shared Task Queue (CO-4 Heap Allocation & CO-6 Primitives) */
    task_queue_t queue;
    if (task_queue_init(&queue, DEFAULT_QUEUE_CAPACITY) != 0) {
        fprintf(stderr, "[Error] Failed to initialize task queue.\n");
        logger_close();
        return EXIT_FAILURE;
    }
    g_queue_ptr = &queue;

    /* Base sample tasks list */
    Task base_tasks[] = {
        {101, "File System Compression", 150},
        {102, "Kernel Log Parsing",       100},
        {103, "Process Memory Analysis",  180},
        {104, "Cryptographic Hash Calc",  120},
        {105, "Network Packet Inspection", 90},
        {106, "Database Index Sync",      140}
    };
    int base_count = sizeof(base_tasks) / sizeof(base_tasks[0]);
    int total_tasks = (num_custom_tasks > 0) ? num_custom_tasks : base_count;

    print_process_info(num_workers, total_tasks);
    logger_log("SCHEDULER", "Starting scheduler process with %d workers and %d tasks.",
               num_workers, total_tasks);

    /* 3. Spawn Worker Threads (CO-6 Threads waiting on condition variable) */
    worker_arg_t *workers = (worker_arg_t *)malloc(sizeof(worker_arg_t) * num_workers);
    if (!workers) {
        perror("[Error] malloc failed for worker arguments");
        task_queue_destroy(&queue);
        logger_close();
        return EXIT_FAILURE;
    }

    printf("[Scheduler] Spawning %d worker threads...\n", num_workers);
    for (int i = 0; i < num_workers; i++) {
        workers[i].id = i + 1;
        workers[i].queue = &queue;

        if (pthread_create(&workers[i].thread_id, NULL, worker_function, (void *)&workers[i]) != 0) {
            fprintf(stderr, "[Error] Failed to create Worker %d\n", i + 1);
            task_queue_set_shutdown(&queue);
            for (int j = 0; j < i; j++) {
                pthread_join(workers[j].thread_id, NULL);
            }
            free(workers);
            task_queue_destroy(&queue);
            logger_close();
            return EXIT_FAILURE;
        }
    }

    /* Small delay allowing worker threads to transition to condition variable wait state */
    usleep(50000);
    printf("--------------------------------------------------------\n");

    /* 4. Producer Task Dispatching */
    for (int i = 0; i < total_tasks; i++) {
        if (g_shutdown_signal) {
            logger_log("SCHEDULER", "Signal received! Aborting remaining task submissions.");
            break;
        }

        Task task;
        if (i < base_count) {
            task = base_tasks[i];
        } else {
            task.id = 100 + i + 1;
            snprintf(task.name, sizeof(task.name), "Dynamic Background Job #%d", i + 1);
            task.duration_ms = 80 + ((i * 37) % 120);
        }

        logger_log("PRODUCER", "Dispatched Task %d: \"%s\" (duration: %d ms)",
                   task.id, task.name, task.duration_ms);
        task_queue_push(&queue, task);
        usleep(35000);
    }

    /* Wait until all queued tasks are picked up by workers or shutdown requested */
    while (!g_shutdown_signal && task_queue_size(&queue) > 0) {
        usleep(20000);
    }

    /* 5. Graceful Shutdown Phase (CO-6 Broadcast & Thread Joining) */
    printf("--------------------------------------------------------\n");
    logger_log("SCHEDULER", "Initiating graceful worker shutdown sequence...");
    task_queue_set_shutdown(&queue);

    /* Join each worker thread to reclaim stack and OS thread descriptors */
    for (int i = 0; i < num_workers; i++) {
        pthread_join(workers[i].thread_id, NULL);
        logger_log("SCHEDULER", "Worker %d joined and reaped successfully.", workers[i].id);
    }

    printf("--------------------------------------------------------\n");
    logger_log("SCHEDULER", "All %d worker threads terminated safely.", num_workers);

    /* 6. Free Heap Allocations and Destroy Synchronization Primitives (CO-4 & CO-6) */
    free(workers);
    g_queue_ptr = NULL;
    task_queue_destroy(&queue);
    logger_log("SCHEDULER", "Task queue destroyed and synchronization primitives freed.");

    /* 7. Safely close log file (CO-5) */
    logger_close();
    printf("[Scheduler] Process completed successfully. Log persisted to: %s\n\n", LOG_FILE_PATH);

    return EXIT_SUCCESS;
}

int main(int argc, char *argv[]) {
    print_banner();
    setup_signal_handlers();

    int num_workers = NUM_WORKERS;
    int num_tasks = 0;

    if (argc > 1) {
        if (strcmp(argv[1], "--race-demo") == 0) {
            run_race_condition_demo();
            return EXIT_SUCCESS;
        } else if (strcmp(argv[1], "--help") == 0) {
            print_usage(argv[0]);
            return EXIT_SUCCESS;
        } else if (strcmp(argv[1], "--workers") == 0 && argc >= 3) {
            num_workers = atoi(argv[2]);
            if (num_workers <= 0 || num_workers > 32) {
                fprintf(stderr, "[Error] Invalid worker count (1-32): %s\n", argv[2]);
                return EXIT_FAILURE;
            }
            if (argc >= 5 && strcmp(argv[3], "--tasks") == 0) {
                num_tasks = atoi(argv[4]);
            }
        } else {
            fprintf(stderr, "[Error] Unknown argument: %s\n\n", argv[1]);
            print_usage(argv[0]);
            return EXIT_FAILURE;
        }
    }

    return run_scheduler(num_workers, num_tasks);
}
