#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include "shm.h"

#ifndef TEMPMENU_H
#define TEMPMENU_H

int settemp(void);
void changepref();
void* temperature_sensor(void* arg);
void displaytempmenu();

#endif
