#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include "lightmenu.h"
#include "e_manage.h"
#include "appmenu.h"
#include "tempmenu.h"
#include "shm.h"
#include "passwordcheck.h"

//KIYA KIYA REHTA HAI: SHARED MEMORY LAGANA & TASK SCHEDULING

SmartHome* shm; //pointer to shared memory

int usernum = 0;
struct User users[MAX_USERS];

void header(void){
	printf("\n\n\t\t\tSmart-Home System\n");
}

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
	
/*	int log = login();
	
	if (log == 0){
		printf("Unable to access portal account.\n");
		stop();
		exit(EXIT_FAILURE);
	} else
		printf("\n\n\nP R O C E E D\n\n");
		; //initialise(shm);
*/
start:
	header();
	printf(" E F  Y O U  O S :");
	shm = getshm();
	if (shm == NULL) {
		printf("Failed to initialize shared memory.\n");
		exit(EXIT_FAILURE);
	}
	initialise(shm);
	setshm(shm);
	
	
	
	setinput();
	
	
	if (shm->enerin >= 10.0)
	printf("\nSunny | High Energy Input today - %.1fkWh", shm->enerin);
	else if (shm->enerin < 10.0 && shm->enerin >= 5.0)
	printf("\nCloudy | Average Energy Input today - %.1fkWh", shm->enerin);
	else if (shm->enerin < 5.0 && shm->enerin > 0.0)
	printf("\nGloomy | Low Energy Input today - %.1fkWh", shm->enerin);
	else {
		printf("\nERROR | No Input");
		sleep(1);
		exit(EXIT_FAILURE);
	}
	printf("\nIn consumption: %.1fkWh", shm->inuse);
	if (shm->inuse > shm->enerin)
	printf("\nBackup Batteries are in use.");
	
	int ch;
	printf("\n\nMenu:");
	printf("\n1, Light Automation\n2, Temperature Control\n3, Appliances Control\n4, Exit");
	printf("\nEnter Choice: ");
	scanf("%d", &ch);
	printf("%d\n", ch);
	switch(ch){
		case 1:{
			//printf("\nLight selected!");
			displaylightmenu();
			goto start;
		break;
		}
		
		case 2:{
			printf("\nTemp selected");
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
		break;
		}
	    
		default:{
			printf("\nInvalid Choice.");
			//goto start;
		break;
		}
	}
}
