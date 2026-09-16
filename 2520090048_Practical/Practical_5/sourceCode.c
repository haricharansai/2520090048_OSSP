#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include <string.h>
#include <bits/time.h>

#define COUNT 100000

int main() {
    int fd[2];
    pid_t pid;

    if (pipe(fd) == -1) {
        perror("pipe");
        return 1;
    }

    pid = fork();

    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if (pid > 0) {
        
        close(fd[0]);

        char buffer[64];
        struct timespec start, end;

        clock_gettime(CLOCK_MONOTONIC, &start);

        for (int i = 0; i < COUNT; i++) {
            snprintf(buffer, sizeof(buffer), "DATA-%d\n", i);

            if (write(fd[1], buffer, strlen(buffer)) == -1) {
                perror("write");
                break;
            }
        }

        close(fd[1]);

        wait(NULL);

        clock_gettime(CLOCK_MONOTONIC, &end);

        double time_taken =
            (end.tv_sec - start.tv_sec) +
            (end.tv_nsec - start.tv_nsec) / 1e9;

        printf("\nProducer completed.\n");
        printf("Messages transferred : %d\n", COUNT);
        printf("Time taken           : %.6f seconds\n", time_taken);
        printf("Communication rate   : %.2f messages/sec\n",
               COUNT / time_taken);
    }

    else {
        
        close(fd[1]);

        char buffer[64];
        ssize_t bytes_read;
        long total_bytes = 0;

        while ((bytes_read = read(fd[0], buffer, sizeof(buffer) - 1)) > 0) {
            buffer[bytes_read] = '\0';

            
            total_bytes += bytes_read;
        }

        close(fd[0]);

        printf("Consumer received %ld bytes.\n", total_bytes);
        exit(0);
    }

    return 0;
}