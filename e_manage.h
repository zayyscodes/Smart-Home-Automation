#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define max 256

#ifndef ECONS_H
#define ECONS_H

float getenergy(void);
float consumingInitial(void);
void* get_energy_data(void* arg);

#endif
