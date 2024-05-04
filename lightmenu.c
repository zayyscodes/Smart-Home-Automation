#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include "e_manage.h"
#include "lightmenu.h"
#include "shm.h"
#include "scheduling.h"
#include "shm.h"
#define MAX 9
#define LEN 1024

extern pthread_mutex_t mutex;
extern pthread_mutex_t update;
extern pthread_mutex_t file_mutex;

extern SmartHome *shm; //pointer to shared memory
extern int lightupdated;

/*  	  S O R T I N G   F U N C T I O N S   	  */

int areasortt(const void *a, const void *b) {
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

int lowtohighh(const void *a, const void *b) {
    if (((LightData *)a)->watt < ((LightData *)b)->watt)
   	 return -1;
    else if (((LightData *)a)->watt > ((LightData *)b)->watt)
   	 return 1;
    else
   	 return 0;
}

int hightoloww(const void *a, const void *b) {
    if (((LightData *)a)->watt < ((LightData *)b)->watt) {
   	 return 1;
    } else if (((LightData *)a)->watt > ((LightData *)b)->watt) {
   	 return -1;
    } else {
   	 return 0;
    }
}

int statsortt(const void *a, const void *b) {
    return -(((LightData *)a)->stat - ((LightData *)b)->stat);
}



/*  	   T H R E A D   T O   U P D A T E    	*/

void* switchincsv(void* arg) {
    LightData* light = (LightData*)arg;
    int onoff;

    int ci;
    printf("\nWhich area's lights do you want to switch on/off? Enter Serial Number (0 to exit): ");
    scanf("%d", &ci);
    int sno = ci - 1;

    if (sno < 0 || sno >= MAX)
        return NULL;

    printf("Area: %s\nNum: %d\nWatt: %.2fkWh\n", light[sno].area, light[sno].num, light[sno].watt);
    if (light[sno].stat == 0) {
        printf("Stat: OFF\n");
        printf("Do you want to switch the lights ON? ");
        onoff = 1;
    } else if (light[sno].stat == 1) {
        printf("Stat: ON\n");
        printf("Do you want to switch the lights OFF? ");
        onoff = 0;
    }

    char ch;
    printf("Enter choice (Y/N): ");
    fflush(stdin);
    scanf(" %c", &ch);

    if (ch == 'Y' || ch == 'y') {
        if (shm->inuse >= shm->enerin && onoff == 1){
            printf("\nUnable to switch area lights on; exceeds input\n");
            lightupdated = 0;
        } else {
            light[sno].stat = (light[sno].stat == 0) ? 1 : 0;

            FILE* file = fopen("lights_data (copy).csv", "w");
            if (file == NULL) {
                perror("Failed to open file");
                return NULL;
            }

            fprintf(file, "area,num,watt,flag\n");

            for (int i = 0; i < MAX; i++) {
                fprintf(file, "%s,%d,%.2f,%d\n", light[i].area, light[i].num, light[i].watt, light[i].stat);
            }

            fclose(file);

            pthread_mutex_lock(&mutex);
            write_task_to_pipe("lighton");
            printf("Written to ARRAY\n\n");
            pthread_mutex_unlock(&mutex);
            
            FILE* txt_file = fopen("task_assign_info.txt", "a");
            if (txt_file == NULL) {
                perror("Failed to open text file");
                return NULL;
            }

            fprintf(txt_file, "\n\n\n****LIGHT AUTOMATION****\n");

            fprintf(txt_file, "Area: %s\nNum of Bulbs: %d\nWatt: %.2fkWh\n", light[sno].area, light[sno].num, light[sno].watt);
            fprintf(txt_file, "Status: %s\n\n", (light[sno].stat == 0) ? "OFF" : "ON");

            fclose(txt_file);

            printf("Text file updated successfully.\n");
            lightupdated = 1;
        }
    }

    pthread_exit(NULL);
}


void togglee(LightData light[]) {
    pthread_t toggle;
    pthread_mutex_lock(&file_mutex);
    if (pthread_create(&toggle, NULL, switchincsv, (void*)light) != 0) {
        perror("Failed to create thread.\n");
        pthread_mutex_unlock(&file_mutex); 
        return;
    }
    if (pthread_join(toggle, NULL)) {
        perror("Failed to join thread.\n");
        pthread_mutex_unlock(&file_mutex);
        return;
    }
    pthread_mutex_unlock(&file_mutex);
}



void displaylistt(void) {
again:
    int ch;
    printf("Display options:\n1, As Default\n2, Sorted\nEnter choice: ");
    scanf("%d", &ch);
    switch (ch) {
	    case 1: {
		pthread_mutex_lock(&update);
		FILE* file = fopen("lights_data (copy).csv", "r");
		if (!file) {
		    perror("Failed to open file");
		    goto again;
		}

		char line[LEN];
		int count = 0;
		LightData* light = malloc(MAX * sizeof(LightData));
		int i = 1; // Initialize i outside the loop
		int header = 0; // Flag to indicate if header is skipped

		printf("\n\n\t\t\tLights List:");

		while (fgets(line, sizeof(line), file)) {
		    if (!header) {
		        header = 1;
		        continue;
		    }
		    char area[LEN];
		    int num;
		    float watt;
		    int flag;

		    if (sscanf(line, "%[^,],%d,%f,%d", area, &num, &watt, &flag) != 4) {
		        fprintf(stderr, "Invalid line format: %s", line);
		        continue;
		    }

		    strncpy(light[count].area, area, sizeof(light[count].area) - 1);
		    light[count].area[sizeof(light[count].area) - 1] = '\0';
		    light[count].num = num;
		    light[count].watt = watt;
		    light[count].stat = flag;
		    count++;

		    printf("\nSno: %d\nlightliance: %s\nnum: %d\nEnergy Consumption: %.2fkWh\n", i, area, num, watt);
		    if (flag == 0)
		        printf("Stat: OFF\n");
		    else if (flag == 1)
		        printf("Stat: ON\n");
		    i++;
		}

		fclose(file);
		pthread_mutex_unlock(&update);

		togglee(light);
		break;
	    }

	    case 2: {
		pthread_mutex_lock(&update);
		FILE* file = fopen("lights_data (copy).csv", "r");
		if (!file) {
		    perror("Failed to open file");
		    goto again;
		}

		char line[LEN];
		int count = 0;
		LightData* light = malloc(MAX * sizeof(LightData));
		int i = 1; // Initialize i outside the loop
		int header = 0; // Flag to indicate if header is skipped

		while (fgets(line, sizeof(line), file)) {
		    if (!header) {
		        header = 1;
		        continue;
		    }
		    else {
		        char area[LEN];
		        int num;
		        float watt;
		        int flag;

		        if (sscanf(line, "%[^,],%d,%f,%d", area, &num, &watt, &flag) != 4) {
		            fprintf(stderr, "Invalid line format: %s", line);
		            continue;
		        }

		        strncpy(light[count].area, area, sizeof(light[count].area) - 1);
		        light[count].area[sizeof(light[count].area) - 1] = '\0';
		        light[count].num = num;
		        light[count].watt = watt;
		        light[count].stat = flag;
		        count++;
		    }
		}
		fclose(file);
		pthread_mutex_unlock(&update);

	sort:
		int c;
		printf("\n\nSorted list display options:\n1, Alphabetically\n2, Sorted num Wise\n3, Lowest to Highest Consumption\n4, Highest to Lowest Consumption\n5, By their status\nEnter choice: ");
		scanf("%d", &c);
		switch (c) {
			case 1: {
			    qsort(light, count, sizeof(LightData), areasortt);
			    printf("Sorted Lights List (by area):\n");
			    for (int i = 0; i < count; i++) {
				printf("\nSno: %d\n", i + 1);
				printf("Area: %s\nNum: %d\nWatt: %.2fkWh\n", light[i].area, light[i].num, light[i].watt);
				if (light[i].stat == 0)
				    printf("Stat: OFF\n");
				else if (light[i].stat == 1)
				    printf("Stat: ON\n");
			    }
			    break;
			}

			case 2: {
			    qsort(light, count, sizeof(LightData), areasortt);
			    printf("Sorted Lights List (by num):\n");
			    for (int i = 0; i < count; i++) {
				printf("\nSno: %d\n", i + 1);
				printf("Area: %s\nNum: %d\nWatt: %.2fkWh\n", light[i].area, light[i].num, light[i].watt);
				if (light[i].stat == 0)
				    printf("Stat: OFF\n");
				else if (light[i].stat == 1)
				    printf("Stat: ON\n");
			    }
			    break;
			}

			case 3: {
			    qsort(light, count, sizeof(LightData), lowtohighh);
			    printf("Sorted Lights List (by watt, lowest to highest):\n");
			    for (int i = 0; i < count; i++) {
				printf("\nSno: %d\n", i + 1);
				printf("Area: %s\nNum: %d\nWatt: %.2fkWh\n", light[i].area, light[i].num, light[i].watt);
				if (light[i].stat == 0)
				    printf("Stat: OFF\n");
				else if (light[i].stat == 1)
				    printf("Stat: ON\n");
			    }
			    break;
			}

			case 4: {
			    qsort(light, count, sizeof(LightData), hightoloww);
			    printf("Sorted Lights List (by watt, highest to lowest):\n");
			    for (int i = 0; i < count; i++) {
				printf("\nSno: %d\n", i + 1);
				printf("Area: %s\nNum: %d\nWatt: %.2fkWh\n", light[i].area, light[i].num, light[i].watt);
				if (light[i].stat == 0)
				    printf("Stat: OFF\n");
				else if (light[i].stat == 1)
				    printf("Stat: ON\n");
			    }
			    break;
			}

			case 5: {
			    qsort(light, count, sizeof(LightData), statsortt);
			    printf("Sorted Lights List (by status):\n");
			    for (int i = 0; i < count; i++) {
				printf("\nSno: %d\n", i + 1);
				printf("Area: %s\nNum: %d\nWatt: %.2fkWh\n", light[i].area, light[i].num, light[i].watt);
				if (light[i].stat == 0)
				    printf("Stat: OFF\n");
				else if (light[i].stat == 1)
				    printf("Stat: ON\n");
			    }

			    break;
			}

			default: {
			    printf("\nInvalid Choice.");
			    goto sort;
			    break;
			}
		}
		togglee(light);
		break;
	    }

	    default: {
		printf("\nInvalid Choice.");
		//goto again;
		break;
	    }

    }
}


void* readflag(void* arg) {
    FILE* file = fopen("lights_data (copy).csv", "r");
    if (!file) {
        perror("Failed to open file");
        return NULL;
    }

    char line[LEN];
    
    LightData* light = malloc(MAX * sizeof(LightData));
    if (!light) {
        perror("Failed to allocate memory");
        fclose(file);
        return NULL;
    }

    int count = 0;
    int header = 0;

    while (fgets(line, sizeof(line), file)) {
    	if (!header) {
		header = 1;
		continue;
	}
        char area[LEN];
        int num;
        float watt;
        int flag;

        if (sscanf(line, "%[^,],%d,%f,%d", area, &num, &watt, &flag) == 4) {
            if (flag == 1) {
                strncpy(light[count].area, area, sizeof(light[count].area) - 1);
                light[count].area[sizeof(light[count].area) - 1] = '\0';
                light[count].num = num;
                light[count].watt = watt;
                count++;
            }
        } else {
            fprintf(stderr, "Error parsing line: %s\n", line);
        }
    }

    fclose(file);

    if (count == 0) {
        free(light);
        return NULL;
    } else {
        return light;
    }
}


void displaylightmenu() {
menu:
    if (lightupdated == 1) {
        pthread_mutex_lock(&(shm->mutex_inuse));
        setconsume();
        pthread_mutex_unlock(&(shm->mutex_inuse));
        int choice;
        printf("\n\nMenu:\n1, Switch Off/On Light\n2, Check Lights\n3, Go to Main menu\nEnter Choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1: {
                displaylistt();
                goto menu;
                break;
            }

            case 2: {
                pthread_t thread;
                if (pthread_create(&thread, NULL, readflag, NULL)) {
                    perror("Failed to create thread.");
                    break;
                }

                LightData* entries;
                if (pthread_join(thread, (void**)&entries)) {
                    perror("Failed to join");
                    break;
                }
                
                if (entries == NULL) {
                    printf("Lights aren't open for any area.");
                    break;
                }

                printf("Entries with flag = 1:\n");
                for (int i = 0; entries[i].area[0] != '\0' && entries[i].num != 0; i++) {
                    printf("Area: %s, Num: %d, Watt: %.2fkWh\n", entries[i].area, entries[i].num, entries[i].watt);
                }
                goto menu;
                break;
            }

            case 3: {
                return;
            }

            default: {
                printf("\nInvalid Choice.");
                goto menu;
                break;
            }
        }
    } else
        return;
}


