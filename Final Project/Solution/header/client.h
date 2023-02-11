#ifndef __CLIENT_H_
#define __CLIENT_H_
#define BLKSIZE 100
#include "Date.h"
#include "Container.h"
int getNumberOfLines(char const * fileName);
void signalHandler(int sig);
// typedef struct clientThreadArgs{
//     char * request;
// }clientThreadArgs;
void * threadFunc(void *p);

typedef struct ClientThreadArg{
    Date start_date;
    Date end_date;
    char type[MAX_TYPE_LENGTH];
    char city[MAX_CITY_NAME_LENGTH];
}ClientThreadArg;

ClientThreadArg *createThreadArgs(char ** requests,int numberOfRequests);
int sendRequestToServer(ClientThreadArg * message);
#endif