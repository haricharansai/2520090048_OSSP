
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_INPUT 1024
#define MAX_ARGS 64

/* ---------- Variable Expansion ---------- */

char *expandVariables(const char *input) {

    char *output = malloc(MAX_INPUT);

    if (output == NULL) {
        return NULL;
    }

    int i = 0;
    int j = 0;

    while (input[i] != '\0' && j < MAX_INPUT - 1) {

        /* Detect $ */
        if (input[i] == '$') {

            i++;

            char variable[128];
            int k = 0;

            /* Support ${VAR} */
            if (input[i] == '{') {

                i++;

                while (input[i] != '\0' &&
                       input[i] != '}' &&
                       k < 127) {

                    variable[k++] = input[i++];
                }

                if (input[i] == '}')
                    i++;
            }

            /* Support $VAR */
            else {

                while ((input[i] >= 'A' && input[i] <= 'Z') ||
                       (input[i] >= 'a' && input[i] <= 'z') ||
                       (input[i] >= '0' && input[i] <= '9') ||
                       input[i] == '_') {

                    variable[k++] = input[i++];
                }
            }

            variable[k] = '\0';

            /* Undefined variable */
            char *value = getenv(variable);

            if (value != NULL) {

                int len = strlen(value);

                if (j + len < MAX_INPUT - 1) {
                    strcpy(&output[j], value);
                    j += len;
                }
            }

        }
        else {

            output[j++] = input[i++];
        }
    }

    output[j] = '\0';

    return output;
}


/* ---------- Built-in Commands ---------- */

int builtin_pwd(char *args[]) {

    char cwd[MAX_INPUT];

    if (getcwd(cwd, sizeof(cwd)) != NULL)
        printf("%s\n", cwd);
    else
        perror("pwd");

    return 0;
}


int builtin_cd(char *args[]) {

    if (args[1] == NULL) {

        printf("cd: missing argument\n");
        return 1;
    }

    if (chdir(args[1]) != 0) {

        perror("cd");
        return 1;
    }

    return 0;
}


int builtin_echo(char *args[]) {

    int i = 1;

    while (args[i] != NULL) {

        printf("%s", args[i]);

        if (args[i + 1] != NULL)
            printf(" ");

        i++;
    }

    printf("\n");

    return 0;
}


int builtin_export(char *args[]) {

    if (args[1] == NULL) {

        printf("export: missing argument\n");
        return 1;
    }

    char *equal = strchr(args[1], '=');

    if (equal == NULL) {

        printf("export: use NAME=value\n");
        return 1;
    }

    *equal = '\0';

    char *name = args[1];
    char *value = equal + 1;

    if (setenv(name, value, 1) != 0) {

        perror("export");
        return 1;
    }

    return 0;
}


int builtin_exit(char *args[]) {

    exit(0);
}


/* ---------- Dispatch Table ---------- */

typedef int (*BuiltinFunction)(char **);

struct Builtin {

    char *name;
    BuiltinFunction function;
};


/* Built-in command table */

struct Builtin builtins[] = {

    {"pwd", builtin_pwd},
    {"cd", builtin_cd},
    {"echo", builtin_echo},
    {"export", builtin_export},
    {"exit", builtin_exit},

    {NULL, NULL}
};


/* ---------- Find Built-in ---------- */

BuiltinFunction findBuiltin(const char *command) {

    int i = 0;

    while (builtins[i].name != NULL) {

        if (strcmp(command, builtins[i].name) == 0)
            return builtins[i].function;

        i++;
    }

    return NULL;
}


/* ---------- Tokenizer ---------- */

int tokenize(char *input, char *args[]) {

    int count = 0;

    char *token = strtok(input, " ");

    while (token != NULL && count < MAX_ARGS - 1) {

        args[count++] = token;

        token = strtok(NULL, " ");
    }

    args[count] = NULL;

    return count;
}


/* ---------- Main Shell ---------- */

int main() {

    char input[MAX_INPUT];

    printf("Mini Shell\n");
    printf("Supports variables and built-in commands.\n\n");

    while (1) {

        printf("shell> ");
        fflush(stdout);

        if (fgets(input, sizeof(input), stdin) == NULL)
            break;

        input[strcspn(input, "\n")] = '\0';

        if (strlen(input) == 0)
            continue;


        /* Variable expansion */

        char *expanded = expandVariables(input);

        if (expanded == NULL) {

            printf("Memory allocation failed\n");
            continue;
        }


        printf("Expanded: %s\n", expanded);


        /* Tokenize */

        char *args[MAX_ARGS];

        int count = tokenize(expanded, args);

        if (count == 0) {

            free(expanded);
            continue;
        }


        /* Find built-in */

        BuiltinFunction function = findBuiltin(args[0]);


        if (function != NULL) {

            /* Execute built-in inside shell process */

            function(args);

        }
        else {

            /* Invalid command */

            printf("shell: command not found: %s\n",
                   args[0]);
        }


        free(expanded);
    }

    return 0;
}

