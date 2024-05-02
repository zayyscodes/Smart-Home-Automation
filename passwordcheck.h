#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "shm.h"

#define MAX_USERS 100
#define idlen 50
#define pwlen 50

// Structure to hold user information
struct User {
    char id[idlen];
    char password[pwlen];
};

// Function to sign up
void signUp(struct User users[], int *usernum);

// Function to log in
int logIn(struct User users[], int usernum);

// Function to load user data from file
int loadUserData(struct User users[], int *usernum);

int getchoice();

