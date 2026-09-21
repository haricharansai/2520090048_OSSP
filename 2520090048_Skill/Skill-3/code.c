#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>

#define INITIAL_BUFFER 32



typedef struct Node {
    char *command;
    struct Node *next;
} Node;

Node *history_head = NULL;
Node *history_tail = NULL;

void add_history(const char *command)
{
    if (strlen(command) == 0)
        return;

    Node *new_node = malloc(sizeof(Node));

    if (new_node == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    new_node->command = malloc(strlen(command) + 1);

    if (new_node->command == NULL) {
        perror("malloc");
        free(new_node);
        exit(EXIT_FAILURE);
    }

    strcpy(new_node->command, command);
    new_node->next = NULL;

    if (history_head == NULL) {
        history_head = new_node;
        history_tail = new_node;
    } else {
        history_tail->next = new_node;
        history_tail = new_node;
    }
}

void free_history()
{
    Node *current = history_head;

    while (current != NULL) {
        Node *temp = current->next;

        free(current->command);
        free(current);

        current = temp;
    }

    history_head = NULL;
    history_tail = NULL;
}


int history_count()
{
    int count = 0;
    Node *current = history_head;

    while (current != NULL) {
        count++;
        current = current->next;
    }

    return count;
}

char *get_history_command(int index)
{
    Node *current = history_head;
    int i = 0;

    while (current != NULL) {
        if (i == index)
            return current->command;

        current = current->next;
        i++;
    }

    return NULL;
}


struct termios original_terminal;

void disable_raw_mode()
{
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_terminal);
}

void enable_raw_mode()
{
    tcgetattr(STDIN_FILENO, &original_terminal);

    atexit(disable_raw_mode);

    struct termios raw = original_terminal;

    raw.c_lflag &= ~(ECHO | ICANON);

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}



char *read_command()
{
    int capacity = INITIAL_BUFFER;
    int length = 0;

    char *buffer = malloc(capacity);

    if (buffer == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    buffer[0] = '\0';

    int history_index = history_count();

    while (1) {

        char c;

        if (read(STDIN_FILENO, &c, 1) <= 0)
            break;

        /* ENTER */
        if (c == '\n' || c == '\r') {
            printf("\n");
            break;
        }

        /* BACKSPACE */
        if (c == 127 || c == '\b') {

            if (length > 0) {
                length--;
                buffer[length] = '\0';

                printf("\b \b");
                fflush(stdout);
            }

            continue;
        }

        /* ESCAPE SEQUENCE */
        if (c == 27) {

            char seq[2];

            if (read(STDIN_FILENO, &seq[0], 1) <= 0)
                continue;

            if (read(STDIN_FILENO, &seq[1], 1) <= 0)
                continue;

            /*
                UP    = ESC [ A
                DOWN  = ESC [ B
            */

            if (seq[0] == '[' && seq[1] == 'A') {

                /* UP ARROW */
                if (history_index > 0) {

                    history_index--;

                    char *old_command =
                        get_history_command(history_index);

                    if (old_command != NULL) {

                        /* Clear current input */
                        while (length > 0) {
                            printf("\b \b");
                            length--;
                        }

                        /* Copy recalled command */
                        int new_length = strlen(old_command);

                        if (new_length + 1 > capacity) {

                            capacity = new_length + 1;

                            char *temp =
                                realloc(buffer, capacity);

                            if (temp == NULL) {
                                free(buffer);
                                perror("realloc");
                                exit(EXIT_FAILURE);
                            }

                            buffer = temp;
                        }

                        strcpy(buffer, old_command);
                        length = new_length;

                        printf("%s", buffer);
                        fflush(stdout);
                    }
                }

                continue;
            }

            if (seq[0] == '[' && seq[1] == 'B') {

                /* DOWN ARROW */

                int count = history_count();

                if (history_index < count - 1) {

                    history_index++;

                    char *old_command =
                        get_history_command(history_index);

                    while (length > 0) {
                        printf("\b \b");
                        length--;
                    }

                    int new_length = strlen(old_command);

                    if (new_length + 1 > capacity) {

                        capacity = new_length + 1;

                        char *temp =
                            realloc(buffer, capacity);

                        if (temp == NULL) {
                            free(buffer);
                            perror("realloc");
                            exit(EXIT_FAILURE);
                        }

                        buffer = temp;
                    }

                    strcpy(buffer, old_command);
                    length = new_length;

                    printf("%s", buffer);
                    fflush(stdout);

                } else {

                    /* Move to empty input */
                    history_index = count;

                    while (length > 0) {
                        printf("\b \b");
                        length--;
                    }

                    buffer[0] = '\0';
                    fflush(stdout);
                }

                continue;
            }

            continue;
        }

        /* Normal character */

        /* Prevent buffer overflow */
        if (length + 1 >= capacity) {

            capacity *= 2;

            char *temp = realloc(buffer, capacity);

            if (temp == NULL) {
                free(buffer);
                perror("realloc");
                exit(EXIT_FAILURE);
            }

            buffer = temp;
        }

        buffer[length++] = c;
        buffer[length] = '\0';

        putchar(c);
        fflush(stdout);
    }

    return buffer;
}

/* =========================
   Main
   ========================= */

int main()
{
    printf("====================================\n");
    printf(" Linux Command History Demo\n");
    printf("====================================\n");

    printf("Type commands and press ENTER.\n");
    printf("Use UP/DOWN arrows for history.\n");
    printf("Type 'exit' to quit.\n\n");

    enable_raw_mode();

    while (1) {

        printf("myshell> ");
        fflush(stdout);

        char *command = read_command();

        if (strcmp(command, "exit") == 0) {
            free(command);
            break;
        }

        if (strlen(command) > 0) {

            add_history(command);

            printf("Command stored: %s\n", command);
        }

        free(command);
    }

    disable_raw_mode();

    free_history();

    printf("\nMemory cleanup completed.\n");

    return 0;
}