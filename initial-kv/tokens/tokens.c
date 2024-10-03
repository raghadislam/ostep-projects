#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void tokenize(const char* input, const char* delimiter, char*** tokens, int* token_count) {
  
    char* str = strdup(input);
    if (str == NULL) {
        perror("Failed to allocate memory");
        exit(EXIT_FAILURE);
    }

    *token_count = 0;

    char* temp = strtok(str, delimiter);
    while (temp != NULL) {
        (*token_count)++;
        temp = strtok(NULL, delimiter);
    }

    *tokens = malloc(*token_count * sizeof(char*));
    if (*tokens == NULL) {
        perror("Failed to allocate memory for tokens");
        free(str); 
        exit(EXIT_FAILURE);
    }

    strcpy(str, input);
    char* token = strtok(str, delimiter);
    int index = 0;

    while (token != NULL) {
        (*tokens)[index++] = strdup(token);  
        token = strtok(NULL, delimiter);
    }

    free(str); 
}

void free_tokens(char** tokens, int token_count) {
    for (int i = 0; i < token_count; i++) {
        free(tokens[i]);  
    }
    free(tokens); 
}

