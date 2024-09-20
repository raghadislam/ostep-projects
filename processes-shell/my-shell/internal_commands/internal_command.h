#ifndef INT_CMD_H
#define INT_CMD_H

#include "mycp.h"
#include "myecho.h"
#include "mymv.h"
#include "mypwd.h"
#include "mycat.h"
#include "mygrep.h"
#include "../piping/pipe.h"

int check_for_internal_command(char cmd[], char*argv[]);


#endif
