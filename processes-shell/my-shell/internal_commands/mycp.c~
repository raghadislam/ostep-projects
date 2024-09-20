#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "mycp.h"


int mycp(int argc, char* argv[])
{
	/* make sure the number of arguments is true */
	if(argc != 3) 
	{
		printf("error, usage: mycp <src> <dest>\n");
		return -1;
	}

	char buf[100];			/* buffer to save the source file content */
	char* src = argv[1];		/* source file path */
	char* dest = argv[2];		/* destination file path */
	
	int fd1 = open(src, O_RDONLY, 0644);	        	/* open the source file and save its descriptor */

	/* make sure that the source file exists */
	if(fd1 == -1)
	{
		printf("error: source file doesn't exist\n");
		return -1;
	}

	int fd2 = open(dest, O_WRONLY|O_CREAT, 0644);		/* open the destination file and save its descriptor */
	int count;
	
	while( (count = read(fd1, buf, 100)) != 0)		/* read the source file */
	write(fd2, buf, count);					/* write in the destination file */

	/* close source and destination files */
	close(fd1);					
	close(fd2);
	return 0;
}
