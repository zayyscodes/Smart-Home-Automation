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

extern SmartHome* shm;

void initialise(SmartHome* shm) {
    // Initialize the mutexes
    pthread_mutex_init(&(shm->mutex_enerin), NULL);
    pthread_mutex_init(&(shm->mutex_inuse), NULL);
    pthread_mutex_init(&(shm->mutex_preftemp), NULL);
    pthread_mutex_init(&(shm->mutex_svmd), NULL);
    pthread_mutex_init(&(shm->mutex_tasks), NULL);
}

void setshm(SmartHome* shm) {
    // Lock mutexes before initializing shared memory
    pthread_mutex_lock(&(shm->mutex_enerin));
    pthread_mutex_lock(&(shm->mutex_inuse));
    pthread_mutex_lock(&(shm->mutex_preftemp));
    pthread_mutex_lock(&(shm->mutex_svmd));
    pthread_mutex_lock(&(shm->mutex_tasks));

init:
    fflush(stdin);
    system("clear");

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
    int tm;
    printf("\nPrefered temperature in °C: ");
    scanf("%d", &tm);
    *shm->preftemp=tm;
	printf("Yhn tk shi hi");
    shm->svmd = 0;
    shm->cnt = 0;

    // Unlock mutexes after initialization
    pthread_mutex_unlock(&(shm->mutex_enerin));
    pthread_mutex_unlock(&(shm->mutex_inuse));
    pthread_mutex_unlock(&(shm->mutex_preftemp));
    pthread_mutex_unlock(&(shm->mutex_svmd));
    pthread_mutex_unlock(&(shm->mutex_tasks));
    
    
}


SmartHome* getshm() {
    int shm_fd = shm_open(shm_name, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        return NULL;
    }

    if (ftruncate(shm_fd, sizeof(SmartHome)) == -1) {
        perror("ftruncate");
        return NULL;
    }

    SmartHome *data = mmap(NULL, sizeof(SmartHome), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    close(shm_fd);

    if (data == MAP_FAILED) {
        perror("mmap");
        return NULL;
    }

    return data;
}


void detachSharedMemory(SmartHome* shm) {
    munmap(shm, sizeof(SmartHome));
    shm_unlink(shm_name);
}



