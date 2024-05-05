#include <stdio.h>  // Including standard input/output library
#include <stdlib.h> // Including standard library for memory allocation and other utilities
#include <pthread.h> // Including pthread library for multithreading support
#include <string.h> // Including string library for string manipulation functions
#include "appmenu.h" // Including header file for application menu functions
#include "e_manage.h" // Including header file for energy management functions
#include "shm.h" // Including header file for shared memory operations
#include "scheduling.h" // Including header file for scheduling functions

#define MAX 29 // Defining maximum value for some operations
#define LEN 1024 // Defining maximum length for strings

// Declaring global variables and functions from other files
extern pthread_mutex_t update; // Mutex for update synchronization
extern pthread_mutex_t mutex; // Mutex for thread synchronization
extern SmartHome *shm; // Pointer to shared memory structure
extern int flags[5]; // Array to store flags
extern int appupdated; // Variable to track if the app has been updated

/* S O R T I N G F U N C T I O N S */

// Function to compare names of appliances for sorting
int namesort(const void *a, const void *b) {
    return strcmp(((AppData *)a)->name, ((AppData *)b)->name);
}

// Function to compare areas of appliances for sorting
int areasort(const void *a, const void *b) {
    return strcmp(((AppData *)a)->area, ((AppData *)b)->area);
}

// Function to compare energy consumption of appliances in low to high order
int lowtohigh(const void *a, const void *b) {
    if (((AppData *)a)->watt < ((AppData *)b)->watt)
        return -1;
    else if (((AppData *)a)->watt > ((AppData *)b)->watt)
        return 1;
    else
        return 0;
}

// Function to compare energy consumption of appliances in high to low order
int hightolow(const void *a, const void *b) {
    if (((AppData *)a)->watt < ((AppData *)b)->watt) {
        return 1;
    } else if (((AppData *)a)->watt > ((AppData *)b)->watt) {
        return -1;
    } else {
        return 0;
    }
}

// Function to compare status of appliances for sorting
int statsort(const void *a, const void *b) {
    return -(((AppData *)a)->stat - ((AppData *)b)->stat);
}

/* T H R E A D T O U P D A T E */

// Function to switch appliances on or off in a separate thread
void* switchcsv(void* arg) {
    AppData* app = (AppData*) arg; // Casting argument to AppData pointer
    int onoff; // Variable to store on/off status

shuru: // Label for a loop
    int ci;
    printf("\nWhich appliance do you want to switch on/off? Enter Serial Number (0 to exit): ");
    scanf("%d", &ci); // Reading input for appliance serial number

    int sno = (ci - 1); // Calculating index in array based on serial number

    if (sno < 0) // If input is 0 or negative, exit
        return NULL;

    // Displaying details of the selected appliance
    printf("Name: %s\nArea: %s\nWatt: %.2fkWh\n", app[sno].name, app[sno].area, app[sno].watt);
    if (app[sno].stat == 0)
        printf("Stat: OFF\n");
    else if (app[sno].stat == 1)
        printf("Stat: ON\n");

    // Prompting user to switch the appliance on or off
    if (app[sno].stat == 0) {
        printf("Do you want to switch the appliance ON? ");
        onoff = 1;
    } else if (app[sno].stat == 1) {
        printf("Do you want to switch the appliance OFF? ");
        onoff = 0;
    }

    char ch;
    printf("Enter choice (Y/N): ");
    fflush(stdin); // Flushing input buffer
    scanf(" %c", &ch); // Reading user choice

    if (ch == 'N' || ch == 'n') // If user chooses not to switch, loop back
        goto shuru;
    else if (ch == 'Y' || ch == 'y') { // If user chooses to switch
        // Checking conditions for switching appliance on
        if (shm->inuse >= shm->enerin || (shm->inuse+app[sno].watt) > shm->enerin && onoff == 1){
            printf("Unable to switch appliance ON; exceeds input");
            appupdated = 0; // Setting app updated status
        } else if (shm->svmd == 1 && app[sno].watt >= 1.75) {
            printf("Unable to switch appliance ON; Energy Saver Mode enabled\n");
        } else {
            // Switching appliance status
            app[sno].stat = (app[sno].stat == 0) ? 1 : 0;
            // Writing task to pipe based on appliance wattage
            if (app[sno].watt > 1.75){
                pthread_mutex_lock(&mutex); // Locking mutex for thread safety
                write_task_to_pipe("appover"); // Writing to pipe for high wattage appliances
                printf("Written to ARRAY\n\n");
                pthread_mutex_unlock(&mutex); // Unlocking mutex
            } else {
                pthread_mutex_lock(&mutex); // Locking mutex for thread safety
                write_task_to_pipe("appunder"); // Writing to pipe for low wattage appliances
                printf("Written to ARRAY\n\n");
                pthread_mutex_unlock(&mutex); // Unlocking mutex
            }

            // Opening file to write appliance data
            FILE* file = fopen("appliances_data (copy).csv", "w");
            if (file == NULL) {
                perror("Failed to open file");
                return NULL;
            }

            // Writing header to file
            fprintf(file, "name,area,watt,stat\n");

            // Writing appliance data to file
            for (int i = 0; i < MAX; i++) {
                fprintf(file, "%s,%s,%.2f,%d\n", app[i].name, app[i].area, app[i].watt, app[i].stat);
            }

            fclose(file); // Closing file

            // Opening text file to write task information
            FILE* txt_file = fopen("task_assign_info.txt", "a");
            if (txt_file == NULL) {
                perror("Failed to open text file");
                return NULL;
            }

            // Writing appliance control information to text file
            fprintf(txt_file, "\n\n\n****APPLIANCE CONTROL****\n");
            fprintf(txt_file, "Name: %s\nArea: %s\nWatt: %.2fkWh\n", app[sno].name, app[sno].area, app[sno].watt);
            fprintf(txt_file, "Status: %s\n\n", (app[sno].stat == 0) ? "OFF" : "ON");

            fclose(txt_file); // Closing text file

            printf("Text file updated successfully.\n");
            appupdated = 1; // Setting app updated status
        }
        
    }

    pthread_exit(NULL); // Exiting thread
}

