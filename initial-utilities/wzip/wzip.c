#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    /* charck for crrect usage */
    if (argc < 2) {
	printf("wzip: file1 [file2 ...]\n");
	exit(1);
    }

    /* initialize pointers to a temporary file */
    FILE *pf = NULL;
    FILE *p_tmpFile = fopen("p_tmpFile.txt", "w+");

    /* initialize buffer variables */
    char *buffer = NULL;
    size_t size = 0;

    /* iterate on each file passed */
    for (int i = 1; i < argc; i++) {
	/* Open the file to read it */
	pf = fopen(argv[i], "r");

	if (pf == NULL) {
	    printf("wzip: cannot open file\n");
	    exit(1);
	} else {
	    while (getline(&buffer, &size, pf) != EOF) {
		/*put all files togeather im ine file */
		fprintf(p_tmpFile, "%s", buffer);
	    }
	    /* close the file */
	    fclose(pf);
	}
    }

    /* initialize zipping ioeration variables */
    char prev_char = EOF;
    char current_char;
    int count = 0;

    /* sets the file position indicator to the beginning of the temporary file */
    rewind(p_tmpFile);

    /* start zipping */
    while ((current_char = (char) fgetc(p_tmpFile)) != EOF) {
	if (current_char == prev_char) {
	    count++;
	} else {
	    if (prev_char != EOF) {
		fwrite(&count, sizeof(int), 1, stdout);
		putchar(prev_char);
	    }
	    prev_char = current_char;
	    count = 1;
	}
    }

    /* Write the last character */
    if (prev_char != EOF) {
	fwrite(&count, sizeof(int), 1, stdout);
	putchar(prev_char);
    }

    /* close the temporary file */
    fclose(p_tmpFile);

    /* remove the temporary file */
    remove("p_tmpFile.txt");

    p_tmpFile = NULL;

    exit(0);
}
