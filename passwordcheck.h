#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "shm.h"

#define MAX_USERS 100
#define idlen 50
#define pwlen 50

struct User {
    char id[idlen];
    char password[pwlen];
};

void signUp(struct User users[], int *usernum);
int logIn(struct User users[], int usernum);
int loadUserData(struct User users[], int *usernum);
int getchoice();

