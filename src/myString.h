#ifndef __MYSTRING_H_
#define __MYSTRING_H_

#include <stdio.h>
#include <stdlib.h>

/*
    returns length of str
*/

int my_strlen(const char * str);
/*
    returns 0 if the strings are identical
    returns 1 if str1>str2
    returns -1 if str2>str1

*/
int my_strcmp(const char * str1,const char* str2);


/* 
   if contains returns start index of searched in str
   if str does not contains searched returns -1
   if one of the parameters is NULL, returns -2
*/
int my_strcontains(const char *str,const char* searched);

/* returns indexes of occurences of searched string in str
   if returned NULL, 

    bool_first_readed: makes clear that the sent string is the first string readed from file,
        it will be used to in implementation of ^ operator. First line does not follow any \n. This special case will be solved by this argument.
*/
int ** startandEndIndexes(char* str,int str_size,char *searched,char caseInsensitive,char bool_first_readed);

/*
    str contains but case insensitive
*/
int my_strcontains_ins(const char *str,const char* searched);
int alphabetic(char c);
char changeCase(char c);
int upperCase(char c);
int lowerCase(char c);
int sameCharacterButCaseDifference(char c1,char c2);

/*
    -end exclusive
    -if the returned char pointer is same as given element, no new space allocated,
        else you need to deallocate space pointed by returned pointer
*/
char * strReplace(char* source,int start, int end,char* inserted);
/*
    uses strReplace
    Check description of the strReplace function
*/
char* my_strcpy(char * to,char* from);
/* 
    end is not included.
    does not puts \0 at the end
    copies elements start1 to end1

*/
int strcpy_index(char * to,int start1,int end,char* from,int start2);

char ** my_split(char* str,char separator);
int my_numberOf(char* str,char searched);

#endif