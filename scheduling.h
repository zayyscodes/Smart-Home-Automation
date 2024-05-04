//scheduling
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include "shm.h"

#define FIELDS 1000
#define LEN 1024

void scheduling(void);
void fcfs(TASKS *tasks);
void sjf(TASKS *tasks);
void rr(TASKS *tasks, int time_quantum);

