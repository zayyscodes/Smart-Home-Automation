#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include "lightmenu.h"

#define MAX 100
#define LEN 1024

typedef struct{
	char area[LEN];
	int num;
	float watt;
}LightData;

void* readflags(void* arg){
	FILE *file = fopen("lights_data.csv", "r");
	if (!file){
		perror("Failed to open file");
		return NULL;
	}
	
	char line[LEN];
	LightData *entry = malloc(MAX * sizeof(LightData));
	int count = 0;
	
	while (fgets(line, sizeof(line), file)){
		char area[LEN];
		int num;
		float watt;
		int flag;
		
		sscanf(line, "%[^,],%d,%f,%d", area, &num, &watt, &flag);
		
		if (flag==1){
			strncpy(entry[count].area, area, sizeof(entry[count].area) - 1);
                        entry[count].area[sizeof(entry[count].area) - 1] = '\0'; 
                        entry[count].num = num;
                        entry[count].watt = watt;
                        count++;
		}	
	}
	fclose(file);
	
	if (count == 0)
		return NULL;
	else
		return entry;
}

void displaylightmenu(){
start:
	int choice;
	printf("\n\nMenu:\n1, Switch Off/On Light\n2, Check Lights\nEnter Choice:");
	scanf("%d", choice);
	
	switch(choice){
		case 1:{
			
		break;
		}
		
		case 2:{
			pthread_t thread;
			if (pthread_create(&thread, NULL, readflags, NULL)){
				perror("Failed to create thread.");
				break;
			}
			
			LightData *entries;
			if (pthread_join(thread, (void**) &entries)){
				perror("Failed to join");
				break;
			}
			
			if (entries == NULL){
				printf("Lights aren't open for any area.");
				break;
			}
			
			printf("Entries with flag = 1:\n");
		        for (int i = 0; entries[i].area[0] != '\0'; i++) {
		        printf("Area: %s, Num: %d, Watt: %.2f\n", entries[i].area, entries[i].num, entries[i].watt);
		        }
		break;
		}
		
		default:{
			printf("\nInvalid Choice.");
			goto start;
		break;
		}
	}	
	
}
