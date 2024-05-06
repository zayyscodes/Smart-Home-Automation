#include <stdio.h> // Standard I/O operations
#include <stdlib.h> // Standard library functions
#include <unistd.h> // POSIX operating system API
#include <fcntl.h> // File control options
#include <string.h> // String manipulation functions
#include <sys/stat.h> // File information functions
#include <ctype.h> // Character handling functions
#include <pthread.h> // POSIX threads
#include <sys/mman.h> // Memory management functions
#include <sys/types.h> // System data types
#include <sys/wait.h> // Waiting for process termination
#include <time.h> // Time functions
#include "lightmenu.h" // Header file for light menu functions
#include "e_manage.h" // Header file for energy management functions
#include "appmenu.h" // Header file for appliance menu functions
#include "tempmenu.h" // Header file for temperature menu functions
#include "shm.h" // Header file for shared memory functions
#include "passwordcheck.h" // Header file for password check functions
#include "scheduling.h" // Header file for scheduling functions

#define MAX 1000 // Maximum number of users

pthread_mutex_t update = PTHREAD_MUTEX_INITIALIZER; // Mutex for update operations
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; // Mutex for critical sections
pthread_mutex_t file_mutex = PTHREAD_MUTEX_INITIALIZER; // Mutex for file operations

SmartHome *shm; // Pointer to shared memory
int lightupdated = 1; // Flag indicating whether lights have been updated
int appupdated = 1; // Flag indicating whether appliances have been updated
int usernum = 0; // Number of users
struct User users[MAX_USERS]; // Array to store user information

void* turnoff(void* arg); // Thread function to turn off lights and appliances
int login(); // Function to handle user login

// Function to display system header
void header(){
	system("clear");
	printf("\n\n\t\t\t        Smart-Home System\n"); 
	if (shm->svmd == 1)
	printf("\nEnergy-Saver Mode -- ENABLED");
	else if (shm->svmd == 0)
	printf("\nEnergy-Saver Mode -- DISABLED");
		
	printf("\nThermostat set to %d°C", shm->preftemp);
	if (shm->enerin >= 10.0)
	printf("\nSunny | High Energy Input - %.2fkWh", shm->enerin);
	else if (shm->enerin < 10.0 && shm->enerin >= 5.0)
	printf("\nCloudy | Average Energy Input - %.2fkWh", shm->enerin);
	else if (shm->enerin < 5.0 && shm->enerin > 0.0)
	printf("\nGloomy | Low Energy Input - %.2fkWh", shm->enerin);
	else {
		printf("\nERROR | No Input");
		sleep(1);
		exit(EXIT_FAILURE);
	}
	printf("\nIn consumption: %.2fkWh", shm->inuse);
}

// Function to stop and wait for user input
void stop(){
	printf("\n\nPress any key to continue...\n");
	getchar();
}

