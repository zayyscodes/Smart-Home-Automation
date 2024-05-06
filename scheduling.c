#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "passwordcheck.h"
#include "shm.h"
#include "scheduling.h"

#define FIELDS 1000
#define LEN 1024

extern SmartHome* shm;

// Function for First Come, First Serve (FCFS) Scheduling
void fcfs(TASKS *tasks) {
    // Open the text file for appending
    FILE* txt_file = fopen("task_assign_info.txt", "a");
    if (txt_file == NULL) {
        perror("Failed to open text file");
        return;
    }
    
    // Write the header for FCFS scheduling in the text file
    fprintf(txt_file, "\n\n\n\n****FIRST COME, FIRST SERVE SCHEDULING****\n");
    fprintf(txt_file, "\nFCFS Scheduling Algorithm:\n");
    fprintf(txt_file, "Step\t| Task Type\t\t| Module\t| Arrival Time\t| Burst Time |Completion Time\n");
    fprintf(txt_file, "--------------------------------------------------------------------------------------------------------\n");

    // Sort tasks based on arrival time (FCFS)
    for (int i = 0; i < shm->curr - 1; i++) {
        for (int j = 0; j < shm->curr - i - 1; j++) {
            if (tasks[j].arrtime > tasks[j + 1].arrtime) {
                TASKS temp = tasks[j];
                tasks[j] = tasks[j + 1];
                tasks[j + 1] = temp;
            }
        }
    }

    // Perform FCFS scheduling and write to the text file
    int completion_time = 0;
    for (int i = 0; i < shm->curr; i++) {
        fprintf(txt_file, "%d\t| %s\t\t| %s\t\t| %.2f\t\t| %.2f\t\t| %d\t\t|\n", i + 1, tasks[i].type, tasks[i].deet, tasks[i].arrtime, tasks[i].burst, completion_time);
        completion_time += tasks[i].burst;
    }
    
    fclose(txt_file);
}

// Function for Shortest Job First (SJF) Scheduling
void sjf(TASKS *tasks) {
    // Open the text file for appending
    FILE* txt_file = fopen("task_assign_info.txt", "a");
    if (txt_file == NULL) {
        perror("Failed to open text file");
        return;
    }
    
    // Write the header for SJF scheduling in the text file
    fprintf(txt_file, "\n\n\n\n****SHORTEST JOB FIRST SCHEDULING****\n");
    fprintf(txt_file, "\nSJF Scheduling Algorithm:\n");
    fprintf(txt_file, "Step\t| Task Type\t\t| Module\t| Arrival Time\t| Burst Time |Completion Time\n");
    fprintf(txt_file, "--------------------------------------------------------------------------------------------------------\n");

    // Sort tasks based on burst time (SJF)
    for (int i = 0; i < shm->curr - 1; i++) {
        for (int j = 0; j < shm->curr - i - 1; j++) {
            if (tasks[j].burst > tasks[j + 1].burst) {
                TASKS temp = tasks[j];
                tasks[j] = tasks[j + 1];
                tasks[j + 1] = temp;
            }
        }
    }

    // Perform SJF scheduling and write to the text file
    int completion_time = 0;
    for (int i = 0; i < shm->curr; i++) {
        fprintf(txt_file, "%d\t| %s\t\t| %s\t\t| %.2f\t\t| %.2f\t\t| %d\t\t|\n", i + 1, tasks[i].type, tasks[i].deet, tasks[i].arrtime, tasks[i].burst, completion_time);
        completion_time += tasks[i].burst;
    }
    
    fclose(txt_file);
}

// Function for Round Robin (RR) Scheduling
void rr(TASKS *tasks, int time_quantum) {
    // Open the text file for appending
    FILE* txt_file = fopen("task_assign_info.txt", "a");
    if (txt_file == NULL) {
        perror("Failed to open text file");
        return;
    }
    
    // Write the header for Round Robin scheduling in the text file
    fprintf(txt_file, "\n\n\n\n****ROUND ROBIN SCHEDULING****\n");
    fprintf(txt_file, "\nRound Robin Scheduling Algorithm (Time Quantum: %d):\n", time_quantum);
    fprintf(txt_file, "Step\t| Task Type\t\t| Module\t| Burst Time\t| Arrival Time |Completion Time\n");
    fprintf(txt_file, "--------------------------------------------------------------------------------------------------------\n");

    // Sort tasks based on arrival time (FCFS) before RR scheduling
    for (int i = 0; i < shm->curr - 1; i++) {
        for (int j = 0; j < shm->curr - i - 1; j++) {
            if (tasks[j].arrtime > tasks[j + 1].arrtime) {
                TASKS temp = tasks[j];
                tasks[j] = tasks[j + 1];
                tasks[j + 1] = temp;
            }
        }
    }

    // Create an array to store remaining burst times for each task
    int *rem = (int *)malloc(shm->curr * sizeof(int));
    for (int i = 0; i < shm->curr; i++) {
        rem[i] = tasks[i].burst;
    }

    // Perform Round Robin scheduling and write to the text file
    int current_time = 0;
    int completed = 0;
    while (completed < shm->curr) {
        for (int i = 0; i < shm->curr; i++) {
            if (rem[i] > 0) {
                int time_slice = (rem[i] < time_quantum) ? rem[i] : time_quantum;
                current_time += time_slice;
                rem[i] -= time_slice;

                if (rem[i] == 0) {
                    completed++;
                    fprintf(txt_file, "%d\t| %s\t\t| %s\t\t| %.2f\t\t| %.2f\t\t| %d\t\t|\n", completed, tasks[i].type, tasks[i].deet, tasks[i].arrtime, tasks[i].burst, current_time);
                }
            }
        }
    }
    fclose(txt_file);
    free(rem);
}

// Main function for scheduling tasks
void scheduling(void) {
algos:
    system("clear");

    // Check if there are tasks available for scheduling
    if (shm->curr != 0) {
        printf("\n\nWhich scheduling would you like to implement?\n");
        printf("1, First Come-First Serve\n2, Shortest Job First\n3, Round Robin\n4, Exit\n");
        int chs;
        scanf("%d", &chs);

        switch (chs) {
            case 1:
                fcfs(shm->tasks); // Perform FCFS scheduling
                break;

            case 2:
                sjf(shm->tasks); // Perform SJF scheduling
                break;

            case 3:
                int qt;
                printf("\nEnter Time Quantum for Round Robin Scheduling (in ms): ");
                scanf("%d", &qt);
                rr(shm->tasks, qt); // Perform Round Robin scheduling
                break;

            case 4:
                return; // Exit from scheduling
                break;

            default:
                printf("Invalid choice.");
                goto algos;
                break;
        }
    } else {
    	printf("\n\nThere are no tasks to schedule.\n");
    	return;
    }
}
