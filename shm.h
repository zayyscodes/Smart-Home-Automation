// shared_memory.h
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#ifndef SHARED_MEMORY_H
#define SHARED_MEMORY_H

#define SHM_SIZE 1024  // Size of shared memory segment
#define LEN 1024 

// Structure to hold shared data
typedef struct {
    float enerin;
    float inuse;
    int secure; //flag whether security setting is enabled
    int preftemp;
    int svmd; //flag whether energy save mode is on or not
    pthread_mutex_t mutex_enerin;  // Mutex for protecting enerin
    pthread_mutex_t mutex_inuse;  // Mutex for protecting inuse
    pthread_mutex_t mutex_preftemp;  // Mutex for protecting preftemp
    pthread_mutex_t mutex_svmd;  // Mutex for protecting svmd
    pthread_mutex_t mutex_tasks;  // Mutex for protecting tasks 
} SmartHome;


SmartHome *getshm();
void setshm(SmartHome *shm);
void initialise(SmartHome *shm);
void tempchange(SmartHome *shm);
void detachSharedMemory(SmartHome *shm);

#endif

