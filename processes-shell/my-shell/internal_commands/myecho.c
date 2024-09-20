#include <stdio.h>
#include "myecho.h"

int myecho(int argc, char *argv[])
{
    /* print all arguments taken */
    if (argv != NULL) {
	for (int i = 1; i < argc; i++) {
	    printf("%s ", argv[i]);
	}
    }
    printf("\n");
    return 0;
}
