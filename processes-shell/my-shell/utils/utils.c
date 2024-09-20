#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include "config.h"
#include "utils.h"


/* function to initialize the path */
void initialize_path(char* env[])
{
    env[0] = "/bin";
    env[1] = NULL;
}

/* function split the parallel comamnds */
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
	commands[i] = strdup(token);	
	i++;
	token = strtok(NULL, "&");
    }

    /* Store the number of commands found */
    *num_commands = i;
}


/* function to check the file existence */
char *is_file_exist(char *filename, char* env[])
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

/* function to display error message */
void display_error()
{
    char error_message[30] = "An error has occurred\n";
    write(STDERR_FILENO, error_message, strlen(error_message));

}



/* function to parse the command and extract arguments */
void parse_command(char *cmd, char **args) {

    /* Iterate over the arguments */
    for (int i = 0; i < MAX_ARGS; i++) {
        args[i] = strsep(&cmd, " \t\r\n");

        /* If no more tokens, break */
        if (args[i] == NULL)
            break;

        /* Handle output redirection ">" without spaces */
        char *redir_out = strchr(args[i], '>');
        if (redir_out != NULL) {
            if (*(redir_out + 1) == '>') {
                /* Handle "2>" for error redirection */
                if (*(args[i]) == '2') {
                    *redir_out = '\0';
                    if (strlen(args[i]) == 0) i--;
                    continue;
                }
            } else {
                /* Standard output redirection */
                *redir_out = '\0';
                if (strlen(args[i]) == 0) i--;
                continue;
            }
        }

        /* Handle input redirection "<" without spaces */
        char *redir_in = strchr(args[i], '<');
        if (redir_in != NULL) {
            *redir_in = '\0';
            if (strlen(args[i]) == 0) i--;
            continue;
        }

        /* Handle redirection operators with spaces */
        if (strcmp(args[i], ">") == 0) {
            args[i] = NULL;
            break;
        } else if (strcmp(args[i], "2>") == 0) {
            args[i] = NULL;
            break;
        } else if (strcmp(args[i], "<") == 0) {
            args[i] = NULL;
            break;
        }

        /* If an argument is empty, adjust the index */
        if (*args[i] == '\0')
            i--;
    }
}

