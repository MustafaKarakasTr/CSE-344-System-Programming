#include "../header/my_queue.h"
#include  <stdlib.h>
#include  <stdio.h>



int my_queue_init(my_queue * q){
    q->arr = (int *) malloc(sizeof(int) * INITIAL_CAPACITY);
    if(q->arr == NULL){
        return -1;
    }
    q->capacity = INITIAL_CAPACITY;
    q->size = 0;
    return 0;
}
int my_queue_insert(my_queue * q,int data){
    //("size : %d cap: %d\n",q->size,q->capacity);
    if(q->capacity==0){
        return -1;
    }
    if(q->size == q->capacity){
        
        int newCap = q->capacity * 2;
        int *temp = (int *) malloc(sizeof(int) * newCap);
        if(temp == NULL){
            return -1;
        }
        for (int i = 0; i < q->size; i++)
        {
            temp[i+1] = q->arr[i];
        }
        free(q->arr);
        q->arr = temp;
        q->arr[0] = data;
        q->capacity = q->capacity * 2;
    } else{
        for (int i = q->size-1; i >=0; i--)
        {
            q->arr[i+1] = q->arr[i];
        }
        q->arr[0] = data;
    }
    //q->arr[size++] = data;
    q->size++;
    return 0;
}
int my_queue_is_empty(my_queue *q){
    return (q == NULL || q->size == 0);
}
int my_queue_remove(my_queue * q,int *removed){
    if(my_queue_is_empty(q))
        return -1;
    /*for (size_t i = 0; i < q->size-1; i++)
    {
        q->arr[i] = q->arr[i+1];
    }*/
    *removed = q->arr[q->size-1];
    q->size--;
    return 0;
}
int my_queue_destroy(my_queue * q){
    if(q!=NULL && q->capacity>1){
        free(q->arr);
        return 0;
    }
    return -1;
    
}