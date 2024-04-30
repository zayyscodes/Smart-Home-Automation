#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "e_manage.h"
#include <time.h>
#include "shm.h"

#define MAX 100

extern SmartHome* shm; //pointer to shared memory

// Thread function
void* get_energy_data(void* arg) {
    int ssno = *(int*)arg;
    
    const char *filename = "energy_in.csv"; 
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Failed to open the file");
        return NULL; // Return NULL on file open error
    }

    char line[MAX];
    int sno;
    float energy_in;
    int found = 0;
    
    pthread_mutex_lock(&(shm->mutex_enerin));   

    while (fgets(line, sizeof(line), file)) {
        sscanf(line, "%d,%f", &sno, &energy_in);
        if (sno == ssno) {
            found = 1;
            shm->enerin = energy_in; // Store the found energy value
            break;
        }
    }

    fclose(file);

    if (!found) {
        printf("Serial number %d not found in the file.\n", ssno);
        shm->enerin  = 0; // Set energy to 0 if serial number not found
    }
    
        pthread_mutex_unlock(&(shm->mutex_enerin));   

}

// Main function
void getenergy(void) {
    srand(time(NULL));
    int sno = rand() % 200 + 1;
    pthread_t input;

    if (pthread_create(&input, NULL, get_energy_data, &sno) != 0) {
        fprintf(stderr, "Error creating thread\n");
        return;
    }

    // Join the thread and retrieve the energy value
    if (pthread_join(input, NULL) != 0) {
        fprintf(stderr, "Error joining thread\n");
        return;
    }
    
}

void* checkingcsv(void* arg) {
    FILE *file1 = fopen("lights_data.csv", "r");
    if (!file1) {
        perror("Failed to open the file");
        return NULL;
    }
    
    pthread_mutex_lock(&(shm->mutex_inuse));
	
    char line1[MAX];
    int num;
    float watt;
    int flag;

    while (fgets(line1, sizeof(line1), file1)) {
        sscanf(line1, "%*[^,],%d,%f,%d", &num, &watt, &flag);
        if (flag == 1) {
            shm->inuse += (num * watt);
        }
    }

    fclose(file1);

    // Now for appliances
    FILE *file2 = fopen("appliances_data.csv", "r");
    if (!file2) {
        perror("Failed to open the file");
        return NULL;
    }

    char line2[MAX];
    float energy;
    int status;

    while (fgets(line2, sizeof(line2), file2)) {
        sscanf(line2, "%*[^,],%*[^,],%f,%d", &energy, &status);
        if (status == 1) {
            shm->inuse += energy;
        }
    }

    fclose(file2);

    pthread_mutex_unlock(&(shm->mutex_inuse));
}

void consumingInitial(void) {
    pthread_t total;
    float* energy;

    // Create a thread
    if (pthread_create(&total, NULL, checkingcsv, NULL) != 0) {
        fprintf(stderr, "Error creating thread\n");
        return;
    }

    // Join the thread and retrieve the energy value
    if (pthread_join(total, NULL) != 0) {
        fprintf(stderr, "Error joining thread\n");
        return;
    }
}

void setinput(){
	//shm = getshm(); //pointer to shared memory
        
	getenergy();
	consumingInitial(); 
	//usleep(1000)
        //detachSharedMemory(shm);
}

