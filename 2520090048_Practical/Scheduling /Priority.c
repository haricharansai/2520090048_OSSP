#include <stdio.h>

int main() {
    int n, i, j;
    int bt[20], priority[20], wt[20], tat[20], p[20]; // Added 'p' array to track IDs
    int temp;
    float total_wt = 0, total_tat = 0;

    printf("Enter number of processes: ");
    if (scanf("%d", &n) != 1) return 1;

    printf("Enter burst time and priority:\n");
    for (i = 0; i < n; i++) {
        printf("P%d Burst Time: ", i + 1);
        if (scanf("%d", &bt[i]) != 1) return 1;

        printf("P%d Priority: ", i + 1);
        if (scanf("%d", &priority[i]) != 1) return 1;

        p[i] = i + 1; 
    }

    
    for (i = 0; i < n - 1; i++) {
        for (j = i + 1; j < n; j++) {
            if (priority[i] > priority[j]) {
                
                temp = priority[i];
                priority[i] = priority[j];
                priority[j] = temp;

                
                temp = bt[i];
                bt[i] = bt[j];
                bt[j] = temp;

                
                temp = p[i];
                p[i] = p[j];
                p[j] = temp;
            }
        }
    }

    
    wt[0] = 0;
    for (i = 1; i < n; i++) {
        wt[i] = wt[i - 1] + bt[i - 1];
        total_wt += wt[i];
    }

    
    for (i = 0; i < n; i++) {
        tat[i] = wt[i] + bt[i];
        total_tat += tat[i];
    }

    
    printf("\nProcess\tBT\tPriority\tWT\tTAT\n");
    for (i = 0; i < n; i++) {
        printf("P%d\t%d\t%d\t\t%d\t%d\n",
               p[i], bt[i], priority[i], wt[i], tat[i]); // Replaced i + 1 with p[i]
    }


    printf("\nAverage Waiting Time: %.2f", total_wt / n);
    printf("\nAverage Turnaround Time: %.2f\n", total_tat / n);

    return 0;
}
