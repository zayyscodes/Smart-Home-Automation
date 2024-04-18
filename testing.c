#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define MAX 100

float getenergyforday(int serial_number);

typedef struct {
    int serial_number;
    float energy;
} EnergyData;

// Function to get energy data for a specific serial number
void* get_energy_data(void* arg) {
    int serial_number = *(int*)arg;
    EnergyData *data = malloc(sizeof(EnergyData));
    data->serial_number = serial_number;
    data->energy = getenergyforday(serial_number);
    return data;
}

// Modify getenergyforday to accept a serial number as parameter
float getenergyforday(int serial_number) {
    const char *filename = "energy_in.csv";
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Failed to open the file");
        return 0;
    }

    char line[MAX];
    int sno;
    float energy_in;
    int found = 0;

    while (fgets(line, sizeof(line), file)) {
        sscanf(line, "%d,%f", &sno, &energy_in);
        if (sno == serial_number) {
            found = 1;
            break;
        }
    }

    fclose(file);
    if (found) {
        return energy_in;
    } else {
        printf("Serial number %d not found in the file.\n", serial_number);
        return 0;
    }
}

int main() {
    pthread_t thread;
    srand(time(NULL));
    int sno = rand() % 200 + 1;


    // Create threads
   if (pthread_create(&thread, NULL, get_energy_data, &sno)) {
               fprintf(stderr, "Error creating thread\n");
         return 1;
   }

    // Join threads and print results
        EnergyData* data;
        if (pthread_join(thread, (void**)&data)) {
            fprintf(stderr, "Error joining thread\n");
            return 1;
        }
        printf("Serial number %d: Energy = %.2f\n", data->serial_number, data->energy);
        free(data);

    return 0;
}

