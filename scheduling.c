//scheduling
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "scheduling.h"
#define FIELDS 1000
#define LEN 1024

// Predefined file descriptors for pipe operations
int pipe_fd[2];
int current_index = 0; // Global variable to keep track of the current index

TASKS* readcsv(const char* filename, const char* name) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Failed to open the file");
        return NULL;
    }

    int header = 0;
    char line[LEN];
    TASKS* task = NULL;

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
            task = (TASKS*)malloc(sizeof(TASKS));
            if (!task) {
                perror("Memory allocation failed");
                fclose(file);
                return NULL;
            }
	    
	    strcpy(task->type, type);
            strcpy(task->deet, deet);
            task->arrtime = clock();
            task->burst = burst;

            break; // Exit the loop after the task is found
        }
    }

    fclose(file);

    return task;
}


void write_task_to_pipe(const char* type) {
    TASKS* task = readcsv("schedule.csv", type);
    if (task != NULL) { // Check if task is not NULL
        // Check if the task details are valid before printing
        if (strlen(task->type) > 0 && strlen(task->deet) > 0) {
            // Print task details only if they are valid
            printf("\n\nTask details: %s\n", task->deet);
            printf("Task type: %s\n", task->type);
            printf("Task arrtime: %.2fms\n", task->arrtime);
            printf("Task burst: %.1fms\n", task->burst);
            // Write task details to the pipe
            close(pipe_fd[0]);
            write(pipe_fd[1], task, sizeof(TASKS));
            close(pipe_fd[1]);
        }
        free(task); // Free the allocated memory for task
    }
}



// Function to read a TASKS struct from a pipe
void read_task_from_pipe(TASKS *task_array, int *current_index) {
    ssize_t bytes_read;
    while ((bytes_read = read(pipe_fd[0], &task_array[*current_index], sizeof(TASKS))) > 0) {
        (*current_index)++;
        // Print the received task
        printf("Received task: type=%s, arrtime=%.2f, burst=%.2f\n",
               task_array[*current_index - 1].type, task_array[*current_index - 1].arrtime,
               task_array[*current_index - 1].burst);
    }
}

void scheduling(void){
algos:
	system("clear");
	
	int chs;
	printf("\n\nWhich scheduling would you like to implement?\n");
	printf("1, First Come-First Serve\n2, Shortest Job First\n3, Round Robin\n4, Exit");
	scanf("%d", &chs);
	
	switch(chs){
		case 1:
			fcfs();
		break;
		
		case 2:
			sjf();
		break;
		
		case 3:
			rr();
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


