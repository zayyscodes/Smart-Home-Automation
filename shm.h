// shared_memory.h
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#ifndef SHARED_MEMORY_H
#define SHARED_MEMORY_H

#define SHM_SIZE 1024  // Size of shared memory segment

// Structure to hold shared data
typedef struct {
    // Define your shared data structure here
    float enerin;
    int inuse;
    int secure; //flag whether security setting is enabled
    int preftemp;
    int svmd; //flag whether energy save mode is on or not
} SmartHome;

SmartHome* getshm();

#endif

