#ifndef LOGGER_H
#define LOGGER_H

#include "common.h"

#define LOG_FILE_PATH "logs/scheduler.log"

/**
 * Initialize the thread-safe logger module and open the log file.
 * Demonstrates CO-5 (File Systems and Linux File I/O).
 * @param filepath Path to the log file (e.g., "logs/scheduler.log")
 * @return 0 on success, -1 on failure
 */
int logger_init(const char *filepath);

/**
 * Write a formatted log entry to both stdout and the persistent log file.
 * Thread-safe via internal mutex protection.
 * @param level Log severity level string (e.g., "INFO", "WARN", "TASK")
 * @param format Printf-style format string
 */
void logger_log(const char *level, const char *format, ...);

/**
 * Flush and close the log file and release logger synchronization primitives.
 */
void logger_close(void);

#endif /* LOGGER_H */
