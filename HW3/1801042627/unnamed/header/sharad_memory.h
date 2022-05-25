#ifndef __SHAREDMEMORY_H__
#define __SHAREDMEMORY_H__
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <semaphore.h>
#include <sys/mman.h>
// indexes
#define MILK 0
#define FLOUR 1
#define WALNUTS 2
#define SUGAR 3

// #define MILKSEMAPHORENAME "/MILK"
// #define FLOURSEMAPHORENAME "/FLOUR"
// #define WALNUTSSEMAPHORENAME "/WALNUTS"
// #define SUGARSEMAPHORENAME "/SUGAR"

//char *ingredientSemaphoreNames = {"/MILK","/FLOUR","/WALNUTS","/SUGAR"};
//char * sharedMemoryName = NULL; // main program will initialize it
int createSharedMemory(const char* programName,const char *name);
int removeSharedMemory(const char* programName,int sharedFd,const char* sharedMemoryName);

#endif