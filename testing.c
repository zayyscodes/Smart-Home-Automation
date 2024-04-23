#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LENGTH 1024

int main() {
    const char *filename = "energy_in (another copy).csv"; // Replace with your CSV file's name
    int sno_to_update = 10; // Replace with the sno you want to update
    float new_energy_in = 99.9; // Replace with the new energy_in value

    FILE *file = fopen(filename, "r+");
    if (!file) {
        perror("Failed to open the file");
        return 1;
    }

    // Read the header line and store it for later writing
    char header[MAX_LINE_LENGTH];
    if (!fgets(header, sizeof(header), file)) {
        perror("Failed to read header");
        fclose(file);
        return 1;
    }

    char line[MAX_LINE_LENGTH];
    long int header_end_position = ftell(file);

    // Find the position of the line with the sno to be updated
    while (fgets(line, sizeof(line), file)) {
        int sno;
        float energy_in;
        long int line_position = ftell(file) - strlen(line);

        if (sscanf(line, "%d,%f", &sno, &energy_in) != 2) {
            fprintf(stderr, "Invalid line format: %s", line);
            continue;
        }

        if (sno == sno_to_update) {
            // Modify the line with the updated energy_in value
            fseek(file, line_position, SEEK_SET);

            // Remove the newline character from the line
            char *newline_pos = strchr(line, '\n');
            if (newline_pos)
                *newline_pos = '\0';

            fprintf(file, "%d,%.1f", sno_to_update, new_energy_in); // Write without newline
            break;
        }
    }

    fclose(file);

    printf("CSV file updated successfully.\n");

    return 0;
}

