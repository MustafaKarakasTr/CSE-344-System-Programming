#ifndef __MATRIX_H__
#define __MATRIX_H__

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include "../header/helper.h"
void freeMatrix(void ** matrix,int size);
int** allocateMatrix(int n);
ImaginaryNumber** allocateMatrixImaginaryNumber(int n);
void printMatrix(int ** matrix,int n);
void printMatrixImaginaryNumber(ImaginaryNumber ** matrix,int n);
int readMatrixFromFile(int ** matrix,int size,const char *inputFile);
void subSquareMatrixMultiplication(int ** ans_matrix,int **matrix1,int **matrix2,int size,int columnStartIndex,int columnEndIndex);
void subSquareMatrix2DDiscreteFourierTransform(ImaginaryNumber ** out_matrix,int ** in_matrix,int columnStartIndex,int columnEndIndex,int size);
#endif