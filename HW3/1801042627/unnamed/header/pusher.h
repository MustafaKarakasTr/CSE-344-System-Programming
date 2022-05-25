#ifndef __PUSHER_H__
#define __PUSHER_H__
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include <fcntl.h> /* Defines O_* constants */
#include <sys/stat.h> /* Defines mode constants */
#include <semaphore.h>
#include <string.h>


sig_atomic_t terminate = 0;
int hexCharToInt(char c);
void signalHandler(int sig);
#endif