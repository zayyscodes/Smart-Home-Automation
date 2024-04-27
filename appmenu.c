#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include "appmenu.h"
#include "shm.h"

#define MAX 100
#define LEN 1024

extern SmartHome* shm; //pointer to shared memory

typedef struct{
	char name[LEN];
	char area[LEN];
	float watt;
	int stat;
}AppData;


/*  	  S O R T I N G   F U N C T I O N S   	  */

int namesort(const void *a, const void *b) {
    return strcmp(((AppData *)a)->name, ((AppData *)b)->name);
}

int areasort(const void *a, const void *b) {
    return strcmp(((AppData *)a)->area, ((AppData *)b)->area);
}

int lowtohigh(const void *a, const void *b) {
    if (((AppData *)a)->watt < ((AppData *)b)->watt)
   	 return -1;
    else if (((AppData *)a)->watt > ((AppData *)b)->watt)
   	 return 1;
    else
   	 return 0;
}

int hightolow(const void *a, const void *b) {
    if (((AppData *)a)->watt < ((AppData *)b)->watt) {
   	 return 1;
    } else if (((AppData *)a)->watt > ((AppData *)b)->watt) {
   	 return -1;
    } else {
   	 return 0;
    }
}

int statsort(const void *a, const void *b) {
    return -(((AppData *)a)->stat - ((AppData *)b)->stat);
}



/*	     T H R E A D   T O   U P D A T E		*/

void* switchcsv(void* arg){
	AppData app = *(AppData *) arg; //appliance name
	const char* filename = "appliances_data.csv";
	FILE *file = fopen(filename, "r+");
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
		
  	  char name[LEN];
  	  char area[LEN];
  	  float watt;
  	  int flag;
  	  long int linepos = ftell(file) - strlen(line);
  	 
  	  if (sscanf(line,  "%[^,],%[^,],%f,%d", name, area, &watt, &flag) != 4) {
  	      fprintf(stderr, "Invalid line format: %s", line);
  	      continue;
  	  }
  	 
  	  if (strcmp(name, app.name) == 0) {
  	      fseek(file, linepos, SEEK_SET);
  	      char *newline_pos = strchr(line, '\n');
  	      if (newline_pos)
  	 		 *newline_pos = '\0';

            app.stat = (app.stat == 0) ? 1 : 0;
            flag = app.stat;

              fprintf(file, "%s,%s,%.2f,%d\n", app.name, app.area, app.watt, flag);
  	      break; // Exit the loop after the modification is done
  	  }
	}
    
	fclose(file);
	
	
         printf("\n\nName: %s\nArea: %s\nWatt: %.2f\n", app.name, app.area, app.watt);
         printf("Stat: %s\n", (app.stat == 0) ? "OFF" : "ON");
   	 printf("\nCSV file updated successfully.\n");
}


