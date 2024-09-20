#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <pwd.h>
#include "piping/pipe.h"
#include "shell_variables/shell_variables.h"
#include "internal_commands/internal_command.h"
#include "external_commands/external_command.h"

/* ANSI escape codes for color formatting */
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#define MAX_COMMAND_LENGTH 100
#define MAX_ARGS 20
#define MAX_COMMANDS 15

/* the command will be taken from the user */
char cmd[MAX_COMMAND_LENGTH];

/* arguments array */
char *args[MAX_ARGS];

/* array to store shell variables */
shell_var ShellVariables[MAX_VAR];


int pid[MAX_COMMANDS];


/* environment variables */
extern char **environ;
char *env[500];

void initialize_path(void)
{
    env[0] = "/bin";
    env[1] = NULL;
}

void split_commands(char *input, char **commands, int *num_commands)
{
    char *token;
    int i = 0;

    /* Use strtok to split the string by '&' */
    token = strtok(input, "&");
    while (token != NULL && i < MAX_COMMANDS) {
	/* Remove leading spaces */
	while (*token == ' ') {
	    token++;
	}

	/* Remove trailing spaces */
	char *end = token + strlen(token) - 1;
	while (end > token && *end == ' ') {
	    *end = '\0';
	    end--;
	}

	/* Store the command in the array */
	commands[i] = strdup(token);	// Duplicate the command string
	i++;
	token = strtok(NULL, "&");
    }

    /* Store the number of commands found */
    *num_commands = i;
}



char *is_file_exist(char *filename)
{

    for (size_t i = 0; env[i] != NULL; i++) {
	char path[100];

	snprintf(path, sizeof(path), "%s/%s", env[i], filename);

	if (access(path, X_OK) == 0) {

	    return strdup(path);
	}
    }

    return NULL;

}

int main(int argc, char *argv[])
{
    if (argc > 2) {
	display_error();
	exit(1);
    }
    FILE *fp;
    if (argc == 1) {
	fp = stdin;

	printf(ANSI_COLOR_GREEN "wish> " ANSI_COLOR_RESET);
    } else {
	fp = fopen(argv[1], "r");
	if (fp == NULL) {
	    display_error();
	    exit(1);
	}
    }

    /* Initialize the environment variable with the "/bin" directory */
    initialize_path();


    /* initialize the buffer and its size */
    char *input = NULL;
    size_t size = MAX_COMMAND_LENGTH;
    int is_parallel = 0;

    /* take input */
    while (getline(&input, &size, fp) != EOF) {

	/* remove the newline character from the input */
	int len = strlen(input);
	input[len - 1] = 0;

	/* check the exist comamnd */
	if (strcmp(input, "exit") == 0) {
	    exit(0);
	}

	/* remove leading white spaces */
	while(*input == ' ')
	{
		input++;
	}


	if (fp == stdin)
	{
	    /* printnig the prompt of the shell */
	    printf(ANSI_COLOR_GREEN "wish> " ANSI_COLOR_RESET);
	}

	/* Ignore empty commands */
	if (input[0] == 0)
	    continue;

	/* check the parallel commands */
	int num_commands = 0;
	char *commands[MAX_COMMANDS];
	if (strstr(input, "&") != NULL) {
	    split_commands(input, commands, &num_commands);
	    is_parallel = 1;
	} else {
	    num_commands = 1;
	    commands[0] = input;
	    is_parallel = 0;
	}

	/* start executing */
	for (int i = 0; i < num_commands; i++) {

	    /* copy the full command to save it and manipulate the original */
	    char buffer[MAX_COMMAND_LENGTH];
	    strcpy(buffer, commands[i]);

	    char cmd[MAX_COMMAND_LENGTH];
	    strcpy(cmd, buffer);


	    /* check for the exit command */
	    if (strcmp(buffer, "exit") == 0) {
		exit(0);
	    }

	    /* Ignore empty commands */
	    if (buffer[0] == 0)
		continue;

	    /* parse the input */
	    parse_command(buffer, args);


	    /* check for my internal shell commands */
	    if (check_for_internal_command(buffer, args) == 1)
		continue;

	    /* check if the file exits */
	    char *file;
	    if ((file = is_file_exist(buffer)) == NULL) {
		display_error();
		continue;
	    }

	    /* execute the external command */
	    if (execute_external
		(file, args, env, cmd, pid, i, is_parallel))
		return -1;
	}

	/* wait for zombie processes */
	wait_for_processes(pid, num_commands);
    }
    return 0;
}
