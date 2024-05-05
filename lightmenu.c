#include <stdio.h>           // Include standard input-output library
#include <stdlib.h>          // Include standard library
#include <pthread.h>         // Include POSIX threads library
#include <unistd.h>          // Include functions to perform various operations such as sleep
#include <string.h>          // Include string manipulation functions
#include "e_manage.h"        // Include custom header files
#include "lightmenu.h"
#include "shm.h"
#include "scheduling.h"
#include "shm.h"             // Include shared memory header file
#define MAX 9                // Define constant MAX with value 9
#define LEN 1024             // Define constant LEN with value 1024

extern pthread_mutex_t mutex;             // Declare external variables
extern pthread_mutex_t update;
extern pthread_mutex_t file_mutex;

extern SmartHome *shm;                    // Declare external variables
extern int lightupdated;

/* SORTING FUNCTIONS */

int areasortt(const void *a, const void *b) {  // Function to sort by area
    return strcmp(((LightData *)a)->area, ((LightData *)b)->area);
}

int numsort(const void *a, const void *b) {    // Function to sort by number
    if (((LightData *)a)->num < ((LightData *)b)->num)
        return -1;
    else if (((LightData *)a)->num > ((LightData *)b)->num)
        return 1;
    else
        return 0;
}

int lowtohighh(const void *a, const void *b) { // Function to sort from low to high by wattage
    if (((LightData *)a)->watt < ((LightData *)b)->watt)
        return -1;
    else if (((LightData *)a)->watt > ((LightData *)b)->watt)
        return 1;
    else
        return 0;
}

int hightoloww(const void *a, const void *b) { // Function to sort from high to low by wattage
    if (((LightData *)a)->watt < ((LightData *)b)->watt) {
        return 1;
    } else if (((LightData *)a)->watt > ((LightData *)b)->watt) {
        return -1;
    } else {
        return 0;
    }
}

int statsortt(const void *a, const void *b) {   // Function to sort by status
    return -(((LightData *)a)->stat - ((LightData *)b)->stat);
}

/* THREAD TO UPDATE */

void* switchincsv(void* arg) {                  // Function to switch lights on/off
    LightData* light = (LightData*)arg;
    int onoff;

    int ci;
    printf("\nWhich area's lights do you want to switch on/off? Enter Serial Number (0 to exit): ");
    scanf("%d", &ci);
    int sno = ci - 1;

    if (sno < 0 || sno >= MAX)
        return NULL;

    printf("Area: %s\nNum: %d\nWatt: %.2fkWh\n", light[sno].area, light[sno].num, light[sno].watt);
    if (light[sno].stat == 0) {
        printf("Stat: OFF\n");
        printf("Do you want to switch the lights ON? ");
        onoff = 1;
    } else if (light[sno].stat == 1) {
        printf("Stat: ON\n");
        printf("Do you want to switch the lights OFF? ");
        onoff = 0;
    }

    char ch;
    printf("Enter choice (Y/N): ");
    fflush(stdin);
    scanf(" %c", &ch);

    if (ch == 'Y' || ch == 'y') {
        if (shm->inuse >= shm->enerin && onoff == 1){
            printf("\nUnable to switch area lights on; exceeds input\n");
            lightupdated = 0;
        } else {
            light[sno].stat = (light[sno].stat == 0) ? 1 : 0;

            FILE* file = fopen("lights_data (copy).csv", "w"); // Open file for writing
            if (file == NULL) {
                perror("Failed to open file");             // Print error if failed to open file
                return NULL;
            }

            fprintf(file, "area,num,watt,flag");             // Write header to file

            for (int i = 0; i < MAX; i++) {                 // Loop through lights
                fprintf(file, "%s,%d,%.2f,%d\n", light[i].area, light[i].num, light[i].watt, light[i].stat);  // Write light data to file
            }

            fclose(file);                                    // Close file

            pthread_mutex_lock(&mutex);                      // Lock mutex
            write_task_to_pipe("lighton");                   // Write task to pipe
            printf("Written to ARRAY\n\n");
            pthread_mutex_unlock(&mutex);                    // Unlock mutex
            
            FILE* txt_file = fopen("task_assign_info.txt", "a");  // Open text file for appending
            if (txt_file == NULL) {
                perror("Failed to open text file");
                return NULL;
            }

            fprintf(txt_file, "\n\n\n****LIGHT AUTOMATION****\n");  // Write to text file

            fprintf(txt_file, "Area: %s\nNum of Bulbs: %d\nWatt: %.2fkWh\n", light[sno].area, light[sno].num, light[sno].watt);
            fprintf(txt_file, "Status: %s\n\n", (light[sno].stat == 0) ? "OFF" : "ON");

            fclose(txt_file);   // Close text file

            printf("Text file updated successfully.\n");
            lightupdated = 1;   // Set light update status to 1
        }
    }

    pthread_exit(NULL);       // Exit thread
}
