#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    int fd[2];
    pid_t pid1, pid2;

    // Create pipe
    if (pipe(fd) == -1) {
        perror("pipe");
        return 1;
    }

    // Create first child
    pid1 = fork();

    if (pid1 < 0) {
        perror("fork");
        return 1;
    }

    if (pid1 == 0) {
        // Child 1: ls -l

        // Close unused read end
        close(fd[0]);

        // Redirect stdout to pipe
        dup2(fd[1], STDOUT_FILENO);

        // Close original pipe descriptor
        close(fd[1]);

        // Execute ls -l
        execlp("ls", "ls", "-l", (char *)NULL);

        // If exec fails
        perror("execlp ls");
        exit(1);
    }

    // Create second child
    pid2 = fork();

    if (pid2 < 0) {
        perror("fork");
        return 1;
    }

    if (pid2 == 0) {
        // Child 2: grep ".c"

        // Close unused write end
        close(fd[1]);

        // Redirect stdin from pipe
        dup2(fd[0], STDIN_FILENO);

        // Close original pipe descriptor
        close(fd[0]);

        // Execute grep ".c"
        execlp("grep", "grep", ".c", (char *)NULL);

        // If exec fails
        perror("execlp grep");
        exit(1);
    }

    // Parent does not use the pipe
    close(fd[0]);
    close(fd[1]);

    cd
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);

    return 0;
}