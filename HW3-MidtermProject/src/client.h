#ifndef __CLIENT_H_
#define __CLIENT_H_

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


static void removeFifo(void);              /* Invoked on exit to delete client FIFO */
int** readMatrixFromFile(char *filename,int * size);
void freeMatrix(int ** matrix,int size);
#endif