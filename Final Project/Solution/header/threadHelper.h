#ifndef __THREADHELPER_H__
#define __THREADHELPER_H__
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include "client.h"

pthread_t* createThreads(int numberOfThreads,void *(*start_routine) (void *),ClientThreadArg* arg);
void joinThreads(pthread_t * threads,int size);

#endif