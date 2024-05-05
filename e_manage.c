#include <stdio.h> // Include standard input-output header file
#include <stdlib.h> // Include standard library header file
#include <pthread.h> // Include pthread library header file
#include "e_manage.h" // Include header file for energy management
#include "shm.h" // Include header file for shared memory operations
#include <time.h> // Include time library header file
#include "shm.h" // Include header file for shared memory operations (duplicate inclusion)

#define MAX 100 // Define maximum size constant

extern pthread_mutex_t mutex; // Declare mutex variable for synchronization
extern SmartHome *shm; // Declare pointer to shared memory
extern int flags[5]; // Declare array of flags

// Thread function to retrieve energy data
void* get_energy_data(void* arg) {
    int ssno = *(int*)arg; // Extract serial number from argument
    
    const char *filename = "energy_in.csv"; // Define filename
    FILE *file = fopen(filename, "r"); // Open file for reading
    if (!file) { // Check if file opening failed
        perror("Failed to open the file"); // Print error message
        return NULL; // Return NULL
    }

    char line[MAX]; // Declare line buffer
    int sno; // Declare variables for serial number and energy input
    float energy_in;
    int found = 0; // Flag to indicate if serial number is found

    while (fgets(line, sizeof(line), file)) { // Read file line by line
        sscanf(line, "%d,%f", &sno, &energy_in); // Parse serial number and energy input
        if (sno == ssno) { // Check if serial number matches
            found = 1; // Set found flag to true
            shm->enerin = energy_in; // Store energy value in shared memory
            break; // Exit loop
        }
    }

    fclose(file); // Close file

    if (!found) { // If serial number is not found
        printf("Serial number %d not found in the file.\n", ssno); // Print error message
        shm->enerin  = 0; // Set energy value to 0 in shared memory
    }
}

// Main function to retrieve energy data
void getenergy(void) {
    srand(time(NULL)); // Seed random number generator
    int sno = rand() % 200 + 1; // Generate random serial number
    pthread_t input; // Declare thread variable
    if (pthread_create(&input, NULL, get_energy_data, &sno) != 0) { // Create thread
        fprintf(stderr, "Error creating thread\n"); // Print error message if creation fails
        return; // Return
    }

    // Join the thread and retrieve the energy value
    if (pthread_join(input, NULL) != 0) { // Join thread
        fprintf(stderr, "Error joining thread\n"); // Print error message if joining fails
        return; // Return
    }
}

// Thread function to check appliances and update energy consumption
void* checkingcsv(void* arg) {
    // Open file containing data for lights
    FILE *file1 = fopen("lights_data (copy).csv", "r");
    if (!file1) { // Check if file opening failed
        perror("Failed to open the file"); // Print error message
        return NULL; // Return NULL
    }
	
    char line1[MAX]; // Declare line buffer
    int num; // Declare variables for number, wattage, and status
    float watt;
    int flag;

    while (fgets(line1, sizeof(line1), file1)) { // Read file line by line
        sscanf(line1, "%*[^,],%d,%f,%d", &num, &watt, &flag); // Parse data
        if (flag == 1) { // Check if appliance is on
            if (shm->svmd == 0) // Check if energy saver mode is disabled
            	shm->inuse += (num * watt); // Update energy consumption
            else if (shm->svmd == 1) // Check if energy saver mode is enabled
            	shm->inuse += ((num * 0.5) * watt); // Update energy consumption with reduced power
        }
    }

    fclose(file1); // Close file

    // Open file containing data for appliances
    FILE *file2 = fopen("appliances_data (copy).csv", "r");
    if (!file2) { // Check if file opening failed
        perror("Failed to open the file"); // Print error message
        return NULL; // Return NULL
    }

    char line2[MAX]; // Declare line buffer
    float energy; // Declare variables for energy and status
    int status;

    while (fgets(line2, sizeof(line2), file2)) { // Read file line by line
        sscanf(line2, "%*[^,],%*[^,],%f,%d", &energy, &status); // Parse data
        if (status == 1) { // Check if appliance is on
            shm->inuse += energy; // Update energy consumption
        }
    }

    fclose(file2); // Close file
}

// Function to initialize energy consumption
void consumingInitial(void) {
	shm->inuse=0.0; // Initialize energy consumption to 0
    pthread_t total; // Declare thread variable
    float* energy;

    // Create a thread
    if (pthread_create(&total, NULL, checkingcsv, NULL) != 0) { // Create thread
        fprintf(stderr, "Error creating thread\n"); // Print error message if creation fails
        return; // Return
    }

    // Join the thread and retrieve the energy value
    if (pthread_join(total, NULL) != 0) { // Join thread
        fprintf(stderr, "Error joining thread\n"); // Print error message if joining fails
        return; // Return
    }
}

// Function to set input energy data
void setinput(){
    pthread_mutex_lock(&(shm->mutex_inuse)); // Lock mutex for synchronization
	pthread_mutex_lock(&(shm->mutex_enerin)); // Lock mutex for energy input
	getenergy(); // Retrieve energy input
	consumingInitial(); // Initialize energy consumption
	pthread_mutex_unlock(&(shm->mutex_inuse)); // Unlock mutex
	pthread_mutex_unlock(&(shm->mutex_enerin));  // Unlock mutex
}

// Function to update energy consumption
void setconsume(){
	shm->inuse=0.0; // Initialize energy consumption to 0
    FILE *file1 = fopen("lights_data (copy).csv", "r"); // Open file for lights data
    if (!file1) { // Check if file opening failed
        perror("Failed to open the file"); // Print error message
        return; // Return
    }
	
    char line1[MAX]; // Declare line buffer
    int num; // Declare variables for number, wattage, and status
    float watt;
    int flag;

    while (fgets(line1, sizeof(line1), file1)) { // Read file line by line
        sscanf(line1, "%*[^,],%d,%f,%d", &num, &watt, &flag); // Parse data
        if (flag == 1) { // Check if appliance is on
            shm->inuse += (num * watt); // Update energy consumption
        }
    }

    fclose(file1); // Close file

    // Open file containing data for appliances
    FILE *file2 = fopen("appliances_data (copy).csv", "r"); // Open file for appliances data
    if (!file2) { // Check if file opening failed
        perror("Failed to open the file"); // Print error message
        return; // Return
    }

    char line2[MAX]; // Declare line buffer
    float energy; // Declare variables for energy and status
    int status;

    while (fgets(line2, sizeof(line2), file2)) { // Read file line by line
        sscanf(line2, "%*[^,],%*[^,],%f,%d", &energy, &status); // Parse data
        if (status == 1) { // Check if appliance is on
            shm->inuse += energy; // Update energy consumption
        }
    }

    fclose(file2); // Close file
}

// Function to toggle energy saver mode
void setenergysave(){
	pthread_mutex_lock(&shm->mutex_svmd); // Lock mutex for synchronization
	if (shm->svmd == 1){ // If energy saver mode is enabled
		shm->svmd = 0; // Disable energy saver mode
		printf("\nEnergy Saver Mode disabled.\n"); // Print status message
	}
	else { // If energy saver mode is disabled
		shm->svmd = 1; // Enable energy saver mode
		printf("\nEnergy Saver Mode enabled.\n"); // Print status message
	}
	pthread_mutex_unlock(&shm->mutex_svmd); // Unlock mutex
}


