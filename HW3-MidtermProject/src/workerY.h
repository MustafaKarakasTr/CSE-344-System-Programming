#ifndef __WORKERY_H_
#define __WORKERY_H_
#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>     /* Prototypes for many system calls */
#include <string.h> 
#include <stdlib.h>
#include <signal.h>
#include <semaphore.h>
#include "error_functions.h"
#include "determinant.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <time.h>

#include <fcntl.h>
/* Template for building client FIFO name */
#define CLIENT_FIFO_RESULT_TEMPLATE "/tmp/result_cl.%ld"
#define CLIENT_FIFO_NAME_LEN (sizeof(CLIENT_FIFO_RESULT_TEMPLATE) + 20)
static char clientFifoResult[CLIENT_FIFO_NAME_LEN];



#endif