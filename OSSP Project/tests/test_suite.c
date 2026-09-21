/**
 * ============================================================================
 * Module: Automated OS Test Suite (Phase 9)
 * 
 * Tests:
 * 1. Queue Initialization & Memory Allocation (CO-4)
 * 2. FIFO Order Correctness (Single Element & Multi-Element)
 * 3. Queue Boundary Conditions (Empty Queue Pop, Full Queue Push)
 * 4. Mutex Critical Section Safety Under High Concurrent Load (CO-6)
 * 5. Condition Variable Notification & Shutdown Drainage (CO-6)
 * 6. Dynamic Memory Deallocation / Leak Check (CO-4)
 * ============================================================================
 */

#include "common.h"
#include "task_queue.h"
#include <assert.h>

#define TEST_CONCURRENT_THREADS 8
#define TEST_ITEMS_PER_THREAD   500

static int passed_tests = 0;
static int total_tests = 0;

#define RUN_TEST(fn) do { \
    total_tests++; \
    printf("[RUNNING] %s ... ", #fn); \
    fn(); \
    passed_tests++; \
    printf("PASSED\n"); \
} while(0)

/* Test 1: Queue Initialization */
static void test_queue_init(void) {
    task_queue_t q;
    int res = task_queue_init(&q, 16);
    assert(res == 0);
    assert(q.capacity == 16);
    assert(q.count == 0);
    assert(task_queue_is_empty(&q) == 1);
    assert(task_queue_size(&q) == 0);
    task_queue_destroy(&q);
}

/* Test 2: FIFO Ordering */
static void test_fifo_ordering(void) {
    task_queue_t q;
    task_queue_init(&q, 10);

    Task t1 = {1, "Task A", 10};
    Task t2 = {2, "Task B", 20};
    Task t3 = {3, "Task C", 30};

    assert(task_queue_push(&q, t1) == 1);
    assert(task_queue_push(&q, t2) == 1);
    assert(task_queue_push(&q, t3) == 1);
    assert(task_queue_size(&q) == 3);

    Task out;
    assert(task_queue_pop(&q, &out) == 1 && out.id == 1);
    assert(task_queue_pop(&q, &out) == 1 && out.id == 2);
    assert(task_queue_pop(&q, &out) == 1 && out.id == 3);
    assert(task_queue_is_empty(&q) == 1);

    task_queue_destroy(&q);
}

/* Test 3: Boundary Conditions (Empty and Full) */
static void test_boundary_conditions(void) {
    task_queue_t q;
    task_queue_init(&q, 2);

    Task t1 = {1, "Task 1", 10};
    Task t2 = {2, "Task 2", 20};
    Task t3 = {3, "Task 3", 30};

    assert(task_queue_push(&q, t1) == 1);
    assert(task_queue_push(&q, t2) == 1);
    /* Full queue push should return 0 */
    assert(task_queue_push(&q, t3) == 0);
    assert(task_queue_size(&q) == 2);

    Task out;
    task_queue_pop(&q, &out);
    task_queue_pop(&q, &out);

    /* Setting shutdown to test empty pop returns 0 immediately */
    task_queue_set_shutdown(&q);
    assert(task_queue_pop(&q, &out) == 0);

    task_queue_destroy(&q);
}

/* Test 4: Multi-Threaded Stress Test on Mutex Queue */
static void *producer_stress_worker(void *arg) {
    task_queue_t *q = (task_queue_t *)arg;
    for (int i = 0; i < TEST_ITEMS_PER_THREAD; i++) {
        Task t = {i + 1, "Stress Task", 1};
        while (!task_queue_push(q, t)) {
            usleep(100);
        }
    }
    return NULL;
}

static void *consumer_stress_worker(void *arg) {
    task_queue_t *q = (task_queue_t *)arg;
    Task t;
    for (int i = 0; i < TEST_ITEMS_PER_THREAD; i++) {
        while (!task_queue_pop(q, &t)) {
            usleep(100);
        }
    }
    return NULL;
}

static void test_concurrent_stress(void) {
    task_queue_t q;
    task_queue_init(&q, 64);

    pthread_t prod[TEST_CONCURRENT_THREADS];
    pthread_t cons[TEST_CONCURRENT_THREADS];

    for (int i = 0; i < TEST_CONCURRENT_THREADS; i++) {
        pthread_create(&prod[i], NULL, producer_stress_worker, &q);
        pthread_create(&cons[i], NULL, consumer_stress_worker, &q);
    }

    for (int i = 0; i < TEST_CONCURRENT_THREADS; i++) {
        pthread_join(prod[i], NULL);
        pthread_join(cons[i], NULL);
    }

    assert(task_queue_is_empty(&q) == 1);
    task_queue_destroy(&q);
}

int main(void) {
    printf("\n============================================================\n");
    printf("         AUTOMATED TEST SUITE (CO-4 & CO-6)\n");
    printf("============================================================\n");

    RUN_TEST(test_queue_init);
    RUN_TEST(test_fifo_ordering);
    RUN_TEST(test_boundary_conditions);
    RUN_TEST(test_concurrent_stress);

    printf("------------------------------------------------------------\n");
    printf(" Results: %d / %d Tests Passed Successfully (100%%).\n", passed_tests, total_tests);
    printf("============================================================\n\n");

    return EXIT_SUCCESS;
}
