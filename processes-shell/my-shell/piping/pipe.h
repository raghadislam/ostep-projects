#ifndef PIPE_H
#define PIPE_H

#define  MAX_ARGS 20
#define	 MAX_COMMAND_LENGTH 100

/* function to display error */
void display_error();

/* function to parse the command and extract arguments */
void parse_command(char *cmd, char **args);


#endif
