#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "../utils/utils.h"
#include "internal_command.h"

extern char *env[500];

int number_of_args(char *args[])
{
    int counter = 0;
    while (args[counter]) {
	counter++;
    }
    return counter;
}


void update_path(char **argv, int argc)
{
    /* erase the environment variable and overwrite it */
    memset(env, 0, sizeof(env));

    int i = 1;
    while (i < argc) {
	env[i - 1] = strdup(argv[i]);
	i++;
    }

}


int check_for_internal_command(char cmd[], char *argv[])
{
    int argc = number_of_args(argv);

    if (strcmp(cmd, "mycp") == 0) {
	mycp(argc, argv);
	return 1;
    }

    else if (strcmp(cmd, "myecho") == 0) {
	myecho(argc, argv);
	return 1;
    }
    else if (strcmp(cmd, "mymv") == 0) {
	mymv(argc, argv);
	return 1;
    }
    else if (strcmp(cmd, "mypwd") == 0) {
	mypwd();
	return 1;
    }
    else if (strcmp(cmd, "mycat") == 0) {
	mycat(argc, argv);
	return 1;
    }
    else if (strcmp(cmd, "mygrep") == 0) {
	mygrep(argc, argv);
	return 1;
    }
    else if (strcmp(cmd, "exit") == 0 && argc == 1) {
	exit(0);
    }
    else if (strcmp(cmd, "cd") == 0) {
	if (argc != 2 || chdir(argv[1]) == -1) {
	    display_error();
	    exit(0);
	}
	return 1;
    }
    else if (strcmp(cmd, "path") == 0) {
	update_path(argv, argc);
	return 1;
    }
    return 0;
}
