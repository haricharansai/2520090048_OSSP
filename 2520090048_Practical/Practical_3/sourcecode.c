#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main()
{
    pid_t pid;

    pid = fork();

    if (pid < 0)
    {
        printf("Fork failed!\n");
        return 1;
    }

    if (pid == 0)
    {
        // Child Process
        printf("\n===== CHILD PROCESS =====\n");
        printf("Child PID  : %d\n", getpid());
        printf("Parent PID : %d\n", getppid());

        printf("Child is sleeping for 10 seconds...\n");
        sleep(10);

        printf("Child process terminating.\n");
        exit(0);
    }
    else
    {
        // Parent Process
        printf("\n===== PARENT PROCESS =====\n");
        printf("Parent PID : %d\n", getpid());
        printf("Child PID  : %d\n", pid);

        printf("Parent waiting for child...\n");
        wait(NULL);

        printf("Child has terminated.\n");
        printf("Parent process terminating.\n");
    }

    return 0;
}