#include <stdio.h>
#include <stdlib.h>

#define MAX_BUFFER_SIZE 256

int mycat(int argc, char *argv[])
{
    FILE *fp;
    char buffer[MAX_BUFFER_SIZE];

    /* check that at least on file exists */
    if (argc < 2) {
	fprintf(stderr, "Usage: mycat file1 file2 ...\n");
	return 1;
    }

    /* iterate for files */
    for (int i = 1; i < argc; i++) {
	/* open the file */
	fp = fopen(argv[i], "r");

	/* check if it has been opend successfully */
	if (fp == NULL) {
	    fprintf(stdout, "wcat: %s: No such file or directory\n",
		    argv[i]);
	    continue;
	}

	while (fgets(buffer, MAX_BUFFER_SIZE, fp) != NULL) {
	    /* print out file data */
	    fprintf(stdout, "%s", buffer);
	}

	/* reclose the file */
	fclose(fp);
    }

    return 0;
}
