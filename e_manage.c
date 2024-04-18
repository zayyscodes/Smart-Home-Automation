#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "e_manage.h"
#include <time.h>

#define MAX 100

// Thread function
void* get_energy_data(void* arg) {
    int ssno = *(int*)arg;
    float* energy = malloc(sizeof(float)); // Allocate memory for float return value

    if (!energy) {
        fprintf(stderr, "Memory allocation failed\n");
        return NULL; // Return NULL on memory allocation failure
    }

    const char *filename = "energy_in.csv"; 
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Failed to open the file");
        free(energy);
        return NULL; // Return NULL on file open error
    }

    char line[MAX];
    int sno;
    float energy_in;
    int found = 0;

    while (fgets(line, sizeof(line), file)) {
        sscanf(line, "%d,%f", &sno, &energy_in);
        if (sno == ssno) {
            found = 1;
            *energy = energy_in; // Store the found energy value
            break;
        }
    }

    fclose(file);

    if (!found) {
        printf("Serial number %d not found in the file.\n", ssno);
        *energy = 0; // Set energy to 0 if serial number not found
    }

    return (void*)energy; // Return the pointer to energy
}

// Main function
float getenergy(void) {
    srand(time(NULL));
    int sno = rand() % 200 + 1;
    pthread_t input;
    float* energy;

    // Create a thread
    if (pthread_create(&input, NULL, get_energy_data, &sno) != 0) {
        fprintf(stderr, "Error creating thread\n");
        return 0;
    }

    // Join the thread and retrieve the energy value
    if (pthread_join(input, (void**)&energy) != 0) {
        fprintf(stderr, "Error joining thread\n");
        return 0;
    }

    // Store the energy value in a float variable and free the memory
    float result = *energy;
    free(energy); // Free the allocated memory

    return result;
}

void* checkingcsv(void* arg) {
    FILE *file1 = fopen("lights_data.csv", "r");
    if (!file1) {
        perror("Failed to open the file");
        return NULL;
    }

    char line1[MAX];
    int num;
    float watt;
    int flag;
    float total = 0.0;

    while (fgets(line1, sizeof(line1), file1)) {
        sscanf(line1, "%*[^,],%d,%f,%d", &num, &watt, &flag);
        if (flag == 1) {
            total += (num * watt);
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
            total += energy;
        }
    }

    fclose(file2);

    // Allocate memory for the total energy value
    float* total_energy = malloc(sizeof(float));
    if (total_energy == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return NULL;
    }

    // Store the calculated total in allocated memory
    *total_energy = total;
    return (void*)total_energy;
}

float consumingInitial(void) {
    pthread_t total;
    float* energy;

    // Create a thread
    if (pthread_create(&total, NULL, checkingcsv, NULL) != 0) {
        fprintf(stderr, "Error creating thread\n");
        return 0;
    }

    // Join the thread and retrieve the energy value
    if (pthread_join(total, (void**)&energy) != 0) {
        fprintf(stderr, "Error joining thread\n");
        return 0;
    }

    // Store the energy value in a float variable
    float result = *energy;

    // Free the allocated memory
    free(energy);

    return result;
}

