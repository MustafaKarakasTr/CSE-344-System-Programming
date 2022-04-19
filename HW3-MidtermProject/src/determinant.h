#ifndef _DETERMINANT_H__
#define _DETERMINANT_H__
#include<stdlib.h>
#include <stdio.h>
int isInvertible(int **matrix,int size);
int determinant(int **arr,int size,int startIndex);
int cofactor(int ** arr,int size,int row,int column);
#endif