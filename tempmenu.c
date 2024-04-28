// get input from csv file and set thermostat accordingly
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include "tempmenu.h"
#include "shm.h"
#include <time.h>
#include <unistd.h>

#define MAX 1024

extern SmartHome *shm; //pointer to shared memory

// F U N C   T O   R E A D   C S V   F I L E    T H R O U G H    T H R E A D
void* get_temp_data(void* arg) {
    int ssno = *(int*)arg;
    int* temp = malloc(sizeof(int)); // Allocate memory for int return value

    if (!temp) {
        fprintf(stderr, "Memory allocation failed\n");
        return NULL; // Return NULL on memory allocation failure
    }

    const char *filename = "temp_rand.csv"; 
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Failed to open the file");
        free(temp);
        return NULL; // Return NULL on file open error
    }

    char line[MAX];
    int sno;
    int temp_in;
    int found = 0;

    while (fgets(line, sizeof(line), file)) {
        sscanf(line, "%d,%d", &sno, &temp_in);
        if (sno == ssno) {
            found = 1;
            *temp = temp_in; // Store the found temp value
            break;
        }
    }

    fclose(file);

    if (!found) {
        printf("Serial number %d not found in the file.\n", ssno);
        *temp = 0; // Set temp to 0 if serial number not found
    }

    return (void*)temp; // Return the pointer to temp
}

int settemp(void) {
    srand(time(NULL));
    int sno = rand() % 100 + 1;
    pthread_t input;
    int* temp;

    // Create a thread
    if (pthread_create(&input, NULL, get_temp_data, &sno) != 0) {
        fprintf(stderr, "Error creating thread\n");
        return 0;
    }

    // Join the thread and retrieve the temp value
    if (pthread_join(input, (void**)&temp) != 0) {
        fprintf(stderr, "Error joining thread\n");
        return 0;
    }

    // Store the temp value in a int variable and free the memory
    int result = *temp;
    free(temp); // Free the allocated memory

    return result;
}


// F U N C   T O   S E T   T H E R M O S T A T 
void* temperature_sensor(void* arg) {
    shm = getshm();
    while (1) {
        // Simulate temperature reading
    	int temp = settemp();
        printf("\n\n\nTemperature Sensor Reading: %d°C\n", temp);
        
        if (temp < shm->preftemp){
        	printf("Thermostat set to %d\n", shm->preftemp);
        	printf("Temperture increased by %d\n\n\n", (shm->preftemp-temp));
        } else if (temp > shm->preftemp){
        	printf("Thermostat set to %d\n", shm->preftemp);
        	printf("Temperture decreased by %d\n\n\n", (temp-shm->preftemp));
        } else {
        	printf("No change to thermostat.\n\n\n");
        }
	
        // Update energy data

        sleep(rand() % 6 + 25); // Simulate reading every 25 to 30 seconds
    }
    detachSharedMemory(shm);
    pthread_exit(NULL);
}





