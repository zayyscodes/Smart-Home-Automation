#include <stdio.h> // Including standard input/output functions
#include <stdlib.h> // Including standard library functions
#include <pthread.h> // Including pthread library for threading
#include <string.h> // Including string manipulation functions
#include "tempmenu.h" // Including header file for temperature menu functions
#include "shm.h" // Including header file for shared memory structures and functions
#include <time.h> // Including time functions
#include <unistd.h> // Including standard symbolic constants and types
#include "scheduling.h" // Including header file for scheduling functions

#define MAX 1024 // Defining maximum buffer size

extern pthread_mutex_t mutex; // Extern declaration for mutex
extern SmartHome *shm; // Extern declaration for shared memory pointer

// Function to get temperature data from file
void* get_temp_data(void* arg) {
    int ssno = *(int*)arg; // Serial number passed as argument
    int* temp = malloc(sizeof(int)); // Allocating memory for temperature

    if (!temp) {
        fprintf(stderr, "Memory allocation failed\n"); // Printing error if memory allocation fails
        return NULL;
    }

    const char *filename = "temp_rand.csv"; // Filename for temperature data
    FILE *file = fopen(filename, "r"); // Opening file for reading
    if (!file) {
        perror("Failed to open the file"); // Printing error if opening file fails
        free(temp); // Freeing memory allocated for temperature
        return NULL;
    }

    char line[MAX]; // Buffer for reading line from file
    int sno; // Serial number
    int temp_in; // Temperature read from file
    int found = 0; // Flag to indicate if serial number is found

    while (fgets(line, sizeof(line), file)) {
        sscanf(line, "%d,%d", &sno, &temp_in); // Parsing line to get serial number and temperature
        if (sno == ssno) {
            found = 1; // Serial number found
            *temp = temp_in; // Storing temperature
            break;
        }
    }

    fclose(file); // Closing file

    if (!found) {
        printf("Serial number %d not found in the file.\n", ssno); // Printing message if serial number not found
        *temp = 0; // Setting temperature to 0
    }

    return (void*)temp; // Returning temperature data
}

// Function to randomly set temperature
int settemp(void) {
    srand(time(NULL)); // Seeding random number generator
    int sno = rand() % 100 + 1; // Generating random serial number
    pthread_t input; // Thread identifier for temperature data
    int* temp; // Pointer to temperature data

    if (pthread_create(&input, NULL, get_temp_data, &sno) != 0) {
        fprintf(stderr, "Error creating thread\n"); // Printing error if thread creation fails
        return 0;
    }

    if (pthread_join(input, (void**)&temp) != 0) {
        fprintf(stderr, "Error joining thread\n"); // Printing error if thread joining fails
        return 0;
    }

    int result = *temp; // Getting temperature value
    free(temp); // Freeing memory allocated for temperature

    return result; // Returning temperature
}

// Function to simulate temperature sensor
void* temperature_sensor(void* arg) {
    int temp = settemp(); // Getting temperature reading
    printf("\n\n\nTemperature Sensor Reading: %d°C\n", temp); // Printing temperature reading
    
    // Checking temperature against preferred temperature
    if (temp < shm->preftemp) {
        printf("Temperature set to %d\n", shm->preftemp); // Setting temperature to preferred temperature
        printf("Thermostat increased by %d\n\n\n", (shm->preftemp - temp)); // Printing thermostat adjustment
        pthread_mutex_lock(&mutex); // Locking mutex for shared memory access
        write_task_to_pipe("thermoinc"); // Writing task to shared memory for thermostat increase
        pthread_mutex_unlock(&mutex); // Unlocking mutex
    } else if (temp > shm->preftemp) {
        printf("Temperature set to %d\n", shm->preftemp); // Setting temperature to preferred temperature
        printf("Thermostat decreased by %d\n\n\n", (temp - shm->preftemp)); // Printing thermostat adjustment
        pthread_mutex_lock(&mutex); // Locking mutex for shared memory access
        write_task_to_pipe("thermodec"); // Writing task to shared memory for thermostat decrease
        pthread_mutex_unlock(&mutex); // Unlocking mutex
    } else {
        printf("No change to thermostat.\n\n\n"); // No adjustment needed
    }
    
    // Writing temperature data to text file
    FILE* txt_file = fopen("task_assign_info.txt", "a"); // Opening text file in append mode
    if (txt_file == NULL) {
        perror("Failed to open text file"); // Printing error if opening file fails
        return NULL;
    }

    fprintf(txt_file, "\n\n\n****TEMPERATURE CONTROL****\n"); // Writing section header to text file
    fprintf(txt_file, "Temperature Sensor Reading: %d°C\n", temp); // Writing temperature reading to text file
    fprintf(txt_file, "Temperature set to %d°C\n", shm->preftemp); // Writing preferred temperature to text file
    fprintf(txt_file, "Thermostat increased by %d\n", (shm->preftemp - temp)); // Writing thermostat adjustment to text file

    fclose(txt_file); // Closing text file

    printf("Text file updated successfully.\n"); // Printing success message

    pthread_exit(NULL); // Exiting thread
}

// Function to display temperature menu
void displaytempmenu(){
tmenu:
    int choice; // Variable to store user choice
    printf("\n\nMenu:\n1, Set Thermostat\n2, Change Preferred Time\n3, Go to Main menu\nEnter Choice: "); // Displaying menu options
    scanf("%d", &choice); // Reading user choice

    switch (choice) {
        case 1: {
            pthread_t tempcon; // Thread identifier for temperature control
            pthread_create(&tempcon, NULL, temperature_sensor, NULL); // Creating temperature control thread
            pthread_join(tempcon, NULL); // Joining temperature control thread
            goto tmenu; // Returning to menu
            break;
        }

        case 2: {
            tempchange(shm); // Function call to change preferred temperature
            break;
        }
        
        case 3: {
            return; // Exiting function
        }

        default: {
            printf("\nInvalid Choice."); // Printing message for invalid choice
            goto tmenu; // Returning to menu
            break;
        }
    }
}

// Commenting each function for clarity