// Main function
int main(){
	// User login
	int log = login();
	if (log == 0){
		printf("Unable to access portal account.\n");
		exit(EXIT_FAILURE);
	} else
	;
	
	// Initialize mutexes
	pthread_mutex_init(&mutex,NULL);
	pthread_mutex_init(&update,NULL);
	pthread_mutex_init(&file_mutex,NULL);
	
	clock_t start;
	shm = getshm();
    if (shm == NULL) {
    	printf("Failed to initialize shared memory.\n");
     	exit(EXIT_FAILURE);
	}
	initialise(shm);
	setshm(shm);
	
	// Main program loop
	start:
		setinput();
		header();
		
	hey:	
		
		// Automatic shutdown if energy consumption exceeds input
		if (shm->enerin < shm->inuse){
			char chin;
			printf("\n\nOver consumption.\nDo you want to proceed with automatic shutdown? (Y/N): ");
			fflush(stdin);
			scanf(" %c", &chin);
			if (chin == 'N' || chin == 'n')
				printf("You are requested to reduce consumption manually to perform any process.");
			else if (chin == 'Y' || chin == 'y') {
				pthread_t toggle;
				if (pthread_create(&toggle, NULL, turnoff, NULL) != 0) {
					    perror("Failed to create thread.\n");
					    exit(EXIT_FAILURE);
				}
				if (pthread_join(toggle, NULL)) {
					    perror("Failed to join thread.\n");
					    exit(EXIT_FAILURE);
				}
			}
			
		}
		
		// Display menu options
		printf("\n\nMenu:");
		printf("\n1, Light Automation\n2, Temperature Control\n3, Appliances Control\n4, Apply Scheduling\n5, Switch to energy saver mode\n6, Logout");
		
		int ch = getchoice();
		
		// Handle menu choices
		switch(ch){
			case 1:{
				// Light automation
				system("clear");
				header();
				displaylightmenu();
				if (lightupdated == 1)
					goto start;
				else 
					goto hey;
			break;
			}
			
			case 2:{
				// Temperature control
				system("clear");
				header();
				displaytempmenu();
				goto start;
			break;
			}
			
			case 3:{
				// Appliances control
				system("clear");
				header();
				displayappmenu();
				if (lightupdated == 1)
					goto start;
				else 
					goto hey;
			break;
			}
			
			case 4: {
				// Apply scheduling
				scheduling();
				printf("Would you like to logout? (Y/N): ");
				char chlo;
				scanf(" %c", &chlo);
				fflush(stdin);
				if (chlo == 'Y' || chlo =='y')
					break;
				else if (chlo == 'N' || chlo =='n')
					goto start;
			break;
			}
			
			case 5: {
				// Switch to energy saver mode
				if (shm->svmd == 0){
					printf("Energy Saver Mode will only allow fifty-percent of the lights to switch on,\nand appliances under 1.75kWh");
					printf("\nDo you want to enable Energy Saver Mode? (Y/N): ");
					char chlo;
					scanf(" %c", &chlo);
					fflush(stdin);
					if (chlo == 'Y' || chlo =='y')
						setenergysave();
					else if (chlo == 'N' || chlo =='n')
						goto start;
				} else if (shm->svmd == 1){
					printf("Do you want to disable Energy Saver Mode? (Y/N): ");
					char chlo;
					scanf(" %c", &chlo);
					fflush(stdin);
					if (chlo == 'Y' || chlo =='y')
						setenergysave();
					else if (chlo == 'N' || chlo =='n')
						goto start;
				} else 
					printf("Invalid Option");
					goto start;
			break;
			}
			
			case 6: {
			break;
			}
		    
			default:{
				printf("\nInvalid Choice.");
			break;
			}
		}
	
	pthread_mutex_destroy(&mutex); // Destroy mutex
}

// Thread function to turn off lights and appliances
void* turnoff(void* arg) {
    // Define arrays to hold data
    LightData light[MAX];
    AppData app[MAX];
    
    pthread_mutex_lock(&file_mutex); // Lock file mutex

    // Open lights file
    FILE *file1 = fopen("lights_data (copy).csv", "r+");
    if (!file1) {
        perror("Failed to open the file");
        return NULL;
    }
	
    char line1[MAX];
    int index = 0;
    int header = 0;

    // Read and update lights data
    while (fgets(line1, sizeof(line1), file1)) {
    	if (!header) {
		header = 1;
		continue;
	    }
        sscanf(line1, "%[^,],%d,%f,%d", light[index].area, &light[index].num, &light[index].watt, &light[index].stat);
        if (light[index].stat == 1) {
            light[index].stat = 0; // Update the status
            fseek(file1, -strlen(line1), SEEK_CUR); // Move back to the beginning of the line
            fprintf(file1, "%s,%d,%.2f,%d\n", light[index].area, light[index].num, light[index].watt, light[index].stat); // Write the updated line back to the file
        }
        index++;
    }

    fclose(file1);
    pthread_mutex_unlock(&file_mutex); // Unlock file mutex
    
    pthread_mutex_lock(&update); // Lock update mutex
    
    // Open appliances file
    FILE *file2 = fopen("appliances_data (copy).csv", "r+");
    if (!file2) {
        perror("Failed to open the file");
        return NULL;
    }

    char line2[MAX];
    int iapp = 0;
	header = 0;
    while (fgets(line2, sizeof(line2), file2)) {
    	if (!header) {
            header = 1;
            continue;
        }

        sscanf(line2, "%[^,],%[^,],%f,%d", app[iapp].name, app[iapp].area, &app[iapp].watt, &app[iapp].stat);
        if (app[iapp].stat == 1) {
            app[iapp].stat = 0; // Update the status
            fseek(file2, -strlen(line2), SEEK_CUR); // Move back to the beginning of the line
            fprintf(file2, "%s,%s,%.2f,%d\n", app[iapp].name, app[iapp].area, app[iapp].watt, app[iapp].stat); // Write the updated line back to the file
        }
        iapp++;
    }

    fclose(file2);
    
    pthread_mutex_unlock(&update); // Unlock update mutex

    // Update consumption in shared memory
    pthread_mutex_lock(&(shm->mutex_inuse));
    printf("Consumption updated\n");
    setconsume();
    pthread_mutex_unlock(&(shm->mutex_inuse));
    
    lightupdated = 1; // Set light update flag
    appupdated = 1; // Set appliance update flag

    // Lock mutex before calling write_task_to_pipe
    pthread_mutex_lock(&mutex);
    write_task_to_pipe("turnoff");
    printf("Written to ARRAY\n\n");
    pthread_mutex_unlock(&mutex); // Unlock mutex
    
    pthread_exit(NULL);
}

