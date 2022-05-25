#ifndef __WHOLESALER_H__
#define __WHOLESALER_H__

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include<string.h>
#include <sys/types.h>
#include <signal.h>
#include <semaphore.h>
#include <sys/mman.h>

sig_atomic_t terminate = 0;
void signalHandler(int sig);


#endif