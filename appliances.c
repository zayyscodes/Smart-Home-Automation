#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "smarthome.c"

// Function for appliances control thread
void *appliances_control_thread(void *arg) {
    // Open and read from the CSV file containing appliances commands
    FILE *appliances_file = fopen("appliances_commands.csv", "r");
    if (!appliances_file) {
        perror("Failed to open appliances commands file");
        pthread_exit(NULL);
    }

    int time, command;
    while (fscanf(appliances_file, "%d,%d", &time, &command) != EOF) {
        // Simulate waiting for the specified time
        // Sleep or use an appropriate delay mechanism based on the time value

        // Lock the mutex to modify shared data
        pthread_mutex_lock(&state_mutex);
        // Update appliance status based on the command
        home_state.appliance_status = command;
        // Unlock the mutex
        pthread_mutex_unlock(&state_mutex);

        // Log the current appliance status
        printf("Time: %d, Appliance command: %d, Appliance status: %d\n", time, command, home_state.appliance_status);
    }

    // Close the file
    fclose(appliances_file);
    pthread_exit(NULL);
}
