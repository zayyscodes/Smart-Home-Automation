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

void signUp(struct User users[], int *usernum) {
    if (*usernum >= MAX_USERS) {
        printf("Maximum number of users reached!\n");
        return;
    }

    printf("\nEnter login ID: ");
    scanf("%s", users[*usernum].id);

    printf("Enter password: ");
    scanf("%s", users[*usernum].password);

    FILE *file = fopen("user.txt", "a");
    if (file != NULL) {
        fprintf(file, "%s %s\n", users[*usernum].id, users[*usernum].password);
        fclose(file);
    } else {
        printf("Error saving user data to file.\n");
    }

    printf("Account created successfully!\n");
    (*usernum)++;
}

int logIn(struct User users[], int usernum) {
    int tries = 0;
loginn:
    if (tries < 3) {
        char loginID[idlen];
        char password[pwlen];

        printf("Enter login ID: ");
        scanf("%s", loginID);

        printf("Enter password: ");
        scanf("%s", password);

        int found = 0;
        for (int i = 0; i < usernum; i++) {
            if (strcmp(users[i].id, loginID) == 0 && strcmp(users[i].password, password) == 0) {
                printf("Login successful!\n");
                found = 1;
                break;
            }
        }

        if (!found) {
            printf("Incorrect login ID or password. Please try again.\n");
            tries++;
            goto loginn;
        } else 
            return 1;
    } else 
        return 0;
}

int loadUserData(struct User users[], int *usernum) {
    FILE *file = fopen("user.txt", "r");
    if (file != NULL) {
        while ((*usernum) < MAX_USERS && fscanf(file, "%s %s", users[*usernum].id, users[*usernum].password) != EOF) {
            (*usernum)++;
        }
        fclose(file);
        return 1;
    } else {
        printf("No existing user data found.\n");
        return 0;
    }
}

int getchoice() {
    int ch;
    printf("\nEnter Choice: ");
    scanf("%d", &ch);
    while (getchar() != '\n'); // Clear input buffer
    return ch;
}

