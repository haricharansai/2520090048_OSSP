#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_SIZE 10

// Node for linked list
struct Node {
    char *command;
    struct Node *next;
};

// Add command to history
void addHistory(struct Node **head, const char *command) {
    struct Node *newNode = malloc(sizeof(struct Node));

    if (newNode == NULL) {
        printf("Memory allocation failed\n");
        return;
    }

    newNode->command = malloc(strlen(command) + 1);

    if (newNode->command == NULL) {
        free(newNode);
        printf("Memory allocation failed\n");
        return;
    }

    strcpy(newNode->command, command);

    newNode->next = *head;
    *head = newNode;
}

// Display history
void displayHistory(struct Node *head) {
    int count = 1;

    while (head != NULL) {
        printf("%d: %s\n", count++, head->command);
        head = head->next;
    }
}

// Free linked list
void freeHistory(struct Node *head) {
    struct Node *temp;

    while (head != NULL) {
        temp = head;
        head = head->next;

        free(temp->command);
        free(temp);
    }
}

int main() {
    char *buffer;
    int size = INITIAL_SIZE;
    int length = 0;
    int ch;

    struct Node *history = NULL;

    // Dynamically allocate input buffer
    buffer = malloc(size);

    if (buffer == NULL) {
        printf("Memory allocation failed\n");
        return 1;
    }

    printf("Simple Command History Program\n");
    printf("Type commands. Type 'history' to view history.\n");
    printf("Type 'exit' to quit.\n\n");

    while (1) {
        printf("shell> ");

        length = 0;

        // Read input character by character
        while ((ch = getchar()) != '\n' && ch != EOF) {

            // Resize buffer when required
            if (length + 1 >= size) {
                size *= 2;

                char *temp = realloc(buffer, size);

                if (temp == NULL) {
                    printf("Buffer resizing failed\n");
                    free(buffer);
                    freeHistory(history);
                    return 1;
                }

                buffer = temp;
            }

            // Store character
            buffer[length++] = ch;
        }

        buffer[length] = '\0';

        // Handle escape sequence
        if (strcmp(buffer, "\\n") == 0) {
            printf("\n");
            continue;
        }

        if (strcmp(buffer, "\\t") == 0) {
            printf("\tTab escape sequence applied\n");
            continue;
        }

        // Exit
        if (strcmp(buffer, "exit") == 0) {
            break;
        }

        // Empty input
        if (length == 0) {
            continue;
        }

        // History command
        if (strcmp(buffer, "history") == 0) {
            displayHistory(history);
            continue;
        }

        // Store command in history
        addHistory(&history, buffer);

        printf("Command stored: %s\n", buffer);
    }

    // Release memory
    free(buffer);
    freeHistory(history);

    printf("All allocated memory released.\n");

    return 0;
}