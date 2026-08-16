#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main()
{
    pid_t pid;
    int status;

    pid = fork();

    if (pid < 0)
    {
        perror("fork failed");
        exit(EXIT_FAILURE);
    }

    if (pid == 0)
    {
        // Child
        printf("Child process started. PID = %d\n", getpid());

        sleep(2);

        printf("Child process exiting.\n");

        exit(10);
    }
    else
    {
        // Parent
        printf("Parent process started. PID = %d\n", getpid());
        printf("Child PID = %d\n", pid);

        /*
         * Parent waits for child.
         * This collects the child's termination status.
         */
        wait(&status);

        if (WIFEXITED(status))
        {
            printf("Child terminated normally.\n");
            printf("Exit status = %d\n", WEXITSTATUS(status));
        }

        printf("Parent exiting.\n");
    }

    return 0;
}