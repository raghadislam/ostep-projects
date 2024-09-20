#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <string.h>
#include "external_command.h"
#include "../redirection/redirection.h"


int execute_external(char cmd[], char *args[], char **environ, char cmd2[],
		     int pid[], int pid_index, int is_parallel)
{
    /* fork the current process */
    int ret_pid = fork();

    /* check the return value of fork */
    if (ret_pid < 0) {
	/* announce that fork failed */
	//      printf("fork faild\n");
	char error_message[30] = "An error has occurred\n";
	write(STDERR_FILENO, error_message, strlen(error_message));
	return -1;
    } else if (ret_pid > 0) {	/* in the parent process */
	/* wait for the change in the status of the child */
	if (is_parallel == 0) {
	    int state;
	    wait(&state);
	} else
	    pid[pid_index] = ret_pid;
	return 0;
    } else if (ret_pid == 0) {	/* in the child process */
	check_for_redirection(cmd2);
	/* execute the command taken from the user */
	//      execvpe(cmd, args, environ);
	execv(cmd, args);
	//      printf("error: command not found\n");
	char error_message[30] = "An error has occurred\n";
	write(STDERR_FILENO, error_message, strlen(error_message));
	return -1;
    }
    return -1;
}

void wait_for_processes(int pid[], int num_commands)
{
    for (int i = 0; i < num_commands; i++) {
	waitpid(pid[i], NULL, 0);
    }
}
