#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "passwordcheck.h"

#include "scheduling.h"

#define READ 0
#define WRITE 1

#define FIELDS 1000
#define LEN 1024

// Predefined file descriptors for pipe operations
extern int pipe_fd[2];
int curr = 0; // Global variable to keep track of the current index

int checkArray(int array[], int size) {
    for (int i = 0; i < size; i++) {
        if (array[i] == 1) {
            return 0;
        }
    }
    return 1;
}

// Function to read a TASKS struct from a pipe
void read_task_from_pipe(TASKS *arr) {
    ssize_t bytes_read; // Variable to store the number of bytes read

    // Loop to read tasks from the pipe
    while (1) {
        bytes_read = read(pipe_fd[READ], &arr[curr], sizeof(TASKS)); // Read from pipe
        if (bytes_read > 0) {
            curr++; // Increment the index

            // Print the received task
            printf("Received task: type=%s, arrtime=%.2f, burst=%.2f\n",
                   arr[curr - 1].type, arr[curr - 1].arrtime,
                   arr[curr - 1].burst);
        } else {
            break; // Exit the loop if no bytes are read (end of pipe)
        }
    }
}

// Read task from CSV file based on process name
TASKS *readcsv(const char *filename, const char *name) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Failed to open the file");
        return NULL;
    }

    int header = 0;
    char line[LEN];
    TASKS *task = NULL;

    while (fgets(line, sizeof(line), file)) {
        if (!header) {
            header = 1;
            continue;
        }
        char pro[LEN];
        char deet[LEN];
        char type[LEN];
        double arrtime;
        double burst;

        if (sscanf(line, "%[^,],%[^,],%[^,],%lf", pro, deet, type, &burst) != 4) {
            fprintf(stderr, "Invalid line format: %s", line);
            continue;
        }

        if (strcmp(pro, name) == 0) {
            task = (TASKS *)malloc(sizeof(TASKS));
            if (!task) {
                perror("Memory allocation failed");
                fclose(file);
                return NULL;
            }

            strcpy(task->type, deet);
            strcpy(task->deet, type);
            task->arrtime = clock();
            task->burst = burst;

            break; // Exit the loop after the task is found
        }
    }

    fclose(file);

    return task;
}

void write_task_to_pipe(const char *type) {
    // Open the pipe
    if (pipe(pipe_fd) == -1) {
        perror("pipe");
        return;
    }

    TASKS *task = readcsv("schedule.csv", type);
    if (task != NULL) {
        if (strlen(task->type) > 0 && strlen(task->deet) > 0) {
            printf("\n\nTask details: %s\n", task->deet);
            printf("Task type: %s\n", task->type);
            printf("Task arrtime: %.2fms\n", task->arrtime);
            printf("Task burst: %.1fms\n", task->burst);

            // Write to the pipe
            ssize_t bytes_written = write(pipe_fd[WRITE], task, sizeof(TASKS));
            if (bytes_written == -1) {
                perror("write");
            } else if (bytes_written != sizeof(TASKS)) {
                fprintf(stderr, "Incomplete write to pipe\n");
            } else {
                printf("Task written to pipe successfully\n");
            }
        }
        free(task);
    }

    // Close the write end of the pipe
    close(pipe_fd[WRITE]);
}




// Function prototypes for scheduling algorithms
void fcfs(TASKS *tasks) {
    printf("\nFCFS Scheduling Algorithm:\n");
    printf("Step\t| Task Type\t\t| Module\t| Arrival Time\t| Burst Time |Completion Time\n");
    printf("--------------------------------------------------------------------------------------------------------\n");

    // Sort tasks based on arrival time (First Come First Serve)
    for (int i = 0; i < curr - 1; i++) {
        for (int j = 0; j < curr - i - 1; j++) {
            if (tasks[j].arrtime > tasks[j + 1].arrtime) {
                TASKS temp = tasks[j];
                tasks[j] = tasks[j + 1];
                tasks[j + 1] = temp;
            }
        }
    }

    int completion_time = 0;
    for (int i = 0; i < curr; i++) {
        printf("%d\t| %s\t\t| %s\t\t| %.2f\t\t| %.2f\t\t| %d\t\t|\n", i + 1, tasks[i].type, tasks[i].deet, tasks[i].arrtime, tasks[i].burst, completion_time);
        completion_time += tasks[i].burst;
    }
}

