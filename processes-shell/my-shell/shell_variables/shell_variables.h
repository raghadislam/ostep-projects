#ifndef SHELL_VAR
#define SHELL_VAR

#define MAX_VAR	 50

/* struct for the shell variables */
typedef struct 
{
	char* name;
	char* val;
}shell_var;

/* function to check export appearence */
int check_for_export(char* args[], shell_var ShellVariables[]);

/* function to set the shell variable value */
void set_var(char* var_name, char* var_val, shell_var ShellVariables[]);

/* function to check '=' appearence */
int check_for_equal(char cmd[], shell_var ShellVariables[]);

/* function to substitute each accesses variable with its value */
void substitute_var(char* args[], shell_var ShellVariables[]);

#endif
