#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include "pipe.h"



/* check if thee is a pipe operator | */
int check_for_pipe(char *cmd) {
    return strchr(cmd, '|') != NULL;
}



void split_pipe_commands(char cmd[], char** first_cmd, char** second_cmd) {
	
	/* find the piping operator */
	char *pipe_pos = strchr(cmd, '|');
    
	if(pipe_pos != NULL) {

	/* Null-terminate the first command */
        *pipe_pos = '\0';
	
	/* Move to the start of the second command */
        pipe_pos++;

        /* remove additional spaces */
        while (*pipe_pos == ' ') {
            pipe_pos++;
        }

        /* Set the two commands */
        *first_cmd = strdup(cmd);
        *second_cmd = strdup(pipe_pos);
    } 
}


void execute_pipe_command(char *first_cmd[], char *second_cmd[]) {

	/* pipe file descriptors */
	int pipefd[2];
	/* processes ID */
	pid_t pid1, pid2;

	/* create a pipe and check for errors */
	if(pipe(pipefd) == -1) {
		printf("error: cant create a pipe\n");
		return;
	}

	/* execute the first command */
	if((pid1 = fork()) == -1) {
		printf("error: cant fork the process\n");
		return;
	}

	/* in the child */
	if(pid1 == 0) {  
		/* Redirect stdout to the created pipe */
		dup2(pipefd[1], STDOUT_FILENO);

		/* close read and write ends of the pipe */
		close(pipefd[0]); 
		close(pipefd[1]);

		/* execute the first command and the outpute will be dirested to the pipe */
		execvp(first_cmd[0], first_cmd); 
		printf("error: cant execute the process, not found\n");
		exit(EXIT_FAILURE);
	}

	/* execute the second command */
	if((pid2 = fork()) == -1) {
		printf("error: cant fork the process\n");
		return;
	}


	/* in the child */
	if(pid2 == 0) { 
		/* Redirect stdin to the created pipe */
		dup2(pipefd[0], STDIN_FILENO);

		/* close read and write ends of the pipe */
		close(pipefd[1]);
		close(pipefd[0]);

		/* execute the first command and the outpute will be dirested to the pipe */
		execvp(second_cmd[0], second_cmd);
		printf("error: cant execute the process, not found\n");
		exit(EXIT_FAILURE);
	}

	/* in the parent process */
	close(pipefd[0]);
	close(pipefd[1]);
	/*wait for the first and second children */
	waitpid(pid1, NULL, 0);
	waitpid(pid2, NULL, 0);
}

/* function to parse the command and extract arguments */
void parse_command(char *cmd, char **args) {

	/* iterate till the last of the argumants */
    	for (int i = 0; i < MAX_ARGS; i++) {
        	args[i] = strsep(&cmd, " \t\r\n");
       		if (args[i] == NULL)
        		break;
		if((strcmp(args[i],">") == 0) || (strcmp(args[i],"2>") == 0) || (strcmp(args[i],"<") == 0))
		{
			args[i] = NULL;
			break;
		}
        	if (*args[i] == '\0')
            		i--;
    }
}

int is_empty(char cmd[])
{
	if(cmd == NULL) return 1;
	int i = 0;
	while(cmd[i] != '\0')
	{
		i++;
		if(cmd[i] >= 'a' && cmd[i] <= 'z') return 0;
	}
	return 1;
}

void execute_piping(char cmd[])
{
	/* strings to hold the two piped commands */
	char *first_cmd = NULL, *second_cmd = NULL;
	char cmd_to_split[MAX_COMMAND_LENGTH];

	/* copy the original string into the one to be split */
	strcpy(cmd_to_split, cmd);

	/* split the command */
	split_pipe_commands(cmd_to_split, &first_cmd, &second_cmd);

	/* check if the second command is empty */
	if(is_empty(second_cmd) || is_empty(first_cmd))
	{
		printf("error: usage: [command] | [command]\n");
		return;
	}
	
	/* arguments arrays */
	char *args1[MAX_ARGS];
	char *args2[MAX_ARGS];

	/* parse the two commands */
	parse_command(first_cmd,args1);
	parse_command(second_cmd,args2);

	/* execute the piping */
	execute_pipe_command(args1, args2);
}

