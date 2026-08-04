#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {

    pid_t pid = fork();

    if(pid == 0) {

        printf("Child Process\n");
        printf("PID = %d\n", getpid());

        exit(0);
    }
    else {

        printf("Parent PID = %d\n", getpid());

        printf("Sleeping for 30 seconds...\n");

        sleep(30);

        printf("Parent exiting.\n");
    }

    return 0;
}