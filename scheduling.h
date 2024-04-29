//scheduling
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

#define FIELDS 1000
#define LEN 1024

// Structure to define Task array
typedef struct {
    char type[LEN]; 
    char deet[LEN]; //light, temp or appliances
    double arrtime;
    double burst;
} TASKS;

// Predefined file descriptors for pipe operations
extern int pipe_fd[2];

TASKS* readcsv(const char* filename, const char* name);

// Function to write a TASKS struct to a pipe
void write_task_to_pipe(const char* type);

// Function to read a TASKS struct from a pipe
void read_task_from_pipe(TASKS *task_array, int *current_index);

void scheduling(void);

