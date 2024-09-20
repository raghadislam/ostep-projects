#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "shell_variables.h"

int var_cnt = 0;

/* function to check export appearence */
int check_for_export(char* args[], shell_var ShellVariables[])
{

	if(strcmp(args[0],"export") == 0)
	{
		if(args[1])
		{
			for(int i = 0; i < var_cnt; i++)
			{
				if(strcmp(ShellVariables[i].name, args[1]) == 0)
				{
					setenv(args[1], ShellVariables[i].val, 1); 
					break;
				}
			}
		}
		else
		{
			printf("Usage: export VAR_NAME\n");
		}

		return 1;
	}
	return 0;
}


/* function to set the shell variables values */
void set_var(char* var_name, char* var_val, shell_var ShellVariables[])
{
	for(int i = 0; i < var_cnt; i++)
	{
		if(strcmp(ShellVariables[i].name, var_name) == 0)
		{
			free(ShellVariables[i].val);
			ShellVariables[i].val = strdup(var_val);
			return;
		}
	}

	/* if not found assign a new variable */
	if(var_cnt < MAX_VAR)
	{
		ShellVariables[var_cnt].name = strdup(var_name);
		ShellVariables[var_cnt].val = strdup(var_val);
		var_cnt++;
	}
	else
	{
		printf("variables limit reached\n");
	}
}


/* function to check '=' appearence */
int check_for_equal(char cmd[], shell_var ShellVariables[])
{
	/* check for vatiables assigning operation */
	char* equal_occurrence = strchr(cmd, '=');
	if(equal_occurrence)
	{
		*equal_occurrence = '\0';
		char* var_name = cmd;
		char* var_value = equal_occurrence + 1;
		set_var(var_name, var_value, ShellVariables);

		return 1;;
	}
	return 0;
}


/* function to substitute each accessed variable with its value */
void substitute_var(char* args[], shell_var ShellVariables[])
{
	for(int i = 0; args[i] != NULL; i++)
	{
		if(args[i][0] == '$')
		{
			char* var_name = args[i] + 1;
			for (int j = 0; j < var_cnt; j++)
			{
              			if (strcmp(ShellVariables[j].name, var_name) == 0)
			  	{
                	   		args[i] = ShellVariables[j].val;
                	 		break;
          			}
            		}
		}
	}
}
