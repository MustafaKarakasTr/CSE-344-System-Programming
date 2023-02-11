#ifndef _HASHMAP_H_
#define _HASHMAP_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "Container.h"
struct DataItem {
   Container *data;   
   char key[MAX_CITY_NAME_LENGTH];
};


void freeHashArray(struct DataItem ** hashArray,int size);
int hashCode(char* key);
Container *search(char* key);
void insert(char* key,Container *data);
void display();
#endif