void toggle(AppData entry[]){
shuru:
	int ci;
	printf("\nWhich appliance do you want to switch on/off? Enter Serial Number (0 to exit): ");
	scanf("%d", &ci);
    
	int sno = (ci - 1);
    
	if (sno < 0)
  	  return; //no appliance changed

	printf("Name: %s\nArea: %s\nWatt: %.2fkWh\n", entry[sno].name, entry[sno].area, entry[sno].watt);
	if(entry[sno].stat == 0)
  	  printf("Stat: OFF\n");
	else if (entry[sno].stat == 1)
  	  printf("Stat: ON\n");
  		 
	if(entry[sno].stat == 0)
  	  printf("Do you want to switch the appliance ON? ");
	else if (entry[sno].stat == 1)
  	  printf("Do you want to switch the appliance OFF? ");
    
	char ch;
	printf("Enter choice (Y/N): ");
	fflush(stdin);
	scanf(" %c", &ch);
    
	if (ch == 'N' || ch == 'n')
  	  goto shuru;
	else if (ch == 'Y' || ch == 'y'){
  	  pthread_t toggle;
  	  if (pthread_create(&toggle, NULL, switchcsv, (void*)&entry[sno])!=0){
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
  		  FILE *file = fopen("appliances_data.csv", "r");
  		      if (!file){
  			  perror("Failed to open file");
  			  goto again;
  		      }
  			 
  	  	      char line[LEN];
  		      int count=0;
  		      AppData *entry = malloc(MAX * sizeof(AppData));
  	     		 int i = 1; // Initialize i outside the loop
  	     		 int header = 0; // Flag to indicate if header is skipped

  		      printf("\n\n\t\t\tAppliances List:");
  			 
  		      while (fgets(line, sizeof(line), file)){
  			  if (!header) {
  			      header = 1;
  			      continue;
  			  }

  			  char name[LEN];
  			  char area[LEN];
  			  float watt;
  			  int flag;
  			 
  			  sscanf(line, "%[^,],%[^,],%f,%d", name, area, &watt, &flag);
  			  strncpy(entry[count].name, name, sizeof(entry[count].name) - 1);
  			  entry[count].name[sizeof(entry[count].name) - 1] = '\0';
  			  strncpy(entry[count].area, area, sizeof(entry[count].area) - 1);
  			  entry[count].area[sizeof(entry[count].area) - 1] = '\0';
  			  entry[count].watt = watt;
  			  entry[count].stat = flag;
  			  count++;
  		 
  			  sscanf(line, "%[^,],%[^,],%f,%d", name, area, &watt, &flag);
  			  printf("\nSno: %d\nAppliance: %s\nArea: %s\nEnergy Consumption: %.2fkWh\n", i, name, area, watt);
  			  if(flag == 0)
  				  printf("Stat: OFF\n");
  			  else if (flag == 1)
  				  printf("Stat: ON\n");
  			  i++;
  		      }
  			 
  		      fclose(file);
  		      toggle(entry);
  	  break;
  	  } //end of outer switch case 1
  	 
  	  case 2:{ //sorted list
  		  FILE *file = fopen("appliances_data.csv", "r");
  		      if (!file){
  			  perror("Failed to open file");
  			  goto again;
  		      }
  			 
  		  char line[LEN];
  		  int count=0;
  		  AppData *entry = malloc(MAX * sizeof(AppData));
  		  int i = 1; // Initialize i outside the loop
  		  int header = 0; // Flag to indicate if header is skipped
  			 
  		      while (fgets(line, sizeof(line), file)){
  			  if (!header) {
  			      header = 1;
  			  } else {
  				  char name[LEN];
  				  char area[LEN];
  				  float watt;
  				  int flag;
  				 
  				  sscanf(line, "%[^,],%[^,],%f,%d", name, area, &watt, &flag);
  				  strncpy(entry[count].name, name, sizeof(entry[count].name) - 1);
  				  entry[count].name[sizeof(entry[count].name) - 1] = '\0';
  				  strncpy(entry[count].area, area, sizeof(entry[count].area) - 1);
  				  entry[count].area[sizeof(entry[count].area) - 1] = '\0';
  				  entry[count].watt = watt;
  				  entry[count].stat = flag;
  				  count++;
  			  }    
  		  }
  		  fclose(file);
  	  sort:
  		  int c;
  		  printf("\n\nSorted list display options:\n1, Alphabetically\n2, Sorted Area Wise\n3, Lowest to Highest Consumption\n4, Highest to Lowest Consumption\n5, By their status\nEnter choice: ");
  		  scanf("%d", &c);
  		  switch(c){
  			  case 1:{
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
  				  goto sort;
  			  break;
  			  }
  		  }
  		  toggle(entry);
  	  break;
  	  } // end outer switch case 2
  	 
  	  default:{
  		  printf("\nInvalid Choice.");
  		  goto again;
  	  break;
  	  }
  	 
	}

}

void* readflags(void* arg){
	FILE *file = fopen("appliances_data.csv", "r");
	if (!file){
  	  perror("Failed to open file");
  	  return NULL;
	}
    
	char line[LEN];
	AppData *entry = malloc(MAX * sizeof(AppData));
	int count = 0;
    
	while (fgets(line, sizeof(line), file)){
  	  char name[LEN];
  	  char area[LEN];
  	  float watt;
  	  int flag;
  	 
  	  sscanf(line, "%[^,],%[^,],%f,%d", name, area, &watt, &flag);
  	 
  	  if (flag==1){
  		  strncpy(entry[count].name,  name, sizeof(entry[count].name) - 1);
               		 entry[count].name[sizeof(entry[count].name) - 1] = '\0';
  		  strncpy(entry[count].area, area, sizeof(entry[count].area) - 1);
               		 entry[count].area[sizeof(entry[count].area) - 1] = '\0';
               		 entry[count].watt = watt;
               		 count++;
  	  }    
	}
	fclose(file);
    
	if (count == 0)
  	  return NULL;
	else
  	  return entry;
}

void displayappmenu(void){
menuu:
	int choice;
	printf("\n\nMenu:\n1, Switch Off/On appliance\n2, Check appliances\n3, Go to Main menu\nEnter Choice: ");
	scanf("%d", &choice);
    
	switch(choice){
  	  case 1:{
  		  displaylist();    
  		  goto menuu;  	 
  	  break;
  	  }
  	 
  	  case 2:{
  		  pthread_t thread;
  		  if (pthread_create(&thread, NULL, readflags, NULL)){
  			  perror("Failed to create thread.");
  			  break;
  		  }
  		 
  		  AppData *entries;
  		  if (pthread_join(thread, (void**) &entries)){
  			  perror("Failed to join");
  			  break;
  		  }
  		 
  		  if (entries == NULL){
  			  printf("No appliance is currently in use.");
  			  break;
  		  }
  		 
  		  printf("Entries with flag = 1:\n");
  	 		 for (int i = 0; entries[i].area[0] != '\0'; i++) {
  				  printf("Appliance: %s, Area: %s, Energy Consumption: %.2fkWh\n", entries[i].name, entries[i].area, entries[i].watt);
  	 		 }
  	  break;
  	  }
  	  
  	  case 3: {
		return;
	  }
  	 
  	  default:{
  		  printf("\nInvalid Choice.");
  		  goto menuu;
  	  break;
  	  }
	}    
}



