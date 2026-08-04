#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    int i;
    pid_t pid;

    printf("Parent Process PID: %d\n\n", getpid());

    // Create 3 child processes
    for(i = 1; i <= 3; i++) {

        pid = fork();

        if(pid == 0) {
            printf("Child %d started\n", i);
            printf("PID = %d, PPID = %d\n", getpid(), getppid());

            sleep(i * 2);

            printf("Child %d completed\n\n", i);
            exit(0);
        }
    }

    printf("Parent waiting using wait()...\n");

    // Wait for first two children
    for(i = 0; i < 2; i++) {
        wait(NULL);
        printf("One child finished using wait().\n");
    }

    printf("\nParent waiting for remaining child using waitpid()...\n");

    waitpid(pid, NULL, 0);

    printf("Specific child completed using waitpid().\n");

    printf("\nAll child processes completed.\n");

    return 0;
}