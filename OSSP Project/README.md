# Multithreaded Linux Task Processing System Using POSIX Threads and Mutexes

A robust, production-grade Linux systems programming project developed in **C** demonstrating core Operating Systems concepts, with deep technical alignment to **CO-6 (Concurrency and Synchronization)**, **CO-1 (OS as a Service Layer)**, **CO-2 (Processes and Process Control)**, **CO-4 (Memory Management)**, and **CO-5 (File Systems and File I/O)**.

---

## 1. System Architecture

```text
                         +-----------------------------------+
                         |         Linux Terminal Shell      |
                         +-----------------+-----------------+
                                           |
                                           v
                         +-----------------+-----------------+
                         |      Main Scheduler Process       |
                         |   - Process Lifecycle (PID/PPID)  |
                         |   - Signal Handling (SIGINT)      |
                         |   - Producer Task Dispatcher      |
                         +-----------------+-----------------+
                                           |
                                           v
                         +-----------------+-----------------+
                         |        Shared Task Queue          |
                         |   - Dynamic Heap Buffer (malloc)  |
                         |   - POSIX Mutex (Mutual Exclusion)|
                         |   - Condition Variable (not_empty)|
                         +--------+--------+--------+--------+
                                  |        |        |
             +--------------------+        |        +--------------------+
             |                             |                             |
             v                             v                             v
    +-----------------+           +-----------------+           +-----------------+
    | Worker Thread 1 |           | Worker Thread 2 |           | Worker Thread 3 |
    |  POSIX pthread  |           |  POSIX pthread  |           |  POSIX pthread  |
    +--------+--------+           +--------+--------+           +--------+--------+
             |                             |                             |
             +--------------------+        |        +--------------------+
                                  |        |        |
                                  v        v        v
                         +-----------------+-----------------+
                         |      Thread-Safe File Logger      |
                         |  - Append Mode (Linux File I/O)   |
                         |  - logs/scheduler.log             |
                         +-----------------------------------+
```

---

## 2. Operating Systems Course Outcomes (CO) Mapping

| Course Outcome | Focus Area | Concrete Implementation in Project |
| :--- | :--- | :--- |
| **CO-1: OS as a Service Layer** | User Space / Kernel Space & POSIX APIs | Program executes in Linux user space interacting with kernel services via POSIX system calls (`getpid`, `getppid`, `sigaction`, `write`, `usleep`). |
| **CO-2: Processes & Control** | Process Abstraction & Lifecycle | Master scheduler process manages thread lifecycles, execution states, and performs rendezvous synchronization via `pthread_join()`. |
| **CO-3: Inter-Process Comm.** | *Not falsely claimed* | Current design is multi-threaded (shared address space). Outlined for future expansion via POSIX FIFOs or UNIX Domain Sockets. |
| **CO-4: Memory Management** | Virtual Address Space & Heap Allocation | Dynamic allocation (`malloc`/`free`) of task buffers and worker arrays on the heap. Validated with zero leaks via AddressSanitizer. |
| **CO-5: File Systems & File I/O** | Persistent Disk I/O & File Descriptors | Timestamped telemetry recording to `logs/scheduler.log` with buffered stream flushing (`fopen`, `fprintf`, `fflush`, `fclose`). |
| **CO-6: Concurrency & Synchronization** | **(Primary CO)** Mutexes, CondVars & Race Hazards | Thread pool management (`pthread_create`), Critical Section isolation (`pthread_mutex_t`), Producer-Consumer coordination (`pthread_cond_t`), and controlled race condition demonstrations. |

---

## 3. Project Directory Structure

```text
OSSP Project/
├── include/
│   └── common.h          # Global constants, standard includes, and configurations
├── src/
│   ├── main.c            # Process entry point, CLI parser, and scheduler workflow
│   ├── task_queue.h      # Queue definitions, Task struct, and monitor interfaces
│   ├── task_queue.c      # Dynamic circular buffer with mutex and condition variable
│   ├── worker.h          # Worker thread argument structure and entry declaration
│   ├── worker.c          # Worker thread execution loop and task consumption
│   ├── logger.h          # Thread-safe persistent file logging header
│   ├── logger.c          # File I/O implementation with mutex serialization
│   ├── race_demo.h       # Race condition comparison experiment header
│   └── race_demo.c       # Side-by-side assembly-level race hazard demonstration
├── tests/
│   └── test_suite.c      # Automated unit and stress testing suite
├── logs/
│   └── scheduler.log     # Persistent runtime execution log
├── Makefile              # Build automation (GCC with -Wall -Wextra -pthread)
├── .gitignore            # Git exclusion rules
└── README.md             # Project documentation
```

---

## 4. Key Operating Systems Concepts Explained

### A. Process vs. Thread
- **Process**: Independent execution unit created by the Linux kernel with its own private virtual memory space, file descriptor table, and PID.
- **Thread (Lightweight Process)**: Schedulable execution context sharing the process heap, static memory, and file descriptors, but maintaining its own private stack, registers, and Program Counter (PC).

