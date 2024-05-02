#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
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
#include "mutexes.h"

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int flags[5];
int pipe_fd[2];

//KIYA KIYA REHTA HAI: SHARED MEMORY LAGANA & TASK SCHEDULING
// PLUS HAR JAGA E CONSUME KI RESTRICTION

SmartHome *shm; //pointer to shared memory

void header(void){
	//system("clear");
	printf("\n\n\t\t\tSmart-Home System\n");
}

int usernum = 0;
struct User users[MAX_USERS];



void stop(){
	printf("\n\nPress any key to continue...\n");
	getchar();
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

int main(){
	/*int log = login();
	if (log == 0){
		printf("Unable to access portal account.\n");
		stop();
		exit(EXIT_FAILURE);
	} else ;*/
	pthread_mutex_init(&mutex,NULL); 	//initializing as soon as a user has logged in
	for (int i = 0; i < 5; i++)
		flags[i] = 0;
	
	pid_t pidP = pipe(pipe_fd);
	if (pidP == -1 ) {
		perror("pipe");
		exit(EXIT_FAILURE);
	}
	
	clock_t start;
	shm = getshm();
        if (shm == NULL) {
    	   printf("Failed to initialize shared memory.\n");
     	   exit(EXIT_FAILURE);
	}
	initialise(shm);
	setshm(shm);
	
	pthread_t tempcon;
	//temperature_sensor();
	start:
		header();
		
		setinput();
		
		printf("%d\n%d\n", shm->preftemp, shm->secure);
		printf("\n%.2f\n%.2f", shm->enerin, shm->inuse);
		
		
		if (shm->enerin >= 10.0)
		printf("\nSunny | High Energy Input - %.1fkWh", shm->enerin);
		else if (shm->enerin < 10.0 && shm->enerin >= 5.0)
		printf("\nCloudy | Average Energy Input - %.1fkWh", shm->enerin);
		else if (shm->enerin < 5.0 && shm->enerin > 0.0)
		printf("\nGloomy | Low Energy Input - %.1fkWh", shm->enerin);
		else {
			printf("\nERROR | No Input");
			sleep(1);
			exit(EXIT_FAILURE);
		}
		printf("\nIn consumption: %.1fkWh", shm->inuse);
		if (shm->inuse > shm->enerin)
		printf("\nBackup Batteries are in use.");
		
		printf("\n\nMenu:");
		printf("\n1, Light Automation\n2, Temperature Control\n3, Appliances Control\n4, Apply Scheduling\n5, Exit");
		
		int ch = getchoice();
		
		switch(ch){
			case 1:{
				//printf("\nLight selected!");
				displaylightmenu();
				goto start;
			break;
			}
			
			case 2:{
				//printf("\nTemp selected");
				displaytempmenu();
				goto start;
			break;
			}
			
			case 3:{
				//printf("\nAppliances chose.");
				displayappmenu();
				goto start;
			break;
			}
			
			case 4: {
				scheduling();
			break;
			}
			
			case 5: {
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


