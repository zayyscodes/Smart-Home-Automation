#include <stdio.h>  // Standard input-output operations
#include <stdlib.h> // Memory allocation and other utility functions
#include <string.h> // String manipulation functions
#include "shm.h"    // Custom header file for shared memory operations

#define MAX_USERS 100 // Maximum number of users
#define idlen 50      // Maximum length of user ID
#define pwlen 50      // Maximum length of password

// Structure to hold user information
struct User {
    char id[idlen];
    char password[pwlen];
};

// Function to allow users to sign up
void signUp(struct User users[], int *usernum) {
    if (*usernum >= MAX_USERS) { // Check if the maximum number of users has been reached
        printf("Maximum number of users reached!\n");
        return;
    }

    printf("\nEnter login ID: "); // Prompt for login ID
    scanf("%s", users[*usernum].id);

    printf("Enter password: "); // Prompt for password
    scanf("%s", users[*usernum].password);

    FILE *file = fopen("user.txt", "a"); // Open user data file in append mode
    if (file != NULL) {
        fprintf(file, "%s %s\n", users[*usernum].id, users[*usernum].password); // Write user data to file
        fclose(file);
    } else {
        printf("Error saving user data to file.\n");
    }

    printf("Account created successfully!\n");
    (*usernum)++; // Increment user count
}

// Function to allow users to log in
int logIn(struct User users[], int usernum) {
    int tries = 0;
loginn:
    if (tries < 3) { // Allow three login attempts
        char loginID[idlen];
        char password[pwlen];

        printf("Enter login ID: "); // Prompt for login ID
        scanf("%s", loginID);

        printf("Enter password: "); // Prompt for password
        scanf("%s", password);

        int found = 0;
        // Check if the entered credentials match any existing user data
        for (int i = 0; i < usernum; i++) {
            if (strcmp(users[i].id, loginID) == 0 && strcmp(users[i].password, password) == 0) {
                printf("Login successful!\n");
                found = 1;
                break;
            }
        }

        // If login is unsuccessful, allow the user to try again
        if (!found) {
            printf("Incorrect login ID or password. Please try again.\n");
            tries++;
            goto loginn;
        } else
            return 1; // Return 1 for successful login
    } else
        return 0; // Return 0 if maximum login attempts reached
}

// Function to load existing user data from file
int loadUserData(struct User users[], int *usernum) {
    FILE *file = fopen("user.txt", "r"); // Open user data file for reading
    if (file != NULL) {
        while ((*usernum) < MAX_USERS && fscanf(file, "%s %s", users[*usernum].id, users[*usernum].password) != EOF) {
            (*usernum)++; // Read user data until end of file or maximum users reached
        }
        fclose(file);
        return 1; // Return 1 for successful data loading
    } else {
        printf("No existing user data found.\n");
        return 0; // Return 0 if no user data file found
    }
}

// Function to get user choice from input
int getchoice() {
    int ch;
    printf("\nEnter Choice: ");
    scanf("%d", &ch);
    while (getchar() != '\n'); // Clear input buffer
    return ch;
}

/*using *usernum as a pointer provides more flexibility and clarity in managing and updating the number of users throughout the program.*/
