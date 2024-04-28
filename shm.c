#include "shm.h"
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include "e_manage.h"

#define shm_name "/my_osproj_shm"
#define SIZE 10000000

static SmartHome *shm_ptr = NULL;

void initialise(SmartHome *shm) {
    // Initialize the mutexes
    pthread_mutex_init(&(shm->mutex_enerin), NULL);
    pthread_mutex_init(&(shm->mutex_inuse), NULL);
    pthread_mutex_init(&(shm->mutex_preftemp), NULL);
    pthread_mutex_init(&(shm->mutex_svmd), NULL);
    pthread_mutex_init(&(shm->mutex_tasks), NULL);
    
    shm->enerin = 0.0;
    shm->inuse = 0.0;
    shm->secure = 0; //flag whether security setting is enabled
    shm->preftemp = 26; //room temp
    shm->svmd = 0; 
}

void setshm(SmartHome *shm) {
    // Lock mutexes before initializing shared memory
    pthread_mutex_lock(&(shm->mutex_preftemp));
    pthread_mutex_lock(&(shm->mutex_svmd));
    pthread_mutex_lock(&(shm->mutex_tasks));
    
    shm->svmd = 0;
init:
    fflush(stdin);
    system("clear");
    
    pthread_mutex_lock(&(shm->mutex_enerin));
    printf("\nDo you want to enable security setting? (Y/N): ");
    char choice;
    scanf(" %c", &choice); // Note the space before %c to consume whitespace

    if (choice == 'Y' || choice == 'y')
        shm->secure = 1;
    else if (choice == 'N' || choice == 'n') 
        shm->secure = 0;
    else {
        printf("\nInvalid option.");
        goto init;
    }
    
    pthread_mutex_lock(&(shm->mutex_inuse));
    printf("\nPrefered temperature in °C: ");
    scanf("%d", &(shm->preftemp));
    
    // Unlock mutexes after initialization
    pthread_mutex_unlock(&(shm->mutex_enerin));
    pthread_mutex_unlock(&(shm->mutex_inuse));
    pthread_mutex_unlock(&(shm->mutex_preftemp));
    pthread_mutex_unlock(&(shm->mutex_svmd));
    pthread_mutex_unlock(&(shm->mutex_tasks));
}



SmartHome *getshm() {
    size_t sizeshm = sizeof(SmartHome);
    int shm_fd = shm_open(shm_name, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        return NULL;
    }

    if (ftruncate(shm_fd, sizeshm) == -1) {
        perror("ftruncate");
        return NULL;
    }

    SmartHome *data = (SmartHome *)mmap(NULL, sizeshm, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    close(shm_fd); // Close file descriptor after mapping

    if (data == MAP_FAILED) {
        perror("mmap");
        return NULL;
    }

    return data;
}



void tempchange(SmartHome *shm) {
        int temp;
	printf("Your thermostat was previously set to: %d°C\n", shm->preftemp);
	printf("Enter new temperature: ");
	scanf("%d", &temp);
	
	pthread_mutex_lock(&(shm->mutex_preftemp));	
	shm->preftemp = temp;
	printf("Your thermostat was previously set to: %d°C\n", shm->preftemp);
	pthread_mutex_unlock(&(shm->mutex_preftemp));
}




void detachSharedMemory(SmartHome *shm) {
    munmap(shm, sizeof(SmartHome));
    shm_unlink(shm_name);
}





