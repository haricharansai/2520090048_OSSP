#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main()
{
    pid_t children[3];
    int status;

    printf("Parent process started. PID = %d\n", getpid());

    // Create 3 child processes
    for (int i = 0; i < 3; i++)
    {
        children[i] = fork();

        if (children[i] < 0)
        {
            perror("fork failed");
            exit(EXIT_FAILURE);
        }

        if (children[i] == 0)
        {
            // Child process
            printf("Child %d started. PID = %d, PPID = %d\n",
                   i + 1, getpid(), getppid());

            sleep((i + 1) * 2);

            printf("Child %d completed. PID = %d\n",
                   i + 1, getpid());

            exit(10 + i);
        }
    }

    // Parent process
    printf("\nParent: All children created.\n");

    /*
     * wait() waits for ANY one child process to terminate.
     */
    pid_t pid = wait(&status);

    if (pid > 0)
    {
        if (WIFEXITED(status))
        {
            printf("wait(): Child PID %d terminated with exit status %d\n",
                   pid, WEXITSTATUS(status));
        }
    }

    /*
     * waitpid() can wait for a SPECIFIC child.
     * Here we wait for children[0] and children[1].
     */
    for (int i = 0; i < 2; i++)
    {
        pid = waitpid(children[i], &status, 0);

        if (pid > 0)
        {
            if (WIFEXITED(status))
            {
                printf("waitpid(): Child PID %d terminated with exit status %d\n",
                       pid, WEXITSTATUS(status));
            }
        }
    }

    printf("Parent process completed. PID = %d\n", getpid());

    return 0;
}