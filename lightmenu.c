#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include "lightmenu.h"

#define MAX 100
#define LEN 1024

typedef struct{
	char num[LEN];
	int num;
	float watt;
	int stat;
}LightData;


// NUM WALAY KO STRING SE INT MEIN CONVERSION
// NEXT TO DO: Temp and shared memory.


int areasort(const void *a, const void *b) {
    return strcmp(((LightData *)a)->area, ((LightData *)b)->area);
}

int numsort(const void *a, const void *b) {
    if (((LightData *)a)->num < ((LightData *)b)->num)
   	 return -1;
    else if (((LightData *)a)->num > ((LightData *)b)->num)
   	 return 1;
    else
   	 return 0;
}

int lowtohigh(const void *a, const void *b) {
    if (((LightData *)a)->watt < ((LightData *)b)->watt)
   	 return -1;
    else if (((LightData *)a)->watt > ((LightData *)b)->watt)
   	 return 1;
    else
   	 return 0;
}

int hightolow(const void *a, const void *b) {
    if (((LightData *)a)->watt < ((LightData *)b)->watt) {
   	 return 1;
    } else if (((LightData *)a)->watt > ((LightData *)b)->watt) {
   	 return -1;
    } else {
   	 return 0;
    }
}

int statsort(const void *a, const void *b) {
    return -(((LightData *)a)->stat - ((LightData *)b)->stat);
}



/*  	   T H R E A D   T O   U P D A T E    	*/

void* switchcsv(void* arg){
	LightData light = *(LightData *) arg; //lightliance area
	const char* filearea = "lights_data (copy).csv";
	FILE *file = fopen(filearea, "r+");
	if (!file){
  	  perror("Failed to open the file");
  		  return NULL;
	}
 	 
	int header = 0;
	char line[LEN];
	long int header_end = ftell(file);
    
	while (fgets(line, sizeof(line), file)) {
    
    	if (!header) {
   		   header = 1;
  		   continue;
    	}
 	 
  	  char area[LEN];
  	  char num[LEN];
  	  float watt;
  	  int flag;
  	  long int linepos = ftell(file) - strlen(line);
    
  	  if (sscanf(line,  "%[^,],%d,%f,%d", area, &num, &watt &flag) != 4) {
     	   fprintf(stderr, "Invalid line format: %s", line);
     	   continue;
  	  }
    
  	  if (strcmp(area, light.area) == 0) {
     	   fseek(file, linepos, SEEK_SET);
     	   char *newline_pos = strchr(line, '\n');
     	   if (newline_pos)
   			 *newline_pos = '\0';

     	   light.stat = (light.stat == 0) ? 1 : 0;
     	   flag = light.stat;

   		   fprintf(file, "%s,%d,%.2f,%d\n", light.area, light.num, light.watt, flag);
     	   break; // Exit the loop after the modification is done
  	  }
	}
    
	fclose(file);
    
    
  	   printf("\n\nArea: %s\nNum: %d\nWatt: %.2f\n", light.area, light.num, light.watt);
  	   printf("Stat: %s\n", (light.stat == 0) ? "OFF" : "ON");
   	 printf("\nCSV file updated successfully.\n");
}


void toggle(LightData light[]){
shuru:
	int ci;
	printf("\nWhich lightliance do you want to switch on/off? Enter Serial Number (0 to exit): ");
	scanf("%d", &ci);
    
	int sno = (ci - 1);
    
	if (sno < 0)
  	  return; //no lightliance changed

	printf("Area: %s\nNum: %d\nWatt: %.2fkWh\n", light[sno].area, light[sno].num, light[sno].watt);
	if(light[sno].stat == 0)
  	  printf("Stat: OFF\n");
	else if (light[sno].stat == 1)
  	  printf("Stat: ON\n");
  	 
	if(light[sno].stat == 0)
  	  printf("Do you want to switch the lightliance ON? ");
	else if (light[sno].stat == 1)
  	  printf("Do you want to switch the lightliance OFF? ");
    
	char ch;
	printf("Enter choice (Y/N): ");
	fflush(stdin);
	scanf(" %c", &ch);
    
	if (ch == 'N' || ch == 'n')
  	  goto shuru;
	else if (ch == 'Y' || ch == 'y'){
  	  pthread_t toggle;
  	  if (pthread_create(&toggle, NULL, switchcsv, (void*)&light[sno])!=0){
  		  perror("Failed to create thread.\n");
  		  return;
  	  }
  	  if (pthread_join(toggle, NULL)){
  		  perror("Failed to join thread.\n");
  		  return;
  	  }
	}
    
}


