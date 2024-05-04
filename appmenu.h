#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include "shm.h"

#ifndef APPMENU_H
#define APPMENU_H


typedef struct{
	char name[LEN];
	char area[LEN];
	float watt;
	int stat;
}AppData;

int namesort(const void *a, const void *b);
int areasort(const void *a, const void *b);
int lowtohigh(const void *a, const void *b);
int hightolow(const void *a, const void *b);
int statsort(const void *a, const void *b);
void* switchcsv(void* arg);
void toggle(AppData entry[]);
void displaylist(void);
void* readflags(void* arg);
void displayappmenu(void);

#endif
