#ifndef __PARENTPROCESS_H_
#define __PARENTPROCESS_H_

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include<time.h>
#include <sys/types.h>
#include<sys/wait.h>
#include<math.h>
#include<float.h>
#include<signal.h>
#include "linkedList/linkedList.h"
#define CLEAR_AND_WRITE_FLAGS (O_WRONLY | O_TRUNC)
#define NUMBER_OF_POINTS 10
#define BLKSIZE (NUMBER_OF_POINTS*3) // 3 coordinates * 10 points, each matrix is in different buffer
#define CHILD_PROGRAM "childProgram"

typedef struct Matrix{
    float ** matrix;
    int id;
}Matrix;
Matrix * createMatrix(int row,int column);
float frobeniusNorm(Matrix * mt, int row,int column);
void printMatrix(Matrix* matrix,int row,int column);
void freeMatrix(Matrix* matrix,int row);
int takeOutputsFromChild(char *inputFile,int numberOfMatrixes);
void mysighand(int signal_number);


#endif