#include <stdio.h>
#include <stdlib.h>
#include "lightmenu.h"
#include "e_manage.h"
#include "appmenu.h"
#include <unistd.h>

float econsume, cons;

void header(void){
	printf("\n\n\t\t\tSmart-Home System\n");
}

int main(){
	
	//system("clear");
	
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
	
//start:
	int ch;
	printf("\n\nMenu:");
	printf("\n1, Light Automation\n2, Temperature Control\n3, Appliances Control");
	printf("\nEnter Choice: ");
	scanf("%d", &ch);
	printf("%d\n", ch);
	switch(ch){
		case 1:{
			//printf("\nLight selected!");
			displaylightmenu();
		break;
		}
		
		case 2:{
			printf("\nTemp selected");
		break;
		}
		
		case 3:{
			//printf("\nAppliances chose.");
			displayappmenu();
		break;
		}
		
		default:{
			printf("\nInvalid Choice.");
			//goto start;
		break;
		}
	}
}
