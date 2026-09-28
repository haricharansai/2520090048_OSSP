#include <stdio.h>
#include <stdlib.h>

int main()
{
    printf("===== Dynamic Memory Allocation =====\n\n");

    /* ---------------------------------
       1. malloc()
       --------------------------------- */

    int *malloc_ptr = malloc(5 * sizeof(int));

    if (malloc_ptr == NULL)
    {
        printf("malloc failed\n");
        return 1;
    }

    for (int i = 0; i < 5; i++)
    {
        malloc_ptr[i] = (i + 1) * 10;
    }

    printf("malloc():\n");
    for (int i = 0; i < 5; i++)
    {
        printf("%d ", malloc_ptr[i]);
    }
    printf("\n");


    /* ---------------------------------
       2. calloc()
       --------------------------------- */

    int *calloc_ptr = calloc(5, sizeof(int));

    if (calloc_ptr == NULL)
    {
        printf("calloc failed\n");
        free(malloc_ptr);
        return 1;
    }

    printf("\ncalloc():\n");

    for (int i = 0; i < 5; i++)
    {
        printf("%d ", calloc_ptr[i]);
    }

    printf("\n");


    /* ---------------------------------
       3. realloc()
       --------------------------------- */

    int *temp = realloc(malloc_ptr, 10 * sizeof(int));

    if (temp == NULL)
    {
        printf("realloc failed\n");

        free(malloc_ptr);
        free(calloc_ptr);

        return 1;
    }

    malloc_ptr = temp;

    for (int i = 5; i < 10; i++)
    {
        malloc_ptr[i] = (i + 1) * 10;
    }

    printf("\nAfter realloc():\n");

    for (int i = 0; i < 10; i++)
    {
        printf("%d ", malloc_ptr[i]);
    }

    printf("\n");


    /* ---------------------------------
       4. free()
       --------------------------------- */

    free(malloc_ptr);
    free(calloc_ptr);

    malloc_ptr = NULL;
    calloc_ptr = NULL;

    printf("\nMemory successfully released using free().\n");

    return 0;
}
