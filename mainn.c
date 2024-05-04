#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <ctype.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include "lightmenu.h"
#include "e_manage.h"
#include "appmenu.h"
#include "tempmenu.h"
#include "shm.h"
#include "passwordcheck.h"
#include "scheduling.h"

#define MAX 1000

pthread_mutex_t update = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t file_mutex = PTHREAD_MUTEX_INITIALIZER;

SmartHome *shm; //pointer to shared memory
int lightupdated = 1;
int appupdated = 1;
int usernum = 0;
struct User users[MAX_USERS];

void* turnoff(void* arg);
int login();

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

void stop(){
	printf("\n\nPress any key to continue...\n");
	getchar();
}





int main(){
	int log = login();
	if (log == 0){
		printf("Unable to access portal account.\n");
		exit(EXIT_FAILURE);
	} else
	;
	pthread_mutex_init(&mutex,NULL); 	//initializing as soon as a user has logged in
	pthread_mutex_init(&update,NULL); 	//initializing as soon as a user has logged in
	//pthread_mutex_init(&read,NULL); 	//initializing as soon as a user has logged in
	pthread_mutex_init(&file_mutex,NULL); 	//initializing as soon as a user has logged in
	
	clock_t start;
	shm = getshm();
        if (shm == NULL) {
    	   printf("Failed to initialize shared memory.\n");
     	   exit(EXIT_FAILURE);
	}
	initialise(shm);
	setshm(shm);
	
	start:
		setinput();
		header();
		
	hey:	
		
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
		
		printf("\n\nMenu:");
		printf("\n1, Light Automation\n2, Temperature Control\n3, Appliances Control\n4, Apply Scheduling\n5, Switch to energy saver mode\n6, Logout");
		
		int ch = getchoice();
		
		switch(ch){
			case 1:{
				//printf("\nLight selected!");
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
				//printf("\nTemp selected");
				system("clear");
				header();
				displaytempmenu();
				goto start;
			break;
			}
			
			case 3:{
				//printf("\nAppliances chose.");
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
				//x = 0;
			break;
			}
		}
	
	pthread_mutex_destroy(&mutex);
}






void* turnoff(void* arg) {
    // Define arrays to hold data
    LightData light[MAX];
    AppData app[MAX];
    
    pthread_mutex_lock(&file_mutex);

    FILE *file1 = fopen("lights_data (copy).csv", "r+");
    if (!file1) {
        perror("Failed to open the file");
        return NULL;
    }
	
    char line1[MAX];
    int index = 0;
    int header = 0;

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
    pthread_mutex_unlock(&file_mutex);
    
    
    pthread_mutex_lock(&update);
    // Now for appliances
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

    fclose(file2);FILE *file = fopen("appliances_data (copy).csv", "r+");
    if (!file) {
        perror("Failed to open the file");
        return NULL;
    }

    char line[MAX];
    iapp = 0;

    while (fgets(line, sizeof(line), file)) {
        // Check if the first character is a letter, if not, prepend a placeholder character
        if (!isalpha(line[0])) {
            memmove(line + 1, line, strlen(line) + 1);
            line[0] = '#'; // Placeholder character
        }

        // Parse the line using sscanf
        if (sscanf(line, "%[^,],%[^,],%f,%d", app[iapp].name, app[iapp].area, &app[iapp].watt, &app[iapp].stat) == 4) {
            if (app[iapp].stat == 1) {
                app[iapp].stat = 0; // Update the status
                fseek(file, -strlen(line), SEEK_CUR); // Move back to the beginning of the line
                fprintf(file, "%s,%s,%.2f,%d\n", app[iapp].name, app[iapp].area, app[iapp].watt, app[iapp].stat); // Write the updated line back to the file
            }
        } else {
            fprintf(stderr, "Error parsing line: %s\n", line);
        }

        iapp++;
    }

    fclose(file);
    pthread_mutex_unlock(&update);

    pthread_mutex_lock(&(shm->mutex_inuse));
    printf("Consumption updated\n");
    setconsume();
    pthread_mutex_unlock(&(shm->mutex_inuse));
    
    lightupdated = 1;
    appupdated = 1;

    // Lock mutex before calling write_task_to_pipe
    pthread_mutex_lock(&mutex);
    write_task_to_pipe("turnoff");
    printf("Written to ARRAY\n\n");
    pthread_mutex_unlock(&mutex);
    
    pthread_exit(NULL);
}

int login(){
	system("clear");
	login:
	    int choice;
	    int x = 0, data = 0;

	    data = loadUserData(users, &usernum);

	    if (data == 1){
	    	printf("\n1. Sign Up\n2. Log In\n3. Exit\n");
		printf("Enter your choice: ");
		scanf("%d", &choice);

		switch (choice) {
		    case 1:
		        signUp(users, &usernum);
		        goto login;
		        break;
		    case 2:
		        x = logIn(users, usernum);
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
	    	signUp(users, &usernum);
	    	goto login;
	    }
	    
	    if (x == 1)
	    	return 1; //access
	    else
	    	return 0;
}
