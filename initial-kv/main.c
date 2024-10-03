#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "filehandling/filehandling.h"
#include "tokens/tokens.h"

int main(int argc, char* argv[]) {

    char* arr[MAXKEY] = {NULL};  

    ReadFromFile(arr, MAXKEY);

    for(int i = 1; i < argc; i++)
    {
	    const char* input = strdup(argv[i]);
	    const char* delimiter = ",";
	    char** tokens = NULL;
	    int token_count = 0;
	    tokenize(input, delimiter, &tokens, &token_count);
	    if(strcmp(tokens[0], "p") == 0)
	    {
		    int key = atoi(tokens[1]);
		    arr[key] = strdup(tokens[2]);
	    }
	    else if(strcmp(tokens[0], "g") == 0)
	    {
		    int key = atoi(tokens[1]);
		    if(key >= MAXKEY || arr[key] == NULL)
		    {
			    printf("%d not found\n", key);
		    }
		    else
		    {
			    printf("%d,%s\n", key, arr[key]);
		    }
	    }
    	    else if(strcmp(tokens[0], "d") == 0)
	    {
		    int key = atoi(tokens[1]);
		    if(arr[key] == NULL)
		    {
			    printf("%d not found\n", key);
		    }
		    else
		    {
			    arr[key] = NULL;
		    }

	    }
    	    else if(strcmp(tokens[0], "c") == 0)
	    {
		   for(int i = 0; i < MAXKEY; i++)
		   {
			   arr[i] = NULL;
		   }
	    }
    	    else if(strcmp(tokens[0], "a") == 0)
	    {
		   for(int i = 0; i < MAXKEY; i++)
		   {
			   if(arr[i] != NULL) printf("%d,%s\n", i, arr[i]);
		   }
	    }
	    else
	    {
		    printf("bad command\n");
	    }

    	free_tokens(tokens, token_count);

    }

    WriteToFile(arr, MAXKEY);
 
    for (int i = 0; i < MAXKEY; i++) {
        if (arr[i] != NULL) {
            free(arr[i]); 
        }
    }

    return 0;
}
