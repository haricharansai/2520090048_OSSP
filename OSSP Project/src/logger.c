/**
 * ============================================================================
 * Module: Thread-Safe File Logger (Phase 7)
 * 
 * OS Concepts Demonstrated:
 * - CO-5 (File Systems and File I/O):
 *   - File abstraction in UNIX/Linux.
 *   - File open modes (append, create, write), file stream buffering, and flush.
 *   - Persistent storage management in Linux user space.
 * - CO-6 (Concurrency):
 *   - Thread-safe log writing using a dedicated logger mutex to prevent
 *     interleaved garbled log lines from concurrent threads.
 * ============================================================================
 */

#include "logger.h"
#include <stdarg.h>
#include <time.h>
#include <sys/stat.h>

static FILE *log_file = NULL;
static pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;

static void get_current_timestamp(char *buffer, size_t size) {
    time_t now = time(NULL);
    struct tm tm_info;
    localtime_r(&now, &tm_info);
    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", &tm_info);
}

int logger_init(const char *filepath) {
    if (!filepath) {
        return -1;
    }

    pthread_mutex_lock(&log_mutex);

    /* Ensure logs directory exists (POSIX mkdir) */
    struct stat st = {0};
    if (stat("logs", &st) == -1) {
        mkdir("logs", 0755);
    }

    /* Open file in append mode (creates file if it doesn't exist) */
    log_file = fopen(filepath, "a");
    if (!log_file) {
        perror("[Error] Failed to open log file");
        pthread_mutex_unlock(&log_mutex);
        return -1;
    }

    /* Write session initialization marker */
    char timestamp[32];
    get_current_timestamp(timestamp, sizeof(timestamp));
    fprintf(log_file, "\n--- SESSION STARTED AT %s (PID: %d) ---\n", timestamp, getpid());
    fflush(log_file);

    pthread_mutex_unlock(&log_mutex);
    return 0;
}

void logger_log(const char *level, const char *format, ...) {
    char timestamp[32];
    get_current_timestamp(timestamp, sizeof(timestamp));

    char message[512];
    va_list args;
    va_start(args, format);
    vsnprintf(message, sizeof(message), format, args);
    va_end(args);

    /* CRITICAL SECTION: Serialize console and file writes across threads */
    pthread_mutex_lock(&log_mutex);

    /* 1. Print formatted message to standard output (Terminal) */
    printf("[%s] %s\n", level ? level : "INFO", message);
    fflush(stdout);

    /* 2. Write formatted message to log file (CO-5 File I/O) */
    if (log_file) {
        fprintf(log_file, "[%s] [%s] %s\n", timestamp, level ? level : "INFO", message);
        fflush(log_file); /* Ensure data is committed to OS buffer */
    }

    pthread_mutex_unlock(&log_mutex);
}

void logger_close(void) {
    pthread_mutex_lock(&log_mutex);

    if (log_file) {
        char timestamp[32];
        get_current_timestamp(timestamp, sizeof(timestamp));
        fprintf(log_file, "--- SESSION ENDED AT %s ---\n", timestamp);
        fflush(log_file);
        fclose(log_file);
        log_file = NULL;
    }

    pthread_mutex_unlock(&log_mutex);
    pthread_mutex_destroy(&log_mutex);
}
