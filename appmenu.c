#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#define MAX 100
#define LEN 1024

typedef struct{
	char name[LEN];
	char area[LEN];
	float watt;
	int stat;
}AppData;

int namesort(const void *a, const void *b) {
    return strcmp(((AppData *)a)->name, ((AppData *)b)->name);
}

int areasort(const void *a, const void *b) {
    return strcmp(((AppData *)a)->area, ((AppData *)b)->area);
}

int lowtohigh(const void *a, const void *b) {
    if (((AppData *)a)->watt < ((AppData *)b)->watt) {
        return -1;
    } else if (((AppData *)a)->watt > ((AppData *)b)->watt) {
        return 1;
    } else {
        return 0;
    }
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

void* switchcsv(void* arg){
	AppData *name = (AppData *) arg; //appliance name
	const char* filename = "dupappliances.csv";
	FILE *file = fopen(filename, "r+");
	if (!file){
		perror("Failed to open the file");
        	return 1;
	}
	// YAHAN
	// CHORA
	// HAI
	// CONTINUE
	// YAHAN
	// SE
	// !!
}


void toggle(AppData entry[]){
shuru:
	int ci;
	printf("\nWhich appliance do you want to switch on/off? Enter Serial Number (0 to exit): ");
	scanf("%d", &ci);
	
	int sno = (ci - 1);
	
	if (sno < 0)
		return; //no appliance changed
	
	printf("\nName: %s, Area: %s, Watt: %.2f,", entry[sno].name, entry[sno].area, entry[sno].watt);
	if(entry[sno].stat == 0)
		printf("Stat: OFF\n");
	else if (entry[sno] == 1)
		printf("Stat: ON\n");
			
	if(entry[sno].stat == 0)
		printf("Do you want to switch the appliance ON? ");
	else if (entry[sno] == 1)
		printf("Do you want to switch the appliance OFF? ");
	
	char ch;
	printf("Enter choice (Y/N): ");
	scanf("%c", &ch);
	
	if (ch == 'N' || ch == 'n')
		goto shuru;
	else if (ch == 'Y' || ch == 'y'){
		pthread_t toggle;
		if (pthread_create(&toggle, NULL, switchcsv, (void*)&entry[sno])!=0){
			perror("Failed to create thread.\n");
			return;
		} 
		if (pthread_join(toggle, NULL){
			perror("Failed to join thread.\n");
			return;
		}
		
		printf("\nName: %s, Area: %s, Watt: %.2f,", entry[sno].name, entry[sno].area, entry[sno].watt);
	if(entry[sno].stat == 0)
		printf("Stat: OFF\n");
	else if (entry[sno] == 1)
		printf("Stat: ON\n");
		
		
	} else {
		printf("Invalid choice.");
		goto shuru;
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
				count++;
			
				sscanf(line, "%[^,],%[^,],%f,%d,%d", name, area, &watt, &flag, &stat);
				printf("\nSno: %d\nAppliance: %s\nArea: %s\nEnergy Consumption: %.2fkWh\n", i, name, area, watt);
				if(entry[sno].stat == 0)
					printf("Stat: OFF\n");
				else if (entry[sno] == 1)
					printf("Stat: ON\n");
				i++;
			    }
			    
			    fclose(file);
		break;
		}
		
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
					count++;
				}	
			}
			fclose(file);
		sort:
			int c;
			printf("Display options:\n1, Sorted Alphabetically\n2, Sorted Area Wise\n3, Lowest to Highest Consumption\n4, Highest to Lowest Consumption\nEnter choice: ");
			scanf("%d", &c);
			switch(c){
				case 1:{
					qsort(entry, count, sizeof(AppData), namesort);
					printf("Sorted Appliances List (by area):\n");
				        for (int i = 0; i < count; i++){
						printf("Name: %s, Area: %s, Watt: %.2f\n", entry[i].name, entry[i].area, entry[i].watt);
						if(entry[sno].stat == 0)
							printf("Stat: OFF\n");
						else if (entry[sno] == 1)
							printf("Stat: ON\n");
				        }
				break;
				}
				
				case 2:{
				    qsort(entry, count, sizeof(AppData), areasort);
				    printf("Sorted Appliances List (by area):\n");
				    for (int i = 0; i < count; i++) {
						printf("Name: %s, Area: %s, Watt: %.2f\n", entry[i].name, entry[i].area, entry[i].watt);
						if(entry[sno].stat == 0)
							printf("Stat: OFF\n");
						else if (entry[sno] == 1)
							printf("Stat: ON\n");
				        }
				break;
				}
				
				case 3:{
				    qsort(entry, count, sizeof(AppData), lowtohigh);
				    printf("Sorted Appliances List (by watt, lowest to highest):\n");
				    for (int i = 0; i < count; i++) {
						printf("Name: %s, Area: %s, Watt: %.2f\n", entry[i].name, entry[i].area, entry[i].watt);
						if(entry[sno].stat == 0)
							printf("Stat: OFF\n");
						else if (entry[sno] == 1)
							printf("Stat: ON\n");
				        }
				break;
				}
				
				case 4:{
				    qsort(entry, count, sizeof(AppData), hightolow);
				    printf("Sorted Appliances List (by watt, lowest to highest):\n");
				    for (int i = 0; i < count; i++) {
						printf("Name: %s, Area: %s, Watt: %.2f\n", entry[i].name, entry[i].area, entry[i].watt);
						if(entry[sno].stat == 0)
							printf("Stat: OFF\n");
						else if (entry[sno] == 1)
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
	printf("\n\nMenu:\n1, Switch Off/On appliance\n2, Check appliances\nEnter Choice: ");
	scanf("%d", &choice);
	
	switch(choice){
		case 1:{
			displaylist();
			printf("Which appliance you want to switch off or switch on?");
			
			
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
		
		default:{
			printf("\nInvalid Choice.");
			goto menuu;
		break;
		}
	}
		
}



