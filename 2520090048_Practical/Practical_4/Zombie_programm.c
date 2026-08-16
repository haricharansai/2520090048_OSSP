#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main()
{
    pid_t pid = fork();

    if (pid < 0)
    {
        perror("fork failed");
        exit(EXIT_FAILURE);
    }

    if (pid == 0)
    {
        // Child process
        printf("Child process started. PID = %d\n", getpid());

        printf("Child process exiting...\n");

        exit(0);
    }
    else
    {
        // Parent process
        printf("Parent process started. PID = %d\n", getpid());
        printf("Child PID = %d\n", pid);

        printf("Parent sleeping for 30 seconds...\n");

        /*
         * Parent does NOT call wait().
         * Therefore, after the child terminates,
         * the child becomes a zombie.
         */
        sleep(30);

        printf("Parent exiting...\n");
    }

    return 0;
}