// Function to toggle appliances on or off
void toggle(AppData entry[]){
      pthread_mutex_lock(&update); // Locking mutex for update synchronization
      pthread_t toggle; // Declaring thread variable
      if (pthread_create(&toggle, NULL, switchcsv, (void*)entry)!=0){ // Creating thread to switch appliances
          perror("Failed to create thread.\n");
          return;
      }
      if (pthread_join(toggle, NULL)){ // Joining thread
          perror("Failed to join thread.\n");
          return;
      }
      pthread_mutex_unlock(&update); // Unlocking mutex
}

// Function to display list of appliances
void displaylist(void){
again:
    int ch;
    printf("Display options:\n1, As Default\n2, Sorted\nEnter choice: ");
    scanf("%d", &ch); // Reading user choice
    switch(ch){
        case 1: {
            pthread_mutex_lock(&update); // Locking mutex for update synchronization
            FILE *file = fopen("appliances_data (copy).csv", "r"); // Opening file to read appliance data
            if (!file){
                perror("Failed to open file");
                goto again;
            }
            
            char line[LEN]; // Variable to store each line read from file
            int count=0; // Variable to count number of appliances
            AppData *entry = malloc(MAX * sizeof(AppData)); // Allocating memory for appliance data
            int i = 1;  // Variable to store serial number
            int header = 0; // Variable to track header line in file

            printf("\n\n\t\t\tAppliances List:");

            while (fgets(line, sizeof(line), file)){
                if (!header) { // Skip header line
                    header = 1;
                    continue;
                }

                char name[LEN]; // Variable to store appliance name
                char area[LEN]; // Variable to store appliance area
                float watt; // Variable to store appliance wattage
                int flag; // Variable to store appliance status
                
                // Parsing data from line
                sscanf(line, "%[^,],%[^,],%f,%d", name, area, &watt, &flag);
                // Copying data to entry structure
                strncpy(entry[count].name, name, sizeof(entry[count].name) - 1);
                entry[count].name[sizeof(entry[count].name) - 1] = '\0';
                strncpy(entry[count].area, area, sizeof(entry[count].area) - 1);
                entry[count].area[sizeof(entry[count].area) - 1] = '\0';
                entry[count].watt = watt;
                entry[count].stat = flag;
                count++;
                
                // Displaying appliance details
                sscanf(line, "%[^,],%[^,],%f,%d", name, area, &watt, &flag);
                printf("\nSno: %d\nAppliance: %s\nArea: %s\nEnergy Consumption: %.2fkWh\n", i, name, area, watt);
                if(flag == 0)
                    printf("Stat: OFF\n");
                else if (flag == 1)
                    printf("Stat: ON\n");
                i++;
            }
            
            fclose(file); // Closing file
            pthread_mutex_unlock(&update); // Unlocking mutex
            toggle(entry); // Toggling appliances
            break;
        }
        
        case 2: {
            pthread_mutex_lock(&update); // Locking mutex for update synchronization
            FILE *file = fopen("appliances_data (copy).csv", "r"); // Opening file to read appliance data
            if (!file){
                perror("Failed to open file");
                goto again;
            }
            
            char line[LEN]; // Variable to store each line read from file
            int count=0; // Variable to count number of appliances
            AppData *entry = malloc(MAX * sizeof(AppData)); // Allocating memory for appliance data
            int i = 1;  // Variable to store serial number
            int header = 0; // Variable to track header line in file

            while (fgets(line, sizeof(line), file)){
                if (!header) {
                    header = 1;
                } else {
                    char name[LEN]; // Variable to store appliance name
                    char area[LEN]; // Variable to store appliance area
                    float watt; // Variable to store appliance wattage
                    int flag; // Variable to store appliance status
                    
                    // Parsing data from line
                    sscanf(line, "%[^,],%[^,],%f,%d", name, area, &watt, &flag);
                    // Copying data to entry structure
                    strncpy(entry[count].name, name, sizeof(entry[count].name) - 1);
                    entry[count].name[sizeof(entry[count].name) - 1] = '\0';
                    strncpy(entry[count].area, area, sizeof(entry[count].area) - 1);
                    entry[count].area[sizeof(entry[count].area) - 1] = '\0';
                    entry[count].watt = watt;
                    entry[count].stat = flag;
                    count++;
                }    
            }
            fclose(file); // Closing file
            pthread_mutex_unlock(&update); // Unlocking mutex
            
        sort: // Label for sorting options
            int c;
            printf("\n\nSorted list display options:\n1, Alphabetically\n2, Sorted Area Wise\n3, Lowest to Highest Consumption\n4, Highest to Lowest Consumption\n5, By their status\nEnter choice: ");
            scanf("%d", &c); // Reading user choice
            switch(c){
                case 1:{
                    // Sorting appliances alphabetically by name
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
                    // Sorting appliances alphabetically by area
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
                    // Sorting appliances by lowest to highest energy consumption
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
                    // Sorting appliances by highest to lowest energy consumption
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
                    // Sorting appliances by status
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
                    goto sort; // Redirecting to sorting options
                    break;
                }
            }
            toggle(entry); // Toggling appliances
            break;
        }
        
        default:{
            printf("\nInvalid Choice.");
            goto again; // Redirecting to display options
            break;
        }
    }
}

