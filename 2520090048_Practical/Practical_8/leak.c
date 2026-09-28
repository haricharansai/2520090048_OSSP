int *leak = malloc(100 * sizeof(int));

if (leak == NULL)
{
    return 1;
}

/* Intentionally not calling free(leak) */gcc -g leak.c -o leak