#ifndef __CONTAINER_H__
#define __CONTAINER_H__
#include <stdlib.h>
#include <inttypes.h>
#include <stdio.h>
#define TRANSACTIONFILE_NUMBER 10
#define TRANSACTION_NUMBER 10
#define MAX_TYPE_LENGTH 20
#define MAX_DATE_LENGTH 20
#define MAX_IP_LENGTH 20

#define MAX_CITY_NAME_LENGTH 50


#define MAX_STREET_NAME_LENGTH 20

typedef struct Transaction{
    int id;
    char type[MAX_TYPE_LENGTH];
    char nameOfStreet[MAX_STREET_NAME_LENGTH];
    int surfaceSquare;
    int price;
}Transaction;

typedef struct TransactionFile{
    char date[MAX_DATE_LENGTH]; // file name
    //Transaction transactions[TRANSACTION_NUMBER]; // each line represents one transaction
    int numberOfTransactions;
    Transaction *transactions; // each line represents one transaction

}TransactionFile;

typedef struct Container{
    char nameOfCity[MAX_CITY_NAME_LENGTH];
    //TransactionFile transactionFiles[TRANSACTIONFILE_NUMBER]; // each of them is one file
    int numberOfTransactionFiles;
    TransactionFile *transactionFiles; // each of them is one file


}Container;

typedef struct ServantInfo{
    //uint8_t bit;
    char firstCity[MAX_CITY_NAME_LENGTH];
    char lastCity[MAX_CITY_NAME_LENGTH];
    char IP[MAX_IP_LENGTH];
    int pid;
    uint16_t portNum;
}ServantInfo;


#endif