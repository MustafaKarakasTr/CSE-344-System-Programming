#ifndef __LINKEDLIST_H_
#define __LINKEDLIST_H_
#include<stdio.h>
#include<stdlib.h>

typedef struct LinkedList{
    void * data;
    struct LinkedList * next;
}LinkedList;

LinkedList * create(void* element);
int add(LinkedList * head,void * element);
void print(LinkedList * head);
int freeLinkedList(LinkedList * head);
#endif