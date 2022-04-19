#include "linkedList.h"


LinkedList * create(void* element){
    
    LinkedList * head = (LinkedList *) malloc(sizeof(LinkedList));
    // error check
    if(head == NULL){
        perror("malloc error");
        return NULL;
    }
    
    /*void * temp = malloc(sizeOfElement);
    // error check
    if(temp == NULL){
        perror("malloc error");
        return NULL;
    }*/  
    head->data = element;
    head->next = NULL;
    return head;
}
int add(LinkedList * head,void * element){
    // invalid Input
    if(head == NULL){
        return 1; 
    }
    // go to the last element
    for(;head ->next != NULL;head = head->next);

    LinkedList * temp = (LinkedList *) malloc(sizeof(LinkedList));
    if(temp == NULL){
        perror("malloc error");
        return -1;
    }
    // initialize elements
    temp->data = element;
    temp->next = NULL;
    // insert at the end of linked list
    head->next = temp;
    return 0;
}
void print(LinkedList * head){
    for(;head != NULL;head = head->next){
        printf("%s -> ",(char*)head->data);
    }
    printf("NULL\n");
}
int freeLinkedList(LinkedList * head){
    for(;head!=NULL;){
        LinkedList *temp = head->next;
        free(head);
        head = temp;
    }
    return 0;
}