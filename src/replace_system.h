#ifndef __REPLACE_SYSTEM_H
#define __REPLACE_SYSTEM_H

#include "myString.h"

typedef struct replace_parameters
{
    char * replaced;
    char * incomer;
    /*used as bool variable */
    char isCaseInsensitive; 
}replace_parameters;
/*
    parameters : holds each word taken from argv
    file_pat   : holds name of file where replace operation will be done 
    numberOfReplaceOperations: number of replace operations
*/
void programLoop(char ** parameters,char * file_path,int numberOfReplaceOperations);
/*
    if the input is not valid, returns 0
    else returns 1
*/
int checkInputCorrection(char ** parameters);

int replace_program(replace_parameters** rep_params,int rep_params_size,char * file_path);
/* returns updated string, it's address will be same with str if any parameter does not replace string with bigger one.*/
char* replace(replace_parameters* rep_param,char * str,int str_size,char bool_first_readed);

int size(char ** arr);
int checkSquareBracket(char * str);
int checkAsterixOperator(char* str);
int checkOtherOperators(char * str);
int copyfile(int fromfd,int tofd);
#endif