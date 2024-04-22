#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#define MAX 100
#define LEN 1024

typedef struct{
	char appname[LEN];
	char area[LEN];
	float watt;
}AppData;

void* readflags(void* arg);

void displayappmenu(void){
menuu:
	int choice;
	printf("\n\nMenu:\n1, Switch Off/On appliance\n2, Check appliances\nEnter Choice:");
	scanf("%d", &choice);
	
	switch(choice){
		
	}
		
}