### B. The Critical Section Problem & POSIX Mutex
A **critical section** is a section of code accessing shared resources that must not be executed concurrently by more than one thread.
This project uses `pthread_mutex_t` to satisfy the three fundamental requirements:
1. **Mutual Exclusion**: If thread $T_i$ is executing its critical section, no other threads can execute in that section.
2. **Progress**: If no thread is executing in its critical section, only threads not in their remainder section participate in deciding who enters next.
3. **Bounded Waiting**: There is a bound on the number of times other threads can enter critical sections before a requesting thread is granted entry.

### C. Race Condition (Assembly-Level Analysis)
Incrementing a shared counter (`counter++`) in C decomposes into 3 non-atomic CPU instructions:
```text
1. mov eax, [counter]   ; Read value from RAM into CPU register
2. add eax, 1           ; Increment value inside register
3. mov [counter], eax   ; Write value from register back to RAM
```
When multiple threads execute this without synchronization on a multi-core processor, context switches interleave these instructions, causing **Lost Updates**:
- **Unsynchronized**: 4 threads $\times$ 50,000 iterations $\rightarrow$ Result: `~114,451` (Data loss of ~42%).
- **Mutex Synchronized**: 4 threads $\times$ 50,000 iterations $\rightarrow$ Result: `200,000` (100% Deterministic).

### D. Producer-Consumer Synchronization via Condition Variables
Instead of CPU busy-waiting (spinning), worker threads enter the **Blocked state** using `pthread_cond_wait(&queue->not_empty, &queue->lock)`:
- `pthread_cond_wait()` atomically releases the mutex and suspends the thread.
- When the producer inserts a task, it invokes `pthread_cond_signal()` to wake up a sleeping worker.
- All wait checks are wrapped inside a `while (queue->count == 0 && !queue->shutdown)` loop to defend against **spurious wakeups**.
- During shutdown, `pthread_cond_broadcast()` awakens all sleeping workers simultaneously.

---

## 5. Compilation and Execution Guide

### Prerequisites
- Operating System: Linux / Ubuntu
- Compiler: GCC with POSIX Threads support

### Compilation Commands

```bash
# Build the primary scheduler executable
make

# Build and execute the automated test suite
make test

# Build and run the controlled race condition comparison experiment
make demo

# Build and execute with GCC AddressSanitizer (Memory leak & safety check)
make asan

# Clean build artifacts and logs
make clean
```

---

## 6. Execution Modes & Command-Line Options

### 1. Default Scheduler Mode (3 Workers, 6 Standard Tasks)
```bash
./scheduler
```

### 2. Custom Worker Pool and Dynamic Task Generation
```bash
# Spawns 4 worker threads to process 10 dynamic tasks
./scheduler --workers 4 --tasks 10
```

### 3. Controlled Race Condition Demonstration
```bash
./scheduler --race-demo
```

### 4. Help Menu
```bash
./scheduler --help
```

---

## 7. Sample Terminal Outputs

### A. Standard Scheduler Execution
```text
========================================================
  Multithreaded Linux Task Processing System
  Version: 1.0.0
========================================================
[System Init] Process initialized successfully.
[System Info] Current Process ID (PID)        : 12575
[System Info] Parent Process ID (PPID)        : 3309
[System Info] Active Worker Threads           : 3
[System Info] Total Tasks to Process          : 6
[System Info] Persistent Log Output           : logs/scheduler.log
--------------------------------------------------------
[SCHEDULER] Starting scheduler process with 3 workers and 6 tasks.
[Scheduler] Spawning 3 worker threads...
[WORKER] [Worker 1] Online | TID: 134368785856192 | Waiting on condition variable...
[WORKER] [Worker 2] Online | TID: 134368777463488 | Waiting on condition variable...
[WORKER] [Worker 3] Online | TID: 134368769070784 | Waiting on condition variable...
--------------------------------------------------------
[PRODUCER] Dispatched Task 101: "File System Compression" (duration: 150 ms)
[WORKER] [Worker 1] Picked Task 101: "File System Compression" (duration: 150 ms)
[PRODUCER] Dispatched Task 102: "Kernel Log Parsing" (duration: 100 ms)
[WORKER] [Worker 2] Picked Task 102: "Kernel Log Parsing" (duration: 100 ms)
[PRODUCER] Dispatched Task 103: "Process Memory Analysis" (duration: 180 ms)
[WORKER] [Worker 3] Picked Task 103: "Process Memory Analysis" (duration: 180 ms)
[PRODUCER] Dispatched Task 104: "Cryptographic Hash Calc" (duration: 120 ms)
[WORKER] [Worker 2] Completed Task 102: "Kernel Log Parsing"
[WORKER] [Worker 2] Picked Task 104: "Cryptographic Hash Calc" (duration: 120 ms)
[PRODUCER] Dispatched Task 105: "Network Packet Inspection" (duration: 90 ms)
[WORKER] [Worker 1] Completed Task 101: "File System Compression"
[WORKER] [Worker 1] Picked Task 105: "Network Packet Inspection" (duration: 90 ms)
[PRODUCER] Dispatched Task 106: "Database Index Sync" (duration: 140 ms)
[WORKER] [Worker 1] Completed Task 105: "Network Packet Inspection"
[WORKER] [Worker 1] Picked Task 106: "Database Index Sync" (duration: 140 ms)
[WORKER] [Worker 3] Completed Task 103: "Process Memory Analysis"
[WORKER] [Worker 2] Completed Task 104: "Cryptographic Hash Calc"
--------------------------------------------------------
[SCHEDULER] Initiating graceful worker shutdown sequence...
[WORKER] [Worker 3] Received shutdown notification. Terminating.
[WORKER] [Worker 2] Received shutdown notification. Terminating.
[WORKER] [Worker 1] Completed Task 106: "Database Index Sync"
[WORKER] [Worker 1] Received shutdown notification. Terminating.
[SCHEDULER] Worker 1 joined and reaped successfully.
[SCHEDULER] Worker 2 joined and reaped successfully.
[SCHEDULER] Worker 3 joined and reaped successfully.
--------------------------------------------------------
[SCHEDULER] All 3 worker threads terminated safely.
[SCHEDULER] Task queue destroyed and synchronization primitives freed.
[Scheduler] Process completed successfully. Log persisted to: logs/scheduler.log
```

