
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_INPUT 1024
#define MAX_ARGS 64

// Parse command while preserving escaped characters
int parseCommand(char *input, char *args[]) {
    int count = 0;
    int i = 0;
    int j = 0;

    char *buffer = malloc(MAX_INPUT);

    if (buffer == NULL) {
        printf("Memory allocation failed\n");
        return -1;
    }

    while (input[i] != '\0') {

        // Skip spaces
        while (input[i] == ' ') {
            i++;
        }

        if (input[i] == '\0')
            break;

        j = 0;

        while (input[i] != '\0' && input[i] != ' ') {

            // Handle escape sequence
            if (input[i] == '\\' && input[i + 1] != '\0') {
                i++;
                buffer[j++] = input[i];
                i++;
            }
            else {
                buffer[j++] = input[i];
                i++;
            }
        }

        buffer[j] = '\0';

        args[count] = malloc(strlen(buffer) + 1);

        if (args[count] == NULL) {
            printf("Memory allocation failed\n");
            free(buffer);
            return -1;
        }

        strcpy(args[count], buffer);
        count++;

        if (count >= MAX_ARGS - 1)
            break;
    }

    args[count] = NULL;

    free(buffer);

    return count;
}

// Free argument memory
void freeArguments(char *args[], int count) {
    for (int i = 0; i < count; i++) {
        free(args[i]);
    }
}

int main() {

    char input[MAX_INPUT];

    while (1) {

        printf("shell> ");
        fflush(stdout);

        if (fgets(input, sizeof(input), stdin) == NULL)
            break;

        // Remove newline
        input[strcspn(input, "\n")] = '\0';

        if (strcmp(input, "exit") == 0)
            break;

        if (strlen(input) == 0)
            continue;

        char *args[MAX_ARGS];

        // Parse command
        int count = parseCommand(input, args);

        if (count <= 0)
            continue;

        // Display parser output
        printf("Parsed arguments:\n");

        for (int i = 0; i < count; i++) {
            printf("argv[%d] = [%s]\n", i, args[i]);
        }

        // Create child process
        pid_t pid = fork();

        if (pid < 0) {
            perror("fork failed");
            freeArguments(args, count);
            continue;
        }

        if (pid == 0) {

            // Child process
            execvp(args[0], args);

            // Executes only if execvp fails
            perror("exec failed");
            exit(EXIT_FAILURE);
        }

        else {

            // Parent process
            int status;

            waitpid(pid, &status, 0);

            if (WIFEXITED(status)) {
                printf("Child exited with status: %d\n",
                       WEXITSTATUS(status));
            }
        }

        freeArguments(args, count);
    }

    return 0;
}

