#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {

    pid_t pid = fork();

    if(pid == 0) {

        printf("Child Process\n");
        printf("PID = %d\n", getpid());

        exit(0);
    }
    else {

        printf("Parent waiting...\n");

        wait(NULL);

        printf("Child collected successfully.\n");

        sleep(10);

        printf("No Zombie Process Exists.\n");
    }

    return 0;
}