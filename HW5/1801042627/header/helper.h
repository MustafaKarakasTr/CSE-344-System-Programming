#ifndef __HELPER_H__
#define __HELPER_H__
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
typedef struct ImaginaryNumber{
    double real;
    double imaginary;
}ImaginaryNumber;
typedef struct threadArgs{
    int idOfThread;
    int ** matrix1;
    int ** matrix2;
    int ** ans_matrix;
    ImaginaryNumber ** discreteFourierTransformOfAns_Matrix;
    int size;
    int startColumn;
    int endColumn;
    int numberOfThreads;
}threadArgs;


int * createIds(int size,int startId);
threadArgs * createArguments(int ** matrix1,int ** matrix2,int ** ans_matrix,ImaginaryNumber ** fourier_matrix,int size,int numberOfThreads,int* IDs);
pthread_t* createThreads(int numberOfThreads,void *(*start_routine) (void *),threadArgs * args);
void joinThreads(pthread_t * threads,int size);
int writeTransform(const char* fileName,ImaginaryNumber ** discreteFourierTransformOfAns_Matrix,int size);
#endif