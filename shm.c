#include "shm.h" // Including the header file for shared memory structures and functions
#include <stdio.h> // Including standard input/output functions
#include <fcntl.h> // Including file control options
#include <sys/stat.h> // Including data returned by the stat() function
#include <sys/shm.h> // Including shared memory facilities
#include <sys/types.h> // Including data types
#include <sys/wait.h> // Including declarations for waiting
#include <stdlib.h> // Including standard library functions
#include <string.h> // Including string manipulation functions
#include <sys/mman.h> // Including memory management declarations
#include <unistd.h> // Including standard symbolic constants and types
#include "e_manage.h" // Including header file for energy management functions

#define shm_name "/my_osproj_shm" // Defining shared memory name
#define SIZE 10000000 // Defining shared memory size

extern SmartHome *shm; // Extern declaration for shared memory structure pointer

// Function to initialize shared memory structure
void initialise(SmartHome *shm) {
    // Initialize mutexes
    pthread_mutex_init(&(shm->mutex_enerin), NULL);
    pthread_mutex_init(&(shm->mutex_inuse), NULL);
    pthread_mutex_init(&(shm->mutex_preftemp), NULL);
    pthread_mutex_init(&(shm->mutex_svmd), NULL);
    pthread_mutex_init(&(shm->mutex_tasks), NULL);
    
    // Initialize shared memory variables
    shm->curr = 0;
    shm->enerin = 0.0;
    shm->inuse = 0.0;
    shm->preftemp = 26;
    shm->svmd = 0; 
}

// Function to set shared memory variables
void setshm(SmartHome *shm) {
    pthread_mutex_lock(&(shm->mutex_preftemp)); // Locking mutex for preferred temperature
    pthread_mutex_lock(&(shm->mutex_svmd)); // Locking mutex for energy saver mode
    pthread_mutex_lock(&(shm->mutex_tasks)); // Locking mutex for tasks
    
    shm->svmd = 0; // Initialize energy saver mode to disabled
init:
    fflush(stdin); // Flushing input buffer
    
    pthread_mutex_lock(&(shm->mutex_inuse)); // Locking mutex for in-use flag
    printf("\nPrefered temperature in °C: ");
    scanf("%d", &(shm->preftemp)); // Getting preferred temperature from user
    
    // Unlocking mutexes
    pthread_mutex_unlock(&(shm->mutex_enerin));
    pthread_mutex_unlock(&(shm->mutex_inuse));
    pthread_mutex_unlock(&(shm->mutex_preftemp));
    pthread_mutex_unlock(&(shm->mutex_svmd));
    pthread_mutex_unlock(&(shm->mutex_tasks));
}

// Function to get shared memory
SmartHome *getshm() {
    size_t sizeshm = sizeof(SmartHome); // Size of shared memory structure
    int shm_fd = shm_open(shm_name, O_CREAT | O_RDWR, 0666); // Creating or opening shared memory object
    if (shm_fd == -1) {
        perror("shm_open"); // Printing error if opening shared memory fails
        return NULL;
    }

    if (ftruncate(shm_fd, sizeshm) == -1) {
        perror("ftruncate"); // Printing error if truncating shared memory fails
        return NULL;
    }

    SmartHome *data = (SmartHome *)mmap(NULL, sizeshm, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0); // Mapping shared memory to process address space
    close(shm_fd); // Closing file descriptor

    if (data == MAP_FAILED) {
        perror("mmap"); // Printing error if mapping fails
        return NULL;
    }

    return data; // Returning shared memory pointer
}

// Function to check if all elements in an array are zero
int checkArray(int array[], int size) {
    for (int i = 0; i < size; i++) {
        if (array[i] == 1) {
            return 0; // Returning false if any element is non-zero
        }
    }
    return 1; // Returning true if all elements are zero
}

// Function to read tasks from a CSV file
TASKS *readcsv(const char *filename, const char *name) {
    FILE *file = fopen(filename, "r"); // Opening CSV file for reading
    if (!file) {
        perror("Failed to open the file"); // Printing error if opening file fails
        return NULL;
    }

    int header = 0; // Flag to skip header line
    char line[LEN]; // Buffer to store line read from file
    TASKS *task = NULL; // Pointer to TASKS structure

    while (fgets(line, sizeof(line), file)) {
        if (!header) {
            header = 1;
            continue; // Skip header line
        }
        char pro[LEN]; // Buffer for project name
        char deet[LEN]; // Buffer for details
        char type[LEN]; // Buffer for type
        double arrtime; // Arrival time
        double burst; // Burst time

        // Parsing CSV line
        if (sscanf(line, "%[^,],%[^,],%[^,],%lf", pro, deet, type, &burst) != 4) {
            fprintf(stderr, "Invalid line format: %s", line); // Printing error if line format is invalid
            continue;
        }

        if (strcmp(pro, name) == 0) {
            task = (TASKS *)malloc(sizeof(TASKS)); // Allocating memory for TASKS structure
            if (!task) {
                perror("Memory allocation failed"); // Printing error if memory allocation fails
                fclose(file); // Closing file
                return NULL;
            }

            strcpy(task->type, deet); // Copying type to TASKS structure
            strcpy(task->deet, type); // Copying details to TASKS structure
            task->arrtime = clock(); // Setting arrival time to current time
            task->burst = burst; // Setting burst time

            break; // Exiting loop after finding task
        }
    }

    fclose(file); // Closing file

    return task; // Returning pointer to TASKS structure
}

// Function to write task to shared memory
void write_task_to_pipe(const char *type) {
    TASKS *task;

    task = readcsv("schedule.csv", type); // Reading task from CSV file

    if (task != NULL) {
        if (strlen(task->type) > 0 && strlen(task->deet) > 0) {
            if (shm->curr < TMAX) {
                shm->tasks[shm->curr] = *task; // Copying task to shared memory
                shm->curr += 1; // Incrementing current task count
                printf("Task copied to shared memory successfully\n"); // Printing success message
            } else {
                printf("Error: Shared memory task array is full\n"); // Printing error if task array is full
            }
        }
        free(task); // Freeing memory allocated for task
    }
}

// Function to change thermostat temperature
void tempchange(SmartHome *shm) {
    int temp;
    printf("Your thermostat was previously set to: %d°C\n", shm->preftemp); // Printing current thermostat temperature
    printf("Enter new temperature: ");
    scanf("%d", &temp); // Getting new temperature from user
    
    pthread_mutex_lock(&(shm->mutex_preftemp)); // Locking mutex for preferred temperature
    shm->preftemp = temp; // Setting new temperature
    printf("Your thermostat was previously set to: %d°C\n", shm->preftemp); // Printing new thermostat temperature
    pthread_mutex_unlock(&(shm->mutex_preftemp)); // Unlocking mutex for preferred temperature
}

// Function to detach shared memory
void detachSharedMemory(SmartHome *shm) {
    munmap(shm, sizeof(SmartHome)); // Unmapping shared memory
    shm_unlink(shm_name); // Unlinking shared memory object
}

// Comment explaining each line
