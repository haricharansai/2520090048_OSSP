

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <sys/wait.h>

#define MAX_INPUT 100
#define MAX_ARGS 20

void enableRawMode(struct termios *old_terminal) {
    struct termios new_terminal;

    tcgetattr(STDIN_FILENO, old_terminal);

    new_terminal = *old_terminal;

    // Disable canonical mode and echo
    new_terminal.c_lflag &= ~(ICANON | ECHO);

    tcsetattr(STDIN_FILENO, TCSANOW, &new_terminal);
}

void disableRawMode(struct termios *old_terminal) {
    tcsetattr(STDIN_FILENO, TCSANOW, old_terminal);
}

int main() {
    char input[MAX_INPUT];
    struct termios old_terminal;

    while (1) {

        printf("myshell> ");
        fflush(stdout);

        int index = 0;

        enableRawMode(&old_terminal);

        while (1) {
            char c;

            read(STDIN_FILENO, &c, 1);

            // Enter key
            if (c == '\n' || c == '\r') {
                input[index] = '\0';
                printf("\n");
                break;
            }

            // Backspace
            else if (c == 127 || c == '\b') {

                if (index > 0) {
                    index--;

                    // Remove character from terminal
                    printf("\b \b");
                    fflush(stdout);
                }
            }

            // Normal character
            else if (index < MAX_INPUT - 1) {
                input[index] = c;
                index++;

                putchar(c);
                fflush(stdout);
            }
        }

        disableRawMode(&old_terminal);

        // Empty input
        if (index == 0) {
            continue;
        }

        // Exit command
        if (strcmp(input, "exit") == 0) {
            printf("Exiting shell...\n");
            break;
        }

        // Split command into arguments
        char *args[MAX_ARGS];

        int argc = 0;

        char *token = strtok(input, " ");

        while (token != NULL && argc < MAX_ARGS - 1) {
            args[argc] = token;
            argc++;

            token = strtok(NULL, " ");
        }

        args[argc] = NULL;

        // Create child
        pid_t pid = fork();

        if (pid < 0) {
            perror("fork failed");
        }

        else if (pid == 0) {

            // Execute command
            execvp(args[0], args);

            perror("Command not found");

            exit(1);
        }

        else {

            // Parent waits
            wait(NULL);
        }
    }

    return 0;
}