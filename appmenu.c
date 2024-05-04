#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include "appmenu.h"
#include "e_manage.h"
#include "shm.h"
#include "scheduling.h"

#define MAX 29
#define LEN 1024

extern pthread_mutex_t update;
extern pthread_mutex_t mutex;
extern SmartHome *shm; //pointer to shared memory
extern int flags[5];
extern int appupdated;

/*  	  S O R T I N G   F U N C T I O N S   	  */

int namesort(const void *a, const void *b) {
    return strcmp(((AppData *)a)->name, ((AppData *)b)->name);
}

int areasort(const void *a, const void *b) {
    return strcmp(((AppData *)a)->area, ((AppData *)b)->area);
}

int lowtohigh(const void *a, const void *b) {
    if (((AppData *)a)->watt < ((AppData *)b)->watt)
   	 return -1;
    else if (((AppData *)a)->watt > ((AppData *)b)->watt)
   	 return 1;
    else
   	 return 0;
}

int hightolow(const void *a, const void *b) {
    if (((AppData *)a)->watt < ((AppData *)b)->watt) {
   	 return 1;
    } else if (((AppData *)a)->watt > ((AppData *)b)->watt) {
   	 return -1;
    } else {
   	 return 0;
    }
}

int statsort(const void *a, const void *b) {
    return -(((AppData *)a)->stat - ((AppData *)b)->stat);
}



/*	     T H R E A D   T O   U P D A T E		*/

void* switchcsv(void* arg) {
    AppData* app = (AppData*) arg;
    int onoff;
shuru:
    int ci;
    printf("\nWhich appliance do you want to switch on/off? Enter Serial Number (0 to exit): ");
    scanf("%d", &ci);

    int sno = (ci - 1);

    if (sno < 0)
        return NULL;

    printf("Name: %s\nArea: %s\nWatt: %.2fkWh\n", app[sno].name, app[sno].area, app[sno].watt);
    if (app[sno].stat == 0)
        printf("Stat: OFF\n");
    else if (app[sno].stat == 1)
        printf("Stat: ON\n");

    if (app[sno].stat == 0) {
        printf("Do you want to switch the appliance ON? ");
        onoff = 1;
    } else if (app[sno].stat == 1) {
        printf("Do you want to switch the appliance OFF? ");
        onoff = 0;
    }

    char ch;
    printf("Enter choice (Y/N): ");
    fflush(stdin);
    scanf(" %c", &ch);

    if (ch == 'N' || ch == 'n')
        goto shuru;
    else if (ch == 'Y' || ch == 'y') {
        if (shm->inuse >= shm->enerin || (shm->inuse+app[sno].watt) > shm->enerin && onoff == 1){
            printf("Unable to switch appliance ON; exceeds input");
            appupdated = 0;
        } else if (shm->svmd == 1 && app[sno].watt >= 1.75) {
            printf("Unable to switch appliance ON; Energy Saver Mode enabled\n");
        } else {
            app[sno].stat = (app[sno].stat == 0) ? 1 : 0;
            if (app[sno].watt > 1.75){
                pthread_mutex_lock(&mutex);
                write_task_to_pipe("appover");
                printf("Written to ARRAY\n\n");
                pthread_mutex_unlock(&mutex);
            } else {
                pthread_mutex_lock(&mutex);
                write_task_to_pipe("appunder");
                printf("Written to ARRAY\n\n");
                pthread_mutex_unlock(&mutex);
            }

            FILE* file = fopen("appliances_data (copy).csv", "w");
            if (file == NULL) {
                perror("Failed to open file");
                return NULL;
            }

            fprintf(file, "name,area,watt,stat\n");

            for (int i = 0; i < MAX; i++) {
                fprintf(file, "%s,%s,%.2f,%d\n", app[i].name, app[i].area, app[i].watt, app[i].stat);
            }

            fclose(file);
            
            FILE* txt_file = fopen("task_assign_info.txt", "a");
            if (txt_file == NULL) {
                perror("Failed to open text file");
                return NULL;
            }

            fprintf(txt_file, "\n\n\n****APPLIANCE CONTROL****\n");

            fprintf(txt_file, "Name: %s\nArea: %s\nWatt: %.2fkWh\n", app[sno].name, app[sno].area, app[sno].watt);
            fprintf(txt_file, "Status: %s\n\n", (app[sno].stat == 0) ? "OFF" : "ON");

            fclose(txt_file);

            printf("Text file updated successfully.\n");
            appupdated = 1;
        }
        
    }

    pthread_exit(NULL);
}



		
		
		 
	 


