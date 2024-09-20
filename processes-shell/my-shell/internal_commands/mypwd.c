#include <unistd.h>
#include <stdio.h>
#include "mypwd.h"

int mypwd(void)
{
    /* print current working directory */
    printf("%s\n", getcwd(NULL, 0));
    return 0;
}
