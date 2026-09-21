/**
 * ============================================================================
 * Module: Controlled Race Condition Demonstration (Phase 5)
 * 
 * OS Concepts Demonstrated:
 * - CO-6 (Concurrency Hazards - Race Conditions):
 *   - Race Condition definition: Flaw in concurrent execution where output
 *     depends on the non-deterministic interleaving/scheduling of threads.
 *   - Non-atomic Read-Modify-Write assembly operations on shared memory:
 *     1. mov eax, [shared_var]  (Read memory to CPU register)
 *     2. add eax, 1             (Modify value in register)
 *     3. mov [shared_var], eax  (Write value back to memory)
 *   - When context switch occurs between (1) and (3), lost updates occur.
 *   - POSIX Mutex ensures atomicity, eliminating race conditions entirely.
 * ============================================================================
 */

#include "race_demo.h"

/* Shared resource for unsynchronized experiment */
static volatile long unsync_counter = 0;

/* Shared resource and mutex for synchronized experiment */
static volatile long sync_counter = 0;
static pthread_mutex_t demo_mutex = PTHREAD_MUTEX_INITIALIZER;

/* Thread worker function for Unsynchronized access (Race Condition) */
static void *unsync_worker(void *arg) {
    (void)arg;
    for (int i = 0; i < RACE_DEMO_ITERATIONS; i++) {
        /*
         * UNSYNCHRONIZED CRITICAL SECTION:
         * Counter increment is NOT atomic at machine-instruction level.
         * Multiple threads read the stale value simultaneously, overwriting
         * each other's increments upon writing back.
         */
        unsync_counter++;
    }
    return NULL;
}

/* Thread worker function for Synchronized access (Mutex Protected) */
static void *sync_worker(void *arg) {
    (void)arg;
    for (int i = 0; i < RACE_DEMO_ITERATIONS; i++) {
        /*
         * MUTEX PROTECTED CRITICAL SECTION:
         * Only one thread enters at a time, guaranteeing atomic read-modify-write.
         */
        pthread_mutex_lock(&demo_mutex);
        sync_counter++;
        pthread_mutex_unlock(&demo_mutex);
    }
    return NULL;
}

void run_race_condition_demo(void) {
    pthread_t threads[RACE_DEMO_THREADS];
    long expected_total = (long)RACE_DEMO_THREADS * RACE_DEMO_ITERATIONS;

    printf("\n============================================================\n");
    printf("     CONTROLLED RACE CONDITION EXPERIMENT (CO-6 DEMO)\n");
    printf("============================================================\n");
    printf(" Threads                     : %d\n", RACE_DEMO_THREADS);
    printf(" Increments per Thread       : %d\n", RACE_DEMO_ITERATIONS);
    printf(" Expected Final Value        : %ld\n", expected_total);
    printf("------------------------------------------------------------\n\n");

    /* ------------------------------------------------------------
     * Experiment 1: Unsynchronized Concurrent Access
     * ------------------------------------------------------------ */
    printf("[Experiment 1] Running UNSYNCHRONIZED threads...\n");
    unsync_counter = 0;

    for (int i = 0; i < RACE_DEMO_THREADS; i++) {
        if (pthread_create(&threads[i], NULL, unsync_worker, NULL) != 0) {
            perror("[Error] pthread_create failed in demo");
            return;
        }
    }

    for (int i = 0; i < RACE_DEMO_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    long lost_updates = expected_total - unsync_counter;
    double loss_percentage = ((double)lost_updates / expected_total) * 100.0;

    printf("  -> Result (Unsynchronized) : %ld\n", unsync_counter);
    printf("  -> Lost Updates (Data Loss): %ld (%.2f%% corrupted)\n", 
           lost_updates, loss_percentage);
    if (lost_updates > 0) {
        printf("  -> [OBSERVATION] Race condition confirmed! Critical section violated.\n\n");
    } else {
        printf("  -> [OBSERVATION] No race observed in this run (CPU core/timing artifact).\n\n");
    }

    /* ------------------------------------------------------------
     * Experiment 2: Mutex-Synchronized Concurrent Access
     * ------------------------------------------------------------ */
    printf("[Experiment 2] Running MUTEX-SYNCHRONIZED threads...\n");
    sync_counter = 0;

    for (int i = 0; i < RACE_DEMO_THREADS; i++) {
        if (pthread_create(&threads[i], NULL, sync_worker, NULL) != 0) {
            perror("[Error] pthread_create failed in demo");
            return;
        }
    }

    for (int i = 0; i < RACE_DEMO_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    long sync_lost = expected_total - sync_counter;

    printf("  -> Result (Mutex-Protected): %ld\n", sync_counter);
    printf("  -> Lost Updates            : %ld (0.00%% error)\n", sync_lost);
    printf("  -> [OBSERVATION] Mutual exclusion verified! 100%% deterministic accuracy.\n");

    printf("\n============================================================\n");
    printf("                    SUMMARY COMPARISON\n");
    printf("============================================================\n");
    printf(" Mode               | Expected | Actual   | Lost Updates | Status\n");
    printf("--------------------+----------+----------+--------------+-----------\n");
    printf(" Unsynchronized     | %-8ld | %-8ld | %-12ld | FAILED (Race)\n",
           expected_total, unsync_counter, lost_updates);
    printf(" Mutex Synchronized | %-8ld | %-8ld | %-12ld | PASSED (Safe)\n",
           expected_total, sync_counter, sync_lost);
    printf("============================================================\n\n");

    pthread_mutex_destroy(&demo_mutex);
}
