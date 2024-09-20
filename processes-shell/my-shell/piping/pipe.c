#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include "pipe.h"


void display_error()
{
    char error_message[30] = "An error has occurred\n";
    write(STDERR_FILENO, error_message, strlen(error_message));

}



/* function to parse the command and extract arguments */
void parse_command(char *cmd, char **args) {

    /* Iterate over the arguments */
    for (int i = 0; i < MAX_ARGS; i++) {
        args[i] = strsep(&cmd, " \t\r\n");  // Split by spaces, tabs, or newlines

        /* If no more tokens, break */
        if (args[i] == NULL)
            break;

        /* Handle output redirection ">" without spaces (e.g., "ls>file.txt") */
        char *redir_out = strchr(args[i], '>');
        if (redir_out != NULL) {
            if (*(redir_out + 1) == '>') {
                // Handle "2>" for error redirection
                if (*(args[i]) == '2') {
                    *redir_out = '\0';
                    if (strlen(args[i]) == 0) i--;
                    continue;
                }
            } else {
                // Standard output redirection
                *redir_out = '\0';
                if (strlen(args[i]) == 0) i--;
                continue;
            }
        }

        /* Handle input redirection "<" without spaces (e.g., "ls<file.txt") */
        char *redir_in = strchr(args[i], '<');
        if (redir_in != NULL) {
            *redir_in = '\0';
            if (strlen(args[i]) == 0) i--;
            continue;
        }

        /* Handle standalone redirection operators ">", "2>", and "<" with spaces */
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



