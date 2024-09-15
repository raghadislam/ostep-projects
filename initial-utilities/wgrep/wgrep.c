#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>


#define MAX_BUFFER_SIZE	128

int main(int argc, char*argv[])
{
	/* check if no command-line arguments passed */
	if(argc < 2) 
	{
		printf("wgrep: searchterm [file ...]\n");
		exit(1);
	}

	/* initialize the buffer and its size */
	char* buffer = NULL;
	size_t size = MAX_BUFFER_SIZE;

	/* in case no file specified mygrep reads from stdin */
	if(argc == 2)
	{

		while(getline(&buffer, &size, stdin) != EOF)
		{
			if(strstr(buffer, argv[1]) != NULL)
			{
				printf("%s", buffer);
			}
		}

	}
	
	/* in case a file or more soecified iterate on them */
	for(int i = 2; i < argc; i++)
	{
		/* open the file */
		FILE* fp = fopen(argv[i], "r");
		
		/* check for failure in openning */
		if(fp == NULL)
		{
			printf("wgrep: cannot open file\n");
			exit(1);
		}

		/* look fir the search term in the file */
		while(getline(&buffer, &size, fp) != EOF)
		{
			if(strstr(buffer, argv[1]) != NULL)
			{
				printf("%s", buffer);
			}
		}

	}
	
	free(buffer);
	exit(0);
	
}
