#ifndef EXTI_CMD
#define EXTI_CMD

/* function to execute any external commands that exist in the PATH environment variable */
int execute_external(char cmd[], char* args[], char** environ, char cmd2[], int pid[], int pid_index, int is_parallel);

void wait_for_processes(int pid[], int num_commands);

#endif