void sjf(TASKS *tasks) {
    printf("\nSJF Scheduling Algorithm:\n");
    printf("Step\t| Task Type\t\t| Module\t| Arrival Time\t| Burst Time |Completion Time\n");
    printf("--------------------------------------------------------------------------------------------------------\n");

    // Sort tasks based on burst time (Shortest Job First)
    for (int i = 0; i < curr - 1; i++) {
        for (int j = 0; j < curr - i - 1; j++) {
            if (tasks[j].burst > tasks[j + 1].burst) {
                TASKS temp = tasks[j];
                tasks[j] = tasks[j + 1];
                tasks[j + 1] = temp;
            }
        }
    }

    int completion_time = 0;
    for (int i = 0; i < curr; i++) {
        printf("%d\t| %s\t\t| %s\t\t| %.2f\t\t| %.2f\t\t| %d\t\t|\n", i + 1, tasks[i].type,
               tasks[i].deet, tasks[i].arrtime, tasks[i].burst, completion_time);
        completion_time += tasks[i].burst;
    }
}

void rr(TASKS *tasks, int time_quantum) {
    printf("\nRound Robin Scheduling Algorithm (Time Quantum: %d):\n", time_quantum);
    printf("Step\t| Task Type\t\t| Module\t| Burst Time\t| Arrival Time |Completion Time\n");
    printf("--------------------------------------------------------------------------------------------------------\n");

    // Sort tasks based on arrival time (First Come First Serve)
    for (int i = 0; i < curr - 1; i++) {
        for (int j = 0; j < curr - i - 1; j++) {
            if (tasks[j].arrtime > tasks[j + 1].arrtime) {
                TASKS temp = tasks[j];
                tasks[j] = tasks[j + 1];
                tasks[j + 1] = temp;
            }
        }
    }

    int *rem = (int *)malloc(curr * sizeof(int));
    for (int i = 0; i < curr; i++) {
        rem[i] = tasks[i].burst;
    }

    int current_time = 0;
    int completed = 0;
    while (completed < curr) {
        for (int i = 0; i < curr; i++) {
            if (rem[i] > 0) {
                int time_slice = (rem[i] < time_quantum) ? rem[i] : time_quantum;
                current_time += time_slice;
                rem[i] -= time_slice;

                if (rem[i] == 0) {
                    completed++;
                    printf("%d\t| %s\t\t| %s\t\t| %.2f\t\t| %.2f\t\t| %d\t\t|\n", completed, tasks[i].type,
                           tasks[i].deet, tasks[i].arrtime, tasks[i].burst, current_time);
                }
            }
        }
    }

    free(rem);
}

// Main scheduling function
void scheduling(void) {
    algos:
    system("clear");
    
    TASKS *tasks = (TASKS *)malloc((curr + 1) * sizeof(TASKS)); 
    
    read_task_from_pipe(tasks);
    
    printf("\n\nWhich scheduling would you like to implement?\n");
    printf("1, First Come-First Serve\n2, Shortest Job First\n3, Round Robin\n4, Exit\n");
    int chs = getchoice();

    switch (chs) {
        case 1:
            fcfs(tasks);
            break;

        case 2:
            sjf(tasks);
            break;

        case 3:
            int qt;
            printf("\nEnter Time Quantum for Round Robin Scheduling (in ms): ");
            scanf("%d", &qt);
            rr(tasks, qt);
            break;

        case 4:
            return;
            break;

        default:
            printf("Invalid choice.");
            goto algos;
            break;
    }
}


