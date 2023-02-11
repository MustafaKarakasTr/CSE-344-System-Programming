#ifndef __SERVER_H__
#define __SERVER_H__
#include "client.h"
#include <time.h>
//char currenttime[200];
void * serverThreadFunc(void *arg);
int initializeMutexAndCondVar();
void killServants();
int sendRequestToServant(ClientThreadArg* request);
int indexOfServant (char * cityName);
int sendRequestToServantHelper(int portNumber,char const * IP,void * request,int size);
void printTime();
void terminateMessage();
#endif