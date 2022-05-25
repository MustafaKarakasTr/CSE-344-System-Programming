#ifndef _CREATE_SEMAPHORE__H
#define _CREATE_SEMAPHORE__H
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <semaphore.h>
#include <sys/mman.h>
int create_sem(sem_t** sem,char isItNamed,char* name,int flag,int initialVal);

#endif