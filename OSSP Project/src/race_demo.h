#ifndef RACE_DEMO_H
#define RACE_DEMO_H

#include "common.h"

#define RACE_DEMO_THREADS    4
#define RACE_DEMO_ITERATIONS 50000

/**
 * Executes a controlled, side-by-side comparison demonstrating:
 * 1. Race condition with unsynchronized concurrent access (lost updates).
 * 2. Deterministic correctness with POSIX Mutex synchronization.
 */
void run_race_condition_demo(void);

#endif /* RACE_DEMO_H */
