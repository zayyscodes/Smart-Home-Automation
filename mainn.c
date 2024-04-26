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
#include "passwordcheck.c"


//KIYA KIYA REHTA HAI: SHARED MEMORY LAGANA & TASK SCHEDULING



float econsume, cons;
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
	int log = login();
	
	if (log == 0){
		printf("Unable to access portal account.\n");
		stop();
		exit(EXIT_FAILURE);
	} else ;

start:	
	stop();
	system("clear");
	
	econsume = getenergy();
	cons = consumingInitial();
	
	header();
	
	if (econsume >= 10.0)
	printf("\nSunny | High Energy Input today - %.1fkWh", econsume);
	else if (econsume < 10.0 && econsume >= 5.0)
	printf("\nCloudy | Average Energy Input today - %.1fkWh", econsume);
	else if (econsume < 5.0 && econsume > 0.0)
	printf("\nGloomy | Low Energy Input today - %.1fkWh", econsume);
	else {
		printf("\nERROR | No Input");
		sleep(1);
		exit(EXIT_FAILURE);
	}
	printf("\nIn consumption: %.1fkWh", cons);
	if (cons > econsume)
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
