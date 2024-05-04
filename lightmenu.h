#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include "shm.h"

#ifndef LIGHTMENU_H
#define LIGHTMENU_H

typedef struct {
    char area[LEN];
    int num;
    float watt;
    int stat;
} LightData;

int areasortt(const void *a, const void *b);
int numsort(const void *a, const void *b);
int lowtohighh(const void *a, const void *b);
int hightoloww(const void *a, const void *b);
void* switchincsv(void* arg);
void togglee(LightData light[]);
void displaylistt(void);
void* readflag(void* arg);
void displaylightmenu(void);

#endif