void toggle(AppData entry[]){
	  pthread_mutex_lock(&update);
	  pthread_t toggle;
	  if (pthread_create(&toggle, NULL, switchcsv, (void*)entry)!=0){
		  perror("Failed to create thread.\n");
		  return;
	  }
	  if (pthread_join(toggle, NULL)){
		  perror("Failed to join thread.\n");
		  return;
	  }
	  pthread_mutex_unlock(&update);
}


void displaylist(void){
again:
    int ch;
    printf("Display options:\n1, As Default\n2, Sorted\nEnter choice: ");
    scanf("%d", &ch);
    switch(ch){
        case 1: {
            pthread_mutex_lock(&update);
            FILE *file = fopen("appliances_data (copy).csv", "r");
            if (!file){
                perror("Failed to open file");
                goto again;
            }
            
            char line[LEN];
            int count=0;
            AppData *entry = malloc(MAX * sizeof(AppData));
            int i = 1; 
            int header = 0; 

            printf("\n\n\t\t\tAppliances List:");

            while (fgets(line, sizeof(line), file)){
                if (!header) {
                    header = 1;
                    continue;
                }

                char name[LEN];
                char area[LEN];
                float watt;
                int flag;
                
                sscanf(line, "%[^,],%[^,],%f,%d", name, area, &watt, &flag);
                strncpy(entry[count].name, name, sizeof(entry[count].name) - 1);
                entry[count].name[sizeof(entry[count].name) - 1] = '\0';
                strncpy(entry[count].area, area, sizeof(entry[count].area) - 1);
                entry[count].area[sizeof(entry[count].area) - 1] = '\0';
                entry[count].watt = watt;
                entry[count].stat = flag;
                count++;
                
                sscanf(line, "%[^,],%[^,],%f,%d", name, area, &watt, &flag);
                printf("\nSno: %d\nAppliance: %s\nArea: %s\nEnergy Consumption: %.2fkWh\n", i, name, area, watt);
                if(flag == 0)
                    printf("Stat: OFF\n");
                else if (flag == 1)
                    printf("Stat: ON\n");
                i++;
            }
            
            fclose(file);
            pthread_mutex_unlock(&update);
            toggle(entry);
            break;
        }
        
        case 2: {
            pthread_mutex_lock(&update);
            FILE *file = fopen("appliances_data (copy).csv", "r");
            if (!file){
                perror("Failed to open file");
                goto again;
            }
            
            char line[LEN];
            int count=0;
            AppData *entry = malloc(MAX * sizeof(AppData));
            int i = 1; 
            int header = 0; 

            while (fgets(line, sizeof(line), file)){
                if (!header) {
                    header = 1;
                } else {
                    char name[LEN];
                    char area[LEN];
                    float watt;
                    int flag;
                    
                    sscanf(line, "%[^,],%[^,],%f,%d", name, area, &watt, &flag);
                    strncpy(entry[count].name, name, sizeof(entry[count].name) - 1);
                    entry[count].name[sizeof(entry[count].name) - 1] = '\0';
                    strncpy(entry[count].area, area, sizeof(entry[count].area) - 1);
                    entry[count].area[sizeof(entry[count].area) - 1] = '\0';
                    entry[count].watt = watt;
                    entry[count].stat = flag;
                    count++;
                }    
            }
            fclose(file);
            pthread_mutex_unlock(&update);
        sort:
            int c;
            printf("\n\nSorted list display options:\n1, Alphabetically\n2, Sorted Area Wise\n3, Lowest to Highest Consumption\n4, Highest to Lowest Consumption\n5, By their status\nEnter choice: ");
            scanf("%d", &c);
            switch(c){
                case 1:{
                    qsort(entry, count, sizeof(AppData), namesort);
                    printf("Sorted Appliances List (by area):\n");
                    for (int i = 0; i < count; i++){
                        printf("\nSno: %d\n", i+1);
                        printf("Name: %s\nArea: %s\nWatt: %.2fkWh\n", entry[i].name, entry[i].area, entry[i].watt);
                        if(entry[i].stat == 0)
                            printf("Stat: OFF\n");
                        else if (entry[i].stat == 1)
                            printf("Stat: ON\n");
                    }
                    break;
                }
                
                case 2:{
                    qsort(entry, count, sizeof(AppData), areasort);
                    printf("Sorted Appliances List (by area):\n");
                    for (int i = 0; i < count; i++){
                        printf("\nSno: %d\n", i+1);
                        printf("Name: %s\nArea: %s\nWatt: %.2fkWh\n", entry[i].name, entry[i].area, entry[i].watt);
                        if(entry[i].stat == 0)
                            printf("Stat: OFF\n");
                        else if (entry[i].stat == 1)
                            printf("Stat: ON\n");
                    }
                    break;
                }
                
                case 3:{
                    qsort(entry, count, sizeof(AppData), lowtohigh);
                    printf("Sorted Appliances List (by watt, lowest to highest):\n");
                    for (int i = 0; i < count; i++){
                        printf("\nSno: %d\n", i+1);
                        printf("Name: %s\nArea: %s\nWatt: %.2fkWh\n", entry[i].name, entry[i].area, entry[i].watt);
                        if(entry[i].stat == 0)
                            printf("Stat: OFF\n");
                        else if (entry[i].stat == 1)
                            printf("Stat: ON\n");
                    }
                    break;
                }
                
                case 4:{
                    qsort(entry, count, sizeof(AppData), hightolow);
                    printf("Sorted Appliances List (by watt, lowest to highest):\n");
                    for (int i = 0; i < count; i++){
                        printf("\nSno: %d\n", i+1);
                        printf("Name: %s\nArea: %s\nWatt: %.2fkWh\n", entry[i].name, entry[i].area, entry[i].watt);
                        if(entry[i].stat == 0)
                            printf("Stat: OFF\n");
                        else if (entry[i].stat == 1)
                            printf("Stat: ON\n");
                    }
                    break;
                }
                
                case 5:{
                    qsort(entry, count, sizeof(AppData), statsort);
                    printf("Sorted Appliances List (by status):\n");
                    for (int i = 0; i < count; i++){
                        printf("\nSno: %d\n", i+1);
                        printf("Name: %s\nArea: %s\nWatt: %.2fkWh\n", entry[i].name, entry[i].area, entry[i].watt);
                        if(entry[i].stat == 0)
                            printf("Stat: OFF\n");
                        else if (entry[i].stat == 1)
                            printf("Stat: ON\n");
                    }
                    break;
                }
                
                default:{
                    printf("\nInvalid Choice.");
                    goto sort;
                    break;
                }
            }
            toggle(entry);
            break;
        }
        
        default:{
            printf("\nInvalid Choice.");
            goto again;
            break;
        }
    }
}


