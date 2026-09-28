#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int global_var = 100;
int uninitialized_global;

static int static_var = 200;

void code_function()
{
    printf("Inside code_function()\n");
}

int main()
{
    int stack_var = 300;

    int *heap_var = malloc(sizeof(int));

    if (heap_var == NULL)
    {
        perror("malloc");
        return 1;
    }

    *heap_var = 400;

    printf("========================================\n");
    printf("     Linux Process Memory Addresses\n");
    printf("========================================\n");

    printf("PID                         : %d\n", getpid());

    printf("\n--- CODE / TEXT ---\n");
    printf("Address of main()           : %p\n", (void *)main);
    printf("Address of code_function()   : %p\n", (void *)code_function);

    printf("\n--- GLOBAL VARIABLES ---\n");
    printf("Address of global_var       : %p\n", (void *)&global_var);
    printf("Address of uninitialized_global : %p\n",
           (void *)&uninitialized_global);

    printf("\n--- STATIC VARIABLE ---\n");
    printf("Address of static_var       : %p\n", (void *)&static_var);

    printf("\n--- HEAP ---\n");
    printf("Address returned by malloc  : %p\n", (void *)heap_var);

    printf("\n--- STACK ---\n");
    printf("Address of stack_var        : %p\n", (void *)&stack_var);

    printf("\nKeep the process running...\n");
    printf("Press ENTER to terminate.\n");

    getchar();

    free(heap_var);

    return 0;
}