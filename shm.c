#include "shm.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>

#define shm_name "/my_shared_memory"

SmartHome* getshm() {
    int shm_fd = shm_open(shm_name, O_CREAT | O_RDWR, 0666);
    ftruncate(shm_fd, sizeof(SmartHome));
    SmartHome *data = mmap(NULL, sizeof(SmartHome), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    close(shm_fd);
    return data;
}

void destroy_shared_memory(SmartHome *data) {
    munmap(data, sizeof(SmartHome));
    shm_unlink(shm_name);
}



