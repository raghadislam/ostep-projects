#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include "redirection.h"
#include <errno.h>
#include <stdlib.h>
#include "../utils/utils.h"


int check_for_redirection(char cmd[])
{
    /* copy the original command */
    char cmd2[100];
    char cmd3[100];
    strcpy(cmd2, cmd);
    strcpy(cmd3, cmd);

    /* check for '>' output director */
    char *outputRed_occurrence = strchr(cmd, '>');
    if (outputRed_occurrence) {
	
	/* Separate the command from redirection part */
	*outputRed_occurrence = '\0';

	/* Move to the filename part */
	outputRed_occurrence++;

	while (*outputRed_occurrence == ' '
	       || *outputRed_occurrence == '\t') {
	    outputRed_occurrence++;
	}

	char *filename = strsep(&outputRed_occurrence, " \t\r\n");
	if (filename == NULL || strlen(filename) == 0) {
	    display_error();
	    exit(1);
	}


	/* Check if there are extra tokens after the first filename */
	if (outputRed_occurrence != NULL
	    && strlen(outputRed_occurrence) > 0) {
	    /* Skip any leading whitespace in case there is just space after the first filename */
	    while (*outputRed_occurrence == ' '
		   || *outputRed_occurrence == '\t') {
		outputRed_occurrence++;
	    }

	    /* If there is still something after the filename, it's an error */
	    if (*outputRed_occurrence != '\0') {
		display_error();
		exit(1);

	    }
	}
	int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (fd == -1) {
	    display_error();
	    exit(1);
	}
	dup2(fd, 1);	
	close(fd);
    }



    /* check for '2>' error redirector */
    char *errorRed_occurrence = strstr(cmd2, "2>");
    if (errorRed_occurrence) {
      
	/* Separate the command from the redirection part */
	*errorRed_occurrence = '\0';

	/* Move to the part after '2>' */
	errorRed_occurrence++;
	errorRed_occurrence++;

	/* Skip any leading whitespace after '2>' */
	while (*errorRed_occurrence == ' ' || *errorRed_occurrence == '\t') {
	    errorRed_occurrence++;
	}

	char *filename = errorRed_occurrence;
	if (filename == NULL || strlen(filename) == 0) {
	    display_error();

	    exit(1);
	}

	int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (fd == -1) {
	    display_error();

	    exit(1);
	}
	dup2(fd, 2);
	close(fd);
    }


    /* check for '<' input redirector */
    char *inputRed_occurrence = strchr(cmd3, '<');
    if (inputRed_occurrence) {
	/*Separate the command from the redirection part */
	*inputRed_occurrence = '\0';
	/* move to the part after '<' */
	inputRed_occurrence++;

	/* Skip any leading whitespace after '<' */
	while (*inputRed_occurrence == ' ' || *inputRed_occurrence == '\t') {
	    inputRed_occurrence++;
	}

	char *filename = inputRed_occurrence;
	if (filename == NULL || strlen(filename) == 0) {
	    display_error();

	    exit(1);
	}

	int fd = open(filename, O_RDONLY);
	if (fd == -1) {
	    display_error();

	    exit(1);
	}

	dup2(fd, 0);
	close(fd);
    }

    return 1;

}
