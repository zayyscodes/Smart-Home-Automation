#include "shm.h"
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include "e_manage.h"

#define shm_name "/my_osproj_shm"
#define SIZE 10000000

extern SmartHome *shm;

void initialise(SmartHome *shm) {
    pthread_mutex_init(&(shm->mutex_enerin), NULL);
    pthread_mutex_init(&(shm->mutex_inuse), NULL);
    pthread_mutex_init(&(shm->mutex_preftemp), NULL);
    pthread_mutex_init(&(shm->mutex_svmd), NULL);
    pthread_mutex_init(&(shm->mutex_tasks), NULL);
    
    shm->curr = 0;
    shm->enerin = 0.0;
    shm->inuse = 0.0;
    shm->preftemp = 26;
    shm->svmd = 0; 
}

void setshm(SmartHome *shm) {
    pthread_mutex_lock(&(shm->mutex_preftemp));
    pthread_mutex_lock(&(shm->mutex_svmd));
    pthread_mutex_lock(&(shm->mutex_tasks));
    
    shm->svmd = 0;
init:
    fflush(stdin);
    
    pthread_mutex_lock(&(shm->mutex_inuse));
    printf("\nPrefered temperature in °C: ");
    scanf("%d", &(shm->preftemp));
    
    pthread_mutex_unlock(&(shm->mutex_enerin));
    pthread_mutex_unlock(&(shm->mutex_inuse));
    pthread_mutex_unlock(&(shm->mutex_preftemp));
    pthread_mutex_unlock(&(shm->mutex_svmd));
    pthread_mutex_unlock(&(shm->mutex_tasks));
}

SmartHome *getshm() {
    size_t sizeshm = sizeof(SmartHome);
    int shm_fd = shm_open(shm_name, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        return NULL;
    }

    if (ftruncate(shm_fd, sizeshm) == -1) {
        perror("ftruncate");
        return NULL;
    }

    SmartHome *data = (SmartHome *)mmap(NULL, sizeshm, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    close(shm_fd);

    if (data == MAP_FAILED) {
        perror("mmap");
        return NULL;
    }

    return data;
}

int checkArray(int array[], int size) {
    for (int i = 0; i < size; i++) {
        if (array[i] == 1) {
            return 0;
        }
    }
    return 1;
}

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

            break;
        }
    }

    fclose(file);

    return task;
}

void write_task_to_pipe(const char *type) {
    TASKS *task;

    task = readcsv("schedule.csv", type);

    if (task != NULL) {
        if (strlen(task->type) > 0 && strlen(task->deet) > 0) {
            if (shm->curr < TMAX) {
                shm->tasks[shm->curr] = *task;
                shm->curr += 1;
                printf("Task copied to shared memory successfully\n");
            } else {
                printf("Error: Shared memory task array is full\n");
            }
        }
        free(task);
    }
}

void tempchange(SmartHome *shm) {
    int temp;
    printf("Your thermostat was previously set to: %d°C\n", shm->preftemp);
    printf("Enter new temperature: ");
    scanf("%d", &temp);
    
    pthread_mutex_lock(&(shm->mutex_preftemp));    
    shm->preftemp = temp;
    printf("Your thermostat was previously set to: %d°C\n", shm->preftemp);
    pthread_mutex_unlock(&(shm->mutex_preftemp));
}

void detachSharedMemory(SmartHome *shm) {
    munmap(shm, sizeof(SmartHome));
    shm_unlink(shm_name);
}

