#ifndef __MAIN_H__
#define __MAIN_H__

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <time.h> 
#include <sys/time.h> 
#include <fcntl.h>
#include <string.h>

#define NUMBER_OF_CHEFS 6
#define SEMFORWHOLESALER "/semforWholesaler"
#define WHOLESALER_CAN_READ "/wholesalercanread"
#define SHARED_MEM_NAME "/sharedMemoryName1801042627"
#define NUMBEROFINGREDIENTS 4
#define numberOfChildProcesses NUMBER_OF_CHEFS+1+NUMBEROFINGREDIENTS // +1 helper + numberofingredients for pushers

sig_atomic_t terminate = 0;
char ingredients[] = {'S','F','M','W','\0'};
pid_t childProcesses[numberOfChildProcesses];
int activeChildProcesses = 0;
int isValid(char * str,int size);
int forkChefs(char * sharedMemoryName,const char* programName);
void waitForChildren();
void signalHandler(int sig);
int createSharedMemory(const char* programName,const char *name);
int removeSharedMemory(const char* programName,int sharedFd,const char* sharedMemoryName);
int forkWholesaler(char * sharedMemoryName,char* programName);
int createSemaphores(sem_t* sem_arr);
void unlink_semaphores(sem_t *sem_arr);
int forkPushers(char * sharedMemName,const char * programName);
#endif