### B. Race Condition Benchmark Output
```text
============================================================
     CONTROLLED RACE CONDITION EXPERIMENT (CO-6 DEMO)
============================================================
 Threads                     : 4
 Increments per Thread       : 50000
 Expected Final Value        : 200000
------------------------------------------------------------

[Experiment 1] Running UNSYNCHRONIZED threads...
  -> Result (Unsynchronized) : 114451
  -> Lost Updates (Data Loss): 85549 (42.77% corrupted)
  -> [OBSERVATION] Race condition confirmed! Critical section violated.

[Experiment 2] Running MUTEX-SYNCHRONIZED threads...
  -> Result (Mutex-Protected): 200000
  -> Lost Updates            : 0 (0.00% error)
  -> [OBSERVATION] Mutual exclusion verified! 100% deterministic accuracy.

============================================================
                    SUMMARY COMPARISON
============================================================
 Mode               | Expected | Actual   | Lost Updates | Status
--------------------+----------+----------+--------------+-----------
 Unsynchronized     | 200000   | 114451   | 85549        | FAILED (Race)
 Mutex Synchronized | 200000   | 200000   | 0            | PASSED (Safe)
============================================================
```

---

## 8. Verification & Memory Safety Report

The codebase was compiled and analyzed using **GCC AddressSanitizer (ASan)** and **UndefinedBehaviorSanitizer (UBSan)** (`-fsanitize=address,undefined`):

| Test Category | Target Tested | Result | Memory Leaks |
| :--- | :--- | :--- | :--- |
| **Unit Test 1** | Dynamic Queue Initialization & Sizing | PASSED | 0 Bytes |
| **Unit Test 2** | FIFO Ordering & Task Retrieval | PASSED | 0 Bytes |
| **Unit Test 3** | Boundary Conditions (Full Queue / Empty Pop) | PASSED | 0 Bytes |
| **Stress Test 4** | 8 Concurrent Producers & 8 Consumers (4,000 operations) | PASSED | 0 Bytes |
| **Integration Test** | Full Scheduler Execution & Signal Handling | PASSED | 0 Bytes |
| **Race Hazard Demo** | Multi-Threaded Unsync vs Mutex Comparison | PASSED | 0 Bytes |

---

## 9. Review & Viva Voce Quick Reference

1. **Why use mutexes only for queue access and not during task processing?**
   - Holding a mutex during task processing serializes execution, defeating the entire purpose of concurrency. Isolating the critical section solely to queue index updates maximizes parallelism.
2. **What is a spurious wakeup?**
   - A thread waiting on `pthread_cond_wait()` may awaken without an explicit signal (due to kernel signals or OS context interruptions). Guarding the wait in a `while` loop guarantees the predicate is re-evaluated before proceeding.
3. **What is the difference between `pthread_cond_signal()` and `pthread_cond_broadcast()`?**
   - `pthread_cond_signal()` wakes up at least one waiting thread (used when a single task arrives). `pthread_cond_broadcast()` wakes all waiting threads (used during system shutdown).
4. **Why is `pthread_join()` necessary?**
   - `pthread_join()` reaps the terminated thread's resources (kernel descriptor and thread stack). Failing to join non-detached threads creates resource leaks analogous to zombie processes.