// Function to handle user login
int login(){
	system("clear");
login:
    int choice;
    int x = 0, data = 0;

    data = loadUserData(users, &usernum); // Load user data

    if (data == 1){
    	printf("\n1. Sign Up\n2. Log In\n3. Exit\n");
	printf("Enter your choice: ");
	scanf("%d", &choice);

	switch (choice) {
	    case 1:
	        signUp(users, &usernum); // Sign up new user
	        goto login;
	        break;
	    case 2:
	        x = logIn(users, usernum); // Log in existing user
	        break;
	    case 3:
	        printf("Exiting...\n");
	        stop();
	        break;
	    default:
	        printf("Invalid choice. Please try again.\n");
	        stop();
	        break;
	}
    } else if (data == 0) {
    	printf("You are requested to sign up first.\n");
    	signUp(users, &usernum); // Sign up new user
    	goto login;
    }
    
    if (x == 1)
    	return 1; // Access granted
    else
    	return 0; // Access denied
}


/*Certainly! Let's go through each included header and understand why it's used and which functions 
from those headers are being called in the code:

1. **stdio.h**: This header is included for standard input and output operations. Functions like `printf`,
`scanf`, and `getchar` are called for printing messages to the console and reading user input.

2. **stdlib.h**: It provides general utility functions, including memory allocation (`malloc`, `free`) and 
program termination (`exit`). Functions like `exit` are used for terminating the program in case of failure.

3. **unistd.h**: This header provides access to POSIX operating system API. In this code, `system("clear")`
is used to clear the console screen.

4. **fcntl.h**: It provides functions for manipulating file descriptors. Although no specific function from 
this header is called directly in the code you provided, it might be used indirectly by other libraries or system calls.

5. **string.h**: This header is used for string manipulation functions such as `strlen`, `strcpy`, and `strcat`. 
These functions are used extensively for parsing strings and manipulating file contents in the code.

6. **sys/stat.h**: It provides functions to get information about files (e.g., `stat`). Although not explicitly 
called in the code, it might be included for potential future use or as part of other libraries.

7. **ctype.h**: This header provides functions for character handling, such as `isalpha`. These functions are 
used for character validation and manipulation.

8. **pthread.h**: It's used for POSIX threads. Functions like `pthread_create`, `pthread_mutex_init`, and 
`pthread_mutex_lock` are called to create and manage threads and mutexes for thread synchronization.

9. **sys/mman.h**: This header provides memory management functions, such as `mmap`. In this code, it's used 
for memory mapping related operations, likely for shared memory management.

10. **sys/types.h**: It defines various system data types. It's included for data type definitions used in 
conjunction with other system headers.

11. **sys/wait.h**: This header is used for functions related to process management and waiting for process 
termination. In the code, `pthread_join` is called to wait for thread termination.

12. **time.h**: It provides functions for manipulating time, such as `clock_t`. In this code, it's used for 
measuring the execution time of certain operations.
*/
