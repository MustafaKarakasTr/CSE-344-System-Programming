#ifndef __SERVANT_H_
#define __SERVANT_H_
#include "Container.h"
#include "client.h"

#include<inttypes.h>

int takeStartEndIndexes(char * str,int * startEndIndexes);
int isNumber(char const *s);
int saveCity(char const * dataSetDirName,Container * cities,int index,char *cityName);
int saveDateFile(TransactionFile * transactionFile,char * filePath );
int takeTransactionInfoFromLine(char * line,Transaction *tr);
int saveToTransaction(Transaction * tr,char * str,int offset);
static int direntCompare(const void *p1, const void *p2);
int numberOfFilesInDirectory(char *fileName);
void freeCities(Container * cities, int size);
int sendConnectionInfoToServer(char * message,int size);
//int sendConnectionInfoToServer(void * message,int size,uint16_t portNum);
int getNumberOfTransactionsFromFile(char const * fileName);
uint16_t createUniquePortNumber();
int waitForRequests();
int getProcessId();
int createThread(int sockfd);
void * servantThread(void *arg);
int findRequestedCount(ClientThreadArg *request);
int searchInCity(Container *city,ClientThreadArg *request);
int searhInTransactionFile( TransactionFile *tf,ClientThreadArg *request);
int isValidRequest(Date * dateOfTransaction,Transaction *tr,ClientThreadArg *request);
int getNumberOfDirectories(char const * dataSetPath);

#endif