void displaylist(void){
again:
	int ch;
	printf("Display options:\n1, As Default\n2, Sorted\nEnter choice: ");
	scanf("%d", &ch);
	switch(ch){
  	  case 1:{ //as default
  		  FILE *file = fopen("lightliances_data.csv", "r");
  	   	   if (!file){
  			  perror("Failed to open file");
  			  goto again;
  	   	   }
  		 
		   	   char line[LEN];
  	   	   int count=0;
  	   	   LightData *light = malloc(MAX * sizeof(LightData));
   				 int i = 1; // Initialize i outside the loop
   				 int header = 0; // Flag to indicate if header is skipped

  	   	   printf("\n\n\t\t\tlightliances List:");
  		 
  	   	   while (fgets(line, sizeof(line), file)){
  			  if (!header) {
  		   	   header = 1;
  		   	   continue;
  			  }

  			  char area[LEN];
  			  char num[LEN];
  			  float watt;
  			  int flag;
  		 
  			  sscanf(line, "%[^,],%d,%f,%d", area, &num, &watt &flag);
  			  strncpy(light[count].area, area, sizeof(light[count].area) - 1);
  			  light[count].area[sizeof(light[count].area) - 1] = '\0';
  			  strncpy(light[count].num, num, sizeof(light[count].num) - 1);
  			  light[count].num[sizeof(light[count].num) - 1] = '\0';
  			  light[count].watt = watt;
  			  light[count].stat = flag;
  			  count++;
  	 
  			  sscanf(line, "%[^,],%d,%f,%d", area, &num, &watt &flag);
  			  printf("\nSno: %d\nlightliance: %s\nnum: %s\nEnergy Consumption: %.2fkWh\n", i, area, num, watt);
  			  if(flag == 0)
  				  printf("Stat: OFF\n");
  			  else if (flag == 1)
  				  printf("Stat: ON\n");
  			  i++;
  	   	   }
  		 
  	   	   fclose(file);
  	   	   toggle(light);
  	  break;
  	  } //end of outer switch case 1
    
  	  case 2:{ //sorted list
  		  FILE *file = fopen("lightliances_data.csv", "r");
  	   	   if (!file){
  			  perror("Failed to open file");
  			  goto again;
  	   	   }
  		 
  		  char line[LEN];
  		  int count=0;
  		  LightData *light = malloc(MAX * sizeof(LightData));
  		  int i = 1; // Initialize i outside the loop
  		  int header = 0; // Flag to indicate if header is skipped
  		 
  	   	   while (fgets(line, sizeof(line), file)){
  			  if (!header) {
  		   	   header = 1;
  			  } else {
  				  char area[LEN];
  				  char num[LEN];
  				  float watt;
  				  int flag;
  			 
  				  sscanf(line, "%[^,],%d,%f,%d", area, &num, &watt &flag);
  				  strncpy(light[count].area, area, sizeof(light[count].area) - 1);
  				  light[count].area[sizeof(light[count].area) - 1] = '\0';
  				  strncpy(light[count].num, num, sizeof(light[count].num) - 1);
  				  light[count].num[sizeof(light[count].num) - 1] = '\0';
  				  light[count].watt = watt;
  				  light[count].stat = flag;
  				  count++;
  			  }    
  		  }
  		  fclose(file);
  	  sort:
  		  int c;
  		  printf("\n\nSorted list display options:\n1, Alphabetically\n2, Sorted num Wise\n3, Lowest to Highest Consumption\n4, Highest to Lowest Consumption\n5, By their status\nEnter choice: ");
  		  scanf("%d", &c);
  		  switch(c){
  			  case 1:{
  				  qsort(light, count, sizeof(LightData), areasort);
  				  printf("Sorted lightliances List (by num):\n");
  		 			 for (int i = 0; i < count; i++){
  						  printf("\nSno: %d\n", i+1);
  					  printf("Area: %s\nNum: %d\nWatt: %.2fkWh\n", light[i].area, light[i].num, light[i].watt);
  					  if(light[i].stat == 0)
  						  printf("Stat: OFF\n");
  					  else if (light[i].stat == 1)
  						  printf("Stat: ON\n");
  		 			 }
  			  break;
  			  }
  		 
  			  case 2:{
  			   	   qsort(light, count, sizeof(LightData), numsort);
  			   	   printf("Sorted lightliances List (by num):\n");
  			   	   for (int i = 0; i < count; i++){
  						  printf("\nSno: %d\n", i+1);
  					  printf("Area: %s\nNum: %d\nWatt: %.2fkWh\n", light[i].area, light[i].num, light[i].watt);
  					  if(light[i].stat == 0)
  						  printf("Stat: OFF\n");
  					  else if (light[i].stat == 1)
  						  printf("Stat: ON\n");
  		 			 }
  			  break;
  			  }
  		 
  			  case 3:{
  			   	   qsort(light, count, sizeof(LightData), lowtohigh);
  			   	   printf("Sorted lightliances List (by watt, lowest to highest):\n");
  			   	   for (int i = 0; i < count; i++){
  						  printf("\nSno: %d\n", i+1);
  					  printf("Area: %s\nNum: %d\nWatt: %.2fkWh\n", light[i].area, light[i].num, light[i].watt);
  					  if(light[i].stat == 0)
  						  printf("Stat: OFF\n");
  					  else if (light[i].stat == 1)
  						  printf("Stat: ON\n");
  		 			 }
  			  break;
  			  }
  		 
  			  case 4:{
  		   	   qsort(light, count, sizeof(LightData), hightolow);
  		   	   printf("Sorted lightliances List (by watt, lowest to highest):\n");
  		   	   for (int i = 0; i < count; i++){
  						  printf("\nSno: %d\n", i+1);
  					  printf("Area: %s\nNum: %d\nWatt: %.2fkWh\n", light[i].area, light[i].num, light[i].watt);
  					  if(light[i].stat == 0)
  						  printf("Stat: OFF\n");
  					  else if (light[i].stat == 1)
  						  printf("Stat: ON\n");
  		 			 }
  			  break;
  			  }
  		 
  			  case 5:{
  				  qsort(light, count, sizeof(LightData), statsort);
  		   	   printf("Sorted lightliances List (by status):\n");
  		   	   for (int i = 0; i < count; i++){
  						  printf("\nSno: %d\n", i+1);
  					  printf("Area: %s\nNum: %d\nWatt: %.2fkWh\n", light[i].area, light[i].num, light[i].watt);
  					  if(light[i].stat == 0)
  						  printf("Stat: OFF\n");
  					  else if (light[i].stat == 1)
  						  printf("Stat: ON\n");
  		 			 }
  		 
  			  break;
  			  }
  		 
  			  default:{
  				  printf("\nInvalid Choice.");
  				  goto sort;
  			  break;
  			  }
  		  }
  		  toggle(light);
  	  break;
  	  } // end outer switch case 2
    
  	  default:{
  		  printf("\nInvalid Choice.");
  		  goto again;
  	  break;
  	  }
    
	}

}


