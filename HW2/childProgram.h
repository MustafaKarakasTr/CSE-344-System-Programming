#ifndef __CHILDPROCESS_H_
#define __CHILDPROCESS_H_
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include<signal.h>


#define NUMBER_OF_POINTS 10
#define WRITE_FLAGS (O_WRONLY | O_CREAT | O_APPEND)
#define WRITE_PERMS (S_IRUSR | S_IWUSR)
#define MAX 20





int writeToFile(char* outFile,char* xValues,char* yValues,char* zValues,char* indexOfProcess);
void mysighand(int signal_number);
double mean(char *arr,int size);
double covariance(char arr1[], char arr2[], int size);
void covariance_matrix(double **matrix,char arr1[],char arr2[],char arr3[],int n);

#endif