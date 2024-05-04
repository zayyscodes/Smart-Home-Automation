#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include "tempmenu.h"
#include "shm.h"
#include <time.h>
#include <unistd.h>
#include "scheduling.h"

#define MAX 1024

extern pthread_mutex_t mutex;
extern SmartHome *shm;

void* get_temp_data(void* arg) {
    int ssno = *(int*)arg;
    int* temp = malloc(sizeof(int)); 

    if (!temp) {
        fprintf(stderr, "Memory allocation failed\n");
        return NULL;
    }

    const char *filename = "temp_rand.csv"; 
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Failed to open the file");
        free(temp);
        return NULL;
    }

    char line[MAX];
    int sno;
    int temp_in;
    int found = 0;

    while (fgets(line, sizeof(line), file)) {
        sscanf(line, "%d,%d", &sno, &temp_in);
        if (sno == ssno) {
            found = 1;
            *temp = temp_in; 
            break;
        }
    }

    fclose(file);

    if (!found) {
        printf("Serial number %d not found in the file.\n", ssno);
        *temp = 0; 
    }

    return (void*)temp;
}

int settemp(void) {
    srand(time(NULL));
    int sno = rand() % 100 + 1;
    pthread_t input;
    int* temp;

    if (pthread_create(&input, NULL, get_temp_data, &sno) != 0) {
        fprintf(stderr, "Error creating thread\n");
        return 0;
    }

    if (pthread_join(input, (void**)&temp) != 0) {
        fprintf(stderr, "Error joining thread\n");
        return 0;
    }

    int result = *temp;
    free(temp); 

    return result;
}

void* temperature_sensor(void* arg) {
    int temp = settemp();
    printf("\n\n\nTemperature Sensor Reading: %d°C\n", temp);
    
    if (temp < shm->preftemp){
        printf("Temperature set to %d\n", shm->preftemp);
        printf("Thermostat increased by %d\n\n\n", (shm->preftemp-temp));
        pthread_mutex_lock(&mutex);
        write_task_to_pipe("thermoinc");
        pthread_mutex_unlock(&mutex);
    } else if (temp > shm->preftemp){
        printf("Temperature set to %d\n", shm->preftemp);
        printf("Thermostat decreased by %d\n\n\n", (temp-shm->preftemp));
        pthread_mutex_lock(&mutex);
        write_task_to_pipe("thermodec");
        pthread_mutex_unlock(&mutex);
    } else {
        printf("No change to thermostat.\n\n\n");
    }
    
    FILE* txt_file = fopen("task_assign_info.txt", "a");
    if (txt_file == NULL) {
        perror("Failed to open text file");
        return NULL;
    }

    fprintf(txt_file, "\n\n\n****TEMPERATURE CONTROL****\n");
    fprintf(txt_file, "Temperature Sensor Reading: %d°C\n", temp);
    fprintf(txt_file, "Temperature set to %d°C\n", shm->preftemp);
    fprintf(txt_file, "Thermostat increased by %d\n", (shm->preftemp - temp)); 

    fclose(txt_file);

    printf("Text file updated successfully.\n");

    pthread_exit(NULL);
}

void displaytempmenu(){
tmenu:
    int choice;
    printf("\n\nMenu:\n1, Set Thermostat\n2, Change Preferred Time\n3, Go to Main menu\nEnter Choice: ");
    scanf("%d", &choice);

    switch (choice) {
        case 1: {
            pthread_t tempcon;
            pthread_create(&tempcon, NULL, temperature_sensor, NULL);
            pthread_join(tempcon, NULL);
            goto tmenu;
        break;
        }

        case 2: {
        	tempchange(shm);
        break;
        }
        
        case 3: {
        return;
        }

        default: {
		printf("\nInvalid Choice.");
		goto tmenu;
        break;
        }
    }
}

