#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
    /* check correct usage */
    if (argc < 2) {
	printf("wunzip: file1 [file2 ...]\n");
	exit(1);
    }

    FILE *pf = NULL;
    char character = 0;
    int count = 0;
    /* iterate on all files */
    for (int i = 1; i < argc; i++) {
	/* Open the next to be read */
	pf = fopen(argv[i], "r");

	/* check for faliure in opening */
	if (pf == NULL) {
	    printf("wzip: cannot open file\n");
	    exit(1);
	} else {
	    /* Read the next 4-bytes (size of int) as int from the opened file */
	    while (fread(&count, sizeof(int), 1, pf) != 0) {
		/* Read the char next to that int */
		fread(&character, sizeof(char), 1, pf);

		/* print the original form of the zoipped file */
		for (int j = 0; j < count; ++j) {
		    printf("%c", character);
		}
	    }
	}
	/* close the file */
	fclose(pf);
    }

    exit(0);
}
