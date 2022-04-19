#ifndef __SERVER_Z__
#define __SERVER_Z__
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
#include <sys/mman.h>
#include "error_functions.h"
#include "determinant.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h> 
#include <sys/time.h> 


#define CLIENT_FIFO_RESULT_TEMPLATE "/tmp/result_cl.%ld"
#define CLIENT_FIFO_NAME_LEN (sizeof(CLIENT_FIFO_RESULT_TEMPLATE) + 20)
#define MUTEX "/mutex"
#define EMPTY "/empty"
#define FULL "/full"


#define N 1 


static char clientFifoResult[CLIENT_FIFO_NAME_LEN];

//extern void * sharedMemoryAddr;
//void * sharedMemoryAddr;
int sizeOfSharedMemory = 500;
//int indexInSharedMemory = 0;
//int poolSizeGlobal = 0;
/*
For portable
       use, a shared memory object should be identified by a name of the form /somename; that is, a null-terminated string of up to NAME_MAX (i.e.,
       255) characters consisting of an initial slash, followed by one or more characters, none of which are slashes.
*/
static char sharedMemory[] = "/sharedMemory";

int createSharedSpace();
int workerZProcess(void *sharedMemoryAddr,int sleepDuration,int sharedMemFd,int poolsize);
#endif
