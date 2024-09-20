#ifndef UTILS_H
#define UTILS_H


/* function to display error */
void display_error();

/* function to parse the command and extract arguments */
void parse_command(char *cmd, char **args);

/* function to initialize the path */
void initialize_path(char* env[]);

void split_commands(char *input, char **commands, int *num_commands);

char *is_file_exist(char *filename, char* env[]);

#endif