// Function to read flags and display corresponding appliances
void* readflags(void* arg){
    pthread_mutex_lock(&update); // Locking mutex for update synchronization
    FILE *file = fopen("appliances_data (copy).csv", "r"); // Opening file to read appliance data
    if (!file){
        perror("Failed to open file");
        return NULL;
    }
    
    char line[LEN]; // Variable to store each line read from file
    AppData *entry = malloc(MAX * sizeof(AppData)); // Allocating memory for appliance data
    int count = 0; // Variable to count number of appliances
    
    while (fgets(line, sizeof(line), file)){
        char name[LEN]; // Variable to store appliance name
        char area[LEN]; // Variable to store appliance area
        float watt; // Variable to store appliance wattage
        int flag; // Variable to store appliance status
        
        // Parsing data from line
        sscanf(line, "%[^,],%[^,],%f,%d", name, area, &watt, &flag);
        
        if (flag==1){ // If appliance status is 1 (on)
            // Copying data to entry structure
            strncpy(entry[count].name,  name, sizeof(entry[count].name) - 1);
            entry[count].name[sizeof(entry[count].name) - 1] = '\0';
            strncpy(entry[count].area, area, sizeof(entry[count].area) - 1);
            entry[count].area[sizeof(entry[count].area) - 1] = '\0';
            entry[count].watt = watt;
            count++;
        }    
    }
    fclose(file); // Closing file
    pthread_mutex_unlock(&update); // Unlocking mutex
    
    if (count == 0)
        return NULL;
    else
        return entry; // Returning entry data
}

// Function to display application menu
void displayappmenu(void){
menuu:
    if (appupdated == 1){ // If app has been updated
        pthread_mutex_lock(&(shm->mutex_inuse)); // Locking mutex for shared memory access
        setconsume(); // Setting energy consumption
        pthread_mutex_unlock(&(shm->mutex_inuse)); // Unlocking mutex
        
        int choice;
        printf("\n\nMenu:\n1, Switch Off/On appliance\n2, Check appliances\n3, Go to Main menu\nEnter Choice: ");
        scanf("%d", &choice); // Reading user choice
        
        switch(choice){
            case 1:{
                displaylist(); // Displaying list of appliances
                goto menuu; // Redirecting to menu
                break;
            }
            
            case 2:{
                pthread_t thread;
                if (pthread_create(&thread, NULL, readflags, NULL)){ // Creating thread to read flags
                    perror("Failed to create thread.");
                    break;
                }
                
                AppData *entries;
                if (pthread_join(thread, (void**) &entries)){ // Joining thread
                    perror("Failed to join");
                    break;
                }
                
                if (entries == NULL){ // If no appliances are in use
                    printf("No appliance is currently in use.");
                    break;
                }
                
                printf("Entries with flag = 1:\n");
                for (int i = 0; entries[i].name[0] != '\0'; i++) {
                    // Displaying appliances in use
                    printf("Appliance: %s, Area: %s, Energy Consumption: %.2fkWh\n", entries[i].name, entries[i].area, entries[i].watt);
                }
                goto menuu; // Redirecting to menu
                break;
            }
            
            case 3: {
                return; // Exiting function
            }
            
            default:{
                printf("\nInvalid Choice.");
                goto menuu; // Redirecting to menu
                break;
            }
        }   
    } 
}

/*The parameters in the `qsort` function are:

1. **base:** Pointer to the array to be sorted.
2. **num:** Number of elements in the array.
3. **size:** Size in bytes of each element in the array.
4. **compar:** Pointer to a comparison function that returns 
an integer less than, equal to, or greater than zero if the first 
argument is considered to be respectively less than, equal to, or greater 
than the second.*/