void* readflag(void* arg){
	FILE *file = fopen("lights_data.csv", "r");
	if (!file){
    	perror("Failed to open file");
    	return NULL;
	}
    
	char line[LEN];
	LightData *light = malloc(MAX * sizeof(LightData));
	int count = 0;
    
	while (fgets(line, sizeof(line), file)){
    	char num[LEN];
    	int num;
    	float watt;
    	int flag;
 	 
    	sscanf(line, "%[^,],%d,%f,%d", num, &&num, &watt &flag);
 	 
    	if (flag==1){
			strncpy(light[count].num, num, sizeof(light[count].num) - 1);
     				   light[count].num[sizeof(light[count].num) - 1] = '\0';
     				   light[count].num = num;
     				   light[count].watt = watt;
     				   count++;
    	}    
	}
	fclose(file);
    
	if (count == 0)
    	return NULL;
	else
    	return light;
}

void displaylightmenu(){
menu:
	int choice;
	printf("\n\nMenu:\n1, Switch Off/On Light\n2, Check Lights\nEnter Choice: ");
	scanf("%d", &choice);
    
	switch(choice){
    	case 1:{
		 
    	break;
    	}
 	 
    	case 2:{
			pthread_t thread;
			if (pthread_create(&thread, NULL, readflag, NULL)){
				perror("Failed to create thread.");
				break;
			}
		 
			LightData *entries;
			if (pthread_join(thread, (void**) &entries)){
				perror("Failed to join");
				break;
			}
		 
			if (entries == NULL){
				printf("Lights aren't open for any num.");
				break;
			}
		 
			printf("Entries with flag = 1:\n");
     		   for (int i = 0; entries[i].num[0] != '\0'; i++) {
					printf("num: %s, Num: %d, Watt: %.2fkWh\n", entries[i].num, entries[i].num, entries[i].watt);
     		   }
    	break;
    	}
 	 
    	default:{
			printf("\nInvalid Choice.");
			goto menu;
    	break;
    	}
	}    
    
}








