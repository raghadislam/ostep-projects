#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "filehandling.h"


void ReadFromFile(char* arr[], int max_key) {
    FILE* file = fopen("DataBase.txt", "r");
    if (file == NULL) {
        printf("Unable to open file\n");
        exit(1);
    }

    int key;
    char* value = malloc(MAXLENGTH * sizeof(char)); 
    if (value == NULL) {
        printf("Memory allocation failed\n");
        fclose(file);
        exit(1);
    }

    while (fscanf(file, "%d,%100[^\n]", &key, value) == 2) {
        if (key < 0 || key >= max_key) {  
            printf("Key %d out of bounds, skipping...\n", key);
            continue;
        }
        arr[key] = strdup(value);  
    }

    fclose(file);
    free(value);
}

void WriteToFile(char* arr[], int array_size) {
    FILE* file = fopen("DataBase.txt", "w");

    if (file == NULL) {
        printf("Unable to open file\n");
        return;
    }

    for (int i = 0; i < array_size; i++) {
        if (arr[i] != NULL) {
            fprintf(file, "%d,%s\n", i, arr[i]);
        }
    }

    fclose(file);
}

