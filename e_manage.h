#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "shm.h"

#define max 256

#ifndef ECONS_H
#define ECONS_H

void setinput();
void setconsume(float x);
void getenergy(void);
void consumingInitial(void);
void* get_energy_data(void* arg);

#endif
