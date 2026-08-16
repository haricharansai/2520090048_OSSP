#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#define NUM_CHILDREN 3

int main()
{
    pid_t children[NUM_CHILDREN];
    int status;

    printf("Parent PID = %d\n", getpid());

    // Create children
    for (int i = 0; i < NUM_CHILDREN; i++)
    {
        children[i] = fork();

        if (children[i] < 0)
        {
            perror("fork failed");
            exit(EXIT_FAILURE);
        }

        if (children[i] == 0)
        {
            printf("Child %d started. PID = %d\n",
                   i + 1, getpid());

            sleep(i + 1);

            printf("Child %d exiting. PID = %d\n",
                   i + 1, getpid());

            exit(10 + i);
        }
    }

    // Parent waits for every child
    for (int i = 0; i < NUM_CHILDREN; i++)
    {
        pid_t pid = waitpid(children[i], &status, 0);

        if (pid == -1)
        {
            perror("waitpid failed");
            exit(EXIT_FAILURE);
        }

        if (WIFEXITED(status))
        {
            printf("Parent collected child PID %d, exit status = %d\n",
                   pid, WEXITSTATUS(status));
        }
    }

    printf("All child processes have been collected.\n");
    printf("No zombie processes remain.\n");

    return 0;
}