void* readflags(void* arg){
    pthread_mutex_lock(&update);
    FILE *file = fopen("appliances_data (copy).csv", "r");
    if (!file){
        perror("Failed to open file");
        return NULL;
    }
    
    char line[LEN];
    AppData *entry = malloc(MAX * sizeof(AppData));
    int count = 0;
    
    while (fgets(line, sizeof(line), file)){
        char name[LEN];
        char area[LEN];
        float watt;
        int flag;
        
        sscanf(line, "%[^,],%[^,],%f,%d", name, area, &watt, &flag);
        
        if (flag==1){
            strncpy(entry[count].name,  name, sizeof(entry[count].name) - 1);
            entry[count].name[sizeof(entry[count].name) - 1] = '\0';
            strncpy(entry[count].area, area, sizeof(entry[count].area) - 1);
            entry[count].area[sizeof(entry[count].area) - 1] = '\0';
            entry[count].watt = watt;
            count++;
        }    
    }
    fclose(file);
    pthread_mutex_unlock(&update);
    
    if (count == 0)
        return NULL;
    else
        return entry;
}


void displayappmenu(void){
menuu:
    if (appupdated == 1){
        pthread_mutex_lock(&(shm->mutex_inuse));
        setconsume();
        pthread_mutex_unlock(&(shm->mutex_inuse));    
        int choice;
        printf("\n\nMenu:\n1, Switch Off/On appliance\n2, Check appliances\n3, Go to Main menu\nEnter Choice: ");
        scanf("%d", &choice);
        
        switch(choice){
            case 1:{
                displaylist();    
                goto menuu;     
                break;
            }
            
            case 2:{
                pthread_t thread;
                if (pthread_create(&thread, NULL, readflags, NULL)){
                    perror("Failed to create thread.");
                    break;
                }
                
                AppData *entries;
                if (pthread_join(thread, (void**) &entries)){
                    perror("Failed to join");
                    break;
                }
                
                if (entries == NULL){
                    printf("No appliance is currently in use.");
                    break;
                }
                
                printf("Entries with flag = 1:\n");
                for (int i = 0; entries[i].area[0] != '\0'; i++) {
                    printf("Appliance: %s, Area: %s, Energy Consumption: %.2fkWh\n", entries[i].name, entries[i].area, entries[i].watt);
                }
                break;
            }
            
            case 3: {
                return;
            }
            
            default:{
                printf("\nInvalid Choice.");
                goto menuu;
                break;
            }
        }   
    } 
}


